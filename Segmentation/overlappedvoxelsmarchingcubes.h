#ifndef __IMT_OVERLAPPEDVOXELMARCHINGCUBES_H__
#define __IMT_OVERLAPPEDVOXELMARCHINGCUBES_H__

#include "volumeinfo.h"

namespace imt{

  namespace volume{
  
    class OverlappedVoxelsMarchingCubes
	{
	
	  public:
	
	  OverlappedVoxelsMarchingCubes();

	  //void compute(imt::volume::VolumeInfo& volInfo, std::vector< unsigned char >& volumeLabels, std::vector< int >& isoThresholds);
	  void compute(imt::volume::VolumeInfo& volInfo, unsigned char *Segmentation,unsigned int label ,unsigned int num_labels, std::vector<unsigned short>& peaks, 
		  std::vector<Eigen::Vector3f>& vertices, std::vector<unsigned int>& indices);
	  //I changed the defination of the function little bit, added Segmented volume as input as we need final segmentation
	  //label - Corresponds to the material ID of the material for which Iso surface must be extracted
	  //num_labels - Tells the number of the materials in the volume, we assume that the material IDs range from 0 to num_labels-1
	  //peaks - Is an vector (length=num_labels) which contains the peak values of materials

	  void computeAll(imt::volume::VolumeInfo& volInfo, unsigned char* Segmentation, unsigned int num_labels, std::vector<unsigned short>& peaks,
		  std::vector<Eigen::Vector3f>& vertices, std::vector<unsigned int>& indices);

	  bool isVoxelOnSurface( int x , int y , int z , size_t zStep , size_t yStep , int isoValue, unsigned char *labels );


	protected:

		void interpolateEdge3(Eigen::Vector3f& p1, Eigen::Vector3f& p2, float value1, float value2, float iso, Eigen::Vector3f& vertex );


	protected:

		std::vector<unsigned short> Peaks_;
	
	};
  
  
  }	  
	
}


#endif