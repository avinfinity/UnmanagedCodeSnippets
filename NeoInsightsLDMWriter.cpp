#include "NeoInsightsLDMWriter.h"
#include <Inventor/helpers/SbDataTypeMacros.h>
#include <algorithm>
#include "volumetopooctree.h"

NeoInsightsLDMWriter::NeoInsightsLDMWriter()
{

}


void NeoInsightsLDMWriter::setVolumeInfo( int dimX , int dimY , int dimZ , float voxelSizeX , float voxelSizeY , float voxelSizeZ , int valueType )
{
	_DimX = dimX;
	_DimY = dimY;
	_DimZ = dimZ;
	_VoxelSizeX = voxelSizeX;
	_VoxelSizeY = voxelSizeY;
	_VoxelSizeZ = _VoxelSizeZ;
	_ValueType = valueType;
}

void NeoInsightsLDMWriter::setAdditionalData(std::vector< int64_t >& histogram, int minVal, int maxVal)
{
	memcpy(_Histogram, histogram.data(), ( USHRT_MAX + 1 ) * sizeof(int64_t));

	_MinVal = minVal;
	_MaxVal = maxVal;
}

void NeoInsightsLDMWriter::setRawFilePath( std::string filePath )
{
	_RawFilePath = filePath;

	
}

void NeoInsightsLDMWriter::setLDMFilePath( std::string ldmFilePath)
{
	_LDMFilePath = ldmFilePath;
}


int computeStartId(int level)
{
	int sId = 0;

	int startSize = 1;

	for (int ll = 0; ll < level; ll++)
	{
		sId += startSize;

		startSize *= 8;
	}

	return sId;
}


void copySliceDataToTile( unsigned short *data, unsigned short *tileData , int xLen , int yLen , int zLen , int zStride , int yStride )
{
	unsigned short *slicePtr = data;
	unsigned short *tilePtr = tileData;

	//std::cout << xLen << " " << yLen << " " << zLen << std::endl;

	for (int zz = 0; zz < zLen; zz++)
	{
		for (int yy = 0; yy < yLen; yy++)
		{
			memcpy(tilePtr, slicePtr + yStride * yy, xLen * sizeof(unsigned short));

			tilePtr += xLen;
		}

		slicePtr += zStride;
	}

	//std::cout << (tilePtr - tileData) / (_TileDim * _TileDim * _TileDim.0f) << std::endl;
}

