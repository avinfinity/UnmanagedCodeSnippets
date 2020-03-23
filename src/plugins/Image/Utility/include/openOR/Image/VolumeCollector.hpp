//****************************************************************************
// (c) 2012 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
#ifndef openOR_VolumeCollector_hpp
#define openOR_VolumeCollector_hpp

#include "openOR/Image/DataCollector.hpp" //image_Utility

#include <functional>
#include <cstring>
#include <cstddef>
#include <climits>
#include <limits>
#include <memory>
#include <numeric>
#include <algorithm>

#include <omp.h>

#include <openOR/Image/Image1DData.hpp> //Image_ImageData
#include <openOR/Image/Image2DData.hpp> //Image_ImageData
#include <openOR/Image/Image3DData.hpp> //Image_ImageData
#include <openOR/Image/Image3DSizeDescriptor.hpp> //Image_ImageData
#include <openOR/Image/DataWrapper.hpp> //Image_Utility

#include <openOR/Math/vector.hpp> //openOR_core

#include <openOR/Plugin/AnyPtr.hpp> //openOr_core

#include <openOR/Defs/Image_Utility.hpp>

#include <openOR/Log/Logger.hpp> //openOR_core
#   define OPENOR_MODULE_NAME "Image.VolumeCollector"
#   include <openOR/Log/ModuleFilter.hpp> //openOR_core

#define LOG_THROW_RUNTIME(level, fmt) { boost::format __fmt = fmt; LOG(level, OPENOR_MODULE, __fmt); throw std::runtime_error(__fmt.str()); }
#define LOG_THROW_LOGIC(level, fmt) { boost::format __fmt = fmt; LOG(level, OPENOR_MODULE, __fmt); throw std::logic_error(__fmt.str()); }

#undef min
#undef max

namespace openOR {
	namespace Image {

		namespace BinaryTransform {
			template <class T>
			struct max : std::binary_function<T, T, T> {
				T operator() (const T& x, const T& y) const {
					return x > y ? x : y;
				}
			};
		}

		template<typename In, typename Out, typename Trans>
		struct SimpleVolumeReducer {
			SimpleVolumeReducer(const Trans& trans = Trans(), const Out& init = Out()) {
				m_init = init;
				m_trans = trans;
			}
			Out operator()(const std::tr1::shared_ptr<Image3DData<In> >& in) const {
				Math::Vector3ui lastPos = in->size();
				size_t end = static_cast<size_t>(lastPos(0)) * lastPos(1) * lastPos(2);
				return std::accumulate(in->data(), in->data() + end, m_init, m_trans);
			}
		private:
			Trans m_trans;
			Out m_init;
		};

		//! Constructs a volume from chunks provided in a following manner
		//! z-slice after z-slice
		//!  each slice is y-row after y-row
		//!   each row is x-pixel after x-pixel
		template<typename In, typename Out, size_t X = 0, size_t Y = 1, size_t Z = 2>
		class VolumeCollector : public DataCollector<In> {
		private:
			struct Vec3ui {
				Vec3ui() {
					data[0] = 0;
					data[1] = 0;
					data[2] = 0;
				}
				Vec3ui(const Math::Vector3ui& vec) {
					data[0] = vec(0);
					data[1] = vec(1);
					data[2] = vec(2);
				}
				operator Math::Vector3ui() const {
					return Math::create<Math::Vector3ui>(data[0], data[1], data[2]);
				}
				uint32 data[3];
			};

		public:

			typedef enum {
				MEAN_REDUCER, MAX_REDUCER
			} Reducer;

			VolumeCollector():
				m_maxMemorySize(0),
					m_scalingFactor(0),
					m_reduction(),
					m_pCurrentSlice(new size_t(0)),
					m_pCurrentlyAt(new size_t(0)),
					m_pCurrentSubvolume(createInstanceOf<Image3DData<In> >()),
					m_pTodo(new std::vector<AnyPtr>()),
					m_state(VolumeCollector::UNINITIALIZED),
					m_reducer(MAX_REDUCER),
					m_numThreads(1)
				{}

