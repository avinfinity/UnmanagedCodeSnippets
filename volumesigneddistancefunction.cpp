#include "volumesigneddistancefunction.h"
#include "volumeutility.h"
#include <vtkPoints.h>
#include <vtkPolyData.h>



namespace imt 
{


	namespace volume 
	{

		VolumeSignedDistanceFunction::VolumeSignedDistanceFunction( 
			imt::volume::VolumeInfo& volumeInfo , 
			std::vector<Eigen::Vector3f>& marchingCubePoints, int isoThreshold):
			_VolumeInfo(volumeInfo) , mMarchingCubePoints(marchingCubePoints), _ISOThreshold(isoThreshold)
		{


			_PointLocator = vtkSmartPointer<vtkKdTreePointLocator>::New();

			vtkSmartPointer<vtkPoints> points =	vtkSmartPointer<vtkPoints>::New();


			vtkSmartPointer<vtkPolyData> polydata =	vtkSmartPointer<vtkPolyData>::New();
			


			//pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);

			//// Generate pointcloud data
			//cloud->width = mMarchingCubePoints.size();
			//cloud->height = 1;
			//cloud->points.resize(mMarchingCubePoints.size());

			int64_t numPoints = mMarchingCubePoints.size();

			for (size_t i = 0; i < numPoints; ++i)
			{
				//cloud->points[i].x = mMarchingCubePoints[i](0);
				//cloud->points[i].y = mMarchingCubePoints[i](1);
				//cloud->points[i].z = mMarchingCubePoints[i](2);

				points->InsertNextPoint(mMarchingCubePoints[i](0) , mMarchingCubePoints[i](1) , mMarchingCubePoints[i](2));
			}

			polydata->SetPoints(points);

			_PointLocator->SetDataSet(polydata);

			//_PointLocator->Update();

			_PointLocator->BuildLocator();


			//_KDtree = pcl::KdTreeFLANN<pcl::PointXYZ>::Ptr( new pcl::KdTreeFLANN<pcl::PointXYZ>() );

			//_KDtree->setInputCloud(cloud);


			imt::volume::VolumeUtility::volumeGradient(mMarchingCubePoints, _VolumeInfo , mVertexNormals);

			//double diagDist = _VolumeInfo.mWidth * _VolumeInfo.mWidth +
			//	_VolumeInfo.mHeight * _VolumeInfo.mHeight +
			//	_VolumeInfo.mDepth * _VolumeInfo.mDepth;


			//std::cout << "diagonal distance " << sqrt(diagDist * _VolumeInfo.mVoxelStep(0) * _VolumeInfo.mVoxelStep(0));

			//int numPoints = mMarchingCubePoints.size();

			//std::vector< unsigned char > pointMask(numPoints, 1);

		/*	int zStep = volinfo.mWidth * volinfo.mHeight;
			int yStep = volinfo.mWidth;

			unsigned short *vData = (unsigned short*)volinfo.mVolumeData;


			double initT = cv::getTickCount();*/

			//std::vector< Eigen::Vector3f > pts;

			//pts.reserve(volinfo.mWidth * volinfo.mHeight);

			//now generate rays

			double angleStart = -M_PI / 4;
			double angleEnd = M_PI / 4;
			double angleStep = M_PI / 12;


			for (double theta1 = angleStart; theta1 < angleEnd; theta1 += angleStep)
				for (double theta2 = angleStart; theta2 < angleEnd; theta2 += angleStep)
				{
					if (std::abs(theta1) < 0.0001 && std::abs(theta2) < 0.0001)
						continue;

					double cos1 = cos(theta1);
					double cos2 = cos(theta2);
					double theta3 = acos(1 - cos1 * cos1 - cos2 * cos2);

					Eigen::Matrix3f m;
					
					m = Eigen::AngleAxisf(theta1, Eigen::Vector3f::UnitX())
						* Eigen::AngleAxisf(theta2, Eigen::Vector3f::UnitY())
						* Eigen::AngleAxisf(theta3, Eigen::Vector3f::UnitZ());

					_RotationMatrices.push_back(m);

				}



		}


