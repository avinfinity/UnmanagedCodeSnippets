//****************************************************************************
// (c) 2012 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************

#include <openOR/ProbabilitySegmentation.hpp>
#include <limits>

#include <openOR/Log/Logger.hpp> //openOR_core
#   define OPENOR_MODULE_NAME "Zeiss.Segmentator"
#   include <openOR/Log/ModuleFilter.hpp>
# include<openOR/cleanUpWindowsMacros.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>

#ifdef _OPENMP
#include <omp.h>
#endif

//------------------------------------------------------------------------------

//commented unused functions:	getGradientBasedMaterialIndex(...)
//								getMaterialIndex(std::vector<VoxelType*>& vecOriginalSlices, double x, double y, double z, uint nWidth, uint nHeight);
//								getComputeTotalGradientDirection(...)
//								getInterpolatedGreyValueAndProbabilityValue(...)
//------------------------------------------------------------------------------

namespace openOR {

	//---------------------------------------------------------------------------------------------------------------------------
	// Helper functions
	namespace {
		// Linear interpolation
		//    target  - the target point, 0.0 - 1.0
		//    v       - a pointer to an array of size 2 containing the two values
		template<typename T>
		inline static T Linear(float target, T *v) {
			return (T)(target*(v[1])+ (T(1.0f) - target)*(v[0]));
		}

		// BiLinear interpolation, linear interpolation in 2D
		//    target  - a 2D point (X,Y)
		//    v       - an array of size 4 containing values clockwise around the square starting from bottom left
		// cost: performs 3 linear interpolations
		template<typename T>
		inline static T Bilinear(float *target, T *v) {
			T v_prime[2] = {
				Linear(target[1], &(v[0])),
				Linear(target[1], &(v[2]))
			};

			return Linear(target[0], v_prime);
		}

		// TriLinear interpolation, linear interpolation in 2D
		//    target  - a 3D point (X,Y)
		//    v       - an array of size 8 containing the values of the 8 corners of a cube defined as two faces:
		//                 0-3 face one (front face)
		//                 4-7 face two (back face)
		// cost: 7 linear interpolations
		template<typename T>
		inline static T Trilinear(float *target, T *v) {
			T v_prime[2] = {
				Bilinear(&(target[0]), &(v[0])),
				Bilinear(&(target[0]), &(v[4]))
			};

			return Linear(target[2], v_prime);
		}
	}

	//---------------------------------------------------------------------------------------------------------------------------
	// Probability Segmentation

	ProbabilitySegmentation::ProbabilitySegmentation() :
	m_outputMaterialIndex(false),
		m_nFirstMaterialIndex(0),      
		m_nRadius(2),
		m_fProbabilityThreshold(0.7),
		m_fAdditionalOriginalSlicesFactor(2),
		m_nMaskSize(3),
		m_nIter(3),
		//m_nUnsurableIndex(4),
		m_progress(0),
		m_progressMax(1), // to prevent div by zero
		m_canceled(false),
		m_timeFilterRadius(0),
		m_timeNormalDistribution(0),
		m_timePostprocessing(0),
		m_nOpenMPMaxThreads(1),
		m_fCPUPerformanceValue(0.75)
	{
#ifdef _OPENMP
		m_nOpenMPMaxThreads = omp_get_max_threads();
#endif
	}

	ProbabilitySegmentation::~ProbabilitySegmentation() {}

	void ProbabilitySegmentation::setData(const AnyPtr& data, const std::string& tag) {

		if (std::tr1::shared_ptr<Image::Image3DDataUI16> pData = interface_cast<Image::Image3DDataUI16>(data)) {
			if (!((tag == "map") || (tag == "out"))) {
				m_pVolumeData = pData;
			} else {
				m_pVolumeMaterialMap = pData;
			}
		}
		if (std::tr1::shared_ptr<Image::Image1DData<Quad<double> > > pData = interface_cast<Image::Image1DData<Quad<double> > >(data)) {
			m_pVecProbabilities = pData;
		}
	}

	void ProbabilitySegmentation::setRadius(unsigned int nRadius) {
		if (nRadius>=0 && nRadius<3){
			m_nRadius = nRadius;
		}                  
	}

	void ProbabilitySegmentation::setOutputMaterialIndex(bool materialIndex){
		m_outputMaterialIndex = materialIndex;
	}

	void ProbabilitySegmentation::setFirstMaterialIndex(unsigned int nFirstMaterialIndex)
	{
		m_nFirstMaterialIndex = nFirstMaterialIndex;
	}

	void ProbabilitySegmentation::operator ()() {
		// - create a copy or work in place
		// - do the probability segmentation and post processing

		LOG(Log::Level::Debug, OPENOR_MODULE, Log::msg("Starting Segmentation ..."));
		if (m_pVolumeData == m_pVolumeMaterialMap) {
			LOG(Log::Level::Debug, OPENOR_MODULE, Log::msg("   \\--> in place"));
		} else {
				if (m_pVolumeMaterialMap->size()(0) != m_pVolumeData->size()(0) || 
				m_pVolumeMaterialMap->size()(1) != m_pVolumeData->size()(1) || 
				m_pVolumeMaterialMap->size()(2) != m_pVolumeData->size()(2)) 
			{
				//if (Math::isNotEqualTo(m_pVolumeMaterialMap->size(), m_pVolumeData->size())) {
				m_pVolumeMaterialMap->setSize(m_pVolumeData->size());
			}
			m_pVolumeMaterialMap->setSizeMM(m_pVolumeData->sizeMM());
		}
		// do Probability Segmentation
		if (doProbabilitySegmentation()) {
			boost::posix_time::ptime timeDoProb(boost::posix_time::microsec_clock::local_time());
			doProbabilitySegmentationPostProcessing();
			m_timePostprocessing = (boost::posix_time::microsec_clock::local_time() - timeDoProb).total_milliseconds();

#if DEBUGINFOS == 1
			std::cout << "Post processing: " << m_timePostprocessing/1000.0 << " s" << std::endl;
#endif
		}

		m_progress = m_progressMax;
		LOG(Log::Level::Debug, OPENOR_MODULE, Log::msg("Segmentation done."));
	}

	// cancelable interface
	void ProbabilitySegmentation::cancel() { m_canceled = true; }
	bool ProbabilitySegmentation::isCanceled() const { return m_canceled; }

	// progressable interface
	double ProbabilitySegmentation::progress() const { return ((double)m_progress / (double) m_progressMax); }
	std::string ProbabilitySegmentation::description() const { return "Segmentation"; }



