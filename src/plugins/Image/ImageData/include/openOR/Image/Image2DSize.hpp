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

#ifndef openOR_Image_Image2DSize_hpp
#define openOR_Image_Image2DSize_hpp

#include <boost/tr1/memory.hpp>

#include <openOR/Plugin/CreateInterface.hpp>
#include <openOR/Math/vector.hpp>
#include <openOR/Math/matrix.hpp>
#include <openOR/Utility/Types.hpp>

namespace openOR {
	namespace Image {

		/**
		* \brief SliceSize
		* \ingroup Image_ImageData
		*/
		struct SliceSize
		{
			virtual Math::Vector2d sizeMM() const = 0;  
			virtual void setSizeMM(const Math::Vector2d& sizeMM) = 0;
		};


		/**
		* \brief Interface that describes the size of an image.
		* The image is described by its pixel count and its size in millimeter.
		* \ingroup Image_ImageData
		*/
		struct Image2DSize : SliceSize
		{
			virtual Math::Vector2ui size() const = 0;
			virtual void setSize(const Math::Vector2ui& size) = 0;
		};    

	}

	/*
	* Getter for the voxel width in mm
	* @return The voxel width in mm
	*/
	inline const double pixelWidthMM(std::tr1::shared_ptr<const Image::Image2DSize> pImageSize) { return pImageSize->sizeMM()(0) / pImageSize->size()(0); }

	/**
	* Getter for the voxel height in mm
	* @return The voxel height in mm
	*/
	inline const double pixelHeightMM(std::tr1::shared_ptr<const Image::Image2DSize> pImageSize) { return pImageSize->sizeMM()(1) / pImageSize->size()(1); }    


	inline Math::Vector2d pixelSizeMM(std::tr1::shared_ptr<const Image::Image2DSize> pImageSize) 
	{ 
		Math::Vector2d size;
		size(0) = pixelWidthMM(pImageSize);
		size(1) = pixelHeightMM(pImageSize);
		return size; 
	}    

	/**
	* Getter for the width of the whole volume object in mm
	* @return The width of the volume object
	*/
	inline const double getMMWidth(std::tr1::shared_ptr<const Image::Image2DSize> pImageSize) { return pImageSize->sizeMM()(0); }

	/**
	* Getter for the height of the whole volume object in mm
	* @return The height of the volume object
	*/
	inline const double getMMHeight(std::tr1::shared_ptr<const Image::Image2DSize> pImageSize) { return pImageSize->sizeMM()(1); }


	inline Math::Vector2d getMMSize(std::tr1::shared_ptr<const Image::Image2DSize> pImageSize) { return pImageSize->sizeMM(); }


	/**
	* Getter for the width of the whole volume object in indices
	* @return The width of the volume object
	*/
	inline const uint getWidth(std::tr1::shared_ptr<const Image::Image2DSize> pImageSize) { return pImageSize->size()(0); }

	/**
	* Getter for the height of the whole volume object in indices
	* @return The height of the volume object
	*/
	inline const uint getHeight(std::tr1::shared_ptr<const Image::Image2DSize> pImageSize) { return pImageSize->size()(1); }

	/**
	* Getter for the depth of the whole volume object in indices
	* @return The depth of the volume object
	*/
	inline Math::Vector2ui getSize(std::tr1::shared_ptr<const Image::Image2DSize> pImageSize) { return pImageSize->size(); }


	/**
	* Checks whether given relative mm-based point is outside of volume
	* @param  vecPoint The point to check for
	* @return  True, if the point is outside the volume
	*/
	inline const bool isOutside(std::tr1::shared_ptr<const Image::Image2DSize> pImageSize, const Math::Vector2d&) {

		assert(true);
		return true;
	}

	/**
	* Checks whether given relative index-based point is outside of volume
	* @param vecPoint The point to check for
	* @return  True, if the point is outside the volume
	*/      
	inline const bool isOutside(std::tr1::shared_ptr<const Image::Image2DSize> pImageSize, const Math::Vector2i&) {

		assert(true);
		return true;
	}

	/** 
	* Returns the absolute BoundingBox around the volume.
	* @return   The absolute BoundingBox around the volume
	*/
	//Math::BoundingBoxD getBoundingBox(std::tr1::shared_ptr<const Volume> pVolume) {}


	/**
	* Returns index position corresponding to the given mm position.
	* @return The index position
	*/

	inline uint convertToIndex(std::tr1::shared_ptr<const Image::Image2DSize> pImageSize, const Math::Vector2ui& vecPosition)
	{
		return vecPosition(1) * pImageSize->size()(0) + vecPosition(0);
	}


	inline uint convertToIndex(std::tr1::shared_ptr<const Image::Image2DSize> pImageSize, const Math::Vector2i& vecPosition)
	{
		assert(vecPosition(0) >= 0 && vecPosition(0) < static_cast<int>(pImageSize->size()(0)) &&
			vecPosition(1) >= 0 && vecPosition(1) < static_cast<int>(pImageSize->size()(1)));

		Math::Vector2ui vec;
		vec(0) = static_cast<uint>(vecPosition(0));
		vec(1) = static_cast<uint>(vecPosition(1));
		return convertToIndex(pImageSize, vec);
	}


	inline Math::Vector2i convertToIndexPosition(std::tr1::shared_ptr<const Image::Image2DSize> pImageSize, const Math::Vector2d& vecPosition)
	{
		Math::Vector2i result;
		result(0) = static_cast<int>(vecPosition(0) * pImageSize->size()(0) / pImageSize->sizeMM()(0));
		result(1) = static_cast<int>(vecPosition(1) * pImageSize->size()(1) / pImageSize->sizeMM()(1));
		return result;
	}


	/**
	* Returns internal memory index corresponding to the given index/mm position.
	* @return The index
	*/
	inline uint convertToIndex(std::tr1::shared_ptr<const Image::Image2DSize> pImageSize, const Math::Vector2d& vecPosition)
	{
		return convertToIndex(pImageSize, convertToIndexPosition(pImageSize, vecPosition));
	}


}



OPENOR_CREATE_INTERFACE(openOR::Image::SliceSize)
	Math::Vector2d sizeMM() const { return adaptee()->sizeMM(); }
void setSizeMM(const Math::Vector2d& size) { adaptee()->setSizeMM(size); }
OPENOR_CREATE_INTERFACE_END


	OPENOR_CREATE_INTERFACE(openOR::Image::Image2DSize)
	Math::Vector2d sizeMM() const { return adaptee()->sizeMM(); }
void setSizeMM(const Math::Vector2d& size) { adaptee()->setSizeMM(size); }
Math::Vector2ui size() const { return adaptee()->size(); }
void setSize(const Math::Vector2ui& size) { adaptee()->setSize(size); }
OPENOR_CREATE_INTERFACE_END





#endif 
