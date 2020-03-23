//#include "Stdafx.h"
#include "ZxoWriter.h"
#include <algorithm>
#include "volumetopooctree.h"
#include <stdio.h>
#include "atomic"
#include "mutex"
#include "omp.h"

#undef min
#undef max


ZxoWriter::ZxoWriter() : _UpdateProgress(defaultProgressReporter)
{
	_Header._Minor = 0;
	_Header._Major = 1;

	//_LoadRawHeader = true;

	_TopoTree = new imt::volume::VolumeTopoOctree();

	_RawFileSkipSize = 1024;

	_ProgressDataStructure = 0;

	_UseImageProviderCallBack = false;

	_WorkBuffer = 0;
	_WorkBufferSize = 0;

	//std::string path = "log.txt";

	//_Logger.open(path, std::ios::out);

	_UseRawImageWholeSlice = true;//false;// 

	//if (!_Logger.is_open())
	//{
	//      
	//}


}

void ZxoWriter::setVolumeInfo(int w, int h, int d, float voxelSizeX, float voxelSizeY, float voxelSizeZ, int bitDepth, int rawHeaderOffset, int dataType)
{
	_Header.volumeSizeX = w;
	_Header.volumeSizeY = h;
	_Header.volumeSizeZ = d;
	_Header.voxelSizeX = voxelSizeX;
	_Header.voxelSizeY = voxelSizeY;
	_Header.voxelSizeZ = voxelSizeZ;
	_Header._TileDim = 256;
	_Header.numBitsPerVoxel = bitDepth;

	_RawFileSkipSize = rawHeaderOffset;

	_DataType = dataType;

	imt::volume::VolumeTopoOctree::Vec3i dim;

	dim._X = _Header.volumeSizeX;
	dim._Y = _Header.volumeSizeY;
	dim._Z = _Header.volumeSizeZ;

	int minDim = INT_MAX;

	int maxDim = 0;

	minDim = std::min(minDim, _Header.volumeSizeX);
	minDim = std::min(minDim, _Header.volumeSizeY);
	minDim = std::min(minDim, _Header.volumeSizeZ);

	maxDim = std::max(maxDim, _Header.volumeSizeX);
	maxDim = std::max(maxDim, _Header.volumeSizeY);
	maxDim = std::max(maxDim, _Header.volumeSizeZ);


	if (minDim > 256)
	{
		_Header._TileDim = 256;
	}
	else if (minDim > 128)
	{
		_Header._TileDim = 128;
	}
	else
	{
		_Header._TileDim = 64;
	}

	if (maxDim <= 512)
	{
		_Header._TileDim = std::min(_Header._TileDim, 128);
	}


	std::cout << " initializing with tile dim : " << _Header._TileDim << std::endl;

	_TopoTree->init(dim, _Header._TileDim, 0);
}


void ZxoWriter::setRawFilePath(std::string filePath, int skipHeaderSize)
{
	_RawFilePath = filePath;
	_SkipHeaderSize = skipHeaderSize;
}


void ZxoWriter::setZXOFilePath(std::string zxoFilePath)
{
	_ZxoFilePath = zxoFilePath;
}



void ZxoWriter::generate()

{

	_Progress = 0;

	imt::volume::VolumeTopoOctree::Vec3i dim;

	dim._X = _Header.volumeSizeX;
	dim._Y = _Header.volumeSizeY;
	dim._Z = _Header.volumeSizeZ;


	std::cout << " num file and tile ids : " << _TopoTree->getNumFiles() << " " << _TopoTree->getNumTiles() << std::endl;

	int numLevels = _TopoTree->getNumLevels();
	int numThreads = omp_get_num_procs();

	int numFileIds = _TopoTree->getNumFiles();

	buildLevelDimensions();

	_LogicalToDiskMap.resize(numFileIds);

	_DiskFileIdCounter = 0;

	//open zxo file for writing
	_ZXOFile = fopen(_ZxoFilePath.c_str(), "wb");

	_RAWFile = fopen(_RawFilePath.c_str(), "rb");

	if (!_ZXOFile)
	{
		std::cout << " file not opened " << std::endl;

		return;
	}
	//write the zxo header here
	int headerBufferSize = 4096;

	unsigned char headerBuffer[4097];

	memcpy(headerBuffer, &_Header, sizeof(_Header));
	//first write the header
	fwrite(headerBuffer, sizeof(unsigned char), headerBufferSize, _ZXOFile);

	if (_UseImageProviderCallBack)
	{
		processHighestResolutionLevelFromImageCallBack();
	}
	else
	{
		if (_UseRawImageWholeSlice)
		{
			processHighestResolutionLevelWithSliceRead();
		}
		else
		{
			processHighestResolutionLevel();
		}

	}

	/*if (_Logger.is_open())
	{
	_Logger << " Start processing levels " << std::endl;
	}*/

	//process each level
	for (int ll = numLevels - 2; ll >= 0; ll--)
	{
		processLevel(ll);
	}

	//write here two information ( 1. min and max values for each tiles.  2. logical to disk map  )

	fwrite(&_MinVal, sizeof(int), 1, _ZXOFile);
	fwrite(&_MaxVal, sizeof(int), 1, _ZXOFile);

	fwrite(_Histogram, sizeof(int64_t), (USHRT_MAX + 1), _ZXOFile);

	int nLevels = _TopoTree->getNumLevels();

	//first write node information( currently only min , max  values of each tile is recorded )
	for (int ll = 0; ll < nLevels; ll++)
	{
		auto nodes = _TopoTree->getFileNodes(ll);

		for (auto node : nodes)
		{
			std::cout << node->_MinVal << " " << node->_MaxVal << std::endl;

			fwrite((char*)&(node->_MinVal), sizeof(int), 1, _ZXOFile);
			fwrite((char*)&(node->_MaxVal), sizeof(int), 1, _ZXOFile);
		}
	}

	int nFileIds = _TopoTree->getNumFiles();

	//write logical to disk map information
	fwrite(_LogicalToDiskMap.data(), sizeof(int), _LogicalToDiskMap.size(), _ZXOFile);

	fclose(_ZXOFile);
	_ZXOFile = 0;

	_UpdateProgress(0.999f, _ProgressDataStructure, this);

	//generateFinalZXO();

	//remove the temporary files

}