				VolumeCollector(const Reducer& reducer):
				m_maxMemorySize(0),
					m_scalingFactor(0),
					m_reduction(),
					m_transformator(trans),                         // !!!  That is the one actually doing something!
					m_pCurrentSlice(new size_t(0)),
					m_pCurrentlyAt(new size_t(0)),
					m_pCurrentSubvolume(createInstanceOf<Image3DData<In> >()),
					m_pTodo(new std::vector<AnyPtr>()),
					m_state(VolumeCollector::UNINITIALIZED),
					m_reducer(reducer),
					m_numThreads(1)
				{}

				virtual ~VolumeCollector() {
				}

				enum { SCALED, UNSCALED, UNINITIALIZED };

				bool isScaling() { return m_state == VolumeCollector::SCALED; }
				bool isUnscaled() { return m_state == VolumeCollector::UNSCALED; }
				Math::Vector3ui getScaling() { return m_reduction;}

				//! tag      | type                   | meaning
				//! -----------------------------------------------------------
				//! inSize   | Image3DSize            | the size of the input bytestream *
				//! out      | Image3DData<Out>       | the output volume to be constructed *
				//!    **    | Image**Data<In>        | input raw data
				//!
				//! * - needs to be set before calling the ()()-operator
				void setData(const AnyPtr& data, const std::string& name) {
					//std::cout << "setData()" << std::endl;
					if (std::tr1::shared_ptr<Image3DSize> pData = interface_cast<Image3DSize>(data)) {
						if (name == "inSize") {
							m_pInSize = pData;
							calcTransformationRatios();
							return;
						}
					}
					if (std::tr1::shared_ptr<Image3DData<Out> > pData = interface_cast<Image3DData<Out> >(data)) {
						if (name == "out") {
							m_pVolume = pData;
							calcTransformationRatios(); 
						} else {
							if (typeid(In) == typeid(Out)) {
								std::vector<AnyPtr>::iterator it = m_pTodo->begin();
								m_pTodo->insert(it, data);
							}
						}
						return;
					} else if (std::tr1::shared_ptr<DataCollector<Out> > pData = interface_cast<DataCollector<Out> >(data)) {
						m_pDataCollector = pData;
					}
					std::vector<AnyPtr>::iterator it = m_pTodo->begin();
					m_pTodo->insert(it, data);
				}

				void setScalingFactor(const uint& scalingFactor) {
					m_scalingFactor = scalingFactor;
					calcTransformationRatios(); 
				}

				void setMaxMemorySize(const size_t& maxMemory) {
					m_maxMemorySize = maxMemory;
					calcTransformationRatios();
				}

				void setReducer(const Reducer& reducer) {
					m_reducer = reducer;                                     
				}

				void setNumThreads(const size_t& numThreads) {
					m_numThreads = numThreads;
				}

				void operator()() const {
					if (!m_pInSize || (!m_pVolume && !m_pDataCollector) || m_reduction.data[0] == 0) {
						LOG_THROW_LOGIC(Log::Level::Error, Log::msg("[VolumeCollector] Trying to commit data with no size or output volume or scaling set"));
					}

					//as long as the pointer to data is not empty, take the next slice of data
					while (!m_pTodo->empty()) {
						AnyPtr data = m_pTodo->back(); m_pTodo->pop_back();

						const In* pFromData = NULL;
						size_t end = 0;
						//depending on the type of data define the end point  
						if (std::tr1::shared_ptr<Image1DData<In> > pData = interface_cast<Image1DData<In> >(data)) {
							end = getWidth(pData);
							pFromData = pData->data();
						}

						if (std::tr1::shared_ptr<Image2DData<In> > pData = interface_cast<Image2DData<In> >(data)) {
							end = getWidth(pData) * getHeight(pData);
							pFromData = pData->data();
						}

						if (std::tr1::shared_ptr<Image3DData<In> > pData = interface_cast<Image3DData<In> >(data)) {
							end = static_cast<size_t>(width(pData)) * height(pData) * depth(pData);
							pFromData = pData->data();
						}

						if (std::tr1::shared_ptr<DataWrapper<const In> > pData = interface_cast<DataWrapper<const In> >(data)) {
							end = pData->size();
							pFromData = pData->data();
						}

						const size_t sliceSize = static_cast<size_t>(m_subVolSize.data[X]) * m_subVolSize.data[Y] * m_subVolSize.data[Z];

						//copy slice of data
						if (pFromData) {
							In* pMutData = m_pCurrentSubvolume->mutableData();

							size_t atIn = 0;
							size_t currentlyAt = *m_pCurrentlyAt;

							while (atIn < end) {
								size_t todo = sliceSize - currentlyAt;
								const size_t toCopy = std::min<size_t>(end - atIn, todo);

								std::copy(&(pFromData[atIn]), &(pFromData[atIn]) + toCopy, &(pMutData[currentlyAt]));

								atIn += toCopy;
								todo -= toCopy;
								currentlyAt += toCopy;

								if (todo == 0) {
									digest();
									todo = sliceSize;
									currentlyAt = 0;
								}
							}

							*m_pCurrentlyAt = currentlyAt;
						}
					}
				}