void NeoInsightsLDMWriter::generate()
{
	//computeFileIds();

	//_TileDim = 64;

	//open raw file 
	_File = fopen( _RawFilePath.c_str(), "rb" );

	//skip the header
	fseek(_File, 1024, SEEK_SET);

	//Topo tree is needed to created tile hierarchy , it holds information to map tileId to fileId
	//fileId corresponds to non empty tiles in tile hierarchy. ( See VolumeTopoTree for more information. )
	_TopoTree = new imt::volume::VolumeTopoOctree;

	SoLDMTopoOctree *topoTreeLDM = new SoLDMTopoOctree;

	imt::volume::VolumeTopoOctree::Vec3i vDim;

	int tileDim = _TileDim;

	int overlap = 0;

	vDim._X = _DimX;
	vDim._Y = _DimY;
	vDim._Z = _DimZ;

	_TopoTree->init(vDim, tileDim, overlap);

	SbVec3i32 dimension;

	dimension.setValue(_DimX, _DimY, _DimZ);

	topoTreeLDM->init(dimension, _TileDim, 0);

	int highestresolutionLevel = _TopoTree->getNumLevels() - 1;

	std::vector< imt::volume::VolumeTopoOctree::TopoOctreeNode* > highestResolutionNodes = _TopoTree->getFileNodes(highestresolutionLevel);

	int nFileIds = _TopoTree->getNumFiles();

	bool fullMatch = true;

	std::cout << " number of file ids " << nFileIds << " " << topoTreeLDM->getNumFileIDs() << std::endl;

	for (int ff = 0; ff < nFileIds; ff++)
	{
		int tileId1 = _TopoTree->getTileId(ff);

		int tileId2 = topoTreeLDM->getTileID(ff).getID();

		SbBox3i32 tilePosLdm = topoTreeLDM->getTilePos(tileId1);

		auto node = _TopoTree->getFileNode(ff);

		int ldmX, ldmY, ldmZ;

		tilePosLdm.getOrigin(ldmX, ldmY, ldmZ);

		if (node->_Position._X != ldmX && node->_Position._Y != ldmY && node->_Position._Z != ldmZ)
		{
			fullMatch = false;
		}

		if (tileId1 != tileId2)
		{
			fullMatch = false;
		}

		int fileId1 = _TopoTree->getFileId(tileId1);
		int fileId2 = topoTreeLDM->getFileID(tileId2);

		if (fileId1 != fileId2)
		{
			fullMatch = false;
		}
	}

	if (!fullMatch)
	{
		std::cout << " full match failed " << std::endl;
	}
	else
	{
		std::cout << " full match succeeded " << std::endl;
	}


	typedef imt::volume::VolumeTopoOctree::TopoOctreeNode Node;

    //we need to sort highest resolution tiles such that they are sorted in order of slices , this will make sure that we can read volume data from disk
	//in chunk of slices. Reading such a way would be both faster and better candidate for out of core implementation.
	std::sort(highestResolutionNodes.begin(), highestResolutionNodes.end(), [](const Node* item1, const Node* item2)->bool{
	
		return ((item1->_Position._Z < item2->_Position._Z) ||
			    (item1->_Position._Z == item2->_Position._Z && item1->_Position._Y < item2->_Position._Y) ||
			    (item1->_Position._Z == item2->_Position._Z && item1->_Position._Y == item2->_Position._Y && item1->_Position._X < item2->_Position._X));
	});


	//compute number of tiles in each direction( i.e. one dimensional view )
	int nZ = vDim._Z % tileDim == 0 ? vDim._Z / tileDim : vDim._Z / tileDim + 1;
	int nY = vDim._Y % tileDim == 0 ? vDim._Y / tileDim : vDim._Y / tileDim + 1;
	int nX = vDim._X % tileDim == 0 ? vDim._X / tileDim : vDim._X / tileDim + 1;

	//number of voxels in each slice of raw data
	int sliceArea = _DimX * _DimY;

	//preallocate memory rquired for copying slices into tiles 
	unsigned short *vData = new unsigned short[ _DimX * _DimY * _DimZ ];
	//unsigned short *buffer = new unsigned short[sliceArea * tileDim];
	//unsigned short *tileBuffer = new unsigned short[tileDim * tileDim * tileDim * nX * nY * nZ];

	//initialize the tile buffers to 0
	//memset(tileBuffer, 0, tileDim * tileDim * tileDim * nX * nY * nZ * sizeof(unsigned short));

	int zStride = _DimX * _DimY; 
	int yStride = _DimX;

	//unsigned short *tileData = tileBuffer;

	int nodeId = 0;


#if 0
	//Now copy the volume data to highest resolution tiles. Here we keep only size of tile dimension number of slices in memory at one time.
	for ( int zz = 0; zz < nZ ; zz++ )
	{
		if ( zz == nZ - 1 )
		{
			int zLen = std::min(tileDim, _DimZ - (nZ - 1) * tileDim);

			memset( buffer , 0 , sliceArea * tileDim );

			fread(buffer, sizeof(unsigned short), sliceArea * zLen, _File);
		}
		else
		{
			//load the volume slices into memory
			fread( buffer , sizeof(unsigned short) , sliceArea * tileDim , _File );
		}
		
		//now copy the slice data to tiles
		for (int yy = 0; yy < nY; yy++)
		{ 
			for (int xx = 0; xx < nX; xx++)
			{

				//first reach to correct memory location on the slice
				unsigned short *sliceLocPtr = buffer + yy * tileDim * yStride + xx * tileDim;

				int xLen = tileDim;
				int yLen = tileDim;
				int zLen = tileDim;

				//handle the boundary cases as dimensions may not be multiple of tileDim
				if (xx == nX - 1 )
				{
					xLen = std::min( tileDim , _DimX - (nX - 1) * tileDim );		
				}

				if (yy == nY - 1)
				{
					yLen = std::min( tileDim , _DimY - (nY - 1) * tileDim);
				}

				if (zz == nZ - 1)
				{
					zLen = std::min( tileDim , _DimZ - (nZ - 1) * tileDim);
				}
				
				//fill the tile from slice data
				copySliceDataToTile(sliceLocPtr, tileData, xLen, yLen, zLen, zStride, yStride);

				highestResolutionNodes[nodeId]->_Data = tileData;

				tileData += tileDim * tileDim * tileDim;

				nodeId++;
			}

		}

	}
#else
	
	fread(vData, sizeof(unsigned short), _DimX * _DimY * _DimZ, _File);

	int zT = _DimZ % _TileDim == 0 ? _DimZ / _TileDim : _DimZ / _TileDim + 1;
	int yT = _DimY % _TileDim == 0 ? _DimY / _TileDim : _DimY / _TileDim + 1;
	int xT = _DimX % _TileDim == 0 ? _DimX / _TileDim : _DimX / _TileDim + 1;

	int tileSize = _TileDim * _TileDim * _TileDim;

	for (auto node : highestResolutionNodes)
	{
		auto p = node->_Position;

		int zPos = node->_Position._Z ;
		int yPos = node->_Position._Y ;
		int xPos = node->_Position._X ;

		//first reach to location
		unsigned short* memLoc = vData + zPos * zStride + yPos * yStride + xPos;

		node->_Data = new unsigned short[tileDim * tileDim * tileDim];

		memset(node->_Data, 0, tileDim * tileDim * tileDim * sizeof(unsigned short));

		unsigned short *nodeData = node->_Data;

		int xLen, yLen, zLen;

		if (zPos / tileDim != zT - 1)
		{
			zLen = tileDim;
		}
		else
		{
			zLen = std::min(tileDim, _DimZ - (zT - 1) * tileDim);
		}


		if (yPos / tileDim != yT - 1)
		{
			yLen = tileDim;
		}
		else
		{
			yLen = std::min( tileDim, _DimY - (yT - 1) * tileDim);
		}


		if (xPos / tileDim != xT - 1)
		{

			xLen = tileDim;
		}
		else
		{
			xLen = std::min(tileDim, _DimX - (xT - 1) * tileDim);
		}

		for (int zz = 0; zz < zLen; zz++)
		{
			unsigned short *currLoc = memLoc + zz * zStride;

			unsigned short *ndptr = nodeData + zz * tileDim * tileDim;

			for (int yy = 0; yy < yLen; yy++)
			{
				memcpy(ndptr,  currLoc, xLen * sizeof(unsigned short));
         		ndptr += tileDim;

				currLoc += yStride;
			}

		}

		int minVal = USHRT_MAX;
		int maxVal = SHRT_MIN;

		for (int ii = 0; ii < tileSize; ii++)
		{
			maxVal = std::max((int)node->_Data[ii], maxVal);
			minVal = std::min((int)node->_Data[ii], minVal);
		}

		node->_MinVal = minVal;
		node->_MaxVal = maxVal;

		//std::cout << " min and max : " << minVal << " " << maxVal << std::endl;
	}
#endif


	//release the buffer after usage
	
	fclose(_File);

	//now build the tree by decimating the voxels
	int numLevels = _TopoTree->getNumLevels();

	for (int ll = numLevels - 2; ll >= 0; ll--)
	{

		SbVec3i32 currentLevelTileDim;

		int dimScale = 1 << (numLevels - 1 - ll);

		currentLevelTileDim[0] = tileDim;
		currentLevelTileDim[1] = tileDim;
		currentLevelTileDim[2] = tileDim;

		//first stack the current level nodes , we will traverse them one by one and build the tile for them
		std::vector< Node* > currentLevelNodes = _TopoTree->getFileNodes(ll);

		int tileId = 0;

		

		for (auto node : currentLevelNodes)
		{

			//first allocate space for node tile data
			node->_Data = new unsigned short[tileDim * tileDim * tileDim];

			memset( node->_Data , 0 , tileDim * tileDim * tileDim * sizeof(unsigned short) );

			void *octantTiles[8];
			int octantExists[8];

			for ( int ii = 0; ii < 8; ii++ )
			{
				if (node->_Chidren[ii])
				{
					octantTiles[ii] = node->_Chidren[ii]->_Data;
					octantExists[ii] = 1;
				}
				else
				{
					octantExists[ii] = 0;
					octantTiles[ii] = 0;
				}	
			}

			//this is the step which actually samples the low resolution tile from it's 8 high resolution children tiles
			sampleTile( currentLevelTileDim , SbDataType::UNSIGNED_SHORT , 0 , octantTiles, octantExists, node->_Data );

			int minVal = USHRT_MAX;
			int maxVal = SHRT_MIN;

			for ( int ii = 0; ii < tileSize; ii++ )
			{
				maxVal = std::max((int)node->_Data[ii], maxVal);
				minVal = std::min((int)node->_Data[ii], minVal);
			}

			node->_MinVal = minVal;
			node->_MaxVal = maxVal;

			//std::cout << " min and max : " << minVal << " " << maxVal << std::endl;

			tileId++;
		}
	}

	writeTiles();

}

