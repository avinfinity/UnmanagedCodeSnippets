#ifndef __VOLUMESIGNEDDISTANCEFUNCTION_H__
#define __VOLUMESIGNEDDISTANCEFUNCTION_H__

#include "volumeinfo.h"
#include "pclincludes.h"
#include "vtkKdTreePointLocator.h"
#include "vtkSmartPointer.h"

namespace imt {

	namespace volume {



		class VolumeSignedDistanceFunction
		{

		public:

			VolumeSignedDistanceFunction( imt::volume::VolumeInfo& volumeInfo , std::vector<Eigen::Vector3f>& marchingCubePoints , int isoThreshold );

			double signedDistance( Eigen::Vector3f& inputPoint ); 

			Eigen::Vector3f gradient(Eigen::Vector3f& inputPoint);

			void signedDistanceAndGradient( const Eigen::Vector3f& inputPoint , double& signedDistance , Eigen::Vector3f& gradient );

			double voxelStep();


		protected:

			double computeDistanceFromSurface(const Eigen::Vector3f& point, const Eigen::Vector3f& baseSearchDir);

			double findIsoValueCrossing(const Eigen::Vector3f& point, const Eigen::Vector3f& baseSearchDir);


		protected:

			imt::volume::VolumeInfo& _VolumeInfo;
			std::vector<Eigen::Vector3f>& mMarchingCubePoints;
			std::vector<Eigen::Vector3f> mVertexNormals;

			pcl::KdTreeFLANN<pcl::PointXYZ>::Ptr _KDtree;

			std::vector<Eigen::Matrix3f> _RotationMatrices;

			unsigned int _ISOThreshold; 

			vtkSmartPointer<vtkKdTreePointLocator> _PointLocator;

		};






	}

}

#endif