		double VolumeSignedDistanceFunction::voxelStep()
		{
			return _VolumeInfo.mVoxelStep(0);
		}


		Eigen::Vector3f VolumeSignedDistanceFunction::gradient( Eigen::Vector3f& inputPoint )
		{
			//float distanceThreshold = _VolumeInfo.mVoxelStep(0);

			//pcl::PointXYZ pt(inputPoint(0), inputPoint(1), inputPoint(2));

			//std::vector<int> indices;
			//std::vector<float> distances;

			//double wf = (_VolumeInfo.mWidth - 2) * _VolumeInfo.mVoxelStep(0);
			//double hf = (_VolumeInfo.mHeight - 2) * _VolumeInfo.mVoxelStep(0);
			//double df = (_VolumeInfo.mDepth - 2) * _VolumeInfo.mVoxelStep(0);

			//double sign = 1.0;

			//if (  inputPoint(0) > _VolumeInfo.mVoxelStep(0) && inputPoint(1) > _VolumeInfo.mVoxelStep(1)
			//	  && inputPoint(2) > _VolumeInfo.mVoxelStep(0) && inputPoint(0) < wf && inputPoint(1) < hf
			//	  && inputPoint(2) < df )
			//{
			//	double grayValue = _VolumeInfo.valueAt( inputPoint(0), inputPoint(1), inputPoint(2) );

			//	if ( grayValue > _ISOThreshold )
			//	{
			//		sign = -1;
			//	}
			//}

			double x[3] = { inputPoint(0) , inputPoint(1) , inputPoint(2) };

			//vtkIdType closestPointId = _PointLocator->FindClosestPoint(x);

			vtkSmartPointer<vtkIdList> closestPointIds = vtkSmartPointer<vtkIdList>::New();

			_PointLocator->FindClosestNPoints( 3 , x , closestPointIds );

			int id1 = closestPointIds->GetId(0);
			int id2 = closestPointIds->GetId(1);
			int id3 = closestPointIds->GetId(2);

			Eigen::Vector3f gradient = (mVertexNormals[id1] + mVertexNormals[id2] + mVertexNormals[id3]) * 0.3333333;

			gradient.normalize();

			//we need to return distance with sign
			return gradient;

		}



		void VolumeSignedDistanceFunction::signedDistanceAndGradient(const Eigen::Vector3f& inputPoint, double& signedDistance, Eigen::Vector3f& gradient)
		{

		}


		double VolumeSignedDistanceFunction::signedDistance(Eigen::Vector3f& inputPoint)
		{
			//if (kdtree.nearestKSearch(point, 10, indices, distances))
			//{
			//volinfo.mDistanceTransform[id] = sqrt(distances[0]);

			//}

			float distanceThreshold = _VolumeInfo.mVoxelStep(0);

			pcl::PointXYZ pt(inputPoint(0),inputPoint(1), inputPoint(2));

			std::vector<int> indices;
			std::vector<float> distances;

			double wf = ( _VolumeInfo.mWidth - 2 ) * _VolumeInfo.mVoxelStep(0);
			double hf = (_VolumeInfo.mHeight - 2 ) * _VolumeInfo.mVoxelStep(0);
			double df = (_VolumeInfo.mDepth - 2 ) * _VolumeInfo.mVoxelStep(0);

			double sign = 1.0;

			if (inputPoint(0) > _VolumeInfo.mVoxelStep(0) && inputPoint(1) > _VolumeInfo.mVoxelStep(1)
				&& inputPoint(2) > _VolumeInfo.mVoxelStep(0) &&
				inputPoint(0) < wf && inputPoint(1) < hf
				&& inputPoint(2) < df)
			{
				Eigen::Vector3f voxelInputPoint = inputPoint / _VolumeInfo.mVoxelStep(0);

				double grayValue = _VolumeInfo.valueAt(voxelInputPoint(0), voxelInputPoint(1), voxelInputPoint(2));

				if (grayValue > _ISOThreshold)
				{
					sign = -1;
				}


			}


			//if ( _KDtree->nearestKSearch(pt, 10, indices, distances) )
			//{
			//	double d = sqrt(distances[0]);

			//	if (1)//( d > distanceThreshold )
			//	{
			//		//we need to return distance with sign
			//		return d * sign;
			//	}
			//	else
			//	{
			//		//the point is too close to surface , we need to find a point where there is iso threshold crossing

			//		//we can sample 36 directions and find a crossing which minimizes the distance from the point

			//		//we will initialize the base direction as closest point normal and sample other directions with respect to this normal

			//		return std::abs(computeDistanceFromSurface(inputPoint, mVertexNormals[indices[0]])) * sign;
			//	}
			//}


			double x[3] = { inputPoint(0) , inputPoint(1) , inputPoint(2) };

			//vtkIdType closestPointId = _PointLocator->FindClosestPoint(x);

			vtkSmartPointer<vtkIdList> closestPointIds = vtkSmartPointer<vtkIdList>::New();

			_PointLocator->FindClosestNPoints(3, x, closestPointIds);

			int id1 = closestPointIds->GetId(0);
			int id2 = closestPointIds->GetId(1);
			int id3 = closestPointIds->GetId(2);

			double d1 = (mMarchingCubePoints[id1] - inputPoint).norm();
			double d2 = (mMarchingCubePoints[id2] - inputPoint).norm();
			double d3 = (mMarchingCubePoints[id3] - inputPoint).norm();

			return (d1 + d2 + d3) * 0.3333333 * sign; 

		}