	bool ProbabilitySegmentation::doProbabilitySegmentation() {

		// - create normal distribution lookup table 

		boost::posix_time::ptime doProbabilitySegmentation_start = boost::posix_time::microsec_clock::local_time();
		if(m_pVecProbabilities->size()==0) {
			return false;
		}

		//get normal distribution as lookup table
		boost::posix_time::ptime nStartTime(boost::posix_time::microsec_clock::local_time());
		computeNormalDistributedProbabilityLookUpTable(0,65535, 65536);
		boost::posix_time::ptime nComputeNormalTime(boost::posix_time::microsec_clock::local_time());
		m_timeNormalDistribution = (nComputeNormalTime - nStartTime).total_seconds();
		
		// compute weighting vector
		std::vector<Math::Vector4d> vecPrimaryWeightingVector;
		std::vector<Math::Vector4d> vecSecondaryWeightingVector;
		double fCentralVoxelNominator = 1.0;
		double fCentralVoxelDenominator = 3.0;//2.0*m_nRadius+1.0;

	    bool bPrimary = computeWeightingVector(m_nRadius,vecPrimaryWeightingVector,0.0,0.0);
		bool bSecondary = computeWeightingVector(m_nRadius,vecSecondaryWeightingVector,fCentralVoxelNominator,fCentralVoxelDenominator);

		//compute the most probable material index for the slices based on the weighting vectors
		if(bPrimary && bSecondary){
			bool ret = filterVariableRadius(m_nRadius,vecPrimaryWeightingVector,vecSecondaryWeightingVector);
			boost::posix_time::ptime nFilterRadiusTime(boost::posix_time::microsec_clock::local_time());
			m_timeFilterRadius = (nFilterRadiusTime - nComputeNormalTime).total_milliseconds();

#if DEBUGINFOS == 1
			std::cout << "Filter radius overall: " << m_timeFilterRadius / 1000.0 << " s" << std::endl;
#endif
			return ret;

		} else {
			return false;
		}



		return false;
	}

	bool ProbabilitySegmentation::computeWeightingVector( int nRadius, std::vector<Math::Vector4d>& vecWeightingVector, double fCentralVoxelNominator, double fCentralVoxelDenominator )
	{
		// - from the number of voxels inside the speher for the given radius, calculate the the total weighting factor
		// - based on the total weighting calculate the current wweightin factor and write it together with the corresponding indices to the weighing vector

		if (fCentralVoxelNominator> 0.0 && fCentralVoxelDenominator > 0.0) {

			//calculate the total weighting
			double fTotalWeighting = 0.0;
			//iterate over a sphere with the given radius
			for(int x = -nRadius; x <= nRadius; ++x) {
				for(int y = -nRadius; y <= nRadius; ++y) {
					for(int z =- nRadius;z <= nRadius; ++z) {
						double fLength = sqrt( static_cast<double>(x) * static_cast<double>(x) // (*)
							+ static_cast<double>(y) * static_cast<double>(y)
							+ static_cast<double>(z) * static_cast<double>(z));
						
						if((fLength <= static_cast<double>(nRadius) + 0.5) && (fLength!=0.0))
						{
							//changed if(fLength!=0.0) fTotalWeighting += 1.0 / fLength; // zweite if abfrage mit && hinzugefügt
							fTotalWeighting += 1.0 / fLength;
						}
					}
				}
			}

			//changed: double fWeightingFactor = (fTotalWeighting==0.0) ? 0.0 : (fCentralVoxelDenominator-fCentralVoxelNominator) / (fCentralVoxelDenominator * fTotalWeighting);
			double fWeightingFactor;
			// set the weighting factor
			if (fTotalWeighting==0.0)
			{
				fWeightingFactor = 0.0;
			}else
			{
				fWeightingFactor = (fCentralVoxelDenominator-fCentralVoxelNominator) / (fCentralVoxelDenominator * fTotalWeighting);
			}

			// calculate the current weighting factor and write it with the corresponding indices to the weighting vector
			double fCurrentWeightingFactor;
			for(int x = -nRadius; x <= nRadius; ++x) {
				for(int y = -nRadius; y <= nRadius; ++y) {
					for(int z = -nRadius; z <= nRadius; ++z) { 
						//TODO: diese version ist eleganter als (*) (siehe oben)
						Math::Vector3d vecPos = Math::create<Math::Vector3d>(x, y, z);

						if (Math::norm(vecPos) < static_cast<double>(nRadius)+0.5) {
							if(x==0 && y==0 && z==0) {
								fCurrentWeightingFactor = fCentralVoxelNominator / fCentralVoxelDenominator;
							} else {
								fCurrentWeightingFactor = fWeightingFactor * (1.0 / Math::norm(vecPos));
							}
							Math::Vector4d tmp = Math::create<Math::Vector4d>(x, y, z, fCurrentWeightingFactor);
							vecWeightingVector.push_back(tmp);
						}
					}
				}
			}
		} 
		else // nominator or denominator is zero
		{
			// get number of voxels in the sphere 
			unsigned int nTotalSumOfVoxels=0;
			for(int x = -nRadius; x <= nRadius; ++x) {
				for(int y = -nRadius; y <= nRadius; ++y) {
					for(int z = -nRadius; z <= nRadius; ++z) {
						Math::Vector3d vecPos = Math::create<Math::Vector3d>(x, y, z);

						if (Math::norm(vecPos) < static_cast<double>(nRadius)+0.5) {
							nTotalSumOfVoxels++;
						}
					}
				}
			}
			// calculate current weighting factor
			double fCurrentWeightingFactor = 1.0 / nTotalSumOfVoxels;

			// write voxels inices + current weighting factor to the weighting vector
			for(int x = -nRadius; x <= nRadius; ++x) {
				for(int y = -nRadius; y <= nRadius; ++y) {
					for(int z = -nRadius; z <= nRadius; ++z) {
						Math::Vector3d vecPos = Math::create<Math::Vector3d>(x, y, z);

						if (Math::norm(vecPos) < static_cast<double>(nRadius)+0.5) {
							Math::Vector4d tmp = Math::create<Math::Vector4d>(x, y, z, fCurrentWeightingFactor);
							vecWeightingVector.push_back(tmp);
						}
					}
				}
			}
		}
		return true;
	}