void ZxoWriter::processLevel(int level)
{
	// Giving it to utilize maximum number of threads avaliable on system
	//leads to crashes on somesystem where cores are more than > 16.
	//Hence on a safer note it has been sustituted as 4. To be fixed later on.
	int numThreads = 4; // omp_get_num_procs();

	std::vector< FILE* > files(numThreads, 0);
	std::vector< FILE* > dstFiles(numThreads, 0);

	std::vector< unsigned short* > resizedBuffers;
	std::vector< std::string > filePaths(numThreads);

	for (int tt = 0; tt < numThreads; tt++)
	{
		files[tt] = fopen(_ZxoFilePath.c_str(), "rb");

		//fopen_s(&files[tt], _ZxoFilePath.c_str(), "rb");

		if (!files[tt])
		{
			files[tt] = 0;

			/*if (_Logger.is_open())
			{
			_Logger << "failed to open zxo file in read mode" << std::endl;
			}*/
		}
		//else
		//{
		//	/*if (_Logger.is_open())
		//	{
		//		_Logger << " zxo reader opened successfully : "<< tt << std::endl;
		//	}*/
		//}
	}

	/*if (_Logger.is_open())
	{
	_Logger << "processing level : "<< level << std::endl;
	}*/

	int levelW = mLevelDimensions[level]._X;
	int levelH = mLevelDimensions[level]._Y;
	int levelD = mLevelDimensions[level]._Z;

	int nextLevelW = mLevelDimensions[level + 1]._X;
	int nextLevelH = mLevelDimensions[level + 1]._Y;
	int nextLevelD = mLevelDimensions[level + 1]._Z;

	int numBands = levelD / (2 * _Header._TileDim);

	int threadBandRange = numBands / numThreads;
	int incrementOffset = numBands % numThreads;

	bool writeToDisk = true;


	auto currentLevelNodes = _TopoTree->getFileNodes(level);


	//check number of available processors on the system
	int numMaxThreads = omp_get_num_procs();


	//first sort the ndoes
	//we need to sort highest resolution tiles such that they are sorted in order of slices , this will make sure that we can read volume data from disk
	//in chunk of slices. Reading such a way would be both faster and better candidate for out of core implementation.
	std::sort(currentLevelNodes.begin(), currentLevelNodes.end(), [](const imt::volume::VolumeTopoOctree::TopoOctreeNode* item1, const imt::volume::VolumeTopoOctree::TopoOctreeNode* item2)->bool{

		return ((item1->_Position._Z < item2->_Position._Z) ||
			(item1->_Position._Z == item2->_Position._Z && item1->_Position._Y < item2->_Position._Y) ||
			(item1->_Position._Z == item2->_Position._Z && item1->_Position._Y == item2->_Position._Y && item1->_Position._X < item2->_Position._X));
	});


	std::atomic< int > nodeCounter = 0;

	unsigned short *neededBuffer = new unsigned short[_Header._TileDim * _Header._TileDim * _Header._TileDim * numThreads * 9];

	std::mutex writeMutex;

	int tempFileId = 0;

	int diskFileId = _DiskFileIdCounter;

	float totalFileIds = (float)_TopoTree->getNumFiles();

	int numCurrentLevelNodes = (int)currentLevelNodes.size();

	_ZxoFileHeaderSkipSize = 4096;

	//if ( _Logger.is_open() )
	//{
	//	_Logger << "processing level : " << level << std::endl;
	//}

#pragma omp parallel num_threads(numThreads)
	{
		int threadId = omp_get_thread_num();

		int bandWidth = threadBandRange;

		if (threadId <= incrementOffset)
		{
			bandWidth++;
		}

		unsigned short* currentTile = neededBuffer + _Header._TileDim * _Header._TileDim * _Header._TileDim * threadId * 9;

		unsigned short* childrenTiles[8];

		while (1)
		{
			int nodeLoc = nodeCounter++;

			if (nodeLoc >= numCurrentLevelNodes)
			{
				break;
			}

			auto node = currentLevelNodes[nodeLoc];

			int fId = node->_NodeId;

			memset(currentTile, 0, _Header._TileDim * _Header._TileDim * _Header._TileDim * sizeof(unsigned short));

			void *octantTiles[8];
			int octantExists[8];

			for (int ii = 0; ii < 8; ii++)
			{
				childrenTiles[ii] = currentTile + _Header._TileDim * _Header._TileDim * _Header._TileDim * (ii + 1);

				if (node->_Chidren[ii])
				{
					octantTiles[ii] = childrenTiles[ii];
					octantExists[ii] = 1;

					size_t cFId = node->_Chidren[ii]->_NodeId;

					size_t diskFId = _LogicalToDiskMap[cFId];

					size_t filePos = (size_t)_ZxoFileHeaderSkipSize + (size_t)_Header._TileDim * (size_t)_Header._TileDim * (size_t)_Header._TileDim * diskFId * sizeof(unsigned short);

					//_fseeki64( files[threadId], _Header._TileDim * _Header._TileDim * _Header._TileDim * _TileTempInfo[cFId]._TilePos * sizeof(unsigned short), SEEK_SET );
					_fseeki64(files[threadId], filePos, SEEK_SET);

					//read needed tiles into childrenTiles from disk
					fread(octantTiles[ii], sizeof(unsigned short), _Header._TileDim * _Header._TileDim * _Header._TileDim, files[threadId]);
				}
				else
				{
					octantExists[ii] = 0;
					octantTiles[ii] = 0;
				}
			}

			SbVec3i32 tileDim;

			tileDim[0] = _Header._TileDim;
			tileDim[1] = _Header._TileDim;
			tileDim[2] = _Header._TileDim;

			//this is the step which actually samples the low resolution tile from it's 8 high resolution children tiles
			sampleTile(tileDim, 1, 0, octantTiles, octantExists, currentTile); //SbDataType::UNSIGNED_SHORT

			writeMutex.lock();

			//if (_Logger.is_open())
			//{
			//	_Logger << " tile id : " << fId << std::endl;
			//}

			computeMinMax(currentTile, node->_MinVal, node->_MaxVal);

			//note that fwrite is supposed to be a non blocking call as it uses internally a buffering system , so this step should be fast
			//fwrite(currentTile, sizeof(unsigned short), _Header._TileDim * _Header._TileDim * _Header._TileDim, tempFile);
			size_t numElems = fwrite(currentTile, sizeof(unsigned short), _Header._TileDim * _Header._TileDim * _Header._TileDim, _ZXOFile);

			/*if (_Logger.is_open())
			{
			_Logger << " num written tiles : " << numElems << std::endl;
			}*/
			//in case if we want to rearrange the generated tiles
			//_TileTempInfo[fId]._TilePos = tempFileId;

			//maintaining a logical to disk map is more efficient for generating the zxo files as it only require single pass read of raw data.
			_LogicalToDiskMap[fId] = diskFileId;


			if ((diskFileId / totalFileIds - _Progress) > 0.01)
			{
				_Progress = diskFileId / totalFileIds;

				_UpdateProgress(_Progress, _ProgressDataStructure, this);
			}

			tempFileId++;
			diskFileId++;



			writeMutex.unlock();
		}
	}

	_DiskFileIdCounter = diskFileId;

	//fclose(tempFile);

	for (int tt = 0; tt < numThreads; tt++)
	{
		fclose(files[tt]);
	}

	delete[] neededBuffer;

}