void NeoInsightsLDMWriter::writeTiles()
{
	FILE *file = fopen(_RawFilePath.c_str(), "rb");

	char rawHeaderBytes[1025];

	fread(rawHeaderBytes, 1, 1024, file);

	fclose(file);

	_Header._DimX = _DimX;
	_Header._DimY = _DimY;
	_Header._DimZ = _DimZ;

	_Header._VoxelSizeX = _VoxelSizeX;
	_Header._VoxelSizeY = _VoxelSizeY;
	_Header._VoxelSizeZ = _VoxelSizeZ;

	_Header._TileTim = _TileDim;

	_File = fopen(_LDMFilePath.c_str(), "wb");

	int nLevels = _TopoTree->getNumLevels();

	_MinVal = 1000;
	_MaxVal = 30000;

	fwrite(rawHeaderBytes,1, 1024, _File);
	fwrite(&_MinVal, sizeof(int), 1, _File);
	fwrite(&_MaxVal, sizeof(int), 1, _File);

	fwrite(_Histogram, sizeof(int64_t), (USHRT_MAX + 1), _File);
	fwrite(&_Header, sizeof(_Header), 1, _File);

	int nNodes = 0;

	//first write node information( currently only min , max values of each tile is recorded )
	for (int ll = 0; ll < nLevels; ll++)
	{
		auto nodes = _TopoTree->getFileNodes(ll);

		for (auto node : nodes)
		{
			fwrite((char*)&(node->_MinVal), sizeof(int), 1, _File);
			fwrite((char*)&(node->_MaxVal), sizeof(int), 1, _File);
		}
	}

	//Now write the tiles in order of file ids
	for (int ll = 0; ll < nLevels; ll++)
	{
		auto nodes = _TopoTree->getFileNodes(ll);

		for (auto node : nodes)
		{
			fwrite( node->_Data , sizeof(unsigned short) , _TileDim * _TileDim * _TileDim , _File );
		}
	}

	fclose(_File);

}


