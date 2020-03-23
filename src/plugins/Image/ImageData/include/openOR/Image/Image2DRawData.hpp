//****************************************************************************
// (c) 2008, 2009 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
//! OPENOR_INTERFACE_FILE(Image_ImageData)
//****************************************************************************
/**
* @file
* @author Christian Winne
* \ingroup Image_ImageData
*/

#ifndef openOR_Image_Image2DRawData_hpp
#define openOR_Image_Image2DRawData_hpp

#include <boost/tr1/memory.hpp>

#include <openOR/Plugin/CreateInterface.hpp>
#include <openOR/Math/vector.hpp>
#include <openOR/Utility/Types.hpp>

namespace openOR {
	namespace Image {

		/**
		* \brief Image2DRawData
		* \ingroup Image_ImageData
		*/
		template<class Type>
		class Image2DRawData
		{
		public:

			typedef Type ValueType;
			typedef ValueType* Pointer;
			typedef const ValueType* ConstPointer;


			/**
			* Getter for the volume-size as a Size3UI object in indices
			* @return The volume size in indices
			*/
			virtual Math::Vector2ui size() const = 0;

			/** 
			* Setter for the volume-size as a Size3UI object in indices
			* @param size The volume size in indices
			*/
			virtual void setSize(const Math::Vector2ui& size) = 0;

			virtual Pointer mutableData() = 0;

			virtual ConstPointer data() const = 0;

		};    

		typedef Image2DRawData<uint8> Image2DRawDataUI8;
		typedef Image2DRawData<uint16> Image2DRawDataUI16;
		typedef Image2DRawData<float> Image2DRawDataFloat;
		typedef Image2DRawData<double> Image2DRawDataDouble;
		typedef Image2DRawData<UI8Quad> Image2DRawDataUI8Quad;
		typedef Image2DRawData<UI16Quad> Image2DRawDataUI16Quad;
		typedef Image2DRawData<FloatQuad> Image2DRawDataFloatQuad;
	}

	/**
	* Getter for the width of the whole volume object in indices
	* @return The width of the volume object
	*/
	template<class Type>
	const uint width(std::tr1::shared_ptr<const Image::Image2DRawData<Type> > pImageData) { return pImageData->size()(0); }
	template<class Type>
	const uint width(std::tr1::shared_ptr<Image::Image2DRawData<Type> > pImageData) { return pImageData->size()(0); }

	/**
	* Getter for the height of the whole volume object in indices
	* @return The height of the volume object
	*/
	template<class Type>
	const uint height(std::tr1::shared_ptr<const Image::Image2DRawData<Type> > pImageData) { return pImageData->size()(1); }
	template<class Type>
	const uint height(std::tr1::shared_ptr<Image::Image2DRawData<Type> > pImageData) { return pImageData->size()(1); }


	template<class Type>
	Type& voxelIndexRef(std::tr1::shared_ptr<Image::Image2DRawData<Type> > pImageData, const Math::Vector2ui& vecPosition) 
	{
		return *(pImageData->mutableData() + vecPosition(0) + width(pImageData) * (vecPosition(1)));
	}


	template<class Type>
	const Type& voxelIndexRef(std::tr1::shared_ptr<const Image::Image2DRawData<Type> > pImageData, const Math::Vector2ui& vecPosition)
	{
		return *(pImageData->data() + vecPosition(0) + width(pImageData) * (vecPosition(1)));
	}


	template<class Type>
	Type& voxelIndexRef(std::tr1::shared_ptr<Image::Image2DRawData<Type> > pImageData, const uint index)
	{
		return *(pImageData->mutableData() + index);
	}


	template<class Type>
	const Type& voxelIndexRef(std::tr1::shared_ptr<const Image::Image2DRawData<Type> > pImageData, const uint index)
	{
		return *(pImageData->data() + index);
	}





	/************************************************************************/
	/*                                                                      */
	/************************************************************************/

	//template<class Image>
	//struct ImageCreator 
	//{
	//  typedef Image ImageType;

	//  virtual std::tr1::shared_ptr<ImageType> operator()() const = 0;
	//  virtual void setImage(std::tr1::shared_ptr<ImageType> pImage) = 0;
	//};

	//typedef ImageCreator<ImageRawDataUI8> ImageRawDataUI8Creator;
	//typedef ImageCreator<ImageRawDataUI16> ImageRawDataUI16Creator;



	/************************************************************************/
	/*                                                                      */
	/************************************************************************/


}

OPENOR_CREATE_TEMPLATED_INTERFACE_1(Type, openOR::Image::Image2DRawData<Type>)

	Math::Vector2ui size() const { return adaptee()->size(); }
void setSize(const Math::Vector2ui& size) { adaptee()->setSize(size); }

Type* mutableData() { return adaptee()->mutableData(); }
const Type* data() const { return adaptee()->data(); }

OPENOR_CREATE_INTERFACE_END


	//OPENOR_CREATE_TEMPLATED_INTERFACE_1(Image, openOR::ImageCreator<Image>)
	//  std::tr1::shared_ptr<Image> operator()() const { return adaptee()->operator()(); }
	//  void setImage(std::tr1::shared_ptr<Image> pImage) { adaptee()->setImage(pImage); }
	//OPENOR_CREATE_INTERFACE_END
	//


#endif