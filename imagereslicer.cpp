#include "imagereslicer.h"

#define VTK_RESLICE_NEAREST 1

void cross3(double* vec1, double *vec2, double *vec3)
{
	vec3[0] = vec1[1] * vec2[2] - vec1[2] * vec2[1];
	vec3[0] = -vec1[0] * vec2[2] + vec1[2] * vec2[0];
	vec3[0] = vec1[0] * vec2[1] - vec1[1] * vec2[0];
}

ImageReslicer::ImageReslicer( SoVolumeData *volumeData ) : _VolumeData( volumeData )
{
	
}

void ImageReslicer::setResliceCorners( double sliceCorners[12] , int w , int h )
{
	//_InitPos[3], _XVec[3], _YVec[3]

	_InitPos[0] = sliceCorners[0];
	_InitPos[1] = sliceCorners[1];
	_InitPos[2] = sliceCorners[2];

	_XVec[0] = sliceCorners[3] - sliceCorners[0];
	_XVec[1] = sliceCorners[4] - sliceCorners[1];
	_XVec[2] = sliceCorners[5] - sliceCorners[2];

	_YVec[0] = sliceCorners[6] - sliceCorners[0];
	_YVec[1] = sliceCorners[7] - sliceCorners[1];
	_YVec[2] = sliceCorners[8] - sliceCorners[2];

	//fist compute the slice normal and position 

	cross3(_XVec, _YVec, _PlaneNormal);

	_PlanePos[0] = _InitPos[0];
	_PlanePos[1] = _InitPos[1];
	_PlanePos[2] = _InitPos[2];
}

void ImageReslicer::compute( short *outputBuffer )
{
	int numLevels = _TopoTree->getNumLevels();

	auto leafNodes =  _TopoTree->getFileNodes(numLevels - 1);

	int numLeafNodes = leafNodes.size();

	float step = 0;

	auto& ldmDataAccess = _VolumeData->getLdmDataAccess();

	//first find all the needed tiles
	for (int ll = 0; ll < numLeafNodes; ll++)
	{
		auto node = leafNodes[ll];

		float x = node->_Position._X;
		float y = node->_Position._Y;
		float z = node->_Position._Z;

		int above = 0, below = 0;

		for ( int ii = 0; ii < 2; ii++ ) 
			for ( int jj = 0; jj < 2; jj++ )
				for ( int kk = 0; kk < 2; kk++ )
				{
					float xx = x + ii * step;
					float yy = y + jj * step;
					float zz = z + kk * step;

					double val = (xx - _PlanePos[0]) * _PlaneNormal[0] + (yy - _PlanePos[1]) * _PlaneNormal[1] + (zz - _PlanePos[2]) * _PlaneNormal[2];

					if ( val > 0 )
						above++;

					if ( val < 0 )
						below++;
				}

		//now request those tiles
		if (above > 0 && below > 0)
		{
			SbVec3s pos;

			pos[0] = node->_Position._X;
			pos[1] = node->_Position._Y;
			pos[2] = node->_Position._Z;

			//valid tile
			//request the tile from ldm access 
			//Note that , the very moment we call the getData method , the tile gets locked
			//so it is good to call releaseData on those tiles after using them , this will make
			//sure that cpu memory do not increase unneccessarily
			auto dataInfo = ldmDataAccess.getData(0, pos);

			node->_Data = (unsigned short*)dataInfo.tileData;

		}

	}

	//finally compute the intersected image
	for ( int yy = 0; yy < _SliceHeight; yy++ )
		for ( int xx = 0; xx < _SliceWidth; xx++ )
		{
			//compute corresponding 3d coordinate for the pixel
			float vx, vy, vz;

			vx = _InitPos[0] + xx * _XVec[0] + yy * _YVec[0];
			vy = _InitPos[1] + xx * _XVec[1] + yy * _YVec[1];
			vz = _InitPos[2] + xx * _XVec[2] + yy * _YVec[2];

			//we use trilinear interpolation to compute the gray value at the computed coordinates

			float gv = grayValue(vx, vy, vz);

		}

}


float ImageReslicer::grayValue(float vx, float vy, float vz)
{
	float interpolatedValue = 0;

	int vxf = floor(vx);
	int vyf = floor(vy);
	int vzf = floor(vz);

	float tx = vx - vxf, ty = vy - vyf, tz = vz - vzf;

	unsigned short c000 = absoluteGrayValue(vxf, vyf, vzf);
	unsigned short c100 = absoluteGrayValue(vxf + 1, vyf, vzf);
	unsigned short c010 = absoluteGrayValue(vxf, vyf + 1, vzf);
	unsigned short c110 = absoluteGrayValue(vxf + 1, vyf + 1, vzf);
	unsigned short c001 = absoluteGrayValue(vxf, vyf, vzf + 1);
	unsigned short c101 = absoluteGrayValue(vxf + 1, vyf, vzf + 1);
	unsigned short c011 = absoluteGrayValue(vxf, vyf + 1, vzf + 1);
	unsigned short c111 = absoluteGrayValue(vxf + 1, vyf + 1, vzf + 1);

	interpolatedValue = c000 * (1 - tx) * (1 - ty)*(1 - tz) +
		c100 * tx * (1 - ty) * (1 - tz) +
		c010 * (1 - tx) * ty * (1 - tz) +
		c110 * tx * ty * (1 - tz) +
		c001 * (1 - tx) * (1 - ty) * tz +
		c101 * tx * (1 - ty) * tz +
		c011 * (1 - tx) * ty * tz +
		c111 * tx * ty * tz;
	
	return interpolatedValue;
}


unsigned short ImageReslicer::absoluteGrayValue(int x, int y, int z)
{
	unsigned short gVal = 0;

	int xId = x / _TileDim;
	int yId = y / _TileDim;
	int zId = z / _TileDim;

	if ( xId >= _TileSetW || yId >= _TileSetH || zId >= _TileSetD )
	{
		return 0;
	}

	int lTid = zId * _TileSetW * _TileSetH + yId * _TileSetW + xId;

	int tx = x - xId * _TileDim;
	int ty = y - yId * _TileDim;
	int tz = z - zId * _TileDim;

	int tVid = tz * _TileDim * _TileDim + ty * _TileDim + tx;

	unsigned short* buff = _HighestResTiles[lTid]->_Data;

	return buff[tVid];
}


