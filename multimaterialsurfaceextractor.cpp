#include "multimaterialsurfaceextractor.h"
#include "opencvincludes.h"
#include "ipp.h"
#include "ippvm.h"
#include <iostream>
#include <atomic>
#include "display3droutines.h"
#include "poissonmeshgenerator.h"

namespace imt 
{

	namespace volume
	{


		VolumeOctree::VolumeOctree(int w, int h, int d, std::vector<Eigen::Vector3f>& points, std::vector<Eigen::Vector3f>& normals) : 
			mWidth(w) , mHeight(h) , mDepth(d) , mPoints(points) , mNormals(normals)
		{
			buildVolumeOctree();
		}


		void VolumeOctree::buildVolumeOctree()
		{
			int maxDim = std::max( std::max(mWidth, mHeight) , mDepth);

			//compute the min power of 2 which is greater than maxDim

			int dim = 1;

			int md = maxDim;

			while (md > 0)
			{
				dim *= 2;

				md /= 2;
			}

			//std::cout << "maxDim and dim : " << maxDim << " " << dim << std::endl;

			//we now build the octree

				
		}

		void VolumeOctree::buildNode( OctreeNode* node )
		{
			if (node->mPointIndices.size() <= 1)
			{
				node->mIsLeaf = true;

				return;
			}

			int xi = node->xi , yi = node->yi, zi = node->zi;

			int nodeWidth = 1 << node->level;

			int halfWi = (1 << (node->level - 1));

			int nIndices = node->mPointIndices.size();

		}


		MultiMaterialSurfaceExtractor::SobelGradientOperator3x3x3::SobelGradientOperator3x3x3()
		{
			int hx[3] = { 1, 2, 1 }, hy[3] = { 1, 2, 1 }, hz[3] = { 1, 2, 1 };
			int hpx[3] = { 1, 0, -1 }, hpy[3] = { 1, 0, -1 }, hpz[3] = { 1, 0, -1 };

			//build the kernel
			for (int m = 0; m <= 2; m++)
				for (int n = 0; n <= 2; n++)
					for (int k = 0; k <= 2; k++)
					{
						_KernelGx[m][n][k] = hpx[m] * hy[n] * hz[k];
						_KernelGy[m][n][k] = hx[m] * hpy[n] * hz[k];
						_KernelGz[m][n][k] = hx[m] * hy[n] * hpz[k];
					}

		}



		void MultiMaterialSurfaceExtractor::SobelGradientOperator3x3x3::init(size_t volumeW, size_t volumeH, size_t volumeD, double voxelSizeX,
			double voxelSizeY, double voxelSizeZ, unsigned short *volumeData)
		{
			_VolumeW = volumeW;
			_VolumeH = volumeH;
			_VolumeD = volumeD;

			_VoxelSizeX = voxelSizeX;
			_VoxelSizeY = voxelSizeY;
			_VoxelSizeZ = voxelSizeZ;

			_VolumeData = volumeData;



		}



#define grayValue(x , y , z)  volumeData[ zStep * z + yStep * y + x ] 
		unsigned short valueAt(double x, double y, double z, unsigned int width, unsigned int height, unsigned short *volumeData)
		{
			unsigned short interpolatedValue = 0;

			size_t zStep = width * height;
			size_t yStep = width;

			int lx = (int)x;
			int ly = (int)y;
			int lz = (int)z;

			int ux = (int)std::ceil(x);
			int uy = (int)std::ceil(y);
			int uz = (int)std::ceil(z);

			double xV = x - lx;
			double yV = y - ly;
			double zV = z - lz;

			double c000 = grayValue(lx, ly, lz);
			double c100 = grayValue(ux, ly, lz);
			double c010 = grayValue(lx, uy, lz);
			double c110 = grayValue(ux, uy, lz);
			double c001 = grayValue(lx, ly, uz);
			double c101 = grayValue(ux, ly, uz);
			double c011 = grayValue(lx, uy, uz);
			double c111 = grayValue(ux, uy, uz);

			double interpolatedValF = c000 * (1.0 - xV) * (1.0 - yV) * (1.0 - zV) +
				c100 * xV * (1.0 - yV) * (1.0 - zV) +
				c010 * (1.0 - xV) * yV * (1.0 - zV) +
				c110 * xV * yV * (1.0 - zV) +
				c001 * (1.0 - xV) * (1.0 - yV) * zV +
				c101 * xV * (1.0 - yV) * zV +
				c011 * (1.0 - xV) * yV * zV +
				c111 * xV * yV * zV;

			interpolatedValue = (unsigned short)interpolatedValF;

			return interpolatedValue;


		}