void ZxoWriter::processHighestResolutionLevel()
{

	//first get the extended dimensions
	int extendedW = mLevelDimensions[0]._X;
	int extendedH = mLevelDimensions[0]._Y;
	int extendedD = mLevelDimensions[0]._Z;

	int numLevels = _TopoTree->getNumLevels();

	int level = numLevels - 1;

	auto highestResolutionNodes = _TopoTree->getFileNodes(numLevels - 1);


	//check number of available processors on the system

	// Giving it to utilize maximum number of threads avaliable on system
	//leads to crashes on somesystem where cores are more than > 16.
	//Hence on a safer note it has been sustituted as 4. To be fixed later on.
	int numMaxThreads = 4; // omp_get_num_procs();

	float totalFileIds = (float)_TopoTree->getNumFiles();


	//first sort the nodes
	//we need to sort highest resolution tiles such that they are sorted in order of slices , this will make sure that we can read volume data from disk
	//in chunk of slices. Reading such a way would be both faster and better candidate for out of core implementation.
	std::sort(highestResolutionNodes.begin(), highestResolutionNodes.end(), [](const imt::volume::VolumeTopoOctree::TopoOctreeNode* item1, const imt::volume::VolumeTopoOctree::TopoOctreeNode* item2)->bool{

		return ((item1->_Position._Z < item2->_Position._Z) ||
			(item1->_Position._Z == item2->_Position._Z && item1->_Position._Y < item2->_Position._Y) ||
			(item1->_Position._Z == item2->_Position._Z && item1->_Position._Y == item2->_Position._Y && item1->_Position._X < item2->_Position._X));
	});

	int numWrittenNodes = 0;

	int diskFileIdCounter = _DiskFileIdCounter;

	for (int ii = 0; ii < (USHRT_MAX + 1); ii++)
	{
		_Histogram[ii] = 0;
	}

	int zT = _Header.volumeSizeZ % _Header._TileDim == 0 ? _Header.volumeSizeZ / _Header._TileDim : _Header.volumeSizeZ / _Header._TileDim + 1;
	int yT = _Header.volumeSizeY % _Header._TileDim == 0 ? _Header.volumeSizeY / _Header._TileDim : _Header.volumeSizeY / _Header._TileDim + 1;
	int xT = _Header.volumeSizeX % _Header._TileDim == 0 ? _Header.volumeSizeX / _Header._TileDim : _Header.volumeSizeX / _Header._TileDim + 1;

	size_t bufferSize = _Header._TileDim * _Header._TileDim * _Header.volumeSizeX;

	std::vector< FILE* > filePtrs(numMaxThreads, 0);

	for (int ff = 0; ff < filePtrs.size(); ff++)
	{
		filePtrs[ff] = fopen(_RawFilePath.c_str(), "rb");

		//fopen_s(&filePtrs[ff] , _RawFilePath.c_str(), "rb");
	}

	size_t sliceSize = _Header.volumeSizeX * _Header.volumeSizeY;

	unsigned short *buffer = new unsigned short[_Header.volumeSizeX * _Header._TileDim * _Header._TileDim];

	unsigned short *tileBuffer = new unsigned short[_Header._TileDim * _Header._TileDim * _Header._TileDim];

	_MinVal = UINT16_MAX;
	_MaxVal = 0;

	for (int tz = 0; tz < zT; tz++)
		for (int ty = 0; ty < yT; ty++)
		{
			std::atomic< int > counter = 0;

			size_t filePos = (tz * _Header._TileDim * sliceSize + ty * _Header._TileDim * _Header.volumeSizeX) * sizeof(unsigned short) + _RawFileSkipSize;

			memset(buffer, 0, _Header.volumeSizeX * _Header._TileDim * _Header._TileDim * sizeof(unsigned short));

			int zLen = _Header._TileDim;
			int yLen = _Header._TileDim;
			int xLen = _Header._TileDim;

			if (tz == zT - 1)
			{
				zLen = _Header.volumeSizeZ - (zT - 1) * _Header._TileDim;
			}

			if (ty == yT - 1)
			{
				yLen = _Header.volumeSizeY - (yT - 1) * _Header._TileDim;
			}

#pragma omp parallel num_threads( numMaxThreads )
			 {
				 int threadId = omp_get_thread_num();

				 FILE *file = filePtrs[threadId];

				 while (1)
				 {
					 int idx = counter++;

					 if (idx >= zLen)
						 break;

					 _fseeki64(file, filePos + (size_t)idx * (size_t)sliceSize * sizeof(unsigned short), SEEK_SET);

					 //reach to appropriate buffer position
					 unsigned short *buff = buffer + (size_t)idx * (size_t)_Header.volumeSizeX * (size_t)_Header._TileDim;

					 fread(buff, sizeof(unsigned short), _Header.volumeSizeX * _Header._TileDim, file);

#pragma omp critical
					 {
						 for (int ss = 0; ss < _Header.volumeSizeX * _Header._TileDim; ss++)
						 {
							 _Histogram[buff[ss]]++;
							 _MinVal = std::min(_MinVal, (int)buff[ss]);
							 _MaxVal = std::max(_MaxVal, (int)buff[ss]);
						 }
					 }


				 }

			 }



			 //copy the read buffer to tiles and write them to disk
			 for (int tx = 0; tx < xT; tx++)
			 {
				 unsigned short *tbf = tileBuffer;

				 for (int zz = 0; zz < _Header._TileDim; zz++)
					 for (int yy = 0; yy < _Header._TileDim; yy++)
					 {
						 unsigned short *buf = buffer + zz * _Header.volumeSizeX * _Header._TileDim + yy * _Header.volumeSizeX + tx * _Header._TileDim;

						 if (tx == xT - 1)
						 {
							 xLen = _Header.volumeSizeX - (xT - 1) * _Header._TileDim;
						 }

						 memcpy(tbf, buf, xLen * sizeof(unsigned short));

						 tbf += _Header._TileDim;
					 }

				 auto node = highestResolutionNodes[numWrittenNodes];

				 computeMinMax(tileBuffer, node->_MinVal, node->_MaxVal);

				 _LogicalToDiskMap[node->_NodeId] = diskFileIdCounter;

				 //if (_Logger.is_open())
				 //{
				 // _Logger << " Level :  "<<level<<" , tile id : " << node->_NodeId << std::endl;
				 //}

				 size_t numElems = fwrite(tileBuffer, sizeof(unsigned short), _Header._TileDim * _Header._TileDim * _Header._TileDim, _ZXOFile);

				 /* if (_Logger.is_open())
				 {
				 _Logger << " num written tiles : " << numElems << std::endl;
				 }*/

				 numWrittenNodes++;

				 diskFileIdCounter++;

				 if ((diskFileIdCounter / totalFileIds - _Progress) > 0.01)
				 {
					 _Progress = diskFileIdCounter / totalFileIds;

					 _UpdateProgress(_Progress, _ProgressDataStructure, this);
				 }
			 }

		}

	//fclose(highestResTilesFile);
	_DiskFileIdCounter = diskFileIdCounter;

	for (auto file : filePtrs)
	{
		fclose(file);
	}

	delete[] tileBuffer;
	delete[] buffer;
	//}

	/*	if (_Logger.is_open())
	{
	_Logger << " processing highest resolution tiles finished " << std::endl;
	}*/

}