int NeoInsightsLDMWriter::getNumLevels()
{
	return _TopoTree->getNumLevels();
}



void NeoInsightsLDMWriter::getVolumeAtLevel( int level , imt::volume::VolumeInfo& volInfo )
{

	std::vector< imt::volume::VolumeTopoOctree::TopoOctreeNode* > nodes = _TopoTree->getFileNodes(level);

typedef imt::volume::VolumeTopoOctree::TopoOctreeNode Node;

	std::sort(nodes.begin(), nodes.end(), [](const Node* item1, const Node* item2)->bool
	{

		return ( (item1->_Position._Z < item2->_Position._Z) ||
			     (item1->_Position._Z == item2->_Position._Z && item1->_Position._Y < item2->_Position._Y) ||
			     (item1->_Position._Z == item2->_Position._Z && item1->_Position._Y == item2->_Position._Y && 
				  item1->_Position._X < item2->_Position._X));
	});




	int reductionFactor = 1 << (_TopoTree->getNumLevels() - 1 - level);

	int w = _DimX / reductionFactor , h = _DimY / reductionFactor , d = _DimZ / reductionFactor;

	int zT = d % _TileDim == 0 ? d / _TileDim : d / _TileDim + 1;
	int yT = h % _TileDim == 0 ? h / _TileDim : h / _TileDim + 1;
	int xT = w % _TileDim == 0 ? w / _TileDim : w / _TileDim + 1;

	volInfo.mWidth = w;
	volInfo.mHeight = h;
	volInfo.mDepth = d;

	volInfo.mVolumeData = ( unsigned char* )( new unsigned short[w * h * d ] );

	memset(volInfo.mVolumeData, 0, w * h * d * sizeof(unsigned short));

	int nNodes = nodes.size();

	int zStride = w * h , yStride = w;

	std::cout<<" dimensions of the volume : " << w << " " << h << " " << w << std::endl;

	//return;

	unsigned short *vData = (unsigned short*)volInfo.mVolumeData;

	for (auto node : nodes)
	{
		auto p = node->_Position;

		int zPos = node->_Position._Z / reductionFactor;
		int yPos = node->_Position._Y / reductionFactor;
		int xPos = node->_Position._X / reductionFactor;

		//first reach to location
		unsigned short* memLoc = vData + zPos * zStride + yPos * yStride + xPos;

		unsigned short *nodeData = node->_Data;

		int xLen, yLen, zLen;

		if (zPos / _TileDim != zT - 1)
		{
			zLen = _TileDim;
		}
		else
		{
			zLen = std::min(_TileDim, d - (zT - 1) * _TileDim);
		}


		if (yPos / _TileDim != yT - 1)
		{
			yLen = _TileDim;
		}
		else
		{
			yLen = std::min(_TileDim, h - (yT - 1) * _TileDim);
		}


		if (xPos / _TileDim != xT - 1)
		{
			xLen = _TileDim;
		}
		else
		{
			xLen = std::min(_TileDim, w - (xT - 1) * _TileDim);
		}

		
		for (int zz = 0; zz < zLen; zz++)
		{
			unsigned short *currLoc = memLoc + zz * zStride;

			unsigned short *nodePtr = nodeData + zz * _TileDim * _TileDim;

			for (int yy = 0; yy < yLen; yy++)
			{
				memcpy(currLoc, nodePtr, xLen * sizeof(unsigned short));

				nodePtr += _TileDim;

				currLoc += yStride;
			}

		}
			
	//	nodeData[_TileDim * _TileDim * _TileDim - 1];

	}


}