		void MultiMaterialSurfaceExtractor::SobelGradientOperator3x3x3::apply(const Eigen::Vector3f& point, Eigen::Vector3f& gradient)
		{
			double sumx = 0, sumy = 0, sumz = 0;

			double x = point(0);
			double y = point(1);
			double z = point(2);

			size_t zStep = _VolumeW * _VolumeH;
			size_t yStep = _VolumeW;

			unsigned short *volumeData = _VolumeData;

			for (int mm = -1; mm <= 1; mm++)
				for (int nn = -1; nn <= 1; nn++)
					for (int kk = -1; kk <= 1; kk++)
					{
						double xx = x + mm;
						double yy = y + nn;
						double zz = z + kk;

						unsigned short gVal = valueAt(xx, yy, zz, _VolumeW, _VolumeH, _VolumeData); //grayValue(xx, yy, zz);

						sumx += _KernelGx[mm + 1][nn + 1][kk + 1] * gVal;
						sumy += _KernelGy[mm + 1][nn + 1][kk + 1] * gVal;
						sumz += _KernelGz[mm + 1][nn + 1][kk + 1] * gVal;
					}


			gradient(0) = sumx;
			gradient(1) = sumy;
			gradient(2) = sumz;
		}


		void MultiMaterialSurfaceExtractor::SobelGradientOperator3x3x3::computeCentralDifferenceGradient(const Eigen::Vector3f& point, Eigen::Vector3f& gradient)
		{
			float step = 0.5;

			gradient(0) = valueAt(point(0) + step, point(1), point(2), _VolumeW, _VolumeH, _VolumeData) -
				valueAt(point(0) - step, point(1), point(2), _VolumeW, _VolumeH, _VolumeData);

			gradient(1) = valueAt(point(0), point(1) + step, point(2), _VolumeW, _VolumeH, _VolumeData) -
				valueAt(point(0), point(1) - step, point(2), _VolumeW, _VolumeH, _VolumeData);

			gradient(2) = valueAt(point(0), point(1), point(2) + step, _VolumeW, _VolumeH, _VolumeData) -
				valueAt(point(0), point(1), point(2) - step, _VolumeW, _VolumeH, _VolumeData);
		}



		MultiMaterialSurfaceExtractor::MultiMaterialSurfaceExtractor(imt::volume::VolumeInfo& volume,
			std::vector<MaterialRegion>& materialRegions) : _MaterialRegions(materialRegions), _VolumeInfo(volume)
		{

			int64_t w = _VolumeInfo.mWidth, h = _VolumeInfo.mHeight, d = _VolumeInfo.mDepth;

			unsigned short *volumeData = (unsigned short*)_VolumeInfo.mVolumeData;

			_VolumeSlices.resize(d, 0);

			_VolumeData = volumeData;

			_ZStep = w * h;
			_YStep = w;

			for (int zz = 0; zz < d; zz++)
			{
				_VolumeSlices[zz] = volumeData + _ZStep * zz;
			}


			_GradientEvaluator = new SobelGradientOperator3x3x3();

			_GradientEvaluator->init(w, h, d, _VolumeInfo.mVoxelStep(0), _VolumeInfo.mVoxelStep(1), _VolumeInfo.mVoxelStep(2), volumeData);

			std::cout << "width , height , depth : " << w << " " << h << " " << d << std::endl;
		}