void ZxoWriter::processHighestResolutionLevelFromImageCallBack()
{
	//first get the extended dimensions
	int extendedW = mLevelDimensions[0]._X;
	int extendedH = mLevelDimensions[0]._Y;
	int extendedD = mLevelDimensions[0]._Z;

	int numLevels = _TopoTree->getNumLevels();

	int level = numLevels - 1;
	auto highestResolutionNodes = _TopoTree->getFileNodes(numLevels - 1);


	//check number of available processors on the system
	int numMaxThreads = omp_get_num_procs();

	float totalFileIds = (float)_TopoTree->getNumFiles();


	//first sort the nodes
	//we need to sort highest resolution tiles such that they are sorted in order of slices , this will make sure that we can read volume data from disk
	//in chunk of slices. Reading such a way would be both faster and better candidate for out of core implementation.
	std::sort(highestResolutionNodes.begin(), highestResolutionNodes.end(), [](const imt::volume::VolumeTopoOctree::TopoOctreeNode* item1, const imt::volume::VolumeTopoOctree::TopoOctreeNode* item2)->bool{

		return ((item1->_Position._Z < item2->_Position._Z) ||
			(item1->_Position._Z == item2->_Position._Z && item1->_Position._Y < item2->_Position._Y) ||
			(item1->_Position._Z == item2->_Position._Z && item1->_Position._Y == item2->_Position._Y && item1->_Position._X < item2->_Position._X));
	});

	int numWrittenNodes = 0;

	int diskFileIdCounter = _DiskFileIdCounter;

	for (int ii = 0; ii < (USHRT_MAX + 1); ii++)
	{
		_Histogram[ii] = 0;
	}

	size_t intermediateSize = _Header._TileDim * _Header.volumeSizeX * _Header.volumeSizeY;

	unsigned short *intermediateBuffer = new unsigned short[intermediateSize];


	int zT = _Header.volumeSizeZ % _Header._TileDim == 0 ? _Header.volumeSizeZ / _Header._TileDim : _Header.volumeSizeZ / _Header._TileDim + 1;
	int yT = _Header.volumeSizeY % _Header._TileDim == 0 ? _Header.volumeSizeY / _Header._TileDim : _Header.volumeSizeY / _Header._TileDim + 1;
	int xT = _Header.volumeSizeX % _Header._TileDim == 0 ? _Header.volumeSizeX / _Header._TileDim : _Header.volumeSizeX / _Header._TileDim + 1;

	size_t bufferSize = _Header._TileDim * _Header._TileDim * _Header.volumeSizeX;

	size_t sliceSize = _Header.volumeSizeX * _Header.volumeSizeY;

	unsigned short *buffer = new unsigned short[_Header.volumeSizeX * _Header._TileDim * _Header._TileDim];

	unsigned short *tileBuffer = new unsigned short[_Header._TileDim * _Header._TileDim * _Header._TileDim];



	_MinVal = UINT16_MAX;
	_MaxVal = 0;

	for (int tz = 0; tz < zT; tz++)
	{
		int zLen = _Header._TileDim;

		if (tz == zT - 1)
		{
			zLen = _Header.volumeSizeZ - (zT - 1) * _Header._TileDim;
		}

		unsigned short *tbf2 = intermediateBuffer;

		for (int zz = tz * _Header._TileDim; zz < tz * _Header._TileDim + zLen; zz++)
		{
			getImage(zz, tbf2);

			tbf2 += _Header.volumeSizeX * _Header.volumeSizeY;
		}

		for (int ty = 0; ty < yT; ty++)
		{
			size_t filePos = (tz * _Header._TileDim * sliceSize + ty * _Header._TileDim * _Header.volumeSizeX) * sizeof(unsigned short) + _RawFileSkipSize;

			//memset(buffer, 0, _Header.volumeSizeX * _Header._TileDim * _Header._TileDim * sizeof(unsigned short));

			int yInit = ty * _Header._TileDim;

			int yLen = _Header._TileDim;
			int xLen = _Header._TileDim;

			if (ty == yT - 1)
			{
				yLen = _Header.volumeSizeY - (yT - 1) * _Header._TileDim;
			}

			//copy the read buffer to tiles and write them to disk
			for (int tx = 0; tx < xT; tx++)
			{
				unsigned short *tbf = tileBuffer;

				memset(tileBuffer, 0, _Header._TileDim * _Header._TileDim * _Header._TileDim);

				for (int zz = 0; zz < _Header._TileDim; zz++)
				{
					//reinitialize the tbf pointer as Y dim may be less than tile dim around boundary tiles
					tbf = tileBuffer + zz * _Header._TileDim * _Header._TileDim;

					for (int yy = yInit; yy < yInit + yLen; yy++)
					{
						int actualY = yInit + yy;

						unsigned short *buf = intermediateBuffer + zz * _Header.volumeSizeX * _Header.volumeSizeY + yy * _Header.volumeSizeX + tx * _Header._TileDim;

						if (tx == xT - 1)
						{
							xLen = _Header.volumeSizeX - (xT - 1) * _Header._TileDim;
						}

						memcpy(tbf, buf, xLen * sizeof(unsigned short));

						for (int ii = 0; ii < xLen; ii++)
						{
							_Histogram[tbf[ii]]++;
						}

						tbf += _Header._TileDim;
					}
				}

				auto node = highestResolutionNodes[numWrittenNodes];

				computeMinMax(tileBuffer, node->_MinVal, node->_MaxVal);

				_LogicalToDiskMap[node->_NodeId] = diskFileIdCounter;

				fwrite(tileBuffer, sizeof(unsigned short), _Header._TileDim * _Header._TileDim * _Header._TileDim, _ZXOFile);

				numWrittenNodes++;

				diskFileIdCounter++;

				if ((diskFileIdCounter / totalFileIds - _Progress) > 0.01)
				{
					_Progress = diskFileIdCounter / totalFileIds;

					_UpdateProgress(_Progress, _ProgressDataStructure, this);
				}
			}

		}

	}

	_DiskFileIdCounter = diskFileIdCounter;

	delete[] tileBuffer;
	delete[] intermediateBuffer;
	delete[] buffer;

}


