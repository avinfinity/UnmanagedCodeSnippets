#define FLANN_USE_CUDA
//#include <flann/flann.hpp>

#include "opencvincludes.h"
#include "wallthicknessestimator.h"
#include "vector"
#include "iostream"
#include "display3droutines.h"

#include "aabbtree.h"
#include "lineiterator.h"
//#include "pclincludes.h"

#include "embreeincludes.h"
#include "atomic"


namespace imt{

	namespace volume{

		void
			solvePlaneParameters(const Eigen::Matrix3f &covariance_matrix,
			float &nx, float &ny, float &nz, float &curvature);


		unsigned int
			computeMeanAndCovarianceMatrix(const std::vector< Eigen::Vector3f> &cloud,
			Eigen::Matrix3f &covariance_matrix,
			Eigen::Vector4f &centroid);


#define voxelId( vox )  vox.z * mZStep + mYStep * vox.y + vox.x  
#define voxelIdCoords( x , y , z )  mZStep * z + mYStep * y + x  



		WallthicknessEstimator::WallthicknessEstimator(VolumeInfo& volume) : mVolInfo( volume )
		{

		}


		void WallthicknessEstimator::collectSurfaceVoxels( std::vector< Voxel >& surfaceVoxels )
		{
			int w = mVolInfo.mWidth;
			int h = mVolInfo.mHeight;
			int d = mVolInfo.mDepth;


			std::vector< Voxel > q1;

			q1.reserve(5 * w * h);

			long long nIds[26];

			int red = 0;

			for (int z = -1, z1 = 0; z <= 1; z++, z1++)
				for (int y = -1, y1 = 0; y <= 1; y++, y1++)
					for (int x = -1, x1 = 0; x <= 1; x++, x1++)
					{
						if (x == 0 && y == 0 && z == 0)
						{
							red = -1;

							continue;
						}

						nIds[z1 * 9 + y1 * 3 + x1 + red] = z * w * h + y * w + x;
					}


			unsigned char* volMask = mVolInfo.mVolumeData;

			//init surface voxels

			int vId = 0;

			for (int zz = 0; zz < d; zz++)
				for (int yy = 0; yy < h; yy++)
					for (int xx = 0; xx < w; xx++)
					{

						if (volMask[vId] < 30)
						{
							for (int ii = 0; ii < 26; ii++)
							{
								nIds[ii]++;
							}

							vId++;

							continue;
						}


						if (zz > 0 && yy > 0 && xx > 0 && zz < d - 1 && yy < h - 1 && xx < w - 1)
						{
							int id = 0;

							bool isOnSurface = false;

							for (int z = -1; z <= 1; z++)
								for (int y = -1; y <= 1; y++)
									for (int x = -1; x <= 1; x++)
									{
										if (x == 0 && y == 0 && z == 0)
										{
											continue;
										}

										if (volMask[nIds[id]] < 30)
										{
											isOnSurface = true;

											break;
										}

										id++;

									}


							if (isOnSurface)
							{
								Voxel vox;

								vox.x = xx;
								vox.y = yy;
								vox.z = zz;

								q1.push_back(vox);
							}
						}

						for (int ii = 0; ii < 26; ii++)
						{
							nIds[ii]++;
						}

						vId++;
					}


			surfaceVoxels = q1;

		}


		void WallthicknessEstimator::estimateWallThickness()
		{

			int w = mVolInfo.mWidth;
			int h = mVolInfo.mHeight;
			int d = mVolInfo.mDepth;


			std::vector< Voxel > q1, q2;

			q1.reserve(5 * w * h);
			q2.reserve(5 * w * h);

			std::vector<unsigned short> distInfo( w * h * d ) ;

			std::vector< unsigned char > mask2(w * h * d, 0);

			long long nIds[26];

			int red = 0;

			for (int z = -1 , z1 = 0; z <= 1; z++ , z1++)
				for ( int y = -1 , y1 = 0; y <= 1; y++ , y1++)
					for (int x = -1 , x1 = 0; x <= 1; x++ , x1++)
					{
						if (x == 0 && y == 0 && z == 0)
						{
							red = -1;

							continue;	
						}
							 
						nIds[z1 * 9 + y1 * 3 + x1 + red] = z * w * h + y * w + x ;
					}


			unsigned short* volMask = (unsigned short*)mVolInfo.mVolumeData;

			//init surface voxels

			int vId = 0;

			for ( int zz = 0; zz < d ; zz++ )
				for ( int yy = 0; yy < h; yy++ )
					for ( int xx = 0; xx < w ; xx++ )
					{

						if ( volMask[vId] < mVolInfo.mAirVoxelFilterVal)
						{
							for (int ii = 0; ii < 26; ii++)
							{
								nIds[ii]++;
							}

							vId++;

							continue;
						}
							

						if ( zz > 0 && yy > 0 && xx > 0 && zz < d - 1 && yy < h - 1 && xx < w - 1 )
						{
							int id = 0;

							bool isOnSurface = false;

							for ( int z = -1; z <= 1; z++ )
								for ( int y = -1; y <= 1; y++ )
									for ( int x = -1; x <= 1; x++ )
									{
										if (x == 0 && y == 0 && z == 0)
										{
											continue;
										}

										if (volMask[nIds[id]] >= mVolInfo.mAirVoxelFilterVal)
										{
											isOnSurface = true;

											break;
										}

										id++;

									}


							if (isOnSurface)
							{
								Voxel vox;

								vox.x = xx;
								vox.y = yy;
								vox.z = zz;

								q1.push_back(vox);
							}
						}

						for (int ii = 0; ii < 26; ii++)
						{
							nIds[ii]++;
						}

						vId++;
					}




#ifdef __AV_DEBUG

			std::cout << " number of surface voxels : " << q1.size() << std::endl;

			std::vector< Eigen::Vector3f > testPoints( q1.size() ) , testColors( q1.size() , Eigen::Vector3f( 0 , 1 , 0 ) );


			for (int ii = 0; ii < q1.size(); ii++)
			{
				testPoints[ii].x() = q1[ii].x;
				testPoints[ii].y() = q1[ii].y;
				testPoints[ii].z() = q1[ii].z;
			}

			tr::Display3DRoutines::displayPointSet(testPoints, testColors);
#endif


			peelSurface(q1, q2);


			std::vector< Eigen::Vector3f > testPoints(q2.size() + q1.size() ), testColors(q2.size() + q1.size(), Eigen::Vector3f(0, 1, 0));


			for (int ii = 0; ii < q2.size(); ii++)
			{
				testPoints[ii].x() = q2[ii].x;
				testPoints[ii].y() = q2[ii].y;
				testPoints[ii].z() = q2[ii].z;
			}


			for (int ii = 0; ii < q1.size(); ii++)
			{
				testPoints[ii + q2.size() ].x() = q1[ii].x;
				testPoints[ii + q2.size() ].y() = q1[ii].y;
				testPoints[ii + q2.size() ].z() = q1[ii].z;

				testColors[ii + q2.size()] = Eigen::Vector3f(1, 0, 0);
			}


			tr::Display3DRoutines::displayPointSet( testPoints , testColors );

			q1 = q2;
			q2.clear();

			std::fill( distInfo.begin() , distInfo.end() , 0 );

			int nPeeledVoxels = q1.size();

			int yStep = w;
			int zStep = w * h;

			for (int vv = 0; vv < nPeeledVoxels; vv++)
			{
				Voxel& vox = q1[vv];

				int x = vox.x;
				int y = vox.y;
				int z = vox.z;

				int id = z * zStep + y * yStep + x;

				distInfo[id] = 1;

				mask2[id] = 2;

				vox.mInfo = 1;

			}

			//now propagate the distance 
			while (1)
			{

				int nVoxs = q1.size();

				for (int vv = 0; vv < nVoxs; vv++)
				{
					Voxel& vox = q1[vv];

					int x = vox.x;
					int y = vox.y;
					int z = vox.z;

					int id1 = z * zStep + y * yStep + x;

					if (x < 1 || x > w - 2 || y < 1 || y > h - 2 || z < 1 || z > d - 2)
						continue;

					for (int zz = z - 1; zz <= z + 1; zz++)
						for (int yy = y - 1; yy <= y + 1; yy++)
							for (int xx = x - 1; xx <= x + 1; xx++)
							{
								int id = zz * zStep + yy * yStep + xx;

								if (volMask[id] >= mVolInfo.mAirVoxelFilterVal && mask2[id] < 1)
								{
									mask2[id] = 1;

									Voxel v;

									v.x = xx;
									v.y = yy;
									v.z = zz;

									q2.push_back(v);

									distInfo[id] = std::min(distInfo[id], distInfo[id1]);
								}

							}
				}


				int nextLayerSize = q2.size();

				for (auto it = q2.begin(); it != q2.end(); it++)
				{
					int x = it->x;
					int y = it->y;
					int z = it->z;

					int id = z * zStep + y * yStep + x;

					distInfo[id]++;

					it->mInfo = distInfo[id];

					//mask2[id] = 2;
				}

				if (q2.size() == 0)
					break;

				for (auto it = q2.begin(); it != q2.end(); it++)
				{
					int x = it->x;
					int y = it->y;
					int z = it->z;

					int id = z * zStep + y * yStep + x;

					mask2[id] = 2;
				}

				q1 = q2;

				q2.clear();
			}

			std::cout << " expanded voxels : " << q1.size() << std::endl;

			testPoints.resize(q1.size());
			testColors.resize(q1.size());


			for (int ii = 0; ii < q1.size(); ii++)
			{
				testPoints[ii].x() = q1[ii].x;
				testPoints[ii].y() = q1[ii].y;
				testPoints[ii].z() = q1[ii].z;

				testColors[ii](0) = q1[ii].mInfo * 4 / 255.0;
				testColors[ii](1) = 1;
				testColors[ii](2) = 0;
			}

			tr::Display3DRoutines::displayPointSet( testPoints , testColors );


		}


