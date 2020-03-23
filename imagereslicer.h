#ifndef __IMAGE_RESLICER_H__
#define __IMAGE_RESLICER_H__

#include <VolumeViz/nodes/SoVolumeData.h> 
#include "volumetopooctree.h"

class ImageReslicer
{
	
	
public:
	
	ImageReslicer( SoVolumeData *volumeData );

	void setResliceCorners(double sliceCorners[12] , int w , int h);

	void compute( short *outputBuffer );

protected:

	float grayValue(float vx, float vy, float vz);

	unsigned short absoluteGrayValue(int x, int y, int z);


protected:

	unsigned short *_ImageData;

	SoVolumeData *_VolumeData;

	imt::volume::VolumeTopoOctree *_TopoTree;

	double _PlaneNormal[3], _PlanePos[3];

	double _InitPos[3], _XVec[3], _YVec[3];

	int _SliceWidth, _SliceHeight;

	int _TileDim;

	int _TileSetW, _TileSetH, _TileSetD;

	std::vector< imt::volume::VolumeTopoOctree::TopoOctreeNode* > _HighestResTiles;

	
};



#endif