				void reset(const bool& resetOut) {
					m_maxMemorySize = 0;
					m_scalingFactor = 0;
					std::fill_n(m_reduction.data, 3, 0);
					std::fill_n(m_volSize.data, 3, 0);
					std::fill_n(m_subVolSize.data, 3, 0);
					(*m_pCurrentSlice) = 0;
					(*m_pCurrentlyAt) = 0;
					m_pCurrentSubvolume = std::tr1::shared_ptr<Image3DData<In> >();
					m_pVolume = std::tr1::shared_ptr<Image3DData<Out> >();
					m_pInSize = std::tr1::shared_ptr<Image3DSize>();
					m_pTodo->resize(0);
					m_state = VolumeCollector::UNINITIALIZED;
				}

		private:

			//! partitions the slice into subcubes and transforms each into an output pixel
			void digest() const {
				const In* pFromData = m_pCurrentSubvolume->data();
				std::tr1::shared_ptr<Image3DData<Out> > pVolume;
				size_t z;

				if (m_pVolume) {
					pVolume = m_pVolume;
					z = *m_pCurrentSlice;
				} else {
					pVolume = createInstanceOf<Image3DData<Out> >();
					pVolume->setSize(static_cast<Math::Vector3ui>(createVector3ui(m_pInSize->size()(X) / m_reduction.data[X], m_pInSize->size()(Y) / m_reduction.data[Y], 1)));
					z = 0;
				}

				Out* pVolData = pVolume->mutableData();

				std::tr1::shared_ptr<Image3DData<In> > cube = createInstanceOf<Image3DData<In> >();
				if (m_numThreads == 0) cube->setSize((Math::Vector3ui) m_reduction);
				In* pData = cube->mutableData();

				size_t destInd = z * m_volSize.data[X] * m_volSize.data[Y];

				size_t rowSize = m_reduction.data[X] * sizeof(In);
				// make sure to also go back the y-offset
				size_t zStep = static_cast<size_t>(m_subVolSize.data[X]) * m_subVolSize.data[Y] - m_subVolSize.data[X] * m_reduction.data[Y];

				if (m_reduction.data[X] == 1 && m_reduction.data[Y] == 1 && m_reduction.data[Z] == 1) {
					// optimized if no reduction is applied
					size_t numVoxel = m_volSize.data[X] * m_volSize.data[Y];

					if (m_pDataCollector) {
						std::tr1::shared_ptr<Image::DataWrapper<const Out> > pWrapper = createInstanceOf<Image::DataWrapper<const Out>, const Out*, size_t>(pFromData, numVoxel);
						m_pDataCollector->setData(pWrapper, "in");
						(*m_pDataCollector)();
					} else {
						std::copy(pFromData, pFromData + numVoxel, &(pVolData[destInd]));
					}

					(*m_pCurrentSlice)++;
				} else {
					if (m_reducer == MAX_REDUCER) { 
						if (true) {
							if (m_numThreads == 0) {
								for (int y = 0; y < m_volSize.data[Y]; y++) {
									for (size_t x = 0; x < m_volSize.data[X]; x++) {
										size_t wx = x * m_reduction.data[X];
										size_t wy = static_cast<size_t>(y) * m_reduction.data[Y];

										size_t outInd = 0;
										size_t inInd = wy * m_subVolSize.data[X] + wx;

										for (size_t lz = 0; lz < m_reduction.data[Z]; lz++) {
											for (size_t ly = 0; ly < m_reduction.data[Y]; ly++) {
												memcpy(reinterpret_cast<void*>(&(pData[outInd])), reinterpret_cast<const void*>(&(pFromData[inInd])), rowSize);
												inInd += m_subVolSize.data[X];
												outInd += m_reduction.data[X];
											}
											inInd += zStep;
										}
										Out val = m_transformator(cube);
										pVolData[destInd + y * m_volSize.data[X] + x] = val;
									}
								}
							} else {
#pragma                 omp parallel for num_threads(m_numThreads)
								for (int y = 0; y < m_volSize.data[Y]; y++) {
									for (size_t x = 0; x < m_volSize.data[X]; x++) {
										size_t wx = x * m_reduction.data[X];
										size_t wy = static_cast<size_t>(y) * m_reduction.data[Y];

										Out valMax = std::numeric_limits<Out>::min();

										size_t inInd = wy * m_subVolSize.data[X] + wx;
										for (size_t lz = 0; lz < m_reduction.data[Z]; lz++) {
											for (size_t ly = 0; ly < m_reduction.data[Y]; ly++) {
												valMax = std::max(valMax, *std::max_element(&(pFromData[inInd]), &(pFromData[inInd]) + (m_reduction.data[X])));
												inInd += m_subVolSize.data[X];
											}
											inInd += zStep;
										}
										pVolData[destInd + y * m_volSize.data[X] + x] = valMax;
									}
								}
							}
						} else {
							// SSE implementation (multiple of 8 voxels in x-direction) for uint16-volumes
							// static 
							uint nValuesPerRegister = 8;
							uint nUsedValuesPerRegister = 8;
							if (m_reduction.data[X] == 3) nUsedValuesPerRegister = 6;
							if (m_reduction.data[X] == 5) nUsedValuesPerRegister = 5;
							if (m_reduction.data[X] == 7) nUsedValuesPerRegister = 7;
							uint nVoxelsPerRegister = nValuesPerRegister / m_reduction.data[X];

#pragma              omp parallel num_threads(m_numThreads)
							{
								// create two 128 bit integer register for SSE
								__m128i rRegisterA, rRegisterB;
								// aligned temporary results (total of 128 bits) 
								uint16* pAlignedTempResults = (uint16*) _mm_malloc(8 * sizeof(uint16), 32); //void* _mm_malloc (size_t size, size_t align )

#pragma                 omp for
								for (int y = 0; y < m_volSize.data[Y]; y++) {                 
									for (size_t x = 0; x < (m_volSize.data[X] - nValuesPerRegister); x += nVoxelsPerRegister) {
										// compute memory address offset
										size_t wx = x * m_reduction.data[X];
										size_t wy = static_cast<size_t>(y) * m_reduction.data[Y];                           
										size_t inInd = wy * m_subVolSize.data[X] + wx;
										size_t globalindex = destInd + y * m_volSize.data[X] + x;

										for (size_t lz = 0; lz < m_reduction.data[Z]; lz++) {
											for (size_t ly = 0; ly < m_reduction.data[Y]; ly++) {
												//first computation?
												if (ly == 0 && lz == 0) {
													//load initial 128 bit into register
													rRegisterA = _mm_loadu_si128((__m128i*) (pFromData + inInd)); // data from second line
												} else {
													//load current 128 bit into register
													rRegisterB = _mm_loadu_si128((__m128i*) (pFromData + inInd));// data from first line
													// save max integer to Register a __m128i _mm_min_epu16 (__m128i a, __m128i b) // unsigned (!) Integer compare!!!!!
													rRegisterA = _mm_max_epu16(rRegisterA, rRegisterB);
												}                                 
												// go to next line
												inInd += m_subVolSize.data[X];
											}
											// go to next slice
											inInd += zStep;
										}
										// download temp results
										_mm_stream_si128((__m128i*) pAlignedTempResults, rRegisterA);

										// collapse results   
										uint i = 0;
										for (uint nOffset = 0; nOffset < nUsedValuesPerRegister; nOffset += m_reduction.data[X]) {
											//init dummy max value => 16 bit
											Out valMax = 0;                
											// collapse results for every voxel
											for (uint lx = 0; lx < m_reduction.data[X]; ++lx) {
												valMax = std::max(valMax, pAlignedTempResults[lx + nOffset]);
											}                                                        
											// set value
											pVolData[globalindex + i] = valMax;
											++i;
										}
									}                                  
								}
								_mm_free(pAlignedTempResults);
							}
						}
					} else if (m_reducer == MEAN_REDUCER) {
						double norm = m_reduction.data[X] * m_reduction.data[Y] * m_reduction.data[Z];
#pragma           omp parallel for num_threads(m_numThreads)
						for (int y = 0; y < m_volSize.data[Y]; y++) {
							for (size_t x = 0; x < m_volSize.data[X]; x++) {
								size_t wx = x * m_reduction.data[X];
								size_t wy = static_cast<size_t>(y) * m_reduction.data[Y];

								double mean = static_cast<Out>(0);

								size_t outInd = 0;
								size_t inInd = wy * m_subVolSize.data[X] + wx;
								for (size_t lz = 0; lz < m_reduction.data[Z]; lz++) {
									for (size_t ly = 0; ly < m_reduction.data[Y]; ly++) {
										for (int i = 0; i < m_reduction.data[X]; i++) mean += pFromData[inInd + i];
										outInd += m_reduction.data[X];
										inInd += m_subVolSize.data[X];
									}
									inInd += zStep;
								}

								mean /= norm;
								pVolData[destInd + y * m_volSize.data[X] + x] = static_cast<Out>(mean);
							}
						}
					}

					if (m_pDataCollector) {
						m_pDataCollector->setData(pVolume, "in");
						(*m_pDataCollector)();
					}

					(*m_pCurrentSlice)++;
				}
			}