	bool ProbabilitySegmentation::computePobabilitySlices( std::vector<double*>& vecSlices,
		std::vector<VoxelType*>& vecOriginalSlices,
		const unsigned int nRadius,
		const unsigned int nSlice,
		const bool bShift)
	{
		// - create slices and original slices 
		// - compute the probabilities for the slices 
		// - shift them if necessary

		size_t nNumOfRegions = m_pVecProbabilities->size();
		Math::Vector3ui volumeSize = m_pVolumeData->size();
		const VoxelType* pInData = m_pVolumeData->data();

		// create slices
		if(vecSlices.size()==0) {
			for (size_t i = 0; i < (2.0 * nRadius + 1); ++i) {
				double* slice = new double[volumeSize(0) * volumeSize(1) * nNumOfRegions];
				memset(slice, 0, volumeSize(0) * volumeSize(1) * nNumOfRegions * sizeof(double));
				vecSlices.push_back(slice);
			}
		}
		//init slices
		if (!bShift) {
			for (size_t i = 0; i < vecSlices.size(); ++i) {
				size_t nLocalSlice = nSlice - nRadius + i;
				getProbabilitySlice(vecSlices.at(i),nLocalSlice);
			}
		} else {
			//shift slices
			double* pLastSlice = vecSlices.front();
			for (size_t i = 1; i < vecSlices.size(); ++i) {
				vecSlices.at(i - 1) = vecSlices.at(i);
			}
			vecSlices.back() = pLastSlice;
			size_t nLocalSlice = nSlice + nRadius;
			getProbabilitySlice(pLastSlice, nLocalSlice);
		}

		// create original slices
		int nOriginalRadius = m_fAdditionalOriginalSlicesFactor * nRadius;
		if(vecOriginalSlices.size()==0) {
			for (size_t i = 0; i < (2.0 * nRadius + 1); ++i) {
				VoxelType* slice = new VoxelType[volumeSize(0) * volumeSize(1) * nNumOfRegions];
				memset(slice, 0, volumeSize(0) * volumeSize(1) * nNumOfRegions * sizeof(VoxelType));
				vecOriginalSlices.push_back(slice);
			}
		}
		//init original slices
		if (!bShift) {
			for (size_t i = 0; i < vecOriginalSlices.size(); ++i) {
				long long nLocalSlice = static_cast<int>(nSlice) - static_cast<int>(nOriginalRadius) + static_cast<int>(i);

				nLocalSlice = std::min(std::max(0ll, nLocalSlice), (long long) (volumeSize(2)-1)); // clamp to valid range
				uint nSliceArea = volumeSize(0) * volumeSize(1);
				long long z_offset = nLocalSlice * volumeSize(0) * volumeSize(1);
				memcpy(vecOriginalSlices.at(i), pInData + z_offset, nSliceArea * sizeof(VoxelType));
			}
		} else {
			//shift original slices
			VoxelType* pLastSlice = vecOriginalSlices.front();
			for (size_t i = 1; i < vecOriginalSlices.size(); ++i) {
				vecOriginalSlices.at(i-1) = vecOriginalSlices.at(i);
			}
			vecOriginalSlices.back() = pLastSlice;
			long long nLocalSlice = static_cast<int>(nSlice)+static_cast<int>(nOriginalRadius);
			nLocalSlice = std::min(std::max(0ll, nLocalSlice), (long long) (volumeSize(2)-1)); // clamp to valid range
			uint nSliceArea = volumeSize(0) * volumeSize(1);
			long long z_offset = nLocalSlice * volumeSize(0) * volumeSize(1);
			memcpy(pLastSlice, pInData + z_offset, nSliceArea * sizeof(VoxelType));
		}

		return true;
	}

	void ProbabilitySegmentation::getProbabilitySlice(double* pSlice, int nSlice) {
		// - for a given slice compute the corresponding probabilities

		std::vector<double> vecResultVector(m_pVecProbabilities->size(), 0.0);

		const VoxelType* pData = m_pVolumeData->data();
		Math::Vector3ui volumeSize = m_pVolumeData->size();
		size_t nSliceArea = volumeSize(0) * volumeSize(1);
		size_t z = nSlice * nSliceArea;

		uint nNumOfMaterials = m_pVecProbabilities->size();

		// walk over a given slice z
		for (size_t y = 0; y < volumeSize(1); ++y) {
			for (size_t x = 0; x < volumeSize(0); ++x) {
				//compute result vector
				size_t nCurrentIndex = x + volumeSize(0) * (y);
				VoxelType currentVoxelValue = pData[nCurrentIndex+z];
				// get the corresponding probablility from the lookup table
				double* pResultVector = (&m_probabilityLookUpTable.front()) + currentVoxelValue * nNumOfMaterials;

				//copy result vector
				for(size_t r = 0; r < nNumOfMaterials; ++r) {
					pSlice[nCurrentIndex + r * nSliceArea] = pResultVector[r];
				}
			}
		}
	}

	bool ProbabilitySegmentation::filterVariableRadius(unsigned int nRadius, std::vector<Math::Vector4d>& vecPrimaryWeightingVector, std::vector<Math::Vector4d>& vecSecondaryWeightingVector)
	{
		// - initialize sclices and compute their probabilities
		// - compute a material index lookup table
		// - compute the most probable material index for the slices based on the weighting vectors

		Math::Vector3ui volumeSize = m_pVolumeMaterialMap->size();
		const size_t nWidth = volumeSize(0);
		const size_t nHeight = volumeSize(1);
		const size_t nDepth = volumeSize(2);
		const size_t nSliceArea = nWidth * nHeight;

		const size_t nNumOfRegions = m_pVecProbabilities->size();

		VoxelType* pOutData = m_pVolumeMaterialMap->mutableData();
		const Quad<double>* pProbabilities = m_pVecProbabilities->data();

		// initilaize slices and compute probablilities for the slices
		std::vector<double*> vecSlices;
		std::vector<VoxelType*> vecOriginalSlices;
		bool bSlices = computePobabilitySlices(vecSlices, vecOriginalSlices, nRadius, nRadius, false);
		bool bOriginalSlices = bSlices;

		VoxelType maxValue = std::numeric_limits<VoxelType>::max();
		VoxelType greyValueScaling = maxValue / nNumOfRegions;

		uint nCentralSlice = (vecSlices.size()-1)/2;

		// compute material index look-up table
		VoxelType* pMaterialIntensityLookUpTable = new VoxelType[nNumOfRegions+1];
		pMaterialIntensityLookUpTable[0] = 0;
		for (uint i=1;i<nNumOfRegions+1;++i)
		{
			uint nMaxProbIndex = i - 1;
			if(nMaxProbIndex < m_nFirstMaterialIndex){
				pMaterialIntensityLookUpTable[i] = 0;
			} else {
				if(m_outputMaterialIndex){  
					pMaterialIntensityLookUpTable[i] = pProbabilities[nMaxProbIndex].second; //  write peak index to lookup table
				}
				else{
					pMaterialIntensityLookUpTable[i] = greyValueScaling * (nMaxProbIndex+1); 
				}
			}      
		}

#ifdef _OPENMP
		// number of possible concurrent threads multiplied by 0.75
		m_nOpenMPCurrentThreads = static_cast<int>(m_fCPUPerformanceValue * static_cast<float>(omp_get_max_threads()));
#if DEBUGINFOS == 1
		std::cout << "Set num threads to: " << m_nOpenMPCurrentThreads << std::endl;
#endif
		//omp_set_dynamic(1);
		//std::cout << "OpenMP active with " <<nOpenMPMaxThreads << " Threads"<< std::endl;
#endif

		float fSegmentationProgressFactor = 3.0 * m_nIter;
		m_progressMax += fSegmentationProgressFactor * (nDepth - 2 * nRadius) + (nDepth - (m_nMaskSize - 1)) * m_nIter;      

		size_t mySumTime = 0;
		const int nTo = nHeight - nRadius;
		const int nRad = nRadius;
		//double** pResultVector = new double*[m_nOpenMPCurrentThreads];
		//for(int i = 0; i < m_nOpenMPCurrentThreads; i++)
		//{
		//   pResultVector[i] = new double[nNumOfRegions];
		//}

		//std::cout << "vecSlices size: " << vecSlices.size() << std::endl;
		//std::cout << "vecPrimaryWeightingVector size: " << vecPrimaryWeightingVector.size() << std::endl;
		//std::cout << "vecSecondaryWeightingVector size: " << vecSecondaryWeightingVector.size() << std::endl;

		for (int z = nRad; z < (nDepth - nRad); ++z) {
			boost::posix_time::ptime myTestTime(boost::posix_time::microsec_clock::local_time());   // SM

			size_t nZOffset = z * nWidth * nHeight;
			LOG(Log::Level::Debug, OPENOR_MODULE, Log::msg("Filter slice: %1% / %2%") % z % nDepth);

			// create a team of nOpenMPMaxThreads threads
#        if defined(_OPENMP)
#           pragma omp parallel num_threads(m_nOpenMPCurrentThreads)
#        endif
			{
				double* pResultVector = new double[nNumOfRegions];

#           if defined(_OPENMP)                             
#              pragma omp for nowait ///////////// OpenMP loop parallelization STARTS here //////////////
#           endif
				// calculate most probable material index
				for (int y = nRad; y < nTo; ++y) {

					size_t nYOffset = nWidth*y;
					for (int x = nRad; x < (nWidth - nRad); ++x) {
						//calc probability with vecPrimaryWeightingVector
						int nMaxProbIndex = getMaterialIndex(vecSlices, vecPrimaryWeightingVector, pResultVector,
							x, y, nWidth, nHeight, nCentralSlice, nNumOfRegions, nSliceArea);

						if (nMaxProbIndex<0){
							//if result is not distinct  use vecSecondaryWeightingVector
							nMaxProbIndex = getMaterialIndex(vecSlices, vecSecondaryWeightingVector, pResultVector,
								x, y, nWidth, nHeight, nCentralSlice, nNumOfRegions, nSliceArea);

							//if result is not distinct use gradient based material index computation
							if(nMaxProbIndex < 0 && nMaxProbIndex < 0) {                      
								//use the highest probabilities...
								nMaxProbIndex = getMaterialIndex(vecSlices, vecPrimaryWeightingVector, pResultVector,
									x, y, nWidth, nHeight, nCentralSlice, nNumOfRegions, nSliceArea, false);
							}
						}
						pOutData[static_cast<size_t>(x) + nYOffset + nZOffset] = pMaterialIntensityLookUpTable[nMaxProbIndex+1];
					}
				} // OpenMP loop parallelization ENDS here
				delete[] pResultVector;
			}
			mySumTime +=  (boost::posix_time::microsec_clock::local_time() - myTestTime).total_milliseconds();

			if (m_canceled) { break; }

			if ((z + 1) <= (nDepth - nRadius - 1)) {
				computePobabilitySlices(vecSlices, vecOriginalSlices, nRadius, z + 1, true);
			}
			m_progress+=fSegmentationProgressFactor;
		}

		//for(int i = 0; i < m_nOpenMPCurrentThreads; i++)
		//{
		//   delete[] pResultVector[i];
		//}
		//delete[] pResultVector;
#if DEBUGINFOS == 1
		std::cout << "Filter radius openMP: " << mySumTime/1000.0 << " s" << std::endl;
#endif
		// set the border pixels to zero
		setBorderPixelToZero(nRadius);


		// clean up 
		if (bSlices) {
			for (size_t i = 0; i < vecSlices.size(); ++i) { delete[] vecSlices.at(i); }
			vecSlices.clear();
		}
		if (bOriginalSlices) {
			for (size_t i = 0; i < vecOriginalSlices.size(); ++i) { delete[] vecOriginalSlices.at(i); }
			vecOriginalSlices.clear();
		}
		delete[] pMaterialIntensityLookUpTable;
		return true;
	}