		void MultiMaterialSurfaceExtractor::compute()
		{

			int64_t w = _VolumeInfo.mWidth, h = _VolumeInfo.mHeight, d = _VolumeInfo.mDepth;

			unsigned short *volumeData = (unsigned short*)_VolumeInfo.mVolumeData;

			double tg = FLT_MAX;

			int nMaterials = _MaterialRegions.size();

			for (int mm1 = 0; mm1 < nMaterials; mm1++)
				for (int mm2 = mm1 + 1; mm2 < nMaterials; mm2++)
				{
					tg = std::min((double)(_MaterialRegions[mm2]._StartValue - _MaterialRegions[mm1]._StartValue), tg);
				}

			tg = tg / 2.0;

			double tg_ = tg / 10;

			std::vector<short> gradientMagnitudes(w * h * d, -1);

			int64_t zStep = w * h;
			int64_t yStep = w;

			double initT = cv::getTickCount();

			std::atomic<int64_t> nEdgeVoxels = 0;

			std::vector<Eigen::Vector3f> maximaPositions;

			maximaPositions.reserve(4e6);

			double airEnd = 1000;

			double minGradMag = 3000;////_MaterialRegions[1]._StartValue * 2.0 ; 

			std::cout << "min grad magnitude : " << minGradMag << std::endl;

			std::vector<Eigen::Vector3f> normals;

#pragma omp parallel for
			for (int64_t zz = 3; zz < d - 3; zz++)
				for (int64_t yy = 3; yy < h - 3; yy++)
					for (int64_t xx = 3; xx < w - 3; xx++)
					{

						int64_t id = _ZStep * zz + _YStep * yy + xx;

						Eigen::Vector3f point(xx, yy, zz);

						Eigen::Vector3f gradient;

						double grayValue = _VolumeData[id];

						if (grayValue < airEnd)
							continue;

						_GradientEvaluator->computeCentralDifferenceGradient(point, gradient); //centralDifferenceGradientAtVoxel( xx , yy , zz );

						double gradMagnitude = gradient.norm();

						gradient.normalize();

						//now first check that the gradient is a local maxima

						gradientMagnitudes[id] = gradMagnitude;

						if (gradMagnitude < minGradMag)
							continue;

						if (isEdge(point, gradient, gradMagnitude))
						{
							//gradients[zStep * zz + yStep * yy + xx] = new Eigen::Vector3f(gradient(0) , gradient(1) , gradient(2));

							gradientMagnitudes[id] = gradMagnitude / 5;


							Eigen::Vector3f normal;

							_GradientEvaluator->apply(point, normal);

							normal.normalize();

							nEdgeVoxels++;

#pragma omp critical
							{
								maximaPositions.push_back(point);

								normals.push_back(normal);
							}

						}


					}

			//now start the filtering algorithm

			std::cout << "number of edge voxels : " << nEdgeVoxels << std::endl;

			std::cout << " time spent in computing gradients : " << (cv::getTickCount() - initT) / cv::getTickFrequency() << std::endl;


			imt::volume::PoissonMeshGenerator meshGenerator;

			std::vector<Eigen::Vector3f> meshPoints;
			std::vector<unsigned int> surfaceIndices;

			int depth = 8;

			meshGenerator.compute(maximaPositions, normals, depth, meshPoints, surfaceIndices);

			std::vector<Eigen::Vector3f> vertexColors(maximaPositions.size(), Eigen::Vector3f(0, 1, 0));

			tr::Display3DRoutines::displayPointSet(maximaPositions, vertexColors);

			tr::Display3DRoutines::displayMesh(maximaPositions, surfaceIndices);

		}