void ZxoWriter::processHighestResolutionLevelWithSliceRead()
{
	//first get the extended dimensions
	int extendedW = mLevelDimensions[0]._X;
	int extendedH = mLevelDimensions[0]._Y;
	int extendedD = mLevelDimensions[0]._Z;

	int numLevels = _TopoTree->getNumLevels();

	int level = numLevels - 1;
	auto highestResolutionNodes = _TopoTree->getFileNodes(numLevels - 1);


	//check number of available processors on the system
	int numMaxThreads = omp_get_num_procs();

	float totalFileIds = (float)_TopoTree->getNumFiles();


	//first sort the nodes
	//we need to sort highest resolution tiles such that they are sorted in order of slices , this will make sure that we can read volume data from disk
	//in chunk of slices. Reading such a way would be both faster and better candidate for out of core implementation.
	std::sort(highestResolutionNodes.begin(), highestResolutionNodes.end(), [](const imt::volume::VolumeTopoOctree::TopoOctreeNode* item1, const imt::volume::VolumeTopoOctree::TopoOctreeNode* item2)->bool{

		return ((item1->_Position._Z < item2->_Position._Z) ||
			(item1->_Position._Z == item2->_Position._Z && item1->_Position._Y < item2->_Position._Y) ||
			(item1->_Position._Z == item2->_Position._Z && item1->_Position._Y == item2->_Position._Y && item1->_Position._X < item2->_Position._X));
	});

	int numWrittenNodes = 0;

	int diskFileIdCounter = _DiskFileIdCounter;

	for (int ii = 0; ii < (USHRT_MAX + 1); ii++)
	{
		_Histogram[ii] = 0;
	}

	size_t intermediateSize = _Header._TileDim * _Header.volumeSizeX * _Header.volumeSizeY;

	unsigned short *intermediateBuffer = new unsigned short[intermediateSize];


	int zT = _Header.volumeSizeZ % _Header._TileDim == 0 ? _Header.volumeSizeZ / _Header._TileDim : _Header.volumeSizeZ / _Header._TileDim + 1;
	int yT = _Header.volumeSizeY % _Header._TileDim == 0 ? _Header.volumeSizeY / _Header._TileDim : _Header.volumeSizeY / _Header._TileDim + 1;
	int xT = _Header.volumeSizeX % _Header._TileDim == 0 ? _Header.volumeSizeX / _Header._TileDim : _Header.volumeSizeX / _Header._TileDim + 1;

	size_t bufferSize = _Header._TileDim * _Header._TileDim * _Header.volumeSizeX;

	size_t sliceSize = _Header.volumeSizeX * _Header.volumeSizeY;

	unsigned short *tileBuffer = new unsigned short[_Header._TileDim * _Header._TileDim * _Header._TileDim];

	_MinVal = UINT16_MAX;
	_MaxVal = 0;

	for (int tz = 0; tz < zT; tz++)
	{
		int zLen = _Header._TileDim;

		if (tz == zT - 1)
		{
			zLen = _Header.volumeSizeZ - (zT - 1) * _Header._TileDim;

			memset(intermediateBuffer, 0, intermediateSize * sizeof(unsigned short));
		}

		unsigned short *tbf2 = intermediateBuffer;

		for (int zz = tz * _Header._TileDim; zz < tz * _Header._TileDim + zLen; zz++)
		{
			getImageRaw(zz, tbf2);

			tbf2 += _Header.volumeSizeX * _Header.volumeSizeY;
		}

		for (int ty = 0; ty < yT; ty++)
		{

			int yInit = ty * _Header._TileDim;

			int yLen = _Header._TileDim;
			int xLen = _Header._TileDim;

			if (ty == yT - 1)
			{
				yLen = _Header.volumeSizeY - (yT - 1) * _Header._TileDim;
			}

			//copy the read buffer to tiles and write them to disk
			for (int tx = 0; tx < xT; tx++)
			{
				unsigned short *tbf = tileBuffer;

				memset(tileBuffer, 0, _Header._TileDim * _Header._TileDim * _Header._TileDim * sizeof(unsigned short));

				for (int zz = 0; zz < zLen; zz++)
				{

					//reinitialize the tbf pointer as Y dim may be less than tile dim around boundary tiles
					tbf = tileBuffer + zz * _Header._TileDim * _Header._TileDim;

					for (int yy = yInit; yy < yInit + yLen; yy++)
					{
						int actualY = yInit + yy;

						unsigned short *buf = intermediateBuffer + zz * _Header.volumeSizeX * _Header.volumeSizeY + yy * _Header.volumeSizeX + tx * _Header._TileDim;

						if (tx == xT - 1)
						{
							xLen = _Header.volumeSizeX - (xT - 1) * _Header._TileDim;
						}

						memcpy(tbf, buf, xLen * sizeof(unsigned short));

						for (int ii = 0; ii < xLen; ii++)
						{
							_Histogram[tbf[ii]]++;
						}

						tbf += _Header._TileDim;
					}
				}

				auto node = highestResolutionNodes[numWrittenNodes];

				computeMinMax(tileBuffer, node->_MinVal, node->_MaxVal);

				_LogicalToDiskMap[node->_NodeId] = diskFileIdCounter;


				computeMinMax(tileBuffer, node->_MinVal, node->_MaxVal);

				fwrite(tileBuffer, sizeof(unsigned short), _Header._TileDim * _Header._TileDim * _Header._TileDim, _ZXOFile);

				numWrittenNodes++;

				diskFileIdCounter++;

				if ((diskFileIdCounter / totalFileIds - _Progress) > 0.01)
				{
					_Progress = diskFileIdCounter / totalFileIds;

					_UpdateProgress(_Progress, _ProgressDataStructure, this);
				}
			}

		}

	}

	_DiskFileIdCounter = diskFileIdCounter;

	delete[] tileBuffer;
	delete[] intermediateBuffer;
}