	void ProbabilitySegmentation::setBorderPixelToZero(unsigned int nRadius) {
		
		// - set the border pixels in all directions to zero

		VoxelType* pData = m_pVolumeMaterialMap->mutableData();
		Math::Vector3ui volumeSize = m_pVolumeMaterialMap->size();

		//z-slices
		for (size_t z = 0; z < nRadius; ++z) {
			for (size_t y = 0; y < volumeSize(1); ++y) {
				for (size_t x = 0; x < volumeSize(0); ++x) {
					pData[x + volumeSize(0) * y + volumeSize(0) * volumeSize(1) * z] = 0;
				}
			}
		}
		for (size_t z = volumeSize(2) - nRadius; z < volumeSize(2); ++z) {
			for (size_t y = 0; y < volumeSize(1); ++y) {
				for (size_t x = 0; x < volumeSize(0); ++x) {
					pData[x + volumeSize(0) * y + volumeSize(0) * volumeSize(1) * z] = 0;
				}
			}
		}
		//y-slices
		for (size_t y = 0; y < nRadius; ++y) {
			for (size_t z = 0; z < volumeSize(2); ++z) {
				for (size_t x = 0; x < volumeSize(0); ++x) {
					pData[x + volumeSize(0) * y + volumeSize(0) * volumeSize(1) * z] = 0;
				}
			}
		}
		for (size_t y = volumeSize(1) - nRadius; y < volumeSize(1); ++y) {
			for (size_t z = 0; z < volumeSize(2); ++z) {
				for (size_t x = 0; x < volumeSize(0); ++x) {
					pData[x + volumeSize(0) * y + volumeSize(0) * volumeSize(1) * z] = 0;
				}
			}
		}
		//x-sclices
		for (size_t x = 0;x < nRadius; ++x) {
			for (size_t z = 0;z < volumeSize(2); ++z) {
				for (size_t y = 0; y < volumeSize(1); ++y) {
					pData[x + volumeSize(0) * y + volumeSize(0) * volumeSize(1) * z] = 0;
				}
			}
		}
		for (size_t x = volumeSize(0) - nRadius; x < volumeSize(0); ++x) {
			for (size_t z = 0; z < volumeSize(2); ++z) {
				for (size_t y = 0; y < volumeSize(1); ++y) {
					pData[x + volumeSize(0) * y + volumeSize(0) * volumeSize(1) * z] = 0;
				}
			}
		}

		return;
	}

	int ProbabilitySegmentation::getMaterialIndex(  std::vector<double*>& vecSlices,
		std::vector<Math::Vector4d>& vecWeightingVector,
		double* pResultVector,
		unsigned int x, unsigned int y,
		unsigned int nWidth, unsigned int nHeight,
		unsigned int nCentralSlice,
		unsigned int nNumOfRegions,
		unsigned int nSliceArea,
		bool bUseThreshold )
	{
		// - get the material index that is most probable

		memset(pResultVector, 0, nNumOfRegions*sizeof(double));

		for (size_t i = 0; i < vecWeightingVector.size(); ++i) {
			size_t nSliceIndex = (x - vecWeightingVector[i](0)) + (y - vecWeightingVector[i](1)) * nWidth;
			for (size_t prob = 0; prob < nNumOfRegions; ++prob) {
				//get probability
				double fCurrentProb = vecSlices.at(nCentralSlice - vecWeightingVector[i](2))[nSliceIndex + prob * nSliceArea];
				//and weight it
				pResultVector[prob] += vecWeightingVector[i](3) * fCurrentProb;
			}
		}

		// search max probability
		int nMaxProbIndex = -1;
		double fMaxProb = -1.0;
		double fTotalProbability = 0.0;
		for(uint r = 0; r < nNumOfRegions; ++r) {
			fTotalProbability += pResultVector[r];
			if (fMaxProb < pResultVector[r]) {
				fMaxProb = pResultVector[r];
				nMaxProbIndex = r;
			}
		}

		if ((fMaxProb / fTotalProbability) < m_fProbabilityThreshold && bUseThreshold) {
			return -1;
		} else {
			return nMaxProbIndex;
		}
	}

