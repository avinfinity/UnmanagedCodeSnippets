#ifndef __IMT_OVERLAPPEDVOXELMARCHINGCUBES_H__
#define __IMT_OVERLAPPEDVOXELMARCHINGCUBES_H__

#include "volumeinfo.h"

namespace imt{

  namespace volume{
  
    class OverlappedVoxelsMarchingCubes
	{
	
	  public:
	
	  OverlappedVoxelsMarchingCubes();

	  void compute(imt::volume::VolumeInfo& volInfo, std::vector< unsigned char >& volumeLabels, std::vector< int >& isoThresholds);

	  bool isVoxelOnSurface( int x , int y , int z , size_t zStep , size_t yStep , int isoValue, unsigned char *labels );

	  static size_t triangleCount();

	protected:

		void interpolateEdge3(float* p1, float* p2, float val_p1, float val_p2, int levelP1 , int levelP2 ,float* output);


	protected:

		std::vector< int > IsoThresholds_;

	
	};
  
  
  }	  
	
}


#endif