void ZxoWriter::computeMinMax( unsigned short *tile, unsigned int& minVal, unsigned int& maxVal )
{
	size_t tileSize = _Header._TileDim * _Header._TileDim * _Header._TileDim;

	minVal = USHRT_MAX;
	maxVal = 0;

	for ( size_t tt = 0; tt < tileSize; tt++ )
	{
		minVal = std::min(minVal, (unsigned int)tile[tt]);
		maxVal = std::max(maxVal, (unsigned int)tile[tt]);
	}

}

void ZxoWriter::sampleTile(const SbVec3i32& tileDim, int dataType, int border, const void* const octantTile[8], const int octantExists[8], void *parentTile)
{
	for (int octant = 0; octant < 8; octant++)
	{
		if (!octantExists[octant])
			continue;

		sampleDecimation< unsigned short>(tileDim, octantTile[octant], parentTile, octant, border, octantExists);
	}
}

void ZxoWriter::getShiftAndHalfTileDim(SbVec2i32& shiftParentOctant, SbVec3i32& halfTileDim, const SbVec3i32& tileDim, int octant, int border) const
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

void ZxoWriter::getRatio(SbVec3f &ratio, SbVec3i32 &shiftOctant, SbVec3i32 tileDim, SbVec3i32 halfTileDim, int octant, int border, const int octantExists[8]) const
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