		double VolumeSignedDistanceFunction::computeDistanceFromSurface(const Eigen::Vector3f& point, const Eigen::Vector3f& baseSearchDir)
		{
			double distanceFromSurface = std::numeric_limits<double>::max();

			int numDirCandidates = _RotationMatrices.size();

			double distTh = _VolumeInfo.mVoxelStep(0) * -10;

			for ( int dd = 0; dd < numDirCandidates + 1; dd++ )
			{
				Eigen::Vector3f searchDir = baseSearchDir;

				if (dd > 0)
				{
					searchDir = _RotationMatrices[dd - 1] * baseSearchDir;

					 double currentDistance = findIsoValueCrossing(point, searchDir);

					 if (currentDistance > distTh)
					 {
						 if ( distanceFromSurface > std::abs(distTh) )
						 {
							 distanceFromSurface = distTh;
						 }
					 }

				}
			}

			return distanceFromSurface;

		}



		double VolumeSignedDistanceFunction::findIsoValueCrossing( const Eigen::Vector3f& point, const Eigen::Vector3f& baseSearchDir)
		{
			double crossingDistance = -100 * _VolumeInfo.mVoxelStep(0);

			unsigned short* volumeData = (unsigned short*)_VolumeInfo.mVolumeData;

			Eigen::Vector3f voxelPoint = point / _VolumeInfo.mVoxelStep(0);

			double step = 0.5;

			double start = -5 * step;
			double end = 10 * step;

			double prevGrayValue = 0;

			int ctr = 0;

			double grayValue = 0;

			for ( double distance = start; distance < end; distance += step ,ctr++ )
			{
				Eigen::Vector3f voxelPosition = voxelPoint + distance * baseSearchDir;

				if (voxelPosition(0) > 2 && voxelPosition(1) > 2
					&& voxelPosition(2) > 2 &&
					voxelPosition(0) < _VolumeInfo.mWidth - 2 && voxelPosition(1) < _VolumeInfo.mHeight - 2
					&& voxelPosition(2) < _VolumeInfo.mDepth - 2)
				{
					grayValue = _VolumeInfo.valueAt(voxelPosition(0), voxelPosition(1), voxelPosition(2));
				}

				//double grayValue = _VolumeInfo.valueAt(voxelPosition(0), voxelPosition(1), voxelPosition(2));

				if (ctr > 0)
				{
					double alpha1 = _ISOThreshold - grayValue;
					double alpha2 = _ISOThreshold - prevGrayValue;

					if ( alpha1 * alpha2 < 0 ) // found iso value crossing
					{
						crossingDistance = distance - step * std::abs(alpha1) / (std::abs(alpha1) + std::abs(alpha2));

						break;
					}
				}
			}

			return crossingDistance * _VolumeInfo.mVoxelStep(0);
		}





	}

}