		void WallthicknessEstimator::peelSurface( const std::vector< Voxel >& surface , std::vector< Voxel >& peeledSurface )
		{

			unsigned char *vdata = mVolInfo.mVolumeData;

			std::vector< Voxel > q1, q2;

			int surfaceSize = surface.size();

			q1.reserve( surfaceSize );
			q2.reserve( surfaceSize );

			q1 = surface;

			int currD = 0;

			for (int ss = 0; ss < surfaceSize; ss++)
			{
				q1[ss].mInfo = 0;
			}

			int w = mVolInfo.mWidth;
			int h = mVolInfo.mHeight;
			int d = mVolInfo.mDepth;

			int zStep = w * h;
			int yStep = w;


			std::vector< unsigned char > volumeMask(w * h * d, 0);
			std::vector< unsigned short > distMap(w * h * d, std::numeric_limits< unsigned short >::max());

			for (int ss = 0; ss < surfaceSize; ss++)
			{
				int x = q1[ss].x;
				int y = q1[ss].y;
				int z = q1[ss].z;

				volumeMask[z * zStep + y * yStep + x] = 2;

				distMap[z * zStep + y * yStep + x] = 0;
			}


#ifdef __AV_DEBUG
			std::cout << " number of surface voxels " << q1.size() << std::endl;
#endif

			while (1)
			{

				//reset mask to zero
				for (int ss = 0; ss < surfaceSize; ss++)
				{
					int x = q1[ss].x;
					int y = q1[ss].y;
					int z = q1[ss].z;

					int id1 = z * zStep + y * yStep + x;

					if (x < 1 || x > w - 2 || y < 1 || y > h - 2 || z < 1 || z > d - 2)
						continue;

					for (int zz = z - 1; zz <= z + 1; zz++)
						for (int yy = y - 1; yy <= y + 1; yy++)
							for (int xx = x - 1; xx <= x + 1; xx++)
							{

								int id = zz * zStep + yy * yStep + xx;

								if ( vdata[id] > 30 && volumeMask[id] < 1 )
								{
									volumeMask[id] = 1;

									Voxel v;

									v.x = xx;
									v.y = yy;
									v.z = zz;

									q2.push_back(v);

									distMap[id] = std::min(distMap[id], distMap[id1]);
								}

							}
				}


				for (int ss = 0; ss < surfaceSize; ss++)
				{
					int x = q1[ss].x;
					int y = q1[ss].y;
					int z = q1[ss].z;

					int id = z * zStep + y * yStep + x;

					volumeMask[id]++;
				}


				int nextLayerSize = q2.size();

				for (auto it = q2.begin(); it != q2.end(); it++)
				{
					int x = it->x;
					int y = it->y;
					int z = it->z;

					int id = z * zStep + y * yStep + x;

					distMap[id]++;

					it->mInfo = distMap[id];
				}

				if ( q2.size() == 0 )
					break;


#ifdef __AV_DEBUG

				std::cout << " q2 size : " << q2.size() << std::endl;
#endif

				q1 = q2;

				q2.clear();

			}


			q2.clear();

			//now find the peeled voxels
			for (int z = 1; z  < d - 1; z++)
				for (int y = 1; y < h - 1; y++)
					for (int x = 1; x < w - 1; x++)
					{
						int id1 = z * zStep + y * yStep + x;

						if (vdata[id1] <= 30)
							continue;

						bool isPeeledVoxel = true;

						for (int zz = z - 1; zz <= z + 1; zz++)
							for (int yy = y - 1; yy <= y + 1; yy++)
								for (int xx = x - 1; xx <= x + 1; xx++)
								{
									int id2 = zz * zStep + yy * yStep + xx;

									if (distMap[id2] > distMap[id1])
									{
										isPeeledVoxel = false;

										break;
									}

								}

						if (isPeeledVoxel)
						{
							Voxel v;

							v.x = x;
							v.y = y;
							v.z = z;

							q2.push_back(v);
						}

					}


#ifdef __AV_DEBUG


			std::cout << " number of peeled voxels : " << q2.size() << std::endl;
#endif

			peeledSurface = q2;

		}



		void WallthicknessEstimator::rayTrace( std::vector< Voxel >& surface , std::vector< float >& thickness )
		{

			int numSurfaceVoxels = surface.size();

			std::vector< Eigen::Vector3f > voxelNormals;

			computeNormals( surface , voxelNormals );

			std::cout << " normals computed " << std::endl;

			int w = mVolInfo.mWidth;
			int h = mVolInfo.mHeight;
			int d = mVolInfo.mDepth;

			float maxLength = (  w * w * mVolInfo.mVoxelStep(0) * mVolInfo.mVoxelStep(0) +
				                 h * h * mVolInfo.mVoxelStep(1) * mVolInfo.mVoxelStep(1) + 
								 d * d * mVolInfo.mVoxelStep(2) * mVolInfo.mVoxelStep(2) );

			maxLength = sqrtf(maxLength);

			AABBTree searchTree(surface);

			searchTree.build();


			std::vector< AABBNode >& nodes = searchTree.getNodes();

			std::cout << " root stats : " << nodes[0].mAabb.mMin.transpose() << "  ..... " << nodes[0].mAabb.mMax.transpose() << std::endl;

			std::cout << " computing ray tracing " << std::endl;
			for (int ss = 50000; ss < 51000 ; ss++)
			{
				//ray trace in normal direction ( check for first intersection surface voxel )
                Voxel& vox = surface[ss];

				Ray r;

				r.mOrigin(0) = mVolInfo.mVolumeOrigin(0) + ( vox.x + 0.01 ) * mVolInfo.mVoxelStep(0);
				r.mOrigin(1) = mVolInfo.mVolumeOrigin(1) + ( vox.y + 0.01 ) * mVolInfo.mVoxelStep(1);
				r.mOrigin(2) = mVolInfo.mVolumeOrigin(2) + ( vox.z + 0.01 ) * mVolInfo.mVoxelStep(2);

				r.mDir = voxelNormals[ss];

				r.mInvDir(0) = 1.0 / r.mDir(0);
				r.mInvDir(1) = 1.0 / r.mDir(1);
				r.mInvDir(2) = 1.0 / r.mDir(2);

				r.mMinT = 0;
				r.mMaxT = maxLength;
				
				if (searchTree.intersect(r))
				{
					std::cout << " found intersection " << std::endl;
				}

				float thickness = r.mMaxT;

			}

		}

		void WallthicknessEstimator::rayTrace2(std::vector< Voxel >& surface, std::vector< float >& thickness)
		{

			int numSurfaceVoxels = surface.size();

			std::vector< Eigen::Vector3f > voxelNormals;

			computeNormals(surface, voxelNormals);

			std::cout << " normals computed " << std::endl;

			int w = mVolInfo.mWidth;
			int h = mVolInfo.mHeight;
			int d = mVolInfo.mDepth;

			float maxLength = (w * w * mVolInfo.mVoxelStep(0) * mVolInfo.mVoxelStep(0) +
				h * h * mVolInfo.mVoxelStep(1) * mVolInfo.mVoxelStep(1) +
				d * d * mVolInfo.mVoxelStep(2) * mVolInfo.mVoxelStep(2));

			maxLength = sqrtf(maxLength);

			unsigned char *volumeMask = mVolInfo.mVolumeData;

			int zStep = w * h;
			int yStep = w;

			int nVoxels = w * h * d;

			std::cout << " computing wall thickness " << std::endl;

			std::vector< Voxel > intersectedVoxels( numSurfaceVoxels );

//#pragma omp parallel for
			for (int ss = 0; ss < numSurfaceVoxels; ss++)
			//int ss = numSurfaceVoxels / 2;
			{
				voxelNormals[ss] *= -1;

				LineIterator li( voxelNormals[ss] , surface[ss] , mVolInfo.mVolumeOrigin , mVolInfo.mVoxelStep );

				Voxel endVox, prevVox = surface[ss];

				int numLineVoxels = 0;

				while (1)
				{
					Voxel vox = li.next();

					long int id = vox.z * zStep + vox.y * yStep + vox.x;

					if ( id < 0 || id >= nVoxels )
					{
						break;
					}

					if ( volumeMask[id] < 30 )
					{
						break;
					}
					
					prevVox = vox;

					numLineVoxels++;

					
				}

				//std::cout << " num line voxels : " << numLineVoxels << std::endl;

				intersectedVoxels[ss] = prevVox;
			}


			float avgDist = 0;

			std::vector< Eigen::Vector3f > colors( numSurfaceVoxels , Eigen::Vector3f( 1 , 0 , 0 ) ) , points( numSurfaceVoxels );


			for (int ss = 0; ss < numSurfaceVoxels; ss++)
			{
				Voxel vox1 = surface[ss];
				Voxel vox2 = intersectedVoxels[ss];

				float dist = sqrtf( (float)( (vox1.x - vox2.x) * (vox1.x - vox2.x) + (vox1.y - vox2.y) * (vox1.y - vox2.y) + (vox1.z - vox2.z) * (vox1.z - vox2.z) ) );

				avgDist += dist;

				colors[ss](1) = std::min( 1.0 , (5 * dist / 255.0 ) );

				points[ss](0) = vox1.x * mVolInfo.mVoxelStep(0) + mVolInfo.mVolumeOrigin(0);
				points[ss](1) = vox1.y * mVolInfo.mVoxelStep(1) + mVolInfo.mVolumeOrigin(1);
				points[ss](2) = vox1.z * mVolInfo.mVoxelStep(2) + mVolInfo.mVolumeOrigin(2);
			}

			avgDist /= numSurfaceVoxels;

			std::cout << " avg dist " << avgDist << std::endl;

			tr::Display3DRoutines::displayPointSet(points, colors);

		}


		void WallthicknessEstimator::fillRayTraceVolume(unsigned short *volume)
		{

			mZStep = mVolInfo.mWidth * mVolInfo.mHeight;
			mYStep = mVolInfo.mWidth;

			size_t size = mVolInfo.mWidth * mVolInfo.mHeight * mVolInfo.mDepth;

			float rayStep = mVolInfo.mVoxelStep(0) * 0.5;

			memset(volume, 0, size * sizeof(unsigned short));

			int numVertices = mVolInfo.mVertices.size();

			float invStepX = 1.0 / mVolInfo.mVoxelStep(0);
			float invStepY = 1.0 / mVolInfo.mVoxelStep(1);
			float invStepZ = 1.0 / mVolInfo.mVoxelStep(2);

			float minThickness = 200, maxThickness = 0, avgThickness = 0;;

			for (int vv = 0; vv < numVertices; vv++)
			{
				if (mVolInfo.mWallthickness[vv] < 0)
					continue;

				minThickness = std::min(minThickness, mVolInfo.mWallthickness[vv]);
				maxThickness = std::max(maxThickness, mVolInfo.mWallthickness[vv]);

				avgThickness += mVolInfo.mWallthickness[vv];
			}

			minThickness = 3 * mVolInfo.mVoxelStep(0);

			maxThickness = 100 * mVolInfo.mVoxelStep(0);

			float thicknessRange = maxThickness - minThickness;

			std::cout << " thickness range : " << thicknessRange<< " " <<avgThickness / numVertices << std::endl;

			size_t nFilledVoxels = 0;

			for (int vv = 0; vv < numVertices; vv++)
			{
				Eigen::Vector3f vert = mVolInfo.mVertices[vv];

				Eigen::Vector3f dir = -mVolInfo.mVertexNormals[vv];

				float thickness = mVolInfo.mWallthickness[vv];

				unsigned short discreteThickness = thickness * USHRT_MAX / thicknessRange;

				for ( float ll = 0.0f; ll < thickness; ll += rayStep )
				{
					Eigen::Vector3f pt = vert + ll * dir;

					Eigen::Vector3f diff = pt - mVolInfo.mVolumeOrigin;

					int vx = diff(0) * invStepX;
					int vy = diff(1) * invStepY;
					int vz = diff(2) * invStepZ;

					//if (vx >= mVolInfo.mWidth || vy >= mVolInfo.mHeight || vz >= mVolInfo.mDepth)
					//{
					//	std::cout << " bad voxel : " << vx << " " << vy << " " << vz << std::endl;
					//}

					size_t vId = voxelIdCoords( vx , vy , vz );

					if ( !volume[vId] )
					{
						volume[vId] = discreteThickness;
					}
					else
					{
						volume[vId] = std::min( discreteThickness , volume[vId] );
					}

					nFilledVoxels++;

				}


			}

			std::cout << " num filled voxels : " << (double)nFilledVoxels / (double)size << std::endl;

		}