		void MultiMaterialSurfaceExtractor::compute( int startVal, int endVal, int isoThreshold )
		{

			_IsoThreshold = isoThreshold;

			int64_t w = _VolumeInfo.mWidth, h = _VolumeInfo.mHeight, d = _VolumeInfo.mDepth;

			unsigned short *volumeData = (unsigned short*)_VolumeInfo.mVolumeData;

			std::vector<short> gradientMagnitudes(w * h * d, -1);

			int64_t zStep = w * h;
			int64_t yStep = w;

			double initT = cv::getTickCount();

			std::atomic<int64_t> nEdgeVoxels = 0;

			std::vector<Eigen::Vector3f> maximaPositions;

			maximaPositions.reserve(4e6);

			double airEnd = 1000;

			double minGradMag = (endVal - startVal) * 0.5;////_MaterialRegions[1]._StartValue * 2.0 ; 

			double highGradTh = (endVal - startVal) * 0.05;

			double lowGradTh = (endVal - startVal) * 0.02;

			std::cout << "min grad magnitude : " << minGradMag << std::endl;


			std::vector<Eigen::Vector3f> normals;

			std::vector<unsigned char> gradientMask(w * h * d, 0);

#pragma omp parallel for
			for (int64_t zz = 3; zz < d - 3; zz++)
				for (int64_t yy = 3; yy < h - 3; yy++)
					for (int64_t xx = 3; xx < w - 3; xx++)
					{

						int64_t id = _ZStep * zz + _YStep * yy + xx;

						Eigen::Vector3f point(xx, yy, zz);

						Eigen::Vector3f gradient;

						double grayValue = _VolumeData[id];

						if (grayValue < airEnd)
							continue;

						_GradientEvaluator->computeCentralDifferenceGradient(point, gradient); 

						double gradMagnitude = gradient.norm();

						gradient.normalize();

						//now first check that the gradient is a local maxima

						gradientMagnitudes[id] = gradMagnitude;

						if (gradMagnitude < lowGradTh)
							continue;

						if (isEdge(point, gradient, gradMagnitude, 10))
						{
							//gradients[zStep * zz + yStep * yy + xx] = new Eigen::Vector3f(gradient(0) , gradient(1) , gradient(2));


							if (gradMagnitude < highGradTh)
							{
								gradientMask[id] = 1;
							}
							else
							{
								gradientMask[id] = 2;
							}

							gradientMagnitudes[id] = gradMagnitude / 5;


							Eigen::Vector3f normal;

							//_GradientEvaluator->apply(point, normal);

							//normal.normalize();

							nEdgeVoxels++;

#pragma omp critical
							{
								maximaPositions.push_back(point);

								//normals.push_back(normal);
							}

						}


					}


			traceMaximaVoxels(gradientMask);

			//now start the filtering algorithm
			std::cout << "number of edge voxels : " << nEdgeVoxels << std::endl;

			std::cout << " time spent in computing gradients : " << (cv::getTickCount() - initT) / cv::getTickFrequency() << std::endl;


			imt::volume::PoissonMeshGenerator meshGenerator;

			std::vector<Eigen::Vector3f> meshPoints;
			std::vector<unsigned int> surfaceIndices;

			int depth = 11;

			//meshGenerator.compute(maximaPositions, normals, depth, meshPoints, surfaceIndices);

			std::vector<Eigen::Vector3f> vertexColors(maximaPositions.size(), Eigen::Vector3f(0, 1, 0));

			tr::Display3DRoutines::displayPointSet(maximaPositions, vertexColors);

			//tr::Display3DRoutines::displayMesh(meshPoints, surfaceIndices);
		}




		bool MultiMaterialSurfaceExtractor::isEdge(Eigen::Vector3f& point, Eigen::Vector3f& gradient, double gradientMagnitude, int checkSize)
		{
			int profileLength = 2 * checkSize + 1;

			std::vector<float> profiles(profileLength);

			float step = 0.5;

			float *xMap = new float[profileLength];
			float *yMap = new float[profileLength];
			float *zMap = new float[profileLength];

			std::vector<float> extract(profileLength);

			for (int ii = -checkSize; ii <= checkSize; ii++)
			{
				Eigen::Vector3f currPoint = point + ii * step * gradient;

				xMap[ii + checkSize] = currPoint(0);
				yMap[ii + checkSize] = currPoint(1);
				zMap[ii + checkSize] = currPoint(2);

			}

			IpprVolume srcVolume = { _VolumeInfo.mWidth,_VolumeInfo.mHeight,_VolumeInfo.mDepth };
			IpprCuboid srcVoi = { 0,0,0, _VolumeInfo.mWidth,_VolumeInfo.mHeight,_VolumeInfo.mDepth };
			IpprVolume dstVolume = { profileLength , 1 , 1 };

			unsigned short *temp;
			temp = new unsigned short[profileLength];
			ippsSet_16s(0, (short*)temp, profileLength);

			auto sts = ipprRemap_16u_C1PV(_VolumeSlices.data(), srcVolume, sizeof(Ipp16u) * _VolumeInfo.mWidth, srcVoi,
				&xMap, &yMap, &zMap, sizeof(Ipp32f) * profileLength, &temp,
				sizeof(Ipp16u) * profileLength, dstVolume, IPPI_INTER_LINEAR);

			ippsConvert_16u32f(temp, extract.data(), profileLength);

			std::vector<float> gradients(2 * checkSize - 1);

			float maxGradient = 0;

			int maxGradientPosition = -1;

			for (int ii = -checkSize + 1; ii < checkSize; ii++)
			{
				int id = ii + checkSize;

				gradients[id - 1] = extract[id + 1] - extract[id - 1];

				if (std::abs(maxGradient) < std::abs(gradients[id - 1]))
				{
					maxGradient = gradients[id - 1];

					maxGradientPosition = id - 1;
				}

			}

			delete[] xMap;
			delete[] yMap;
			delete[] zMap;
			
			if (maxGradientPosition != checkSize)
				return false;

			if ( ( maxGradient < 0 &&  extract[0] < _IsoThreshold) || ( maxGradient > 0 && extract[profileLength - 1] < _IsoThreshold ))
			{
				return false;
			}

			return computeProfile(point, 10, gradient);
		}




