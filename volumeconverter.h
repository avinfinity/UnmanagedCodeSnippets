#pragma region Copyright
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Carl Zeiss IMT GmbH
 * Softwaresystem New Generation
 * (c) Carl Zeiss 2011
 *
 * Dateiname: VolumeConverter.h
 * Autor: Sylvie Schöniger
 *
 * Beschreibung:
 *	Klasse, um Volumen Daten in das LDM-Format zu konvertieren.
 *	Die konvertierte Files werden als *.ldm und *.dat geschrieben
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma endregion

#pragma once

#include <VolumeViz/readers/SoVolumeReader.h>
#include <VolumeViz/converters/SoVolumeConverter.h>
#include <VolumeViz/readers/SoVRGenericFileReader.h> 

class SoVolumeData;

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

#if _MANAGED
public
#endif

class VolumeConverter : public SoVolumeConverter
{

public:

	#pragma region Constructor/Destructor

	/// <summary>
	/// Constructor.
	/// </summary>
	VolumeConverter();

	/// <summary>
	/// Copy Constructor.
	/// </summary>
	VolumeConverter(SoVRGenericFileReader* reader);

	/// <summary>
	/// Constructor.
	/// </summary>
	VolumeConverter(SoVRGenericFileReader* reader,  std::wstring const& progressMessage);

	/// <summary>
	/// Destructor.
	/// </summary>
	virtual ~VolumeConverter();
	
	#pragma endregion
	
	/// <summary>
	/// Specify a special Reader.
	/// If NULL then VolumeViz will search the best one depending on extension.
	/// </summary>
	void specifyReader(float width, float depth, float height, float voxelSize, const SbString& filename);

protected:

	/// <summary>
	/// Returns the specified VolumeReader.
	/// If NULL then VolumeViz will search the best one depending on extension.
	/// </summary>
	virtual SoVolumeReader* getReader( const SbString& filename, const SbString& fileExt );

	/// <summary>
	/// progress callback
	/// </summary>
	SoVolumeConverter::Abort progress( int numTilesGenerated, int numTilesToGenerate )
	{
		//if (_Progress != nullptr)
		//{
		//	double progress = (double)numTilesGenerated / (double)numTilesToGenerate;
		//	_Progress->Report(progress);
		//}

		//if (_Progress->IsCanceled())
		//	return CVT_ABORT;

		return CVT_CONTINUE;
	};

	#pragma region SampleTile Algorithm
	/// <summary>
	/// custom sampleTile algorithm redefined from SoBaseLDMConverter
	/// </summary>	
	virtual void sampleTile( const SbVec3i32& tileDim, int dataType, int border, const void* const octantTile[8], const int octantExists[8], void *parentTile );