		void WallthicknessEstimator::getWallthicknessSliceImage(cv::Mat& image, unsigned short *volume)
		{
			image.create( mVolInfo.mDepth, mVolInfo.mHeight, CV_8UC3 );

			int x = mVolInfo.mWidth / 2;

			cv::Vec3b *col = (cv::Vec3b*)image.data;

			cv::Mat testImage(mVolInfo.mDepth, mVolInfo.mHeight, CV_32FC1);

			testImage.setTo(cv::Scalar(1.0));

			float *ti = (float*)testImage.data;

			for ( int dd = 0; dd < mVolInfo.mDepth; dd++ )
				for ( int yy = 0; yy < mVolInfo.mHeight; yy++ )
				{
					float th = (float)volume[voxelIdCoords( x , yy , dd )] / USHRT_MAX;

					ti[dd * mVolInfo.mHeight + yy] = th;

				}

			cv::namedWindow("thicknessSlice", 0);
			cv::imshow("thicknessSlice", testImage);
			cv::waitKey();

		}





		float WallthicknessEstimator::pointToTriangleDistance( const Eigen::Vector3f& point, const Eigen::Vector3f& p1, const Eigen::Vector3f& p2, const Eigen::Vector3f&  p3)
		{
			//taking a as origin

			Eigen::Vector3f E0 = p2 - p1;
			Eigen::Vector3f E1 = p3 - p1;
			Eigen::Vector3f B = p1;

			Eigen::Vector3f D = B - point;

			float a = E0.dot(E0);
			float b = E0.dot(E1);
			float c = E1.dot(E1);

			float d = E0.dot(D);

			float e = E1.dot(D);
			float f = D.dot(D);


			float det = a * c - b * b;
			float s = b * e - c *d;
			float t = b *d - a * e;


			if (s + t <= det)
			{
				if (s < 0)
				{
					if (t < 0)
					{
						//region 4
					}
					else
					{
						//region 3
					}
				}
				else if (t < 0)
				{
					//region 5
				}
				else
				{
					//region 0
				}
			}
			else
			{
				if (s < 0)
				{
					//region 2
				}
				else if (t < 0)
				{
					//region 6
				}
				else
				{
					//region 1
				}
			}


			return 0.0;
		}



		void WallthicknessEstimator::computeNormals(std::vector< Voxel >& voxels, std::vector< Eigen::Vector3f >& normals)
		{

			//find nearest voxels
			int numSurfaceVoxels = voxels.size();

			int w = mVolInfo.mWidth;
			int h = mVolInfo.mHeight;
			int d = mVolInfo.mDepth;

			int zStep = w * h;
			int yStep = w;

			unsigned char *vdata = mVolInfo.mVolumeData;

			std::vector< unsigned char > isSurfVox(w * h * d, 0);

			for (int vv = 0; vv < numSurfaceVoxels; vv++)
			{
				Voxel& vox = voxels[vv];

				int x = vox.x;
				int y = vox.y;
				int z = vox.z;

				int id1 = z * zStep + y * yStep + x;

				isSurfVox[id1] = 1;

			}


			std::vector< Voxel > candVoxs;

			candVoxs.reserve(125);

			normals.resize(numSurfaceVoxels);

			Eigen::Vector3f refPoint( 0 , 0 , 0 ) , avgPoint( 0 , 0 , 0 );

			std::vector< Eigen::Vector3f > points;

			points.resize(numSurfaceVoxels);

			for (int vv = 0; vv < numSurfaceVoxels; vv++)
			{
				Voxel& vox = voxels[vv];

				int x = vox.x;
				int y = vox.y;
				int z = vox.z;

				int id1 = z * zStep + y * yStep + x;

				if (x < 2 || x > w - 3 || y < 2 || y > h - 3 || z < 2 || z > d - 3)
					continue;

				refPoint(0) = mVolInfo.mVolumeOrigin(0) + x * mVolInfo.mVoxelStep(0);
				refPoint(1) = mVolInfo.mVolumeOrigin(1) + y * mVolInfo.mVoxelStep(1);
				refPoint(2) = mVolInfo.mVolumeOrigin(2) + z * mVolInfo.mVoxelStep(2);

				points[vv] = refPoint;

				avgPoint.setZero();

				int nPoints = 0;
			
				std::vector< Eigen::Vector3f > pts;


				for ( int zz = z - 2; zz <= z + 2; zz++)
					for ( int yy = y - 2; yy <= y + 2; yy++)
						for ( int xx = x - 2; xx <= x + 2; xx++)
						{
							int id = zz * zStep + yy * yStep + xx;

							if ( vdata[id] > 30 )
							{
								Eigen::Vector3f pt;

								pt(0) = mVolInfo.mVolumeOrigin(0) + xx * mVolInfo.mVoxelStep(0);
								pt(1) = mVolInfo.mVolumeOrigin(1) + yy * mVolInfo.mVoxelStep(1);
								pt(2) = mVolInfo.mVolumeOrigin(2) + zz * mVolInfo.mVoxelStep(2);

								avgPoint += pt;

								nPoints++;

								if (isSurfVox[id])
								{
									Voxel v;

									v.x = xx;
									v.y = yy;
									v.z = zz;

									candVoxs.push_back(v);

									pts.push_back(pt);
								}
							}

						}


				avgPoint /= nPoints;

				
				Eigen::Matrix3f covarianceMatrix;
				Eigen::Vector4f centroid;
				Eigen::Vector3f normal;

				//fit normal to voxel point
				computeMeanAndCovarianceMatrix(pts, covarianceMatrix, centroid);

				float curvature;
				
				solvePlaneParameters(covarianceMatrix, normal(0), normal(1), normal(2) , curvature );

				if (pts.size() < 5)
				{
					normal = Eigen::Vector3f( 1 , 0 , 0 );
				}

				//find normal's direction
				Eigen::Vector3f vec = refPoint - avgPoint;

				if ( vec.dot(normal) < 0 )
				{
					normal *= -1;
				}



				normal.normalize();

				//if ( normal.norm() > 1.5 || normal.hasNaN())
				//{
				//	std::cout << " bad normal " << std::endl;
				//}

				normals[vv] = normal;

			}


#ifdef __AV_DEBUG
			std::vector< Eigen::Vector3f > pointColors( numSurfaceVoxels , Eigen::Vector3f( 1 , 0 , 0 ) );


			std::cout << " sample normal  : " << normals[50000].transpose() << std::endl;

			std::cout << " points and normals sizes : " << points.size() << " " << normals.size() << std::endl;
			
			//tr::Display3DRoutines::displayPointSet(normals, pointColors);

			points.resize(50000);
			normals.resize(50000);
			tr::Display3DRoutines::displayNormals(points, normals);
#endif
			
		}



		void WallthicknessEstimator::computeDistanceTransform(VolumeInfo& volinfo)
		{
			volinfo.mDistanceTransform.resize(volinfo.mWidth * volinfo.mHeight * volinfo.mDepth);
		
			std::fill(volinfo.mDistanceTransform.begin(), volinfo.mDistanceTransform.end(), -1);

#if 0

			std::vector<int> indexMapping;

			flann::Matrix<float> mat( (float*)volinfo.mVertices.data(), indexMapping.size(),3 );
			
			flann::Index< flann::L2<float> > flannIndex( mat , flann::KDTreeSingleIndexParams( 15 ) );

			flannIndex.buildIndex();

			int k = 1;

			std::vector< Eigen::Vector3f > query;

			std::vector< int > kIndices;
			std::vector< float > kDistances;

			flann::Matrix<int> k_indices_mat( &kIndices[0] , 1 , 1 );
			flann::Matrix<float> k_distances_mat( &kDistances[0] , 1 , 1);


			flann::SearchParams paramsK( -1 , 0.0 );

			// Wrap the k_indices and k_distances vectors (no data copy)
			flannIndex.knnSearch( flann::Matrix<float>( (float*)&query[0], 1, 3),
				                                        k_indices_mat, k_distances_mat,
				                                        k, paramsK);



			pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);

			// Generate pointcloud data
			cloud->width = volinfo.mVertices.size();
			cloud->height = 1;
			cloud->points.resize(volinfo.mVertices.size());

			for (size_t i = 0; i < cloud->points.size(); ++i)
			{
				cloud->points[i].x = volinfo.mVertices[i](0);
				cloud->points[i].y = volinfo.mVertices[i](1);
				cloud->points[i].z = volinfo.mVertices[i](2);
			}


	

			pcl::KdTreeFLANN<pcl::PointXYZ> kdtree;

			kdtree.setInputCloud(cloud);

			int numPoints = volinfo.mVertices.size();

			std::vector< unsigned char > pointMask( numPoints , 1 );

			int zStep = volinfo.mWidth * volinfo.mHeight;
			int yStep = volinfo.mWidth;

			unsigned short *vData = (unsigned short*)volinfo.mVolumeData;


			double initT = cv::getTickCount();

			std::vector< Eigen::Vector3f > pts;

			pts.reserve(volinfo.mWidth * volinfo.mHeight);

			//tbb::atomic< double > distSum ;


#pragma omp parallel for
			for (int zz = 0; zz < volinfo.mDepth; zz++)
			{ 
				for (int yy = 0; yy < volinfo.mHeight; yy++)
					for (int xx = 0; xx < volinfo.mWidth; xx++)
					{
						pcl::PointXYZ point;

						point.x = xx * volinfo.mVoxelStep( 0 ) + volinfo.mVolumeOrigin( 0 );
						point.y = yy * volinfo.mVoxelStep(1) + volinfo.mVolumeOrigin(1);
						point.z = zz * volinfo.mVoxelStep(2) + volinfo.mVolumeOrigin(2);

						std::vector< int > indices;
						std::vector< float > distances;

						long int id = zz * zStep + yy * yStep + xx;

						if (vData[ id ] < volinfo.mAirVoxelFilterVal)
						{
							continue;
						}

						if ( kdtree.nearestKSearch( point, 10, indices, distances ) )
						{
							volinfo.mDistanceTransform[id] = sqrt(distances[0]);

#ifdef __AV_DEBUG
#pragma omp critical
				            {

							   pts.push_back(Eigen::Vector3f(point.x, point.y, point.z));
							}
#endif
						}

					}

				//std::cout << zz << std::endl;


			}


#ifdef __AV_DEBUG
			std::vector< Eigen::Vector3f > ptcolors(pts.size(), Eigen::Vector3f(1, 0, 0));
         	tr::Display3DRoutines::displayPointSet(pts, ptcolors);
#endif


#else
#endif


			//std::cout << volinfo.mDistanceTransform.size() << std::endl;

			//std::cout << " time spent : " << (cv::getTickCount() - initT) / cv::getTickFrequency() << std::endl;

			//std::vector< Eigen::Vector3f > medialAxisPoints ;

			//medialAxisPoints.reserve(volinfo.mVertices.size());

			//for (int zz = 1; zz < volinfo.mDepth - 1; zz++)
			//{
			//	for ( int yy = 1; yy < volinfo.mHeight - 1 ; yy++)
			//		for ( int xx = 1; xx < volinfo.mWidth - 1; xx++)
			//		{
			//			long int id = zz * zStep + yy * yStep + xx;

			//			if (id > volinfo.mDistanceTransform.size() - 1)
			//			{
			//				std::cout << "  bad id " << id << " " << volinfo.mDistanceTransform.size() << "  ----  " << xx << " " << yy << " " << zz << " ( " << volinfo.mWidth << " , " << volinfo.mHeight << " , " << volinfo.mDepth<<" ) " << std::endl;
			//			}