			void calcTransformationRatios() {
				if (!m_pInSize || (m_maxMemorySize == 0 && m_scalingFactor == 0)) {
					return;
				}

				int scalingFactor = 1;

				if (m_maxMemorySize != 0) {
					size_t usedSize = static_cast<size_t>(sizeof(In)) * m_pInSize->size()(0) * m_pInSize->size()(1) * m_pInSize->size()(2);
					if (usedSize <= m_maxMemorySize) {
						scalingFactor = 1;
					} else if (usedSize / 8 <= m_maxMemorySize) {
						scalingFactor = 2;
					} else if (usedSize / 27 <= m_maxMemorySize) {
						scalingFactor = 3;
					} else if (usedSize / 64 <= m_maxMemorySize) {
						scalingFactor = 4;
					} else {
						LOG_THROW_LOGIC(Log::Level::Error, Log::msg("[VolumeCollector] Scaling factor bigger than 4 needed - not supported"));
					}
				} else {
					scalingFactor = m_scalingFactor;
				}

				m_reduction.data[X] = scalingFactor;
				m_reduction.data[Y] = scalingFactor;
				m_reduction.data[Z] = scalingFactor;

				m_subVolSize = createVector3ui(m_pInSize->size()(X), m_pInSize->size()(Y), scalingFactor);
				m_pCurrentSubvolume->setSize(static_cast<Math::Vector3ui>(m_subVolSize));

				Vec3ui outSize = createVector3ui(
					m_pInSize->size()(X) / scalingFactor,
					m_pInSize->size()(Y) / scalingFactor,
					m_pInSize->size()(Z) / scalingFactor
					);

				m_volSize = outSize;

				if (m_pVolume) {
					m_pVolume->setSize(static_cast<Math::Vector3ui>(outSize));
					m_pVolume->setSizeMM(m_pInSize->sizeMM());
				}

				if (m_pDataCollector) {
					std::tr1::shared_ptr<Image3DSize> pSize = createInstanceOf<Image3DSizeDescriptor>();

					pSize->setSize(static_cast<Math::Vector3ui>(outSize));
					pSize->setSizeMM(m_pInSize->sizeMM());

					m_pDataCollector->setData(pSize, "inSize");
				}

				if (scalingFactor == 1) m_state = VolumeCollector::UNSCALED;
				else m_state = VolumeCollector::SCALED;
			}