void ZxoWriter::buildLevelDimensions()
{

	int numLevels = _TopoTree->getNumLevels();

	mLevelDimensions.resize(numLevels);

	for (int ll = 0; ll < numLevels; ll++)
	{
		if (ll == 0)
		{
			mLevelDimensions[ll]._X = _Header.volumeSizeX % _Header._TileDim == 0 ? _Header.volumeSizeX : (_Header.volumeSizeX / _Header._TileDim + 1) * _Header._TileDim;
			mLevelDimensions[ll]._Y = _Header.volumeSizeY % _Header._TileDim == 0 ? _Header.volumeSizeY : (_Header.volumeSizeY / _Header._TileDim + 1) * _Header._TileDim;
			mLevelDimensions[ll]._Z = _Header.volumeSizeZ % _Header._TileDim == 0 ? _Header.volumeSizeZ : (_Header.volumeSizeZ / _Header._TileDim + 1) * _Header._TileDim;
		}
		else
		{
			int w = mLevelDimensions[ll - 1]._X / 2;
			int h = mLevelDimensions[ll - 1]._Y / 2;
			int d = mLevelDimensions[ll - 1]._Z / 2;

			mLevelDimensions[ll]._X = w  % _Header._TileDim == 0 ? w : (w / _Header._TileDim + 1) * _Header._TileDim;
			mLevelDimensions[ll]._Y = h % _Header._TileDim == 0 ? h : (h / _Header._TileDim + 1) * _Header._TileDim;
			mLevelDimensions[ll]._Z = d % _Header._TileDim == 0 ? d : (d / _Header._TileDim + 1) * _Header._TileDim;
		}
	}
}