		bool MultiMaterialSurfaceExtractor::computeProfile(Eigen::Vector3f& point, int halfProfileSize, Eigen::Vector3f& direction)
		{
			int profileLength = 2 * halfProfileSize + 1;

			std::vector<float> profiles(profileLength);

			float step = 0.25;

			float *xMap = new float[profileLength];
			float *yMap = new float[profileLength];
			float *zMap = new float[profileLength];
			float *extract = new float[profileLength];

			for (int ii = -halfProfileSize; ii <= halfProfileSize; ii++)
			{
				Eigen::Vector3f currPoint = point + ii * step * direction;

				xMap[ii + halfProfileSize] = currPoint(0);
				yMap[ii + halfProfileSize] = currPoint(1);
				zMap[ii + halfProfileSize] = currPoint(2);

			}

			IpprVolume srcVolume = { _VolumeInfo.mWidth,_VolumeInfo.mHeight,_VolumeInfo.mDepth };
			IpprCuboid srcVoi = { 0,0,0, _VolumeInfo.mWidth,_VolumeInfo.mHeight,_VolumeInfo.mDepth };
			IpprVolume dstVolume = { profileLength , 1 , 1 };

			unsigned short *temp;
			temp = new unsigned short[profileLength];
			ippsSet_16s(0, (short*)temp, profileLength);

			auto sts = ipprRemap_16u_C1PV(_VolumeSlices.data(), srcVolume, sizeof(Ipp16u) * _VolumeInfo.mWidth, srcVoi,
				&xMap, &yMap, &zMap, sizeof(Ipp32f) * profileLength, &temp,
				sizeof(Ipp16u) * profileLength, dstVolume, IPPI_INTER_LINEAR);

			ippsConvert_16u32f(temp, extract, profileLength);


			//apply 1d gaussian


			std::vector<float> gradients(2 * halfProfileSize - 1);

			float maxGradient = 0;

			int maxGradientPosition = -1;

			for (int ii = -halfProfileSize + 1; ii < halfProfileSize; ii++)
			{
				int id = ii + halfProfileSize;

				gradients[id - 1] = 2 * ( extract[id + 1] - extract[id - 1] );

				if (std::abs(maxGradient) < std::abs(gradients[id - 1]))
				{
					maxGradient = gradients[id - 1];

					maxGradientPosition = id - 1;
				}

			}

			delete[] xMap;
			delete[] yMap;
			delete[] zMap;
			delete[] extract;


			if (maxGradientPosition <= 0 || maxGradientPosition == 2 * profileLength - 1)
			{
				return false;

				//delete[] xMap;
				//delete[] yMap;
				//delete[] zMap;
				//delete[] extract;

			}
			else
			{
				//find the interpolated position
				float dg1 = maxGradient - gradients[maxGradientPosition - 1];
				float dg2 = maxGradient - gradients[maxGradientPosition + 1];

				float alpha = (dg2 - dg1) / (dg2 + dg1);

				if (std::abs(alpha) > 1)
					alpha = 0;

				point = point + (maxGradientPosition - halfProfileSize + 1 - alpha) * direction * 0.25;
			}



			return true;
		}