void NeoInsightsLDMWriter::computeHitogram()
{

}


void NeoInsightsLDMWriter::computeFileIds()
{
	int maxDim = 0;

	maxDim = std::max( _DimX, _DimY );
	maxDim = std::max( maxDim, _DimZ);

	int tempDim = maxDim;

	int count = 0;

	while ( tempDim > 0 )
	{
		tempDim = tempDim >> 1;

		count++;
    }

	int extendedDim = 1 << count;

	int highestResolutionLevel = count - 6;

	_ExtendedDimZ = extendedDim;
	_ExtendedDimY = extendedDim;
	_ExtendedDimX = extendedDim;

	_TileDim = _TileDim;

	int nTileZSize = _ExtendedDimZ / _TileDim, nTileYSize = _ExtendedDimY / _TileDim, nTileXSize = _ExtendedDimX / _TileDim;

	int maxValidTileZ = (_DimZ % _TileDim) == 0 ? _DimZ / _TileDim : _DimZ / _TileDim + 1,
		maxValidTileY = (_DimY % _TileDim) == 0 ? _DimY / _TileDim : _DimY / _TileDim + 1, 
		maxValidTileX = (_DimX % _TileDim) == 0 ? _DimX / _TileDim : _DimX / _TileDim + 1;

	mLevelNumFileIds.resize(count);

	std::fill( mLevelNumFileIds.begin(), mLevelNumFileIds.end() , 0);

	int currentLevelExtZ = _ExtendedDimZ;
	int currentLevelExtY = _ExtendedDimY;
	int currentLevelExtX = _ExtendedDimX;

	int currentLevelZDim = _DimZ;
	int currentLevelYDim = _DimY;
	int currentLevelXDim = _DimX;

	int totalNumFileIds = 0;

	for ( int ll = highestResolutionLevel; ll >= 0; ll-- )
	{
		int startId = computeStartId(ll);

		if ( ll == highestResolutionLevel )
		{
			int localMaxFileId = 0;

			for ( int zz = 0; zz < nTileZSize; zz++ )
				for ( int yy = 0; yy < nTileYSize; yy++ )
					for ( int xx = 0; xx < nTileXSize; xx++ )
					{
						if ( zz < maxValidTileZ && yy < maxValidTileY && xx < maxValidTileX )
						{
							localMaxFileId++;
						}
					}


			mLevelNumFileIds[ll] = localMaxFileId;

			currentLevelExtZ /= 2;
			currentLevelExtY /= 2;
			currentLevelExtX /= 2;

			maxValidTileZ = (maxValidTileZ % 2) == 0 ? maxValidTileZ / 2 : maxValidTileZ / 2 + 1;
			maxValidTileY = (maxValidTileY % 2) == 0 ? maxValidTileY / 2 : maxValidTileY / 2 + 1;
			maxValidTileX = (maxValidTileX % 2) == 0 ? maxValidTileX / 2 : maxValidTileX / 2 + 1;

			totalNumFileIds += localMaxFileId;
		}
		else
		{
			nTileZSize = currentLevelExtZ / _TileDim, nTileYSize = currentLevelExtY / _TileDim, nTileXSize = currentLevelExtX / _TileDim;

			int localMaxFileId = 0;

			for (int zz = 0; zz < nTileZSize; zz++)
				for (int yy = 0; yy < nTileYSize; yy++)
					for (int xx = 0; xx < nTileXSize; xx++)
					{
						if ( zz < maxValidTileZ && yy < maxValidTileY && xx < maxValidTileX )
						{
							localMaxFileId++;
						}
					}


			currentLevelExtZ /= 2;
			currentLevelExtY /= 2;
			currentLevelExtX /= 2;

			maxValidTileZ = (maxValidTileZ % 2) == 0 ? maxValidTileZ / 2 : maxValidTileZ / 2 + 1;
			maxValidTileY = (maxValidTileY % 2) == 0 ? maxValidTileY / 2 : maxValidTileY / 2 + 1;
			maxValidTileX = (maxValidTileX % 2) == 0 ? maxValidTileX / 2 : maxValidTileX / 2 + 1;

			mLevelNumFileIds[ll] = localMaxFileId;

			totalNumFileIds += localMaxFileId;
		}

	}


	std::cout << " total number of file ids : " << totalNumFileIds << std::endl;
}