void ZxoWriter::setProgressCallback(ProgressFunc& func, void *progressDataStructure)
{

	_ProgressDataStructure = progressDataStructure;
	_UpdateProgress = func;
}



void ZxoWriter::setImageProviderCallback(ImageProviderFunc& func, void *imageProviderData)
{
	_ImageProviderFunc = func;

	_ImageProviderDataStruct = imageProviderData;

	_UseImageProviderCallBack = true;


}


void ZxoWriter::setInputDataType(int inputDataType)
{
	_InputDataType = inputDataType;
}


void ZxoWriter::getImage(int zz, unsigned short* data)
{
	if (_InputDataType == UNSIGNED_SHORT)
	{
		_ImageProviderFunc(zz, _ImageProviderDataStruct, data);
	}
	else if (_InputDataType == FLOAT)
	{
		size_t imageSize = _Header.volumeSizeX * _Header.volumeSizeY;

		if (imageSize != _WorkBufferSize)
		{
			if (_WorkBuffer)
				delete[] _WorkBuffer;

			_WorkBuffer = new float[imageSize];

			_WorkBufferSize = imageSize * sizeof(float);

			_ImageProviderFunc(zz, _ImageProviderDataStruct, _WorkBuffer);

			float *tempData = (float*)_WorkBuffer;

			unsigned short *uData = (unsigned short*)data;

			for (size_t ii = 0; ii < imageSize; ii++)
			{
				uData[ii] = (unsigned short)tempData[ii];
			}

		}
	}



}


void ZxoWriter::getImageRaw(int zz, unsigned short* data)
{

	size_t sliceSize = _Header.volumeSizeX * _Header.volumeSizeY;

	size_t filePos = (size_t)zz * sliceSize * sizeof(unsigned short) + _RawFileSkipSize;

	_fseeki64(_RAWFile, filePos, SEEK_SET);

	fread(data, sizeof(unsigned short), sliceSize, _RAWFile);

}

void defaultProgressReporter(const float& progress, void *dataStructure, void* zxoWriter)
{
	//empty function
}

void defaultImageProvider(const int&, void*, void*)
{
	//empty function
}


ZxoWriter::~ZxoWriter()
{
	if (_TopoTree)
	{
		delete _TopoTree;
		_TopoTree = 0;
	}
	if (_WorkBuffer)
	{
		delete[] _WorkBuffer;

		_WorkBuffer = 0;
	}
	if (_ZXOFile)
	{
		fclose(_ZXOFile);
		_ZXOFile = 0;
	}
	if (_RAWFile)
	{
		fclose(_RAWFile);
		_RAWFile = 0;
	}

	/*if (_Logger.is_open())
	{
	_Logger.close();
	}*/
}