			inline Vec3ui createVector3ui(const uint32& x, const uint32& y, const uint32& z) const {
				uint32 data[3] = {x, y, z};
				Vec3ui v;
				v.data[X] = data[0];
				v.data[Y] = data[1];
				v.data[Z] = data[2];
				return v;
			}

			size_t m_maxMemorySize;
			uint m_scalingFactor;
			Vec3ui m_reduction, m_volSize, m_subVolSize;
			std::tr1::shared_ptr<size_t> m_pCurrentSlice, m_pCurrentlyAt;
			std::tr1::shared_ptr<Image3DData<In> > m_pCurrentSubvolume;
			std::tr1::shared_ptr<Image3DData<Out> > m_pVolume;
			std::tr1::shared_ptr<DataCollector<Out> > m_pDataCollector;
			std::tr1::shared_ptr<Image3DSize> m_pInSize;
			std::tr1::shared_ptr<std::vector<AnyPtr> > m_pTodo;
			Reducer m_reducer;
			int m_state;
			size_t m_numThreads;

			// obsolete: for testing purposes
			SimpleVolumeReducer<uint16, uint16, BinaryTransform::max<uint16> > m_transformator;
		};

		typedef VolumeCollector<uint16, uint16> VolCollUI16;
		typedef VolumeCollector<uint16, uint16> VolumeCollectorUI16;

