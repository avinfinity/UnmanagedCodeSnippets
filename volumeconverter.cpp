#pragma region Includes

//#include "stdafx.h"
#include "VolumeConverter.h"
#include <Inventor/helpers/SbDataTypeMacros.h>

#pragma endregion

namespace Zeiss
{
namespace IMT
{
namespace NG
{
namespace Metrotom
{
namespace VSGWrapper
{

#pragma region Constructor

VolumeConverter::VolumeConverter() : 
	SoVolumeConverter(),
	_RawReader(NULL),
	//_Progress(nullptr),
	_ProgressMessage(std::wstring())
{

}

VolumeConverter::VolumeConverter(SoVRGenericFileReader* reader) : 
	SoVolumeConverter(),
	_RawReader(reader),
	//_Progress(nullptr),
	_ProgressMessage(std::wstring())
{
	if (_RawReader != nullptr)
		_RawReader->ref();
}

VolumeConverter::VolumeConverter(SoVRGenericFileReader* reader, std::wstring const& progressMessage) :
	SoVolumeConverter(),
	_RawReader(reader),
	_ProgressMessage(progressMessage)
{
	if (_RawReader != nullptr)
		_RawReader->ref();
}

VolumeConverter::~VolumeConverter()
{
	if (_RawReader != nullptr)
		_RawReader->unref();
}

#pragma endregion

void VolumeConverter::specifyReader(float width, float depth, float height, float voxelSize, const SbString& /*filename*/)
{
	SoVRGenericFileReader *_RawReader = new SoVRGenericFileReader();
	_RawReader->setFilename("E:\\MyConverter\\Messe_7 2010-2-10 8-32.uint16_scv");
	_RawReader->setDataChar(SbBox3f(0.0f, 0.0f, 0.0f, width*voxelSize, height*voxelSize, depth*voxelSize), SoDataSet::UNSIGNED_SHORT, SbVec3i32((int)width, (int)height, (int)depth), 1024);
}

SoVolumeReader* VolumeConverter::getReader( const SbString& /*filename*/, const SbString& /*fileExt*/ )
{
	return _RawReader;
};

#pragma region SampleTile Algorithm

void VolumeConverter::sampleTile( const SbVec3i32& tileDim, int dataType, int border, const void* const octantTile[8], const int octantExists[8], void *parentTile )
{
	for ( int octant = 0; octant < 8; octant++ )
	{
		if ( !octantExists[octant] )
			continue;

		SB_DATATYPE_CALL(sampleDecimation, (tileDim, octantTile[octant], parentTile, octant, border, octantExists), dataType);
	}
}

void VolumeConverter::getShiftAndHalfTileDim(SbVec2i32& shiftParentOctant, SbVec3i32& halfTileDim, const SbVec3i32& tileDim, int octant, int border) const
{
	SbVec3i32 halfTile2 = tileDim / 2;
	SbVec3i32 halfTile1 = tileDim - halfTile2;
	int tileDim1 = tileDim[0];
	int tileDim2 = tileDim[0] * tileDim[1];

	halfTileDim = halfTile1;

	shiftParentOctant = SbVec2i32(0, 0);
	if ( octant & 1 )
	{
		shiftParentOctant[0] = halfTile1[0];
		shiftParentOctant[1] = 1;
		halfTileDim[0] = halfTile2[0];
	}

	if ( octant & 2 )
	{
		shiftParentOctant[0] += tileDim1 * halfTile1[1];
		shiftParentOctant[1] += tileDim1;
		halfTileDim[1] = halfTile2[1];
	}

	if ( octant & 4 )
	{
		shiftParentOctant[0] += tileDim2 * halfTile1[2];
		shiftParentOctant[1] += tileDim2;
		halfTileDim[2] = halfTile2[2];
	}

	if ( border == 0 )
		shiftParentOctant[1] = 0;
}

void VolumeConverter::getRatio(SbVec3f &ratio, SbVec3i32 &shiftOctant, SbVec3i32 tileDim, SbVec3i32 halfTileDim, int octant, int border, const int octantExists[8] ) const
{
	shiftOctant = SbVec3i32(0, 0, 0);

	// Border == 1 is a special case
	if ( border == 1 )
	{
		ratio = SbVec3f(2.0,2.0,2.0);
		if ( octant % 2 )
			shiftOctant[0] = border;

		if  ( octant != 0 && octant != 1 && octant != 4 && octant != 5 )
			shiftOctant[1] = border;

		if ( octant >= 4)
			shiftOctant[2] = border;

		return;
	}

	int leftBorder = int (border / 2);
	int rightBorder = border - leftBorder;

	// Compute the ratio for X axis
	if ( octant % 2 )
	{
		ratio[0] = (float)(tileDim[0] - rightBorder) / (float)halfTileDim[0];
		shiftOctant[0] = leftBorder;
	}
	else
	{
		if ( octantExists[octant+1] )
			ratio[0] = (float)(tileDim[0] - leftBorder) / (float) halfTileDim[0];
		else
			ratio[0] = (float)tileDim[0] / (float) halfTileDim[0];

		shiftOctant[0] = 0;
	}

	// Compute ratio for Y axis
	if ( octant == 0 || octant == 1 || octant == 4 || octant == 5  )
	{
		if ( octantExists[octant+2] )
			ratio[1] = (float)(tileDim[1] - leftBorder) / (float) halfTileDim[1];
		else
			ratio[1] = (float)tileDim[1]/ (float) halfTileDim[1];

		shiftOctant[1] = 0;
	}
	else
	{
		ratio[1] = (float)(tileDim[1] - rightBorder) / (float)halfTileDim[1];
		shiftOctant[1] = leftBorder;
	}

	// Compute ratio for Z axis
	if ( octant < 4 )
	{
		if ( octantExists[octant+4] )
			ratio[2] = (float)(tileDim[2] - leftBorder) / (float) halfTileDim[2];
		else
			ratio[2] = (float)tileDim[2] / (float) halfTileDim[2];

		shiftOctant[2] = 0;
	}
	else
	{
		ratio[2] = (float)(tileDim[2] - rightBorder) / (float)halfTileDim[2];
		shiftOctant[2] = leftBorder;
	}
}

#pragma endregion

}
}
}
}
}