	void ProbabilitySegmentation::getVoxelProbabilityVector( double voxelValue, double fWeighting, double* vecResultVector, unsigned int nResultVectorSize)
	{
		// - compute total and current probability for the elements in the result vector
		// - add the weighted probablilites to the elements in the result vector

		memset(vecResultVector, 0, sizeof(double) * nResultVectorSize);
		if (m_pVecProbabilities->size() != nResultVectorSize) { return; }

		const Quad<double>* pProbs = m_pVecProbabilities->data();

		double fTotalProbability = 0.0;

		double fLastMeanMinusStd = 0.0;
		// compute sum of all probability vectors
		for (size_t n = 0; n < nResultVectorSize; ++n) {
			// take peak as mean
			double fMean = pProbs[n].second;
			double fStdDev;
			// take left or right std 
			if(voxelValue <= fMean) {
				fStdDev = pProbs[n].first;
			}
			else {
				fStdDev = pProbs[n].third;
			}
			if (voxelValue > fLastMeanMinusStd) {
				// compute total probability and shift the mean by the std
				fTotalProbability += pProbs[n].fourth * computeNormalDistributedProbability(fMean, fStdDev, voxelValue);
				fLastMeanMinusStd = fMean - pProbs[n].first;
			}
		}

		fLastMeanMinusStd = 0;
		for (size_t n = 0; n < nResultVectorSize; ++n) {
			double fMean = pProbs[n].second;
			double fCurrentProbability = 0.0;
			double fStdDev;
			// take left or right std
			if(voxelValue <= fMean) {
				fStdDev = pProbs[n].first;
			}
			else {
				fStdDev = pProbs[n].third;
			}
			// compute the current probability and shift the mean
			if (voxelValue > fLastMeanMinusStd) {
				fCurrentProbability = pProbs[n].fourth * computeNormalDistributedProbability(fMean, fStdDev, voxelValue);
				fLastMeanMinusStd = fMean - pProbs[n].first;
			}
			// add the weighted probabilites to the current value in the result vector
			if (fTotalProbability > 0.0) {
				vecResultVector[n] += fWeighting * fCurrentProbability / fTotalProbability;
			}
		}  
	}

	double ProbabilitySegmentation::computeNormalDistributedProbability(double mean, double stddev, double x) 
	{
		// - implementation of a normal distribution for a given mean and std at a certain value x 

		if(stddev == 0) {
			if(x==mean) { return 1.0; }
			else     { return 0.0; }
		}

		double result = (static_cast<double>(x)-mean) / stddev;
		result = -0.5 * result * result;
		result = exp(result) / (stddev * sqrt(2.0 * Math::PI));

		return result;
	}


	bool ProbabilitySegmentation::computeNormalDistributedProbabilityLookUpTable( unsigned int minvalue, unsigned int maxvalue, size_t nSize )
	{
		// - create Probability Lookup table

		uint nNumOfMaterials = m_pVecProbabilities->size();
		m_probabilityLookUpTable.resize(nSize * nNumOfMaterials);

		double fDelta = static_cast<double>(maxvalue - minvalue) / (nSize - 1);

		for(uint i = 0; i < nSize; ++i) {
			unsigned int currentValue = minvalue + fDelta * i;
			getVoxelProbabilityVector(currentValue, 1.0, (&m_probabilityLookUpTable.front() + (nNumOfMaterials * i)), nNumOfMaterials);
		}

		return true;
	}

	//---------------------------------------------------------------------------------------------------------------------------
	// Gradient Computation

	//int ProbabilitySegmentation::getGradientBasedMaterialIndex( std::vector<VoxelType*>& vecOriginalSlices, uint nProbabilityRadius, uint x, uint y, uint nWidth, uint nHeight)
	//{
	//	//precondition: x,y,z are valid values >=radius && <= width and height -radius
	//	const Quad<double>* pVecProbabilities = m_pVecProbabilities->data();

	//	const int nCentralSlice = (vecOriginalSlices.size() - 1) / 2;
	//	const int nGradientComputationRadius = nProbabilityRadius;
	//	const int nMaxSearchRadius = nCentralSlice;
	//	const VoxelType VoxelValue = vecOriginalSlices.at(nCentralSlice)[static_cast<uint>(x+y*nWidth)];
	//	float fGradientTresholdFactor = 1.0;

	//	size_t nProbSize = m_pVecProbabilities->size();
	//	size_t nMaxProbIndex = m_pVecProbabilities->size() - 1;
	//	Math::Vector3d vecGradientDirection = getComputeTotalGradientDirection(vecOriginalSlices, x, y, nWidth, nHeight,
	//		nCentralSlice, nGradientComputationRadius);

	//	if(Math::norm(vecGradientDirection) == 0 ) {
	//		int nMaxProbIndex = -1;
	//		double fMaxProb =-1.0;
	//		double* pResultVector = (&m_probabilityLookUpTable.front()) + VoxelValue * nProbSize;
	//		for(uint r = 0; r < nProbSize; ++r) {
	//			if (fMaxProb < pResultVector[r]) {
	//				fMaxProb = pResultVector[r];
	//				nMaxProbIndex = r;
	//			}
	//		}

	//		LOG(Log::Level::Debug, OPENOR_MODULE, Log::msg("no gradient found"));
	//		return nMaxProbIndex;
	//	}

	//	Math::normalize(vecGradientDirection);

	//	//search in both directions for the best solution
	//	const double fStepWidth = 0.5;
	//	const double fStartDistance = 0.5; // in voxel

	//	///////////////////////////////////
	//	//    gradient walking method
	//	///////////////////////////////////

	//	//walk in positive direction
	//	float fDirDistance = 0.0;

	//	double fX = Math::clamp<double>((vecGradientDirection(0) * (fStartDistance) + x), 0.0, nWidth - 1);
	//	double fY = Math::clamp<double>((vecGradientDirection(1) * (fStartDistance) + y), 0.0, nHeight - 1);
	//	double fZ = vecGradientDirection(2) * (fStartDistance) + nCentralSlice;

	//	double fLastVoxelIntensity = getInterpolatedGreyValueAndProbabilityValue(vecOriginalSlices, fX, fY, fZ, nWidth, nHeight)(0);

	//	double fDirGradient;
	//	Math::Vector3d DirResult;
	//	for(float i = (fStartDistance + fStepWidth); i <= nMaxSearchRadius; i = i + fStepWidth) {

	//		double fX = Math::clamp<double>((vecGradientDirection(0) * i + x), 0.0, nWidth - 1);
	//		double fY = Math::clamp<double>((vecGradientDirection(1) * i + y), 0.0, nHeight - 1);
	//		double fZ = vecGradientDirection(2) * i + nCentralSlice;

	//		// compute interpolated voxel grey value, material index with max probability and the probability;
	//		DirResult = getInterpolatedGreyValueAndProbabilityValue(vecOriginalSlices, fX, fY, fZ, nWidth, nHeight);

	//		// calculate intensity gradient
	//		fDirGradient = abs(fLastVoxelIntensity - DirResult(0));
	//		fLastVoxelIntensity = DirResult(0);