 		//				if ( volinfo.mDistanceTransform[id] > 0 )
			//			{

			//				bool isLocalMaxima = true;

			//				for (int z = zz - 1; z <= zz + 1; z++)
			//					for (int y = yy - 1; y <= yy + 1; y++)
			//						for (int x = xx - 1; x <= xx + 1; x++)
			//						{

			//							if (x == xx && y == yy && z == zz)
			//								continue;

			//							long int id2 = z * zStep + y * yStep + x;

			//							if (volinfo.mDistanceTransform[id] < volinfo.mDistanceTransform[id2])
			//								isLocalMaxima = false;

			//						}


			//				if (isLocalMaxima)
			//				{
			//					Eigen::Vector3f point;

			//					point.x() = xx * volinfo.mVoxelStep(0) + volinfo.mVolumeOrigin(0);
			//					point.y() = yy * volinfo.mVoxelStep(1) + volinfo.mVolumeOrigin(1);
			//					point.z() = zz * volinfo.mVoxelStep(2) + volinfo.mVolumeOrigin(2);

			//					medialAxisPoints.push_back(point);
			//				}
			//			}
			//		}
			//}


			//std::cout << " number of medial axis points " << medialAxisPoints.size() << " " <<volinfo.mVertices.size() << std::endl;

			//std::vector< Eigen::Vector3f > medialAxisColors(medialAxisPoints.size(), Eigen::Vector3f(1, 0, 0));

			//tr::Display3DRoutines::displayPointSet(medialAxisPoints, medialAxisColors);



		}

#if 0
		void WallthicknessEstimator::computeDistanceTransformCUDA(VolumeInfo& volinfo)
		{

			volinfo.mDistanceTransform.resize(volinfo.mWidth * volinfo.mHeight * volinfo.mDepth);

			std::fill(volinfo.mDistanceTransform.begin(), volinfo.mDistanceTransform.end(), -1);

#if 1

			std::vector<int> indexMapping;

			flann::Matrix<float> mat((float*)volinfo.mVertices.data(), volinfo.mVertices.size(), 3);

			flann::Index< flann::L2<float> > flannIndex(mat, flann::KDTreeCuda3dIndexParams(64));

			flannIndex.buildIndex();

			int k = 1;

			std::vector< Eigen::Vector3f > query;

			int zStep = volinfo.mWidth * volinfo.mHeight;
			int yStep = volinfo.mWidth;

			unsigned short *vData = (unsigned short*)volinfo.mVolumeData;

			std::vector< long int > pointIds;


			for (int zz = 0; zz < volinfo.mDepth; zz++)
			{
				for (int yy = 0; yy < volinfo.mHeight; yy++)
					for (int xx = 0; xx < volinfo.mWidth; xx++)
					{
						long int id = zz * zStep + yy * yStep + xx;

						if ( vData[id] < volinfo.mAirVoxelFilterVal )
						{
							continue;
						}

						Eigen::Vector3f point;

						point(0) = xx * volinfo.mVoxelStep(0) + volinfo.mVolumeOrigin(0);
						point(1) = yy * volinfo.mVoxelStep(1) + volinfo.mVolumeOrigin(1);
						point(2) = zz * volinfo.mVoxelStep(2) + volinfo.mVolumeOrigin(2);

						query.push_back(point);
						pointIds.push_back(id);
					}
			}


			long int nQueryPoints = query.size();
			

			std::vector< int > kIndices(nQueryPoints);
			std::vector< float > kDistances(nQueryPoints);

			flann::Matrix<int> k_indices_mat(&kIndices[0], nQueryPoints, k);
			flann::Matrix<float> k_distances_mat(&kDistances[0], nQueryPoints, k);

			flann::SearchParams paramsK(-1, 0.0);

			// Wrap the k_indices and k_distances vectors (no data copy)
			flannIndex.knnSearch( flann::Matrix<float>((float*)&query[0], nQueryPoints, 3),
				                  k_indices_mat , k_distances_mat ,
				                  k , paramsK );
#else


			pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);

			// Generate pointcloud data
			cloud->width = volinfo.mVertices.size();
			cloud->height = 1;
			cloud->points.resize(volinfo.mVertices.size());

			for (size_t i = 0; i < cloud->points.size(); ++i)
			{
				cloud->points[i].x = volinfo.mVertices[i](0);
				cloud->points[i].y = volinfo.mVertices[i](1);
				cloud->points[i].z = volinfo.mVertices[i](2);
			}




			pcl::KdTreeFLANN<pcl::PointXYZ> kdtree;

			kdtree.setInputCloud(cloud);

			int numPoints = volinfo.mVertices.size();

			std::vector< unsigned char > pointMask(numPoints, 1);

			int zStep = volinfo.mWidth * volinfo.mHeight;
			int yStep = volinfo.mWidth;

			unsigned short *vData = (unsigned short*)volinfo.mVolumeData;


			double initT = cv::getTickCount();

			std::vector< Eigen::Vector3f > pts;

			pts.reserve(volinfo.mWidth * volinfo.mHeight);

			//tbb::atomic< double > distSum ;


#pragma omp parallel for
			for (int zz = 0; zz < volinfo.mDepth; zz++)
			{
				for (int yy = 0; yy < volinfo.mHeight; yy++)
					for (int xx = 0; xx < volinfo.mWidth; xx++)
					{
						pcl::PointXYZ point;

						point.x = xx * volinfo.mVoxelStep(0) + volinfo.mVolumeOrigin(0);
						point.y = yy * volinfo.mVoxelStep(1) + volinfo.mVolumeOrigin(1);
						point.z = zz * volinfo.mVoxelStep(2) + volinfo.mVolumeOrigin(2);

						std::vector< int > indices;
						std::vector< float > distances;

						long int id = zz * zStep + yy * yStep + xx;

						if (vData[id] < volinfo.mAirVoxelFilterVal)
						{
							continue;
						}

						if (kdtree.nearestKSearch(point, 10, indices, distances))
						{
							volinfo.mDistanceTransform[id] = sqrt(distances[0]);

#ifdef __AV_DEBUG
#pragma omp critical
							{

								pts.push_back(Eigen::Vector3f(point.x, point.y, point.z));
							}
#endif
						}

					}

				//std::cout << zz << std::endl;
			}


#ifdef __AV_DEBUG
			std::vector< Eigen::Vector3f > ptcolors(pts.size(), Eigen::Vector3f(1, 0, 0));
			tr::Display3DRoutines::displayPointSet(pts, ptcolors);
#endif



#endif


			//std::cout << volinfo.mDistanceTransform.size() << std::endl;

			//std::cout << " time spent : " << (cv::getTickCount() - initT) / cv::getTickFrequency() << std::endl;

			//std::vector< Eigen::Vector3f > medialAxisPoints ;

			//medialAxisPoints.reserve(volinfo.mVertices.size());

			//for (int zz = 1; zz < volinfo.mDepth - 1; zz++)
			//{
			//	for ( int yy = 1; yy < volinfo.mHeight - 1 ; yy++)
			//		for ( int xx = 1; xx < volinfo.mWidth - 1; xx++)
			//		{
			//			long int id = zz * zStep + yy * yStep + xx;

			//			if (id > volinfo.mDistanceTransform.size() - 1)
			//			{
			//				std::cout << "  bad id " << id << " " << volinfo.mDistanceTransform.size() << "  ----  " << xx << " " << yy << " " << zz << " ( " << volinfo.mWidth << " , " << volinfo.mHeight << " , " << volinfo.mDepth<<" ) " << std::endl;
			//			}

			//				if ( volinfo.mDistanceTransform[id] > 0 )
			//			{

			//				bool isLocalMaxima = true;

			//				for (int z = zz - 1; z <= zz + 1; z++)
			//					for (int y = yy - 1; y <= yy + 1; y++)
			//						for (int x = xx - 1; x <= xx + 1; x++)
			//						{

			//							if (x == xx && y == yy && z == zz)
			//								continue;

			//							long int id2 = z * zStep + y * yStep + x;

			//							if (volinfo.mDistanceTransform[id] < volinfo.mDistanceTransform[id2])
			//								isLocalMaxima = false;

			//						}


			//				if (isLocalMaxima)
			//				{
			//					Eigen::Vector3f point;

			//					point.x() = xx * volinfo.mVoxelStep(0) + volinfo.mVolumeOrigin(0);
			//					point.y() = yy * volinfo.mVoxelStep(1) + volinfo.mVolumeOrigin(1);
			//					point.z() = zz * volinfo.mVoxelStep(2) + volinfo.mVolumeOrigin(2);

			//					medialAxisPoints.push_back(point);
			//				}
			//			}
			//		}
			//}


			//std::cout << " number of medial axis points " << medialAxisPoints.size() << " " <<volinfo.mVertices.size() << std::endl;

			//std::vector< Eigen::Vector3f > medialAxisColors(medialAxisPoints.size(), Eigen::Vector3f(1, 0, 0));

			//tr::Display3DRoutines::displayPointSet(medialAxisPoints, medialAxisColors);

		}
