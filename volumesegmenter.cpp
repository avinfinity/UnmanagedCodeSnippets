#include "volumesegmenter.h"
#include "iostream"

namespace imt{
	
	namespace volume{
		
		
		VolumeSegmenter::VolumeSegmenter(imt::volume::VolumeInfo& volInfo) : mVolumeInfo( volInfo )
		{
			
		}
		

	
		
		void VolumeSegmenter::compute( int isoValue , int range , int band )
		{
			//first mark volume voxels based on range and band

			int w = mVolumeInfo.mWidth;
			int h = mVolumeInfo.mHeight;
			int d = mVolumeInfo.mDepth;

			unsigned char *volumeMask = new unsigned char[w * h * d];

			memset(volumeMask, 0, w * h * d);

			int zStep = w * h, yStep = w;

			// 1 - definite background , 2 - probable background , 3 - definite foreground , 4 - probable foreground
			const int DB = 1;
			const int PB = 2;
			const int DF = 3;
			const int PF = 4;

			std::vector< Voxel > q1, q2;

			q1.reserve(5 * w * h);
			q2.reserve(5 * w * h);

			std::cout << " iso value : " << isoValue << std::endl;

			unsigned short *vData = (unsigned short*)mVolumeInfo.mVolumeData;


			for (int zz = 0; zz < d; zz++)
				for (int yy = 0; yy < h; yy++)
					for (int xx = 0; xx < w; xx++)
					{
						long int id = zz * zStep + yy * yStep + xx;

						if ( vData[id] == isoValue )
						{
							volumeMask[id] = 4;

							Voxel v;

							v.x = xx;
							v.y = yy;
							v.z = zz;

							q1.push_back(v);
						}
					}


			//first extract surface voxels

			int minIso = isoValue - range;
			int maxIso = isoValue + range;



			std::cout << " band loops " << std::endl;

			for (int bb = 0; bb < band; bb++)
			{


#if 0
				for ( int zz = 1; zz < d-1; zz++)
					for ( int yy = 1; yy < h - 1; yy++)
						for ( int xx = 1; xx < w - 1; xx++)
						{
							long int id = zz * zStep + yy * yStep + xx;

							for ( int z = zz - 1; z <= zz + 1; z++ )
								for ( int y = yy - 1; y <= yy + 1; y++ )
									for ( int x = xx - 1; x <= xx + 1; x++ )
									{
										long int id2 = z * zStep + y * yStep + x;

										if ( volumeMask[id2] )
										{
											volumeMask[id] = 1;

											if ( mVolumeInfo.mVolumeData[id] < minIso )
											{
												volumeMask[id] = DB;
											}

											if ( mVolumeInfo.mVolumeData[id] > maxIso )
											{
												volumeMask[id] = DF;
											}

											if ( mVolumeInfo.mVolumeData[id] > minIso && mVolumeInfo.mVolumeData[id] < isoValue )
											{
												volumeMask[id] = PB;
											}

											if (mVolumeInfo.mVolumeData[id] < maxIso && mVolumeInfo.mVolumeData[id] >= isoValue)
											{
												volumeMask[id] = PF;
											}

										}
									}
						}
#endif


					int nVoxs = q1.size();

					std::cout << " q1 size : " << nVoxs << std::endl;

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

									if (volumeMask[id])
										continue;

									if ( vData[id] < minIso)
									{
										volumeMask[id] = DB;
									}

									if ( vData[id] > maxIso)
									{
										volumeMask[id] = DF;
									}

									if ( vData[id] > minIso && vData[id] < isoValue)
									{
										volumeMask[id] = PB;
									}

									if ( vData[id] < maxIso && vData[id] >= isoValue)
									{
										volumeMask[id] = PF;
									}

									Voxel v;

									v.x = xx;
									v.y = yy;
									v.z = zz;

									q2.push_back(v);

								}
					}


					int nextLayerSize = q2.size();


					if (q2.size() == 0)
						break;

					q1 = q2;

					q2.clear();

				std::cout << " loop : " << bb << std::endl;
			}


			long numSetVoxels = 0;

			for (int zz = 0; zz < d; zz++)
				for (int yy = 0; yy < h; yy++)
					for (int xx = 0; xx < w; xx++)
					{
						long int id = zz * zStep + yy * yStep + xx;

						if (volumeMask[id])
						{
							numSetVoxels++;
						}

					}

			std::cout << " num set voxels : " << numSetVoxels << std::endl;

		}

		void VolumeSegmenter::computeBoundingBox(std::vector< Voxel >& voxels, BoundingBox& bb)
		{
			int xMin = 10000;
			int xMax = 0;
			int yMin = 10000;
			int yMax = 0;
			int zMin = 10000;
			int zMax = 0;

			for (auto it = voxels.begin(); it != voxels.end(); it++)
			{
				xMin = std::min((unsigned short)xMin, it->x);
				xMax = std::max((unsigned short)xMax, it->x);

				yMin = std::min((unsigned short)yMin, it->y);
				yMax = std::max((unsigned short)yMax, it->y);

				zMin = std::min((unsigned short)zMin, it->z);
				zMax = std::max((unsigned short)zMax, it->z);
			}

			bb.mXMin = xMin;
			bb.mXMax = xMax;

			bb.mYMin = yMin;
			bb.mYMax = yMax;

			bb.mZMax = zMax;
			bb.mZMin = zMin;
		}


		void VolumeSegmenter::buildPartitions(std::vector< Voxel >& voxels, std::vector< std::vector<Voxel> >& voxelPartitions, std::vector< BoundingBox >& boundingBoxes)
		{

			int nVoxels = voxels.size();

			BoundingBox bb;

			computeBoundingBox(voxels, bb);

			for ( int vv = 0; vv < nVoxels; vv++ )
			{

			}


		}




		void VolumeSegmenter::detectEdgePoints()
		{

			long int zStep = mVolumeInfo.mWidth * mVolumeInfo.mHeight;
			long int yStep = mVolumeInfo.mWidth;

			for (int zz = 0; zz < mVolumeInfo.mDepth; zz++)
				for (int yy = 0; yy < mVolumeInfo.mHeight; yy++)
					for (int xx = 0; xx < mVolumeInfo.mWidth; xx++)
					{
						for ( int z = zz - 1; z <= zz + 1; z++ )
							for ( int y = yy - 1; y <= yy + 1; y++ )
								for ( int x = xx - 1; x <= xx + 1; x++ )
								{

								}
					}
		}
		
		
	}
	
	
}