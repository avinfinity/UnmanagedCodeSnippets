#ifndef __IMT_VOLUME_CUDA_CTPROFILEEVALUATIONCUDA_H__
#define __IMT_VOLUME_CUDA_CTPROFILEEVALUATIONCUDA_H__

#include "volumeinfo.h"
#include "eigenincludes.h"
#include "ctprofileevaluationcpu.h"
#include <vector> 

namespace imt {

	namespace volume {
		
		namespace cuda {

			class CTPointCloudEvaluationCUDA 
			{

			public:

				CTPointCloudEvaluationCUDA(imt::volume::VolumeInfo& volume, std::vector<Eigen::Vector3f>& points, 
					std::vector<Eigen::Vector3f>& normals , std::vector<unsigned int>& surfaceIndices);


				void compute();



			protected:


				void extractProfiles(std::vector<cpu::CCTProfilsEvaluation>& profiles , unsigned short*& profileGrayValues );

				

			protected:
				
				imt::volume::VolumeInfo& mVolumeInfo;

				std::vector<Eigen::Vector3f>& mPoints, &mNormals;

				std::vector<unsigned int>& mSurfaceIndices;

				std::vector< cpu::f3f* > cloudPoints, cloudNormals, resPoints, sobelNormals;

			};




		}



	}



}



#endif