		void MultiMaterialSurfaceExtractor::computeVolumeGradients()
		{
			//

			//IppStatus ipprFilter_16s_C1PV(const Ipp16s* const pSrc[], int srcStep, Ipp16s* const pDst[], int dstStep, IpprVolume dstVolume, const Ipp32s* pKernel, IpprVolume kernelVolume, IpprPoint anchor, int divisor, Ipp8u* pBuffer);


			//Ipp16s a;




		}

		void MultiMaterialSurfaceExtractor::buildConnectivity(std::vector<Eigen::Vector3f>& gradients, std::vector<unsigned char>& edgeVoxelMask)
		{
			int64_t w = _VolumeInfo.mWidth, h = _VolumeInfo.mHeight, d = _VolumeInfo.mDepth;

			for (int64_t zz = 1; zz < d - 1; zz++)
				for (int64_t yy = 1; yy < h - 1; yy++)
					for (int64_t xx = 1; xx < w - 1; xx++)
					{
						int64_t id = _ZStep * zz + _YStep * yy + xx;

						if (!edgeVoxelMask[id])
							continue;

						//now compute the maginitude of the grdient
						double gradMag = gradients[id].norm();

						//
						if (gradMag > _SurfaceThreshold1)
						{
							while (1)
							{
								//connect the voxels


							}
						}


					}
		}


		Eigen::Vector3f MultiMaterialSurfaceExtractor::centralDifferenceGradientAtVoxel(int voxelX, int voxelY, int volxelZ) const
		{
			Eigen::Vector3f gradient;

			gradient(0) = (_VolumeData[volxelZ * _ZStep + voxelY * _YStep + voxelX + 1] - _VolumeData[volxelZ * _ZStep + voxelY * _YStep + voxelX - 1]) * 0.5;
			gradient(1) = (_VolumeData[volxelZ * _ZStep + (voxelY + 1) * _YStep + voxelX] - _VolumeData[volxelZ * _ZStep + (voxelY - 1) * _YStep + voxelX]) * 0.5;
			gradient(2) = (_VolumeData[(volxelZ + 1) * _ZStep + voxelY * _YStep + voxelX] - _VolumeData[(volxelZ - 1) * _ZStep + voxelY * _YStep + voxelX]) * 0.5;


			return gradient;

		}


		void MultiMaterialSurfaceExtractor::filterNonEdgeVoxels(int z, std::vector<Eigen::Vector3f>& voxelGradients, std::vector<unsigned char>& edgeVoxelFlags)
		{
			int64_t w = _VolumeInfo.mWidth, h = _VolumeInfo.mHeight;

			std::vector<Eigen::Vector3f> cornerGradients(_ZStep * 3);

			//now compute gradient at each corners
			for (int zz = z - 1; zz <= z + 1; zz++)
				for (int yy = 0; yy < h; yy++)
					for (int xx = 0; xx < w; xx++)
					{
						//average the gradients of eight surrounding voxels
						Eigen::Vector3f gradientSum = voxelGradients[zz * _ZStep + yy * _YStep + xx] + voxelGradients[zz * _ZStep + yy * _YStep + xx - 1] +
							voxelGradients[zz * _ZStep + (yy - 1) * _YStep + xx] + voxelGradients[zz * _ZStep + (yy - 1) * _YStep + xx - 1] +
							voxelGradients[(zz - 1) * _ZStep + yy * _YStep + xx] + voxelGradients[(zz - 1) * _ZStep + yy * _YStep + xx - 1] +
							voxelGradients[(zz - 1) * _ZStep + (yy - 1) * _YStep + xx] + voxelGradients[(zz - 1) * _ZStep + (yy - 1) * _YStep + xx - 1];


						gradientSum *= 0.125;

						voxelGradients[zz * _ZStep + yy * _YStep + xx] = gradientSum;
					}


			//now we need to find all the edge voxels on the layer
			for (int yy = 0; yy < h; yy++)
				for (int xx = 0; xx < w; xx++)
				{
					Eigen::Vector3f voxelGradient = voxelGradients[z * _ZStep + yy * _YStep + xx];

					double gradientNorm = voxelGradient.norm();

					//now find the two intersection points and compute the gradient at those points


				}
		}