	/// <summary>
	/// Custom sample algo that use decimation algorithm. 
	/// Just take one voxel out of two to fill the parent tile
	/// </summary>
	template <typename T>
	void sampleDecimation( const SbVec3i32& tileDim, const void* const childTile, void *parentTile, int octant, int border, const int octantExists[8])
	{
		//SbVec2i32 shiftParentOctant;
		//SbVec3i32 halfTileDim;
		//SbVec3f ratio;
		//SbVec3i32 shiftOctant;

		//getShiftAndHalfTileDim(shiftParentOctant, halfTileDim, tileDim, octant, border);
		//getRatio(ratio, shiftOctant, tileDim, halfTileDim, octant, border, octantExists);

		//int tileDim1 = tileDim[0];
		//int tileDim2 = tileDim[0] * tileDim[1];

		//int ijkshiftParent = shiftParentOctant[0];

		//T* parentPtr = (T*)parentTile;
		//T* octantPtr = (T*)childTile;

		//int pLine, pSlice = 0;
		//int iShift, jShift, kShift, ioctant, iparent = 0;

		//for ( int k = 0; k < halfTileDim[2]*tileDim2; k+=tileDim2 )
		//{
		//	pLine = 0;
		//	for ( int j = 0; j < halfTileDim[1]*tileDim1; j+=tileDim1 )
		//	{
		//		for ( int i = 0; i < halfTileDim[0]; i++ )
		//		{
		//			iparent = (k + j + i);
		//			iShift = (int)floor((i * ratio[0] ) + 0.5);
		//			jShift = (int)floor((pLine * ratio[1]) + 0.5) * tileDim1;
		//			kShift = (int)floor((pSlice * ratio[2]) + 0.5) * tileDim2;
		//			ioctant = ( iShift + shiftOctant[0] ) + ( jShift + tileDim1*shiftOctant[1]) + ( kShift + tileDim2*shiftOctant[2]);

		//			iparent += ijkshiftParent;
		//			parentPtr[iparent] = octantPtr[ioctant];
		//		}

		//		// If the octant next to current does not exist,
		//		// fill parent tile with proper value to avoid bad interpolation

		//		// 1 - Fill X axis with the last value of the line
		//		// only for even octant
		//		if ( !(octant % 2) &&  !octantExists[octant+1] )
		//		{
		//			int baseOffset = ijkshiftParent + (pSlice * tileDim2) + (tileDim1 * pLine) + halfTileDim[0];
		//			std::fill(parentPtr + baseOffset, parentPtr + baseOffset + halfTileDim[0], parentPtr[iparent]);
		//		}
		//		pLine++;
		//	}

		//	// 2 - Fill Y axis with the last line of the octant
		//	if ( (octant == 0 || octant == 1 || octant == 4 || octant == 5) && !octantExists[octant+2] )
		//	{
		//		int dstOffset = ( halfTileDim[0] * (octant%2)) + (tileDim2 * pSlice) + tileDim[0] * halfTileDim[1];
		//		int srcOffset = ( halfTileDim[0] * (octant%2)) + (tileDim2 * pSlice) + (tileDim[0] * (halfTileDim[1]-1));

		//		if ( octant == 4 || octant == 5 )
		//			dstOffset += halfTileDim[2] * tileDim2;

		//		for ( int y = 0; y < halfTileDim[0]; y++)
		//		{  
		//			std::copy( std::checked_array_iterator<T*>(parentPtr, m_tileSize) + srcOffset,
		//				stdext::checked_array_iterator<T*>(parentPtr, m_tileSize) + srcOffset + halfTileDim[0],
		//				stdext::checked_array_iterator<T*>(parentPtr, m_tileSize) + dstOffset
		//				);
		//			dstOffset += tileDim[0];
		//		}

		//	}

		//	// 3 - Fill other part of the parent tile
		//	if ( (octant == 0 ||octant == 4) && !octantExists[octant+3] )
		//	{
		//		int dstOffset = halfTileDim[0] + (halfTileDim[1] * tileDim[0] ) + (pSlice * tileDim2);

		//		if ( octant == 4 )
		//			dstOffset += halfTileDim[2] * tileDim2;

		//		int neededByte = ((halfTileDim[1]-1) * tileDim[0]) + halfTileDim[0];
		//		for ( int i = 0; i < halfTileDim[0]; i++)
		//		{
		//			std::fill(parentPtr + dstOffset, parentPtr + dstOffset + halfTileDim[0], parentPtr[neededByte]);
		//			dstOffset += tileDim[0];
		//		}
		//	}
		//	pSlice++;
		//}
	};
	#pragma endregion
			
private:

	#pragma region SampleTile Algorithm

	/// <summary>
	/// used by our custom sample tile algorithm
	/// </summary>
	void getShiftAndHalfTileDim(SbVec2i32& shiftParentOctant, SbVec3i32& halfTileDim, const SbVec3i32& tileDim, int octant, int border) const;

	/// <summary>
	/// used by our custom sample tile algorithm
	/// </summary>
	void getRatio(SbVec3f &ratio, SbVec3i32 &shiftOctant, SbVec3i32 tileDim, SbVec3i32 halfTileDim, int octant, int border, const int octantExists[8] ) const;

	#pragma endregion

	#pragma region Members

	SoVRGenericFileReader *_RawReader;
	std::wstring _ProgressMessage;

	#pragma endregion
};

}
}
}
}
}