	//		// check if voxel intensity is higher or lower than mean value of the current region
	//		if(DirResult(0) > pVecProbabilities[static_cast<size_t>(DirResult(1))].second) {
	//			double fGradientThreshold = fStepWidth * fGradientTresholdFactor
	//				* std::min<double>( pVecProbabilities[static_cast<size_t>(DirResult(1))].third,
	//				pVecProbabilities[static_cast<size_t>(
	//				std::min<double>( DirResult(1) + 1, nMaxProbIndex ))
	//				].first
	//				);
	//			if(fDirGradient < fGradientThreshold) {
	//				fDirDistance = i - fStepWidth;
	//				break;
	//			}
	//		} else {
	//			double fGradientThreshold = fStepWidth * fGradientTresholdFactor
	//				* std::min<double>( pVecProbabilities[static_cast<size_t>(DirResult(1))].first,
	//				pVecProbabilities[static_cast<size_t>(
	//				std::max<double>( DirResult(1) - 1, 0 ))
	//				].third);
	//			if(fDirGradient < fGradientThreshold) {
	//				fDirDistance = i - fStepWidth;
	//				break;
	//			}
	//		}
	//	}

	//	//in negative direction
	//	// TODO: this is duplicated code with positive direction above, make this a function!

	//	fX = Math::clamp<double>((vecGradientDirection(0) * (fStartDistance) + x), 0.0, nWidth - 1);
	//	fY = Math::clamp<double>((vecGradientDirection(1) * (fStartDistance) + y), 0.0, nHeight - 1);
	//	fZ = vecGradientDirection(2) * (fStartDistance) + nCentralSlice;

	//	fLastVoxelIntensity = getInterpolatedGreyValueAndProbabilityValue(vecOriginalSlices, fX, fY, fZ, nWidth, nHeight)(0);

	//	float fOPDirDistance = 0.0;
	//	double fOpDirGradient;
	//	Math::Vector3d OpDirResult;
	//	for(float i = (fStartDistance + fStepWidth); i <= nMaxSearchRadius; i = i + fStepWidth) {
	//		double fX = Math::clamp<double>((vecGradientDirection(0) * i + x), 0.0, nWidth - 1);
	//		double fY = Math::clamp<double>((vecGradientDirection(1) * i + y), 0.0, nHeight - 1);
	//		double fZ = vecGradientDirection(2) * i + nCentralSlice;

	//		// compute interpolated voxel grey value, material index with max probability and the probability;
	//		OpDirResult = getInterpolatedGreyValueAndProbabilityValue(vecOriginalSlices, fX, fY, fZ, nWidth, nHeight);

	//		// calculate intensity gradient
	//		fOpDirGradient = abs(fLastVoxelIntensity - OpDirResult(0));
	//		fLastVoxelIntensity = OpDirResult(0);

	//		// check if voxel intensity is higher or lower than mean value of the current region
	//		if(OpDirResult(0) > pVecProbabilities[static_cast<size_t>(DirResult(1))].second) {
	//			double fGradientThreshold = fStepWidth * fGradientTresholdFactor
	//				* std::min<double>( pVecProbabilities[static_cast<size_t>(OpDirResult(1))].third,
	//				pVecProbabilities[static_cast<size_t>(
	//				std::min<double>( OpDirResult(1) + 1, nMaxProbIndex ))
	//				].first);


	//			if(fOpDirGradient< fGradientThreshold)	{
	//				fOPDirDistance = i - fStepWidth;
	//				break;
	//			}
	//		} else {
	//			double fGradientThreshold = fStepWidth * fGradientTresholdFactor
	//				* std::min<double>( pVecProbabilities[static_cast<size_t>(OpDirResult(1))].first,
	//				pVecProbabilities[static_cast<size_t>(
	//				std::max<double>( OpDirResult(1) - 1, 0 ))
	//				].third);
	//			if(fDirGradient < fGradientThreshold) {
	//				fDirDistance = i - fStepWidth;
	//				break;
	//			}
	//		}
	//	}



	//	double sign = 1.0;
	//	double distance = 0.0;
	//	bool foundSolution = false;

	//	if (fDirDistance == 0.0 && fOPDirDistance == 0.0){
	//		foundSolution = false;
	//	} else if(fDirDistance > 0 && fOPDirDistance == 0) {
	//		// only one solution in direction
	//		sign = 1.0;
	//		distance = fDirDistance;
	//		foundSolution = true;
	//	} else if(fDirDistance == 0 && fOPDirDistance > 0) {
	//		// only one solution in opposite direction
	//		sign = -1.0;
	//		distance = fOPDirDistance;
	//		foundSolution = true;
	//	} else if (abs(DirResult(0) - VoxelValue) > abs(OpDirResult(0) - VoxelValue)) {
	//		// two results => return the nearest intensity
	//		sign = -1.0;
	//		distance = fOPDirDistance;
	//		foundSolution = true;
	//	} else {
	//		sign = 1.0;
	//		distance = fDirDistance;
	//		foundSolution = true;
	//	}

	//	if (foundSolution) {
	//		double fX = Math::clamp<double>((sign * vecGradientDirection(0) * distance + x), 0.0, nWidth - 1);
	//		double fY = Math::clamp<double>((sign * vecGradientDirection(1) * distance + y), 0.0, nHeight - 1);
	//		double fZ = Math::clamp<double>((sign * vecGradientDirection(2) * distance + nCentralSlice), 0.0, vecOriginalSlices.size() - 1);

	//		return getMaterialIndex(vecOriginalSlices, fX, fY, fZ, nWidth, nHeight);
	//	} else {
	//		return -1;
	//	}
	//}

	//Math::Vector3d ProbabilitySegmentation::getComputeTotalGradientDirection( std::vector<VoxelType*>& vecOriginalSlices,
	//	uint x, uint y,
	//	uint nWidth, uint nHeight,
	//	int nCentralSlice,
	//	int nGradientCalcRadius )
	//{
	//	//precondition: x,y,z are valid values >=radius && <= width and height -radius

	//	Math::Vector3d vecResult = Math::create<Math::Vector3d>(0.0, 0.0, 0.0);
	//	VoxelType* slicePlusZ;
	//	VoxelType* sliceMinusZ;

	//	uint nGradientLength = nGradientCalcRadius;
	//	///////////////////
	//	// central slice //
	//	///////////////////
	//	slicePlusZ = vecOriginalSlices.at(nCentralSlice);
	//	sliceMinusZ = vecOriginalSlices.at(nCentralSlice);
	//	Math::norm(Math::create<Math::Vector3d>(0.0, 0.0, 0.0));


	//	vecResult =  Math::normalized(Math::create<Math::Vector3d>(1,0,0))
	//		* ( static_cast<double>(slicePlusZ [x + nGradientLength + y * nWidth])
	//		- static_cast<double>(sliceMinusZ[x - nGradientLength + y * nWidth]));
	//	vecResult += Math::normalized(Math::create<Math::Vector3d>(0,1,0))
	//		* ( static_cast<double>(slicePlusZ [x + (y + nGradientLength) * nWidth])
	//		- static_cast<double>(sliceMinusZ[x + (y - nGradientLength) * nWidth]));
	//	vecResult += Math::normalized(Math::create<Math::Vector3d>(1,1,0))
	//		* ( static_cast<double>(sliceMinusZ[x + nGradientLength + (y + nGradientLength) * nWidth])
	//		- static_cast<double>(slicePlusZ [x - nGradientLength + (y - nGradientLength) * nWidth]));
	//	vecResult += Math::normalized(Math::create<Math::Vector3d>(-1,1,0))
	//		* ( static_cast<double>(sliceMinusZ[x - nGradientLength + (y + nGradientLength) * nWidth])
	//		- static_cast<double>(slicePlusZ [x + nGradientLength + (y - nGradientLength) * nWidth]));