		template<typename Type>
		class SimpleVolumeCollector : public DataCollector<Type> {
		public:
			SimpleVolumeCollector():m_pPosition(new size_t()), m_pTodo(new std::vector<AnyPtr>()) {}

			void setData(const AnyPtr& data, const std::string& name) {
				if (std::tr1::shared_ptr<Image3DSize> pData = interface_cast<Image3DSize>(data)) {
					if (name == "inSize") {
						m_pVolume->setSize(pData->size());
						m_pVolume->setSizeMM(pData->sizeMM());
						return;
					}
				}
				std::tr1::shared_ptr<Image3DData<Type> > pImage3D = interface_cast<Image3DData<Type> >(data);
				if (!pImage3D || name == "in") {
					std::vector<AnyPtr>::iterator it = m_pTodo->begin();
					m_pTodo->insert(it, data);
				}
				else if (pImage3D) m_pVolume = pImage3D;
			}

			void operator()() const {
				Type* pDataPtr = m_pVolume->mutableData();
				size_t pos = *m_pPosition;

				while (!m_pTodo->empty()) {
					AnyPtr data = m_pTodo->back(); m_pTodo->pop_back();

					const Type* pFromDataMut;
					size_t end = 0;

					if (std::tr1::shared_ptr<Image1DData<Type> > pData = interface_cast<Image1DData<Type> >(data)) {
						end = getWidth(pData);
						pFromDataMut = pData->data();
					}

					if (std::tr1::shared_ptr<Image2DData<Type> > pData = interface_cast<Image2DData<Type> >(data)) {
						end = getWidth(pData) * getHeight(pData);
						pFromDataMut = pData->data();
					}

					if (std::tr1::shared_ptr<Image3DData<Type> > pData = interface_cast<Image3DData<Type> >(data)) {
						end = static_cast<size_t>(width(pData)) * height(pData) * depth(pData);
						pFromDataMut = pData->data();
					}

					if (std::tr1::shared_ptr<DataWrapper<const Type> > pData = interface_cast<DataWrapper<const Type> >(data)) {
						end = pData->size();
						pFromDataMut = pData->data();
					}

					if (end > 0) {
						const Type* pFromData = pFromDataMut;

						std::copy(pFromData, pFromData + end, pDataPtr + pos);

						(*m_pPosition) = pos + end;
					}
				}
			}

			void reset(const bool& resetOut) {
				m_pVolume = std::tr1::shared_ptr<Image3DData<Type> >();
				(*m_pPosition) = 0;
				m_pTodo->resize(0);
			}

		private:

			std::tr1::shared_ptr<Image3DData<Type> > m_pVolume;
			std::tr1::shared_ptr<size_t> m_pPosition;
			std::tr1::shared_ptr<std::vector<AnyPtr> > m_pTodo;
		};

		typedef SimpleVolumeCollector<uint16> SimpleVolumeCollectorUI16;
	}
}

#undef LOG_THROW_RUNTIME
#undef LOG_THROW_LOGIC

#endif //openOR_VolumeCollector_hpp