void NeoInsightsLDMWriter::setTileDim(int dim)
{
	_TileDim = dim;
}


void NeoInsightsLDMWriter::sampleTile(const SbVec3i32& tileDim, int dataType, int border, const void* const octantTile[8], const int octantExists[8], void *parentTile)
{
	for ( int octant = 0; octant < 8; octant++ )
	{
		if (!octantExists[octant])
			continue;

		//SB_DATATYPE_CALL(sampleDecimation, (tileDim, octantTile[octant], parentTile, octant, border, octantExists), dataType);

		sampleDecimation< unsigned short>(tileDim, octantTile[octant], parentTile, octant, border, octantExists);
	}
}

void NeoInsightsLDMWriter::getShiftAndHalfTileDim( SbVec2i32& shiftParentOctant, SbVec3i32& halfTileDim, const SbVec3i32& tileDim, int octant, int border) const
{
	SbVec3i32 halfTile2 = tileDim / 2;
	SbVec3i32 halfTile1 = tileDim - halfTile2;
	int tileDim1 = tileDim[0];
	int tileDim2 = tileDim[0] * tileDim[1];

	halfTileDim = halfTile1;

	shiftParentOctant = SbVec2i32(0, 0);
	if (octant & 1)
	{
		shiftParentOctant[0] = halfTile1[0];
		shiftParentOctant[1] = 1;
		halfTileDim[0] = halfTile2[0];
	}

	if (octant & 2)
	{
		shiftParentOctant[0] += tileDim1 * halfTile1[1];
		shiftParentOctant[1] += tileDim1;
		halfTileDim[1] = halfTile2[1];
	}

	if (octant & 4)
	{
		shiftParentOctant[0] += tileDim2 * halfTile1[2];
		shiftParentOctant[1] += tileDim2;
		halfTileDim[2] = halfTile2[2];
	}

	if (border == 0)
		shiftParentOctant[1] = 0;
}