#endif

		void WallthicknessEstimator::computeOtherEnds(std::vector< Eigen::Vector3f >& points, std::vector< Eigen::Vector3f >& normals,
			std::vector< Eigen::Vector3f >& otherEnds, std::vector< Eigen::Vector3f >& otherEndNormals)
		{



		}


		void WallthicknessEstimator::compareMeshes( std::vector< Eigen::Vector3f >& refPoints,  std::vector< unsigned int >& refIndices, 
			std::vector< Eigen::Vector3f >& comparePoints, std::vector< Eigen::Vector3f >& compareNormals , std::vector< unsigned int >& compareIndices )
		{

#if 0
			_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
			_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

			rtcInit(NULL);

			unsigned int geomID;

			mScene = 0;

			mDevice = rtcNewDevice(NULL);

			long int numTriangles = refIndices.size() / 3;
			long int numVertices = refPoints.size();

			std::vector< Eigen::Vector3f > oppVertices(numVertices);

			mScene = rtcDeviceNewScene(mDevice, RTC_SCENE_DYNAMIC, RTC_INTERSECT8);

			geomID = rtcNewTriangleMesh(mScene, RTC_GEOMETRY_DYNAMIC, numTriangles, numVertices, 1);

			EmbreeTriangle* triangles = (EmbreeTriangle*)rtcMapBuffer(mScene, geomID, RTC_INDEX_BUFFER);

			rtcUnmapBuffer(mScene, geomID, RTC_INDEX_BUFFER);

			EmbreeVertex* vertices = (EmbreeVertex*)rtcMapBuffer(mScene, geomID, RTC_VERTEX_BUFFER);

			std::vector< unsigned int > indices;

			for (int tt = 0; tt < numTriangles; tt++)
			{

				triangles[tt].v0 = refIndices[3 * tt];
				triangles[tt].v1 = refIndices[3 * tt + 1];
				triangles[tt].v2 = refIndices[3 * tt + 2];

			}

			for (int pp = 0; pp < numVertices; pp++)
			{

				vertices[pp].x = refPoints[pp](0);
				vertices[pp].y = refPoints[pp](1);
				vertices[pp].z = refPoints[pp](2);
				vertices[pp].a = 1.0;

			}

			rtcUnmapBuffer(mScene, geomID, RTC_VERTEX_BUFFER);

			rtcCommit(mScene);


			std::vector< float > closestSurfaceDistances(comparePoints.size());

			std::fill( closestSurfaceDistances.begin(), closestSurfaceDistances.end(), 1000);

			std::vector< bool > oppositeEndFound(comparePoints.size(), false);

			float near = 0;
			float far = FLT_MAX;

			numVertices = comparePoints.size();

#pragma omp parallel for
			for (int vv = 0; vv < numVertices + 8; vv += 8)
			{
				RTCRay8 ray;

				for (int ii = 0; ii < 8; ii++)
				{
					int id;

					if ((vv + ii) >= numVertices)
					{
						id = numVertices - 1;
					}
					else
					{
						id = vv + ii;
					}

					ray.orgx[ii] = comparePoints[id](0);
					ray.orgy[ii] = comparePoints[id](1);
					ray.orgz[ii] = comparePoints[id](2);
				}


				for (int ii = 0; ii < 8; ii++)
				{
					Eigen::Vector3f dir;

					int id;

					if ((vv + ii) >= numVertices)
					{
						id = numVertices - 1;
					}
					else
					{
						id = vv + ii;
					}

					//std::cout << edgeNormals[id].transpose() << std::endl;

					ray.dirx[ii] = compareNormals[id](0);
					ray.diry[ii] = compareNormals[id](1);
					ray.dirz[ii] = compareNormals[id](2);

					ray.geomID[ii] = RTC_INVALID_GEOMETRY_ID;
					ray.instID[ii] = RTC_INVALID_GEOMETRY_ID;
					ray.primID[ii] = RTC_INVALID_GEOMETRY_ID;

					ray.tnear[ii] = near;
					ray.tfar[ii] = far;

					ray.mask[ii] = 0xFFFFFFFF;
					ray.time[ii] = 0.f;
				}

				__aligned(32) int valid8[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };

				rtcIntersect8(valid8, mScene, ray);

				for (int ii = 0; ii < 8; ii++)
				{
					if ((vv + ii) >= numVertices)
					{
						continue;
					}

					if (ray.primID[ii] != RTC_INVALID_GEOMETRY_ID)
					{
						closestSurfaceDistances[vv + ii] = ray.tfar[ii];

						oppositeEndFound[vv + ii] = true;

					}
				}

			}


#pragma omp parallel for
			for (int vv = 0; vv < numVertices + 8; vv += 8)
			{
				RTCRay8 ray;

				for (int ii = 0; ii < 8; ii++)
				{
					int id;

					if ((vv + ii) >= numVertices)
					{
						id = numVertices - 1;
					}
					else
					{
						id = vv + ii;
					}

					ray.orgx[ii] = comparePoints[id](0);
					ray.orgy[ii] = comparePoints[id](1);
					ray.orgz[ii] = comparePoints[id](2);
				}


				for (int ii = 0; ii < 8; ii++)
				{
					Eigen::Vector3f dir;

					int id;

					if ((vv + ii) >= numVertices)
					{
						id = numVertices - 1;
					}
					else
					{
						id = vv + ii;
					}

					//std::cout << edgeNormals[id].transpose() << std::endl;

					ray.dirx[ii] = -compareNormals[id](0);
					ray.diry[ii] = -compareNormals[id](1);
					ray.dirz[ii] = -compareNormals[id](2);

					ray.geomID[ii] = RTC_INVALID_GEOMETRY_ID;
					ray.instID[ii] = RTC_INVALID_GEOMETRY_ID;
					ray.primID[ii] = RTC_INVALID_GEOMETRY_ID;

					ray.tnear[ii] = near;
					ray.tfar[ii] = far;

					ray.mask[ii] = 0xFFFFFFFF;
					ray.time[ii] = 0.f;
				}

				__aligned(32) int valid8[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };

				rtcIntersect8(valid8, mScene, ray);

				for (int ii = 0; ii < 8; ii++)
				{
					if ((vv + ii) >= numVertices)
					{
						continue;
					}

					if (ray.primID[ii] != RTC_INVALID_GEOMETRY_ID)
					{
						closestSurfaceDistances[vv + ii] = std::min(closestSurfaceDistances[vv + ii], ray.tfar[ii]);

						oppositeEndFound[vv + ii] = true;
					}
				}

			}


			int nPoints = comparePoints.size();

			std::vector< Eigen::Vector3f > vertexColors( nPoints , Eigen::Vector3f(1, 1, 1));

			double thickness = 0;
			int count = 0;

			float coeff = 2;

			float th1 = 0.02 * coeff;
			float th2 = 0.03 * coeff;

			for (int vv = 0; vv < nPoints; vv++)
			{
				if (!oppositeEndFound[vv])
					continue;

				if (closestSurfaceDistances[vv] < th1 )
				{
					vertexColors[vv](0) = 1 - closestSurfaceDistances[vv] / th1;
					vertexColors[vv](1) = closestSurfaceDistances[vv] / th1;
					vertexColors[vv](2) = 0;


				}
				else if (closestSurfaceDistances[vv] > th1  && closestSurfaceDistances[vv] < th2 )
				{
					vertexColors[vv](0) = 0;
					vertexColors[vv](1) = 1 - closestSurfaceDistances[vv] / th2;
					vertexColors[vv](2) = closestSurfaceDistances[vv] / th2;
				}

				thickness += closestSurfaceDistances[vv];
				count++;

			}

			thickness /= count;

			std::cout << " average thickness " << thickness << std::endl;

			tr::Display3DRoutines::displayMesh( comparePoints, vertexColors, compareIndices );

#endif



		}


		void WallthicknessEstimator::computeBrepRayTraceThickness(VolumeInfo& volinfo)
		{

#if 0

			_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
			_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

			rtcInit(NULL);

			unsigned int geomID;

			mScene = 0;

			mDevice = rtcNewDevice(NULL);

			long int numTriangles = volinfo.mFaceIndices.size() / 3;
			long int numVertices = volinfo.mVertices.size();

			volinfo.mOppVertices.resize(numVertices);
			volinfo.mOppositeFaceIndices.resize(numVertices);

			mScene = rtcDeviceNewScene( mDevice, RTC_SCENE_DYNAMIC, RTC_INTERSECT8);

			geomID = rtcNewTriangleMesh( mScene, RTC_GEOMETRY_DYNAMIC, numTriangles, numVertices, 1);

			EmbreeTriangle* triangles = (EmbreeTriangle*)rtcMapBuffer( mScene, geomID, RTC_INDEX_BUFFER);

			rtcUnmapBuffer( mScene, geomID, RTC_INDEX_BUFFER);

			EmbreeVertex* vertices = (EmbreeVertex*)rtcMapBuffer( mScene, geomID, RTC_VERTEX_BUFFER);

			std::vector< unsigned int > indices;

			for (int tt = 0; tt < numTriangles; tt++)
			{

				triangles[tt].v0 = volinfo.mFaceIndices[3 * tt];
				triangles[tt].v1 = volinfo.mFaceIndices[3 * tt + 1];
				triangles[tt].v2 = volinfo.mFaceIndices[3 * tt + 2];

			}

			for (int pp = 0; pp < numVertices; pp++)
			{

				vertices[pp].x = volinfo.mVertices[pp](0);
				vertices[pp].y = volinfo.mVertices[pp](1);
				vertices[pp].z = volinfo.mVertices[pp](2);
				vertices[pp].a = 1.0;

			}

			rtcUnmapBuffer(mScene, geomID, RTC_VERTEX_BUFFER);

			rtcCommit(mScene);

			float maxLength = volinfo.mVoxelStep.norm() * volinfo.mWidth;

			//std::vector< float > wallThickness(numVertices, -1);

			volinfo.mWallthickness.resize(numVertices);

			std::fill(volinfo.mWallthickness.begin(), volinfo.mWallthickness.end(), -1);

			double initT = cv::getTickCount();

			float near = volinfo.mVoxelStep.norm() * 0.01;

#pragma omp parallel for
			for (int vv = 0; vv < numVertices + 8; vv += 8)
			{
				RTCRay8 ray;


				for (int ii = 0; ii < 8; ii++)
				{
					int id;

					if ((vv + ii) >= numVertices)
					{
						id = numVertices - 1;
					}
					else
					{
						id = vv + ii;
					}

					ray.orgx[ii] = volinfo.mVertices[id](0);
					ray.orgy[ii] = volinfo.mVertices[id](1);
					ray.orgz[ii] = volinfo.mVertices[id](2);
				}


				for (int ii = 0; ii < 8; ii++)
				{
					Eigen::Vector3f dir;

					int id;

					if ((vv + ii) >= numVertices)
					{
						id = numVertices - 1;
					}
					else
					{
						id = vv + ii;
					}

					ray.dirx[ii] = -volinfo.mVertexNormals[id](0);
					ray.diry[ii] = -volinfo.mVertexNormals[id](1);
					ray.dirz[ii] = -volinfo.mVertexNormals[id](2);

					ray.geomID[ii] = RTC_INVALID_GEOMETRY_ID;
					ray.instID[ii] = RTC_INVALID_GEOMETRY_ID;
					ray.primID[ii] = RTC_INVALID_GEOMETRY_ID;

					ray.tnear[ii] = near;
					ray.tfar[ii] = maxLength;

					ray.mask[ii] = 0xFFFFFFFF;
					ray.time[ii] = 0.f;
				}

				__aligned(32) int valid8[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };

				rtcIntersect8(valid8, mScene, ray);

				for (int ii = 0; ii < 8; ii++)
				{
					if ((vv + ii) >= numVertices)
					{
						continue;
					}

					if ( ray.primID[ii] != RTC_INVALID_GEOMETRY_ID )
					{
						volinfo.mWallthickness[vv + ii] = ray.tfar[ii];

						volinfo.mOppVertices[vv + ii](0) = ray.orgx[ii] + ray.tfar[ii] * ray.dirx[ii];
						volinfo.mOppVertices[vv + ii](1) = ray.orgy[ii] + ray.tfar[ii] * ray.diry[ii];
						volinfo.mOppVertices[vv + ii](2) = ray.orgz[ii] + ray.tfar[ii] * ray.dirz[ii];

						volinfo.mOppositeFaceIndices[vv + ii] = ray.primID[ii];
					}
				}

			}


			volinfo.mVertexColors.resize(numVertices, Eigen::Vector3f(1, 1, 1));
			volinfo.mRayVertexColors.resize(numVertices, Eigen::Vector3f(1, 1, 1));

			std::fill(volinfo.mVertexColors.begin(), volinfo.mVertexColors.end(), Eigen::Vector3f(1, 1, 1));
			std::fill(volinfo.mRayVertexColors.begin(), volinfo.mRayVertexColors.end(), Eigen::Vector3f(1, 1, 1));

			if ( volinfo.mRayVertexColors.size() != volinfo.mSphereVertexColors.size() )
			{
				volinfo.mSphereVertexColors.resize(volinfo.mRayVertexColors.size(), Eigen::Vector3f(1, 1, 1));
			}

			double thickness = 0;
			int count = 0;

			for (int vv = 0; vv < numVertices; vv++)
			{
				if (volinfo.mWallthickness[vv] < 0)
					continue;

				if (volinfo.mWallthickness[vv] < 2.5)
				{
					volinfo.mVertexColors[vv](0) = 1 - volinfo.mWallthickness[vv] / 2.5;
					volinfo.mVertexColors[vv](1) = volinfo.mWallthickness[vv] / 2.5;
					volinfo.mVertexColors[vv](2) = 0;

					volinfo.mRayVertexColors[vv](0) = 1 - volinfo.mWallthickness[vv] / 2.5;
					volinfo.mRayVertexColors[vv](1) = volinfo.mWallthickness[vv] / 2.5;
					volinfo.mRayVertexColors[vv](2) = 0;

					
				}
				else if (volinfo.mWallthickness[vv] > 2.5 && volinfo.mWallthickness[vv] < 5)
				{
					volinfo.mVertexColors[vv](0) = 0;
					volinfo.mVertexColors[vv](1) = 1 - volinfo.mWallthickness[vv] / 2.5;
					volinfo.mVertexColors[vv](2) = volinfo.mWallthickness[vv] / 2.5;


					volinfo.mRayVertexColors[vv](0) = 0;
					volinfo.mRayVertexColors[vv](1) = 1 - volinfo.mWallthickness[vv] / 2.5;
					volinfo.mRayVertexColors[vv](2) = volinfo.mWallthickness[vv] / 2.5;
				}

				thickness += volinfo.mWallthickness[vv];
				count++;

			}

			volinfo.mWallthicknessDataChanged = true;

			volinfo.updateSurfaceData();
#endif


		}



		void WallthicknessEstimator::collectFaces( Eigen::Vector3f& center , const Eigen::Vector3f& rayDir, const float& coeff, int faceId, std::vector< int >& collectedFaces ,
			                                       std::vector< uchar >& faceMask , float& maxDist , float& minDist , int& maxDistFaceId , int& minDistFaceId )
		{
			collectedFaces.push_back(faceId);

			faceMask[faceId] = 1;

			std::vector< int > nextRingFaces;

			for (auto it = mFaceAdjacency[faceId].begin(); it != mFaceAdjacency[faceId].end(); it++)
			{
				Eigen::Vector3f dir = mFaceCenters[ *it ] - center;
				dir.normalize();

				if ( dir.dot(rayDir) > coeff )
				{
					nextRingFaces.push_back(*it);

					collectedFaces.push_back(*it);
					faceMask[*it] = 1;
				}
			}

			while ( nextRingFaces.size() > 0 )
			{
				for ( auto it = nextRingFaces.begin(); it != nextRingFaces.end(); it++ )
				{
					int fid = *it;

					for (auto it = mFaceAdjacency[faceId].begin(); it != mFaceAdjacency[faceId].end(); it++)
					{
						if (faceMask[*it])
							continue;

						Eigen::Vector3f dir = mFaceCenters[*it] - center;
						dir.normalize();

						if (dir.dot(rayDir) > coeff)
						{

							nextRingFaces.push_back(*it);

							collectedFaces.push_back(*it);

							faceMask[*it] = 1;
						}
					}


				}
			}


			for (auto it = collectedFaces.begin(); it != collectedFaces.end(); it++)
			{
				faceMask[*it] = 0;
			}


		}


		void WallthicknessEstimator::computeBrepRayTraceThicknessMin(VolumeInfo& volinfo)
		{

#if 0
			std::atomic< int > nValidThicknesses = 0;

			_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
			_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

			rtcInit(NULL);

			RTCScene scene;

			unsigned int geomID;

			RTCDevice device;

			scene = 0;

			device = rtcNewDevice(NULL);

			long int numTriangles = volinfo.mFaceIndices.size() / 3;
			long int numVertices = volinfo.mVertices.size();

			scene = rtcDeviceNewScene(device, RTC_SCENE_DYNAMIC, RTC_INTERSECT8);

			geomID = rtcNewTriangleMesh(scene, RTC_GEOMETRY_DYNAMIC, numTriangles, numVertices, 1);

			EmbreeTriangle* triangles = (EmbreeTriangle*)rtcMapBuffer(scene, geomID, RTC_INDEX_BUFFER);

			rtcUnmapBuffer(scene, geomID, RTC_INDEX_BUFFER);

			EmbreeVertex* vertices = (EmbreeVertex*)rtcMapBuffer(scene, geomID, RTC_VERTEX_BUFFER);

			std::vector< unsigned int > indices;

			for (int tt = 0; tt < numTriangles; tt++)
			{

				triangles[tt].v0 = volinfo.mFaceIndices[3 * tt];
				triangles[tt].v1 = volinfo.mFaceIndices[3 * tt + 1];
				triangles[tt].v2 = volinfo.mFaceIndices[3 * tt + 2];

			}

			for (int pp = 0; pp < numVertices; pp++)
			{

				vertices[pp].x = volinfo.mVertices[pp](0);
				vertices[pp].y = volinfo.mVertices[pp](1);
				vertices[pp].z = volinfo.mVertices[pp](2);
				vertices[pp].a = 1.0;

			}

			rtcUnmapBuffer(scene, geomID, RTC_VERTEX_BUFFER);

			rtcCommit(scene);

			float maxLength = volinfo.mVoxelStep.norm() * volinfo.mWidth;

			//std::vector< float > wallThickness(numVertices, -1);

			volinfo.mWallthickness.resize(numVertices);

			std::fill(volinfo.mWallthickness.begin(), volinfo.mWallthickness.end(), -1);

			double initT = cv::getTickCount();

			float near = volinfo.mVoxelStep.norm() * 0.01;

			std::vector< int > closestTriangles( numVertices , -1 );

			

#pragma omp parallel for
			for (int vv = 0; vv < numVertices; vv += 8)
			{
				RTCRay8 ray;


				for (int ii = 0; ii < 8; ii++)
				{
					int id;

					if ((vv + ii) >= numVertices)
					{
						id = numVertices - 1;
					}
					else
					{
						id = vv + ii;
					}

					ray.orgx[ii] = volinfo.mVertices[id](0);
					ray.orgy[ii] = volinfo.mVertices[id](1);
					ray.orgz[ii] = volinfo.mVertices[id](2);
				}


				for (int ii = 0; ii < 8; ii++)
				{
					Eigen::Vector3f dir;

					int id;

					if ((vv + ii) >= numVertices)
					{
						id = numVertices - 1;
					}
					else
					{
						id = vv + ii;
					}

					ray.dirx[ii] = -volinfo.mVertexNormals[id](0);
					ray.diry[ii] = -volinfo.mVertexNormals[id](1);
					ray.dirz[ii] = -volinfo.mVertexNormals[id](2);

					ray.geomID[ii] = RTC_INVALID_GEOMETRY_ID;
					ray.instID[ii] = RTC_INVALID_GEOMETRY_ID;
					ray.primID[ii] = RTC_INVALID_GEOMETRY_ID;

					ray.tnear[ii] = near;
					ray.tfar[ii] = maxLength;

					ray.mask[ii] = 0xFFFFFFFF;
					ray.time[ii] = 0.f;
				}

				__aligned(32) int valid8[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };

				rtcIntersect8(valid8, scene, ray);

				for (int ii = 0; ii < 8; ii++)
				{
					if ((vv + ii) >= numVertices)
					{
						continue;
					}

					if (ray.primID[ii] != RTC_INVALID_GEOMETRY_ID)
					{
						volinfo.mWallthickness[vv + ii] = ray.tfar[ii];// 0.4 + (ray.tfar[ii] - 0.4) * 0.8; //ray.primID[ii];

						closestTriangles[vv + ii] = ray.primID[ii];

						nValidThicknesses++;
					}
				}

			}


			std::cout << " valid thicknesses " << nValidThicknesses << std::endl;

#if 1
			std::cout << " vertex incidence size : " << volinfo.mVertexIncidence.size() << " " << numVertices << std::endl;


			initT = cv::getTickCount();

			std::vector< unsigned char > vertexMask(numVertices, 0);

			std::vector< int > checkvertexIds, setMasks, newCheckIds;

			checkvertexIds.reserve(10000);
			setMasks.reserve(10000);
			newCheckIds.reserve(10000);

			float threshold = sqrt(3.0) / 2.0;

			for (int vv = 0; vv < 0; vv++)//numVertices
			{
				int itid = closestTriangles[vv];

				if (itid < 0)
					continue;

				checkvertexIds.push_back(volinfo.mFaceIndices[3 * itid]);
				checkvertexIds.push_back(volinfo.mFaceIndices[3 * itid + 1]);
				checkvertexIds.push_back(volinfo.mFaceIndices[3 * itid + 2]);

				setMasks.push_back(volinfo.mFaceIndices[3 * itid]);
				setMasks.push_back(volinfo.mFaceIndices[3 * itid + 1]);
				setMasks.push_back(volinfo.mFaceIndices[3 * itid + 2]);

				Eigen::Vector3f refPoint = volinfo.mVertices[vv];

				float closestDistance = volinfo.mWallthickness[vv];

				if (closestDistance < 0)
					continue;

				while ( checkvertexIds.size() > 0 )
				{
					for (auto it = checkvertexIds.begin(); it != checkvertexIds.end(); it++)
					{
						int vId = *it;

						if (vertexMask[vId])
							continue;

						vertexMask[vId] = 1;
						setMasks.push_back(vId);

						Eigen::Vector3f vec = volinfo.mVertices[vId] - refPoint;

						vec.normalize();

						float coeff = volinfo.mVertexNormals[vId].dot(vec);

						if ( coeff > threshold )
						{
							closestDistance = std::min(closestDistance, (refPoint - volinfo.mVertices[vId]).norm());

							for (auto it2 = volinfo.mVertexIncidence[vId].begin(); it2 != volinfo.mVertexIncidence[vId].end(); it2++)
							{
								int vId2 = *it2;

								if ( vertexMask[vId2] )
									continue;

								newCheckIds.push_back(vId2);
							}
						}

					}

					checkvertexIds = newCheckIds;

					newCheckIds.clear();

					//std::swap(checkvertexIds, newCheckIds);
				}


				for (auto it = setMasks.begin(); it != setMasks.end(); it++)
				{
					vertexMask[*it] = 0;
				}

				setMasks.clear();
				

				volinfo.mWallthickness[vv] = closestDistance;
			}

#endif

			std::cout << " time spent in angle range integration " << (cv::getTickCount() - initT) / cv::getTickFrequency() << std::endl;

			volinfo.mVertexColors.resize(numVertices, Eigen::Vector3f(1, 1, 1));
			volinfo.mSphereVertexColors.resize(numVertices, Eigen::Vector3f(1, 1, 1));
			volinfo.mRayVertexColors.resize(numVertices, Eigen::Vector3f(1, 1, 1));

			std::fill(volinfo.mVertexColors.begin(), volinfo.mVertexColors.end(), Eigen::Vector3f(1, 1, 1));
			std::fill(volinfo.mSphereVertexColors.begin(), volinfo.mSphereVertexColors.end(), Eigen::Vector3f(1, 1, 1));

			double thickness = 0;
			int count = 0;

			float halfRange = 0.3;

			for (int vv = 0; vv < numVertices; vv++)
			{
				if (volinfo.mWallthickness[vv] < 0)
					continue;

				if (volinfo.mWallthickness[vv] < halfRange)
				{
					volinfo.mRayVertexColors[vv](0) = 0;
					volinfo.mRayVertexColors[vv](1) = volinfo.mWallthickness[vv] / halfRange;
					volinfo.mRayVertexColors[vv](2) = 1 - volinfo.mWallthickness[vv] / halfRange;

				}
				else if (volinfo.mWallthickness[vv] > halfRange && volinfo.mWallthickness[vv] < 2 * halfRange )
				{
					volinfo.mRayVertexColors[vv](0) = volinfo.mWallthickness[vv] / halfRange;
					volinfo.mRayVertexColors[vv](1) = 1 - volinfo.mWallthickness[vv] / halfRange;
					volinfo.mRayVertexColors[vv](2) = 0;

				}

				thickness += volinfo.mWallthickness[vv];
				count++;

			}

			volinfo.mWallthicknessDataChanged = true;
#endif

		}



		void WallthicknessEstimator::computeBrepSphereThickness(VolumeInfo& volinfo)
		{

#if 0

			std::cout << " computing distance transform " << std::endl;

			double initT = cv::getTickCount();

			computeDistanceTransform(volinfo);

			std::cout << " distance transform computation finished in "<<( cv::getTickCount() - initT ) / cv::getTickFrequency() <<"  seconds " << std::endl;

			long int numTriangles = volinfo.mFaceIndices.size() / 3;
			long int numVertices = volinfo.mVertices.size();

			float step = volinfo.mVoxelStep.norm();

			volinfo.mWallthickness.resize(numVertices);

			std::fill(volinfo.mWallthickness.begin(), volinfo.mWallthickness.end(), -1);

			long int numValidWidths = 0;

			for ( int vv = 0; vv < numVertices; vv++ )
			{

				Eigen::Vector3f dir = volinfo.mVertexNormals[vv];

				Voxel initVox;

				initVox.x = (volinfo.mVertices[vv](0) - volinfo.mVolumeOrigin(0) ) / volinfo.mVoxelStep(0); //+ dir(0) * step * 0.05
				initVox.y = (volinfo.mVertices[vv](1) - volinfo.mVolumeOrigin(1)) / volinfo.mVoxelStep(1); //+ dir(1) * step * 0.05
				initVox.z = (volinfo.mVertices[vv](2) - volinfo.mVolumeOrigin(2)) / volinfo.mVoxelStep(2); //+ dir(2) * step * 0.05

				LineIterator it(dir, initVox, volinfo.mVolumeOrigin, volinfo.mVoxelStep);

				long int zStep = volinfo.mWidth * volinfo.mHeight;
				long int yStep = volinfo.mWidth;

				float initDist = volinfo.mDistanceTransform[zStep * initVox.z + yStep * initVox.y + initVox.x];

				float prevDist = initDist;
				float currentDist = initDist;

				bool medialAxisFound = false;

				while (true)
				{
					Voxel vox = it.next();

					prevDist = currentDist;

					if ( vox.x < 0 || vox.x >= volinfo.mWidth  ||
						 vox.y < 0 || vox.y >= volinfo.mHeight ||
						 vox.z < 0 || vox.z >= volinfo.mDepth    )
					{
						break;
					}

					float currentDist = volinfo.mDistanceTransform[zStep * vox.z + yStep * vox.y + vox.x];

					if (currentDist < 0)
					{
						continue;
					}
						

					if ( currentDist < prevDist  )
					{
						medialAxisFound = true;

						break;
					}

				}

				if ( currentDist > 0 )
				{
					numValidWidths++;
				}

				if ( medialAxisFound && prevDist > 0 )
				{
					volinfo.mWallthickness[vv] = 2 * prevDist / volinfo.mVoxelStep( 0 );
				}

			}


			std::cout << " num valid widths : " << numVertices << " " << numValidWidths << std::endl;

			volinfo.mVertexColors.resize(numVertices, Eigen::Vector3f(1, 1, 1));
			volinfo.mSphereVertexColors.resize(numVertices, Eigen::Vector3f(1, 1, 1));

			std::fill(volinfo.mVertexColors.begin(), volinfo.mVertexColors.end(), Eigen::Vector3f(1, 1, 1));
			std::fill(volinfo.mRayVertexColors.begin(), volinfo.mRayVertexColors.end(), Eigen::Vector3f(1, 1, 1));

			if (volinfo.mRayVertexColors.size() != volinfo.mSphereVertexColors.size())
			{
				volinfo.mRayVertexColors.resize(volinfo.mSphereVertexColors.size(), Eigen::Vector3f(1, 1, 1));
			}


			double thickness = 0;
			int count = 0;

			for (int vv = 0; vv < numVertices; vv++)
			{
				if (volinfo.mWallthickness[vv] < 0)
					continue;

				if (volinfo.mWallthickness[vv] < 2.5)
				{
					volinfo.mVertexColors[vv](0) = 1 - volinfo.mWallthickness[vv] / 2.5;
					volinfo.mVertexColors[vv](1) = volinfo.mWallthickness[vv] / 2.5;
					volinfo.mVertexColors[vv](2) = 0;

					volinfo.mSphereVertexColors[vv](0) = 1 - volinfo.mWallthickness[vv] / 2.5;
					volinfo.mSphereVertexColors[vv](1) = volinfo.mWallthickness[vv] / 2.5;
					volinfo.mSphereVertexColors[vv](2) = 0;


				}
				else if (volinfo.mWallthickness[vv] > 2.5 && volinfo.mWallthickness[vv] < 5)
				{
					volinfo.mVertexColors[vv](0) = 0;
					volinfo.mVertexColors[vv](1) = 1 - volinfo.mWallthickness[vv] / 2.5;
					volinfo.mVertexColors[vv](2) = volinfo.mWallthickness[vv] / 2.5;


					volinfo.mSphereVertexColors[vv](0) = 0;
					volinfo.mSphereVertexColors[vv](1) = 1 - volinfo.mWallthickness[vv] / 2.5;
					volinfo.mSphereVertexColors[vv](2) = volinfo.mWallthickness[vv] / 2.5;
				}

				thickness += volinfo.mWallthickness[vv];
				count++;

			}

			volinfo.mWallthicknessDataChanged = true;

			volinfo.mHasDistanceTransformData = true;
#endif

		}


		void WallthicknessEstimator::aabbtreeDemo()
		{

			std::vector< Voxel > surfaceVoxels;


			//first find the surface voxels
			collectSurfaceVoxels(surfaceVoxels);

			std::cout << " number of surface voxels : " << surfaceVoxels.size() << std::endl;

			AABBTree tree( surfaceVoxels );

			//std::cout << " building aabb tree " << std::endl;
			tree.build();

			//std::cout << " aabb tree construction finished " << std::endl;



		}



		void WallthicknessEstimator::normalEstimationDemo()
		{
			std::vector< Voxel > surfaceVoxels;

			//first find the surface voxels
			collectSurfaceVoxels(surfaceVoxels);

			std::vector< Eigen::Vector3f > normals;

			computeNormals(surfaceVoxels, normals);



		}



		void WallthicknessEstimator::rayTracingDemo()
		{
			std::vector< Voxel > surfaceVoxels;

			//first find the surface voxels
			collectSurfaceVoxels(surfaceVoxels);

			std::vector< Eigen::Vector3f > normals;

			//computeNormals( surfaceVoxels , normals );

			std::vector< float > thicknesses;

			std::cout << " computing ray tracing " << std::endl;

			rayTrace2( surfaceVoxels , thicknesses );


			
		}


		void WallthicknessEstimator::lineVoxelIteratorDemo()
		{
			std::vector< Voxel > surfaceVoxels;

			//first find the surface voxels
			collectSurfaceVoxels(surfaceVoxels);

			std::vector< Eigen::Vector3f > normals;

			computeNormals( surfaceVoxels , normals );

			int nSurfaceVoxels = surfaceVoxels.size();

			//for (int ss = 0; ss < nSurfaceVoxels; ss++)
			{
				int ss = nSurfaceVoxels / 2;

				Eigen::Vector3f dir;

				std::cout << normals[ss].transpose() << std::endl;

				LineIterator li( normals[ ss ] , surfaceVoxels[ss] , mVolInfo.mVolumeOrigin , mVolInfo.mVoxelStep ) ;

				std::cout << surfaceVoxels[ss].x << " " << surfaceVoxels[ss].y << " " << surfaceVoxels[ss].z << std::endl;

				std::vector< Eigen::Vector3f > voxelPoints , voxelColors;

				

				for ( int ii = 0; ii < 70; ii++ )
				{
					Voxel vox = li.next();

					Eigen::Vector3f point;

					point(0) = vox.x * mVolInfo.mVoxelStep(0) + mVolInfo.mVolumeOrigin(0);
					point(1) = vox.y * mVolInfo.mVoxelStep(1) + mVolInfo.mVolumeOrigin(1);
					point(2) = vox.z * mVolInfo.mVoxelStep(2) + mVolInfo.mVolumeOrigin(2);
					
					voxelPoints.push_back(point);
					voxelColors.push_back(Eigen::Vector3f(1, 0, 0));

				}


				Eigen::Vector3f dir2 = voxelPoints[49] - voxelPoints[0];

				dir2.normalize();

				std::cout << dir2.transpose() << std::endl;


				tr::Display3DRoutines::displayPointSet(voxelPoints, voxelColors);


			}


		}

		void WallthicknessEstimator::initSurfaceVoxels( unsigned short* volume , std::vector< Voxel >& initSurfaceVoxels )
		{
			int numVertices = mVolInfo.mVertices.size();

			float invStepX = 1.0 / mVolInfo.mVoxelStep(0);
			float invStepY = 1.0 / mVolInfo.mVoxelStep(1);
			float invStepZ = 1.0 / mVolInfo.mVoxelStep(2);

			size_t zStep = mVolInfo.mWidth * mVolInfo.mHeight;
			size_t yStep = mVolInfo.mWidth;

			float minThickness = 0;
			float maxThickness = 5.0;

			float diffTh = maxThickness - minThickness;

			std::vector< Voxel > surfaceVoxels;

			for ( int vv = 0; vv < numVertices; vv++ )
			{
				Eigen::Vector3f diff = mVolInfo.mVertices[vv] - mVolInfo.mVolumeOrigin;

				int ix = diff(0) * invStepX;
				int iy = diff(1) * invStepY;
				int iz = diff(2) * invStepZ;

				size_t id = iz * zStep + iy * yStep + ix;

				volume[id] = ( unsigned short )( ( ( mVolInfo.mWallthickness[vv] - minThickness ) / diffTh ) * USHRT_MAX );

				Voxel vox;

				vox.x = ix;
				vox.y = iy;
				vox.z = iz;
				vox.mInfo = volume[id];

				surfaceVoxels.push_back(vox);
			}

			
		}


		void WallthicknessEstimator::propagateThickness( unsigned short* volume , std::vector< Voxel >& initVoxels )
		{

			std::vector< Voxel > prevLayer = initVoxels;
			std::vector< Voxel > collectedNeighbors;

			size_t zStep = mVolInfo.mWidth * mVolInfo.mHeight;
			size_t yStep = mVolInfo.mWidth;

			unsigned short *vData = ( unsigned short* )mVolInfo.mVolumeData;
			unsigned short *thicknessVData = volume;

			while ( 1 )
			{
				//first collect extension candidates
				for (auto voxel : prevLayer)
				{
					for (int xx = voxel.x - 1; xx <= voxel.x + 1; xx++)
						for (int yy = voxel.y - 1; yy <= voxel.y + 1; yy++)
							for (int zz = voxel.z - 1; zz <= voxel.z; zz++)
							{
								if ( zz == voxel.z && yy == voxel.y && xx == voxel.x )
									      continue;

								size_t vId = zz * zStep + yy * yStep + xx;

								if ( volume[vId] >= mVolInfo.mAirVoxelFilterVal )
								{
									Voxel vx;

									vx.x = xx;
									vx.y = yy;
									vx.z = zz;

									collectedNeighbors.push_back(vx);

								}
							}
				}


#define performCheck( sign1 , sign2 , sign3 ) id = voxelIdCoords(vox.x sign1 , vox.y sign2 , vox.z sign3);\
				                              if (thicknessVData[id] > 1)\
				                              {\
					                             minVal = std::min(thicknessVData[id], minVal);\
					                             valFound = true;\
				                               }


				//update the collected neighbors
				for ( auto vox : collectedNeighbors )
				{

					unsigned short minVal = USHRT_MAX;

					bool valFound = false;

					size_t id = -1;
					
					//first check the six voxels ( closest voxels )
					performCheck( +1 , , )


					/*id = voxelIdCoords(vox.x - 1, vox.y, vox.z);

					if ( thicknessVData[id] > 1 )
					{
						minVal = std::min(thicknessVData[id], minVal);

						valFound = true;
					}

					id = voxelIdCoords(vox.x + 1, vox.y, vox.z);
					
					if (thicknessVData[id] > 1)
					{
						minVal = std::min(thicknessVData[id], minVal);

						valFound = true;
					}

					id = voxelIdCoords(vox.x , vox.y - 1, vox.z);

					if (thicknessVData[id] > 1)
					{
						minVal = std::min(thicknessVData[id], minVal);

						valFound = true;
					}

					id = voxelIdCoords(vox.x, vox.y + 1, vox.z);

					if (thicknessVData[id] > 1)
					{
						minVal = std::min(thicknessVData[id], minVal);

						valFound = true;
					}

					id = voxelIdCoords(vox.x, vox.y , vox.z - 1 );

					if (thicknessVData[id] > 1)
					{
						minVal = std::min(thicknessVData[id], minVal);

						valFound = true;
					}

					id = voxelIdCoords(vox.x, vox.y, vox.z + 1);

					if (thicknessVData[id] > 1)
					{
						minVal = std::min(thicknessVData[id], minVal);

						valFound = true;
					}

					if (valFound)
					{
						thicknessVData[voxelId(vox)] = minVal;
						continue;
					}*/
						
					//now check edge corner voxels ( second closest voxels )



					//now check the point corner voxels( farthest voxels )
				}
			}
		}


		void
			computeRoots2(const float& b, const float& c, Eigen::Vector3f& roots)
		{
			roots(0) = 0;
			float d = (b * b - 4.0 * c);
			if (d < 0.0)  // no real roots ! THIS SHOULD NOT HAPPEN!
				d = 0.0;

			float sd = ::std::sqrt(d);

			roots(2) = 0.5f * (b + sd);
			roots(1) = 0.5f * (b - sd);
		}

		void
			computeRoots(const Eigen::Matrix3f& m, Eigen::Vector3f& roots)
		{
			typedef float Scalar;

			// The characteristic equation is x^3 - c2*x^2 + c1*x - c0 = 0.  The
			// eigenvalues are the roots to this equation, all guaranteed to be
			// real-valued, because the matrix is symmetric.
			Scalar c0 = m(0, 0) * m(1, 1) * m(2, 2)
				+ Scalar(2) * m(0, 1) * m(0, 2) * m(1, 2)
				- m(0, 0) * m(1, 2) * m(1, 2)
				- m(1, 1) * m(0, 2) * m(0, 2)
				- m(2, 2) * m(0, 1) * m(0, 1);

			Scalar c1 = m(0, 0) * m(1, 1) -
				m(0, 1) * m(0, 1) +
				m(0, 0) * m(2, 2) -
				m(0, 2) * m(0, 2) +
				m(1, 1) * m(2, 2) -
				m(1, 2) * m(1, 2);
			Scalar c2 = m(0, 0) + m(1, 1) + m(2, 2);

			if (fabs(c0) < Eigen::NumTraits < Scalar > ::epsilon())  // one root is 0 -> quadratic equation
				computeRoots2(c2, c1, roots);
			else
			{
				const Scalar s_inv3 = Scalar(1.0 / 3.0);
				const Scalar s_sqrt3 = std::sqrt(Scalar(3.0));
				// Construct the parameters used in classifying the roots of the equation
				// and in solving the equation for the roots in closed form.
				Scalar c2_over_3 = c2 * s_inv3;
				Scalar a_over_3 = (c1 - c2 * c2_over_3) * s_inv3;
				if (a_over_3 > Scalar(0))
					a_over_3 = Scalar(0);

				Scalar half_b = Scalar(0.5) * (c0 + c2_over_3 * (Scalar(2) * c2_over_3 * c2_over_3 - c1));

				Scalar q = half_b * half_b + a_over_3 * a_over_3 * a_over_3;
				if (q > Scalar(0))
					q = Scalar(0);

				// Compute the eigenvalues by solving for the roots of the polynomial.
				Scalar rho = std::sqrt(-a_over_3);
				Scalar theta = std::atan2(std::sqrt(-q), half_b) * s_inv3;
				Scalar cos_theta = std::cos(theta);
				Scalar sin_theta = std::sin(theta);
				roots(0) = c2_over_3 + Scalar(2) * rho * cos_theta;
				roots(1) = c2_over_3 - rho * (cos_theta + s_sqrt3 * sin_theta);
				roots(2) = c2_over_3 - rho * (cos_theta - s_sqrt3 * sin_theta);

				// Sort in increasing order.
				if (roots(0) >= roots(1))
					std::swap(roots(0), roots(1));
				if (roots(1) >= roots(2))
				{
					std::swap(roots(1), roots(2));
					if (roots(0) >= roots(1))
						std::swap(roots(0), roots(1));
				}

				if (roots(0) <= 0)  // eigenval for symetric positive semi-definite matrix can not be negative! Set it to 0
					computeRoots2(c2, c1, roots);
			}
		}


		//////////////////////////////////////////////////////////////////////////////////////////
		void eigen33(const Eigen::Matrix3f& mat, float& eigenvalue, Eigen::Vector3f& eigenvector)
		{
			// Scale the matrix so its entries are in [-1,1].  The scaling is applied
			// only when at least one matrix entry has magnitude larger than 1.

			float scale = mat.cwiseAbs().maxCoeff();

			if (scale <= std::numeric_limits < float > ::min())
				scale = 1.0;

			Eigen::Matrix3f scaledMat = mat / scale;

			Eigen::Vector3f eigenvalues;

			computeRoots(scaledMat, eigenvalues);

			eigenvalue = eigenvalues(0) * scale;

			scaledMat.diagonal().array() -= eigenvalues(0);

			Eigen::Vector3f vec1 = scaledMat.row(0).cross(scaledMat.row(1));
			Eigen::Vector3f vec2 = scaledMat.row(0).cross(scaledMat.row(2));
			Eigen::Vector3f vec3 = scaledMat.row(1).cross(scaledMat.row(2));

			float len1 = vec1.squaredNorm();
			float len2 = vec2.squaredNorm();
			float len3 = vec3.squaredNorm();

			if (len1 >= len2 && len1 >= len3)
				eigenvector = vec1 / std::sqrt(len1);
			else if (len2 >= len1 && len2 >= len3)
				eigenvector = vec2 / std::sqrt(len2);
			else
				eigenvector = vec3 / std::sqrt(len3);
		}

		//////////////////////////////////////////////////////////////////////////////////////////////
		void
			solvePlaneParameters(const Eigen::Matrix3f &covariance_matrix,
			float &nx, float &ny, float &nz, float &curvature)
		{
			// Avoid getting hung on Eigen's optimizers
			//  for (int i = 0; i < 9; ++i)
			//    if (!pcl_isfinite (covariance_matrix.coeff (i)))
			//    {
			//      //PCL_WARN ("[pcl::solvePlaneParameteres] Covariance matrix has NaN/Inf values!\n");
			//      nx = ny = nz = curvature = std::numeric_limits<float>::quiet_NaN ();
			//      return;
			//    }
			// Extract the smallest eigenvalue and its eigenvector
			EIGEN_ALIGN16 Eigen::Vector3f::Scalar eigen_value;
			EIGEN_ALIGN16 Eigen::Vector3f eigen_vector;

			eigen33(covariance_matrix, eigen_value, eigen_vector);

			nx = eigen_vector[0];
			ny = eigen_vector[1];
			nz = eigen_vector[2];

			// Compute the curvature surface change
			float eig_sum = covariance_matrix.coeff(0) + covariance_matrix.coeff(4) + covariance_matrix.coeff(8);

			if (eig_sum != 0)
				curvature = std::fabs(eigen_value / eig_sum);
			else
				curvature = 0;
		}


		//////////////////////////////////////////////////////////////////////////////////////////////
		unsigned int
			computeMeanAndCovarianceMatrix(const std::vector< Eigen::Vector3f> &cloud,
			Eigen::Matrix3f &covariance_matrix,
			Eigen::Vector4f &centroid)
		{
			typedef float Scalar;

			// create the buffer on the stack which is much faster than using cloud[indices[i]] and centroid as a buffer
			Eigen::Matrix<Scalar, 1, 9, Eigen::RowMajor> accu = Eigen::Matrix<Scalar, 1, 9, Eigen::RowMajor>::Zero();
			size_t point_count;

			point_count = cloud.size();

			// For each point in the cloud
			for (size_t i = 0; i < point_count; ++i)
			{
				accu[0] += cloud[i].x() * cloud[i].x();
				accu[1] += cloud[i].x() * cloud[i].y();
				accu[2] += cloud[i].x() * cloud[i].z();
				accu[3] += cloud[i].y() * cloud[i].y(); // 4
				accu[4] += cloud[i].y() * cloud[i].z(); // 5
				accu[5] += cloud[i].z() * cloud[i].z(); // 8
				accu[6] += cloud[i].x();
				accu[7] += cloud[i].y();
				accu[8] += cloud[i].z();
			}


			accu /= static_cast<Scalar> (point_count);

			if (point_count != 0)
			{
				//centroid.head<3> () = accu.tail<3> ();    -- does not compile with Clang 3.0
				centroid(0) = accu[6]; centroid(1) = accu[7]; centroid(2) = accu[8];
				centroid(3) = 1;
				covariance_matrix.coeffRef(0) = accu[0] - accu[6] * accu[6];
				covariance_matrix.coeffRef(1) = accu[1] - accu[6] * accu[7];
				covariance_matrix.coeffRef(2) = accu[2] - accu[6] * accu[8];
				covariance_matrix.coeffRef(4) = accu[3] - accu[7] * accu[7];
				covariance_matrix.coeffRef(5) = accu[4] - accu[7] * accu[8];
				covariance_matrix.coeffRef(8) = accu[5] - accu[8] * accu[8];
				covariance_matrix.coeffRef(3) = covariance_matrix.coeff(1);
				covariance_matrix.coeffRef(6) = covariance_matrix.coeff(2);
				covariance_matrix.coeffRef(7) = covariance_matrix.coeff(5);
			}

			return (static_cast<unsigned int> (point_count));
		}




	}


}