		double MultiMaterialSurfaceExtractor::intersectionWithXYPlane(const Eigen::Vector3f& planePosition, const Eigen::Vector3f& lineCenter, const Eigen::Vector3f& lineDirection) const
		{
			double intersectionLength = (planePosition(2) - lineCenter(2)) / lineDirection(2);

			return intersectionLength;
		}
		double MultiMaterialSurfaceExtractor::intersectionWithYZPlane(const Eigen::Vector3f& planePosition, const Eigen::Vector3f& lineCenter, const Eigen::Vector3f& lineDirection) const
		{
			double intersectionLength = (planePosition(0) - lineCenter(0)) / lineDirection(0);

			return intersectionLength;
		}

		double MultiMaterialSurfaceExtractor::intersectionWithZXPlane(const Eigen::Vector3f& planePosition, const Eigen::Vector3f& lineCenter, const Eigen::Vector3f& lineDirection) const
		{
			double intersectionLength = (planePosition(1) - lineCenter(1)) / lineDirection(1);

			return intersectionLength;
		}


		void MultiMaterialSurfaceExtractor::traceMaximaVoxels(std::vector<unsigned char>& gradientMask)
		{
			int w = _VolumeInfo.mWidth;
			int h = _VolumeInfo.mHeight;
			int d = _VolumeInfo.mDepth;

			std::vector<Eigen::Vector3i> traceableVoxels(w * h * 10), nextVoxelsToTrace(w * h * 10);

			std::vector<Eigen::Vector3f> tracedPoints;

			int64_t traceCount1 = 0, traceCount2 = 0;

			for (int zz = 0; zz < d; zz++)
				for (int yy = 0; yy < h; yy++)
					for (int xx = 0; xx < w; xx++)
					{
						int64_t id = _ZStep * zz + _YStep * yy + xx;

						if (gradientMask[id] == 2)
						{
							traceableVoxels[traceCount1] = Eigen::Vector3i(xx, yy, zz);

							traceCount1++;
						}
					}

	
			while ( 1 )
			{
				for ( int64_t tt = 0; tt < traceCount1; tt++ )
				{
					Eigen::Vector3i pos = traceableVoxels[tt];

					for(int ii = -1; ii <= 1; ii++)
						for(int jj = -1; jj <= 1; jj++)
							for (int kk = -1; kk <= 1; kk++)
							{
								int xx = pos(0) + ii;
								int yy = pos(1) + jj;
								int zz = pos(2) + kk;

								int64_t traceId = _ZStep * zz + _YStep * yy + xx;

								if (gradientMask[traceId] == 1)
								{
									gradientMask[traceId] = 2;

									nextVoxelsToTrace[traceCount2] = Eigen::Vector3i(xx, yy, zz);

									traceCount2++;
								}
							}



				
				}

				//std::cout << "trace count : " << traceCount2 << std::endl;

				if (traceCount2 == 0)
					break;

				memcpy(traceableVoxels.data(), nextVoxelsToTrace.data(), traceCount2 * sizeof(Eigen::Vector3i));

				traceCount1 = traceCount2;

				traceCount2 = 0;

			}



			for (int zz = 0; zz < d; zz++)
				for (int yy = 0; yy < h; yy++)
					for (int xx = 0; xx < w; xx++)
					{
						int64_t id = _ZStep * zz + _YStep * yy + xx;

						if (gradientMask[id] == 2)
						{
							tracedPoints.push_back(Eigen::Vector3f(xx, yy, zz));
						}
					}



			tr::Display3DRoutines::displayPointSet(tracedPoints, std::vector<Eigen::Vector3f>(tracedPoints.size(), Eigen::Vector3f(1, 0, 0)));
		}


		void MultiMaterialSurfaceExtractor::computeIntersections( Eigen::Vector3f& gradient, Eigen::Vector3f& voxelPosition )
		{
			Eigen::Vector3f center = voxelPosition;

			double end1, end2;
			int planeId1, planeId2;
          
			//plane 1 and 2 xx = -1 , xx = 2
			center(0) = voxelPosition(0) - 1;

			//intersectionWithYZPlane( center ,  )

			

			//plane 3 and 4 yy = -1 , yy = 2


			//plane 5 and 6 zz = -1 , zz = 2




		}


		//Eigen::Vector3f MultiMaterialSurfaceExtractor::gradientAtCorner(int voxelX, int voxelY, int voxelZ)
		//{
		//	//Eigen::Vector3f 
		//}


	}


}