	//	///////////////////////////////////////////////////
	//	//  Outer slices                                 //
	//	//  Diagonal gradients in x and y plane + z-axis //
	//	///////////////////////////////////////////////////

	//	slicePlusZ = vecOriginalSlices.at(nCentralSlice+nGradientLength);
	//	sliceMinusZ = vecOriginalSlices.at(nCentralSlice-nGradientLength);

	//	vecResult += Math::normalized(Math::create<Math::Vector3d>(0,0,1))
	//		* ( static_cast<double>(slicePlusZ [x + y * nWidth])
	//		- static_cast<double>(sliceMinusZ[x + y * nWidth]));
	//	vecResult += Math::normalized(Math::create<Math::Vector3d>(1,0,1))
	//		* ( static_cast<double>(slicePlusZ [x + nGradientLength + y * nWidth])
	//		- static_cast<double>(sliceMinusZ[x - nGradientLength + y * nWidth]));
	//	vecResult += Math::normalized(Math::create<Math::Vector3d>(-1,0,1))
	//		* ( static_cast<double>(slicePlusZ [x - nGradientLength + y * nWidth])
	//		- static_cast<double>(sliceMinusZ[x + nGradientLength + y * nWidth]));
	//	vecResult += Math::normalized(Math::create<Math::Vector3d>(0,1,1))
	//		* ( static_cast<double>(slicePlusZ [x + (y + nGradientLength) * nWidth])
	//		- static_cast<double>(sliceMinusZ[x + (y - nGradientLength) * nWidth]));
	//	vecResult += Math::normalized(Math::create<Math::Vector3d>(0,-1,1))
	//		* ( static_cast<double>(slicePlusZ [x + (y - nGradientLength) * nWidth])
	//		- static_cast<double>(sliceMinusZ[x + (y + nGradientLength) * nWidth]));

	//	if(Math::isNotEqualTo(vecResult, Math::create<Math::Vector3d>(0.0, 0.0, 0.0))) { Math::normalize(vecResult); }
	//	return vecResult;
	//}

	//Math::Vector3d ProbabilitySegmentation::getInterpolatedGreyValueAndProbabilityValue( std::vector<VoxelType*>& vecOriginalSlices,
	//	float x,float y, float z,
	//	uint nWidth, uint nHeight )
	//{
	//	const size_t nNumOfRegions = m_pVecProbabilities->size();
	//	VoxelType VoxelValues[8];

	//	//"front" values => lower slice
	//	VoxelType* currentSlice = vecOriginalSlices.at(floor(z));
	//	VoxelValues[0] = currentSlice[static_cast<size_t>(floor(x) + floor(y) * nWidth)];
	//	VoxelValues[1] = currentSlice[static_cast<size_t>(floor(x) + ceil(y)  * nWidth)];
	//	VoxelValues[2] = currentSlice[static_cast<size_t>(ceil(x)  + floor(y) * nWidth)];
	//	VoxelValues[3] = currentSlice[static_cast<size_t>(ceil(x)  + ceil(y)  * nWidth)];
	//	//
	//	currentSlice = vecOriginalSlices.at(ceil(z));
	//	VoxelValues[4] = currentSlice[static_cast<size_t>(floor(x) + floor(y) * nWidth)];
	//	VoxelValues[5] = currentSlice[static_cast<size_t>(floor(x) + ceil(y)  * nWidth)];
	//	VoxelValues[6] = currentSlice[static_cast<size_t>(ceil(x)  + floor(y) * nWidth)];
	//	VoxelValues[7] = currentSlice[static_cast<size_t>(ceil(x)  + ceil(y)  * nWidth)];

	//	float coords[] = {x - floor(x), y - floor(y), z - floor(z)};
	//	VoxelType interpValue = Trilinear(coords, VoxelValues);
	//	double *pResultVector = (&m_probabilityLookUpTable.front()) + static_cast<uint>(interpValue) * nNumOfRegions;

	//	// search max probability
	//	int nMaxProbIndex = -1;
	//	double fMaxProb = -1.0;
	//	for(size_t r = 0; r < nNumOfRegions; ++r) {
	//		if (fMaxProb < pResultVector[r]) {
	//			fMaxProb = pResultVector[r];
	//			nMaxProbIndex = r;
	//		}
	//	}

	//	return Math::create<Math::Vector3d>(interpValue, nMaxProbIndex, fMaxProb);
	//}

	//int ProbabilitySegmentation::getMaterialIndex( std::vector<VoxelType*>& vecOriginalSlices,
	//	double x, double y, double z,
	//	uint nWidth, uint nHeight )
	//{
	//	const size_t nNumOfRegions = m_pVecProbabilities->size();
	//	std::vector<double> vecResultVector(nNumOfRegions, 0);

	//	//voxel 1
	//	VoxelType CurrentVoxelValue = vecOriginalSlices.at(static_cast<size_t>(floor(z)))[static_cast<size_t>(floor(x)+floor(y)*nWidth)];
	//	double* pCurrentProbVector = (&m_probabilityLookUpTable.front()) + CurrentVoxelValue * nNumOfRegions;
	//	for (size_t prob=0;prob<nNumOfRegions;++prob)vecResultVector[prob]+=pCurrentProbVector[prob];
	//	//voxel 2
	//	CurrentVoxelValue = vecOriginalSlices.at(static_cast<size_t>(ceil(z)))[static_cast<size_t>(floor(x)+floor(y)*nWidth)];
	//	pCurrentProbVector = (&m_probabilityLookUpTable.front()) + CurrentVoxelValue*nNumOfRegions;
	//	for (size_t prob=0;prob<nNumOfRegions;++prob)vecResultVector[prob]+=pCurrentProbVector[prob];
	//	//voxel 3
	//	CurrentVoxelValue = vecOriginalSlices.at(static_cast<size_t>(floor(z)))[static_cast<size_t>(ceil(x)+floor(y)*nWidth)];
	//	pCurrentProbVector = (&m_probabilityLookUpTable.front()) +CurrentVoxelValue*nNumOfRegions;
	//	for (size_t prob=0;prob<nNumOfRegions;++prob)vecResultVector[prob]+=pCurrentProbVector[prob];
	//	//voxel 4
	//	CurrentVoxelValue = vecOriginalSlices.at(static_cast<size_t>(ceil(z)))[static_cast<size_t>(ceil(x)+floor(y)*nWidth)];
	//	pCurrentProbVector = (&m_probabilityLookUpTable.front()) +CurrentVoxelValue*nNumOfRegions;
	//	for (size_t prob=0;prob<nNumOfRegions;++prob)vecResultVector[prob]+=pCurrentProbVector[prob];
	//	//voxel 5
	//	CurrentVoxelValue = vecOriginalSlices.at(static_cast<size_t>(floor(z)))[static_cast<size_t>(floor(x)+ceil(y)*nWidth)];
	//	pCurrentProbVector = (&m_probabilityLookUpTable.front()) +CurrentVoxelValue*nNumOfRegions;
	//	for (size_t prob=0;prob<nNumOfRegions;++prob)vecResultVector[prob]+=pCurrentProbVector[prob];
	//	//voxel 6
	//	CurrentVoxelValue = vecOriginalSlices.at(static_cast<size_t>(ceil(z)))[static_cast<size_t>(floor(x)+ceil(y)*nWidth)];
	//	pCurrentProbVector = (&m_probabilityLookUpTable.front()) +CurrentVoxelValue*nNumOfRegions;
	//	for (size_t prob=0;prob<nNumOfRegions;++prob)vecResultVector[prob]+=pCurrentProbVector[prob];
	//	//voxel 7    
	//	CurrentVoxelValue = vecOriginalSlices.at(static_cast<size_t>(floor(z)))[static_cast<size_t>(ceil(x)+ceil(y)*nWidth)];
	//	pCurrentProbVector = (&m_probabilityLookUpTable.front()) +CurrentVoxelValue*nNumOfRegions;
	//	for (size_t prob=0;prob<nNumOfRegions;++prob)vecResultVector[prob]+=pCurrentProbVector[prob];
	//	//voxel 8  
	//	CurrentVoxelValue = vecOriginalSlices.at(static_cast<size_t>(ceil(z)))[static_cast<size_t>(ceil(x)+ceil(y)*nWidth)];
	//	pCurrentProbVector = (&m_probabilityLookUpTable.front()) +CurrentVoxelValue*nNumOfRegions;
	//	for (size_t prob=0;prob<nNumOfRegions;++prob)vecResultVector[prob]+=pCurrentProbVector[prob];