void NeoInsightsLDMWriter::getRatio(SbVec3f &ratio, SbVec3i32 &shiftOctant, SbVec3i32 tileDim, SbVec3i32 halfTileDim, int octant, int border, const int octantExists[8]) const
{
	shiftOctant = SbVec3i32(0, 0, 0);

	// Border == 1 is a special case
	if (border == 1)
	{
		ratio = SbVec3f(2.0, 2.0, 2.0);
		if (octant % 2)
			shiftOctant[0] = border;

		if (octant != 0 && octant != 1 && octant != 4 && octant != 5)
			shiftOctant[1] = border;

		if (octant >= 4)
			shiftOctant[2] = border;

		return;
	}

	int leftBorder = int(border / 2);
	int rightBorder = border - leftBorder;

	// Compute the ratio for X axis
	if (octant % 2)
	{
		ratio[0] = (float)(tileDim[0] - rightBorder) / (float)halfTileDim[0];
		shiftOctant[0] = leftBorder;
	}
	else
	{
		if (octantExists[octant + 1])
			ratio[0] = (float)(tileDim[0] - leftBorder) / (float)halfTileDim[0];
		else
			ratio[0] = (float)tileDim[0] / (float)halfTileDim[0];

		shiftOctant[0] = 0;
	}

	// Compute ratio for Y axis
	if (octant == 0 || octant == 1 || octant == 4 || octant == 5)
	{
		if (octantExists[octant + 2])
			ratio[1] = (float)(tileDim[1] - leftBorder) / (float)halfTileDim[1];
		else
			ratio[1] = (float)tileDim[1] / (float)halfTileDim[1];

		shiftOctant[1] = 0;
	}
	else
	{
		ratio[1] = (float)(tileDim[1] - rightBorder) / (float)halfTileDim[1];
		shiftOctant[1] = leftBorder;
	}

	// Compute ratio for Z axis
	if (octant < 4)
	{
		if (octantExists[octant + 4])
			ratio[2] = (float)(tileDim[2] - leftBorder) / (float)halfTileDim[2];
		else
			ratio[2] = (float)tileDim[2] / (float)halfTileDim[2];

		shiftOctant[2] = 0;
	}
	else
	{
		ratio[2] = (float)(tileDim[2] - rightBorder) / (float)halfTileDim[2];
		shiftOctant[2] = leftBorder;
	}
}



void NeoInsightsLDMWriter::initTiles()
{
	_TopoTree = new imt::volume::VolumeTopoOctree;

	int overlap = 0;

	imt::volume::VolumeTopoOctree::Vec3i tileDim, volumeDim;

	volumeDim._X = _DimX;
	volumeDim._Y = _DimY;
	volumeDim._Z = _DimZ;

	_TopoTree->init( volumeDim , _TileDim , overlap );


}


NeoInsightsLDMWriter::~NeoInsightsLDMWriter()
{

}