	//	// search max probability
	//	int nMaxProbIndex = -1;
	//	double fMaxProb =-1.0;
	//	for(size_t r = 0; r < nNumOfRegions; ++r) {
	//		if (fMaxProb<vecResultVector[r]) {
	//			fMaxProb=vecResultVector[r];
	//			nMaxProbIndex = r;
	//		}
	//	}

	//	return nMaxProbIndex;
	//}

	//---------------------------------------------------------------------------------------------------------------------------
	// Post Processing

	bool ProbabilitySegmentation::doProbabilitySegmentationPostProcessing() {

		// - iterate through the volume an look at the 2*nMaskDelta+1 Neighbourhood of the current voxel and count the occurance of the intensities
		// - if the occurance of voxel intensitiy is below a threshold check if the intensity of the current voxel is equal an entry of the material count vector
		// - find the intensity with the highest occurrence and write it to the slice
		VoxelType* pData = m_pVolumeMaterialMap->mutableData();
		Math::Vector3ui volumeSize = m_pVolumeMaterialMap->size();
		const size_t nWidth = volumeSize(0);
		const size_t nHeight = volumeSize(1);
		const size_t nDepth = volumeSize(2);

		std::vector<VoxelType> Values;
		Values.resize(m_nMaskSize * m_nMaskSize * m_nMaskSize);
		const int nMaskDelta = (m_nMaskSize - 1) / 2.0;
		const uint nThreshold = floor((static_cast<double>(m_nMaskSize * m_nMaskSize * m_nMaskSize) / 10.0 ) + 0.5 );

		size_t nNumOfVoxelsChanged = 0;

		//m_progressMax += (nDepth - 2 * nMaskDelta) * m_nIter;

		int iterations = m_nIter;
		while(iterations!=0) {
			nNumOfVoxelsChanged = 0;

			// iterate through voxels
			for(size_t z = nMaskDelta; z < (nDepth - nMaskDelta); z++) {

				LOG(Log::Level::Debug, OPENOR_MODULE, Log::msg("PostProcessing: slice: %1% / %2%") % z % nDepth);
				for(size_t x = nMaskDelta; x < (nWidth - nMaskDelta); x++) {
					for(size_t y = nMaskDelta; y < (nHeight - nMaskDelta); y++) {

						VoxelType VoxelValue = pData[x + y * nWidth + z * nWidth * nHeight];
						size_t nCounter=0;
						uint nCurrentVoxelIntensityCount = 0;

						// find values in a 2*nMaskDelta+1 neighborhood
						for(int Mz = -nMaskDelta; Mz <= nMaskDelta; Mz++) {
							size_t nMZIndex = (static_cast<size_t>(z + Mz)) * nWidth * nHeight;
							for(int My = -nMaskDelta; My <= nMaskDelta; My++) {
								size_t nMYZIndex = static_cast<size_t>(y + My) * nWidth + nMZIndex;
								for(int Mx = -nMaskDelta; Mx <= nMaskDelta; Mx++) {
									size_t nGlobalIndex = (static_cast<size_t>(x + Mx) + nMYZIndex);
									Values[nCounter] = pData[nGlobalIndex];

									// count how often the current voxel intensity occurs in neighborhood
									if(Values[nCounter] == VoxelValue) {
										++nCurrentVoxelIntensityCount;
									}
									++nCounter;
								}
							}
						}

						// check if the occurrence of the voxel intensity is under the threshold
						if (nCurrentVoxelIntensityCount <= nThreshold) {
							std::vector<std::pair<VoxelType,uint> > vecMaterialCount;
							vecMaterialCount.clear();
							vecMaterialCount.reserve(6);

							// insert first value and its occurrence (1) into material count vector
							vecMaterialCount.push_back(std::pair<VoxelType,uint>(Values[0], 1));

							// do for all other values
							for(size_t i = 1; i < Values.size(); i++) {
								// go through material count vector
								for(size_t j = 0; j < vecMaterialCount.size(); j++) {
									// check if the intensity of the current voxel is equal an entry of the material count vector
									if(Values[i]==vecMaterialCount[j].first) {
										vecMaterialCount[j].second++;
										break;
									} else if(j==(vecMaterialCount.size()-1)) {
										// if current intensity is obsolete in material count vector add the value and its count
										vecMaterialCount.push_back(std::pair<VoxelType,uint>(Values[i],1));
										break;
									}
								}
							}

							uint valueCount = vecMaterialCount.at(0).second;
							uint nIndex = 0;

							// find the intensity with the highest occurrence
							for(size_t i = 1; i < vecMaterialCount.size(); i++) {
								if(valueCount < vecMaterialCount[i].second) {
									valueCount = vecMaterialCount[i].second;
									nIndex = i;
								}
							}

							// copy "corrected" value into slice
							pData[static_cast<size_t>(x) + y * nWidth + z * nWidth * nHeight] = vecMaterialCount[nIndex].first;
							++nNumOfVoxelsChanged;
						}
					}
				}

				++m_progress;
				if (m_canceled) { break; }
			}

			LOG(Log::Level::Debug, OPENOR_MODULE, Log::msg("PostProcessing: %1% voxels changed") % nNumOfVoxelsChanged);

			// TODO: BUG??
			if (iterations < 0) {
				if (nNumOfVoxelsChanged < abs(iterations)) {
					iterations = 0;
				}
			} else {
				--iterations;
			}

			if (m_canceled) { break; }
		}

		return true;
	}

	void ProbabilitySegmentation::setGPU(bool bGPU)
	{
		m_bGPU = bGPU;
	}

	void ProbabilitySegmentation::setCPUPerformanceValue(float fCPUPerformanceValue)
	{
		m_fCPUPerformanceValue = fCPUPerformanceValue;
	}

	//---------------------------------------------------------------------------------------------------------------------------

}
