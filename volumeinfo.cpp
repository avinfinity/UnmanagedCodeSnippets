#include "volumeinfo.h"
#include "QDataStream"
#include "QFile"
#include "QDebug"
#include "lineiterator.h"
#include "vtkincludes.h"
#include "meshutility.h"

//#include "OpenMesh/Core/IO/MeshIO.hh"
//#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
//#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>
//#include <OpenMesh/Tools/Utils/Timer.hh>
//#include <OpenMesh/Tools/Decimater/DecimaterT.hh>
//#include <OpenMesh/Tools/Decimater/ModAspectRatioT.hh>
//#include <OpenMesh/Tools/Decimater/ModEdgeLengthT.hh>
//#include <OpenMesh/Tools/Decimater/ModHausdorffT.hh>
//#include <OpenMesh/Tools/Decimater/ModNormalDeviationT.hh>
//#include <OpenMesh/Tools/Decimater/ModNormalFlippingT.hh>
//#include <OpenMesh/Tools/Decimater/ModQuadricT.hh>
//#include <OpenMesh/Tools/Decimater/ModProgMeshT.hh>
//#include <OpenMesh/Tools/Decimater/ModIndependentSetsT.hh>
//#include <OpenMesh/Tools/Decimater/ModRoundnessT.hh>
#include "QMutex"
#include "QPixmap"
#include "display3droutines.h"

#include "vtkSTLReader.h"
#include "vtkPLYReader.h"

#include "set"
#include "unordered_set"

//#include "tbb/concurrent"


//typedef std::vector< int > SplitNeighbors;
//
//typedef std::pair< int, int > SplitCamera;
//
//struct MeshTraits : public OpenMesh::DefaultTraits
//{
//	VertexAttributes(OpenMesh::Attributes::Status);
//	FaceAttributes(OpenMesh::Attributes::Status);
//	EdgeAttributes(OpenMesh::Attributes::Status);
//};
//
//typedef OpenMesh::TriMesh_ArrayKernelT<MeshTraits> SurfaceMesh;

namespace imt{

	namespace volume
	{


		VolumeInfo::VolumeInfo()
		{

			mWidth = 0;
			mHeight = 0;
			mDepth = 0;

			mVolumeData = 0;

			mVolumeElementSize = 1;
			mVolumeElementComponents = 1;

			mVolumeOrigin = Eigen::Vector3f(0, 0, 0);
			mVoxelStep = Eigen::Vector3f(1, 1, 1);

			mWallthicknessDataChanged = false;

			mHasDistanceTransformData = false;


			mSBRefDataPath = "C:/projects/Wallthickness/data/benchmarkdata/MSB_Demo Part STL_Aligned.stl";
			mSBActualDataPath = "C:/projects/Wallthickness/data/benchmarkdata/surface.ply";
		}


		void VolumeInfo::convertToByteArray( unsigned char background , unsigned char material , unsigned char border )
		{
			if ( mVolumeElementSize == 1)
				  return;

			unsigned char *byteData = new unsigned char[mWidth * mHeight * mDepth];

			memset(byteData, 0, mWidth * mHeight * mDepth);


			unsigned short *vData = (unsigned short*)mVolumeData;

			long int id = mWidth * mHeight;

			int zStep = mWidth * mHeight;
			int yStep = mHeight;

			for (int zz = 1; zz < mDepth - 1; zz++)
				for (int yy = 1; yy < mHeight - 1; yy++)
					for (int xx = 1; xx < mWidth - 1; xx++)
					{
						if ( vData[id] < mAirVoxelFilterVal )
						{
							byteData[id] = 0;
						}
						else
						{
							bool isBorder = false;

							for (int z = zz - 1; z <= zz + 1; z++)
								for (int y = yy - 1; y <= yy + 1; y++)
									for (int x = xx - 1; x <= xx + 1; x++)
							       {
									   long int id2 = zStep * z + yStep * y + x;

									   if (vData[id2] < mAirVoxelFilterVal)
									   {
										   isBorder = true;

										   break;
									   }

							       }

							byteData[id] = isBorder ? border : material;
						}

						id++;
					}


			delete [] vData;

			mVolumeData = byteData;

		}


		void VolumeInfo::loadVolume( QString filePath )
		{
			QFile file(filePath);

			if ( !file.open( QIODevice::ReadOnly ) )
			{
				qDebug() << " could not open file " << filePath << " for reading :  " << file.errorString() << endl;
			}

			QDataStream reader(&file);

			//write volume resolution
			reader >> mWidth >> mHeight >> mDepth >> mVolumeElementSize >> mVolumeElementComponents;
			reader >> mVolumeOrigin(0) >> mVolumeOrigin(1) >> mVolumeOrigin(2) >> mVoxelStep(0) >> mVoxelStep(1) >> mVoxelStep(2);
			reader >> mAirVoxelFilterVal;

			if (mVolumeData)
			{
				delete [] mVolumeData;
			}

			if (mVolumeElementSize == 2)
			{
				mVolumeData = (unsigned char*)(new unsigned short[mWidth * mHeight * mDepth]);
			}
			else
			{
				mVolumeData = new unsigned char[mWidth * mHeight * mDepth];
			}

			reader.readRawData((char*)mVolumeData, mWidth * mHeight * mDepth * mVolumeElementSize * mVolumeElementComponents);

			qlonglong numVertices = 0;
			qlonglong numTriangles = 0;

			reader >> numVertices >> numTriangles;

			mVertices.resize(numVertices);
			mVertexNormals.resize(numVertices);
			mFaceIndices.resize(numTriangles * 3);
			mOppositeFaceIndices.resize(numVertices);
			std::fill(mOppositeFaceIndices.begin(), mOppositeFaceIndices.end(), -1);

			reader.readRawData((char*)mVertices.data(), numVertices * 3 * sizeof(float));
			reader.readRawData((char*)mVertexNormals.data(), numVertices * 3 * sizeof(float));
			reader.readRawData((char*)mFaceIndices.data(), numTriangles * 3 * sizeof(unsigned int));

			//tr::Display3DRoutines::displayPointSet(mVertices, mVertexNormals);

			//std::vector< Eigen::Vector3f > faceNormals( numTriangles );

			//for ( int tt = 0; tt < numTriangles ; tt++ )
			//{
			//	int vid1 = mFaceIndices[3 * tt];
			//	int vid2 = mFaceIndices[3 * tt + 1];
			//	int vid3 = mFaceIndices[3 * tt + 2];

			//	Eigen::Vector3f vec1 = mVertices[vid2] - mVertices[vid1];
			//	Eigen::Vector3f vec2 = mVertices[vid3] - mVertices[vid2];

			//	Eigen::Vector3f n = vec1.cross(vec2);

			//	n.normalize();

			//	n(0) = std::abs(n(0));
			//	n(1) = std::abs(n(1));
			//	n(2) = std::abs(n(2));
			//	
			//	
			//	faceNormals[tt] = n;
			//}

			//tr::Display3DRoutines::displayMeshWithFaceColors(mVertices, mFaceIndices, faceNormals);

			file.close();

			//buildVertexIncidence();

			//fixNormals();

			//std::cout << " surface loaded " << std::endl;

			emit surfaceDataChangedS();

		}


		void VolumeInfo::loadBenchmarkData()
		{
			vtkSmartPointer< vtkSTLReader > reader1 = vtkSmartPointer< vtkSTLReader >::New();
			vtkSmartPointer< vtkPLYReader > reader2 = vtkSmartPointer< vtkPLYReader >::New();

			//mSBRefDataPath, mSBActualDataPath

			reader1->SetFileName( mSBRefDataPath.toStdString().c_str() );

			reader2->SetFileName( mSBActualDataPath.toStdString().c_str() );

			reader1->Update(); 
			reader2->Update();

			vtkSmartPointer< vtkPolyData > mesh1 = vtkSmartPointer< vtkPolyData >::New();
			vtkSmartPointer< vtkPolyData > mesh2 = vtkSmartPointer< vtkPolyData >::New();

			mesh1->ShallowCopy(reader1->GetOutput());
			mesh2->ShallowCopy(reader2->GetOutput());

			vtkSmartPointer< vtkPolyDataNormals > normalEstimator = vtkSmartPointer< vtkPolyDataNormals >::New();

			normalEstimator->SetInputData(mesh1);

			normalEstimator->Update();

			mesh1->ShallowCopy(normalEstimator->GetOutput());

			normalEstimator->SetInputData(mesh2);

			normalEstimator->Update();

			mesh2->ShallowCopy(normalEstimator->GetOutput());

			std::vector< Eigen::Vector3f > refPoints, refNormals, comparePoints, compareNormals;
			std::vector< unsigned int > compareIndices, refIndices;

			imt::volume::MeshUtility::convertToVertexAndIndices(mesh1, comparePoints, compareNormals, compareIndices);
			imt::volume::MeshUtility::convertToVertexAndIndices(mesh2, refPoints , refNormals, refIndices);

			mSBRefPoints = refPoints;
			mSBRefNormals = refNormals;
			mSBDataPoints = comparePoints;
			mSBDataNormals = compareNormals;
			mSBRefIndices = refIndices;
			mSBDataIndices = compareIndices;

		}



		double VolumeInfo::valueAt(double x, double y, double z)
		{
#define grayValue(x , y , z)  mVolumeDataU16[ mZStep * z + mYStep * y + x ] 

				unsigned short interpolatedValue = 0;

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

				return interpolatedValF;
		}


		void VolumeInfo::saveVolume( QString filePath )
		{
			QFile file(filePath);

			if ( !file.open(QIODevice::WriteOnly) )
			{
				qDebug() << " could not open file " << filePath << " for writing :  " << file.errorString() << endl;
			}

			QDataStream writer(&file);

			//write volume resolution
			writer << mWidth << mHeight << mDepth << mVolumeElementSize << mVolumeElementComponents;
			writer << mVolumeOrigin(0) << mVolumeOrigin(1) << mVolumeOrigin(2) << mVoxelStep(0) << mVoxelStep(1) << mVoxelStep(2);
			writer << mAirVoxelFilterVal;

			writer.writeRawData((char*)mVolumeData, mWidth * mHeight * mDepth * mVolumeElementSize * mVolumeElementComponents);

			qlonglong numVertices = mVertices.size();
			qlonglong numTriangles = mFaceIndices.size() / 3;

			writer << numVertices << numTriangles;

			writer.writeRawData((char*)mVertices.data(), numVertices * 3 * sizeof(float));
			writer.writeRawData((char*)mVertexNormals.data(), numVertices * 3 * sizeof(float));
			writer.writeRawData((char*)mFaceIndices.data(), numTriangles * 3 * sizeof(unsigned int));

			file.close();
		}

		typedef Eigen::SparseMatrix<unsigned char> VertexMap;

		typedef Eigen::Triplet< unsigned char > Triplet;

		void VolumeInfo::buildVertexIncidence()
		{

			double initT = cv::getTickCount();

			int numVertices = mVertices.size();

			std::unordered_set<int> *vertexIncidence = new std::unordered_set<int>[numVertices];

			//VertexMap vmap( numVertices , numVertices );

			int nTris = mFaceIndices.size() / 3;

			//std::vector< Eigen::Triplet< unsigned char > > triplets( nTris * 6 );

			for (int tt = 0; tt < nTris; tt++)
			{
				int id1 = mFaceIndices[3 * tt];
				int id2 = mFaceIndices[3 * tt + 1];
				int id3 = mFaceIndices[3 * tt + 2];

				//int mId1, mId2, mId3;

				//if (id1 < id2 && id1 < id3)
				//{
				//	mId1 = id1;

				//	if (id2 < id3)
				//	{
				//		mId2 = id2;
				//		mId3 = id3;
				//	}
				//	else
				//	{
				//		mId2 = id3;
				//		mId3 = id2;
				//	}
				//}
				//else if (id1 < id3)
				//{
				//	mId1 = id2;
				//	mId2 = id1;
				//	mId3 = id3;
				//}
				//else
				//{
				//	mId1 = id3;
				//	mId2 = id2;
				//	mId3 = id1;
				//}

				vertexIncidence[id1].insert(id2);
				vertexIncidence[id1].insert(id3);

				vertexIncidence[id2].insert(id1);
				vertexIncidence[id2].insert(id3);

				vertexIncidence[id3].insert(id1);
				vertexIncidence[id3].insert(id2);

				//triplets[6 * tt] = Triplet( id1 , id2 , 1 );
				//
				//triplets[6 * tt + 1] = Triplet(id1, id3, 1);


				//triplets[6 * tt + 2] = Triplet( id2, id1, 1);

				//triplets[6 * tt + 3] = Triplet( id2, id3, 1);


				//triplets[6 * tt + 4] = Triplet(id3, id1, 1);

				//triplets[6 * tt + 5] = Triplet(id3, id2, 1);

			}

			std::cout << "time spent in  vertex insertion : " << (cv::getTickCount() - initT) / cv::getTickFrequency() << std::endl;


			//vmap.setFromTriplets(triplets.begin(), triplets.end());

			mVertexIncidence.resize(numVertices);


#pragma omp parallel for
			for (int vv = 0; vv < numVertices; vv++ )
			{
				auto& incidence = vertexIncidence[vv];

				mVertexIncidence[vv].reserve(10);

				for (auto vid : incidence)
				{
					mVertexIncidence[vv].push_back(vid);
				}

			}

			delete [] vertexIncidence;

			std::cout << "time spent in building vertex incidence : " << (cv::getTickCount() - initT) / cv::getTickFrequency() << std::endl;

		
		}


		void VolumeInfo::updateSurfaceData()
		{

			emit surfaceDataChangedS();
		}



		void VolumeInfo::fixNormals()
		{
			//LineIterator lineIter(;

			float disp = mVoxelStep.norm() * 0.05;

			int numVerts = mVertices.size();

			unsigned short *vData = (unsigned short*)mVolumeData;

			int checkWidth = 5;

			for (int vv = 0; vv < numVerts; vv++)
			{
				Eigen::Vector3f v = mVertices[vv];
				Eigen::Vector3f n = mVertexNormals[vv];

				Eigen::Vector3f vp = v + n *  disp;
				Eigen::Vector3f vn = v - n *  disp;

				int vx = ( int )( vp(0) / mVoxelStep(0) ) + 1;
				int vy = ( int )( vp(1) / mVoxelStep(1) ) + 1;
				int vz = ( int )( vp(2) / mVoxelStep(2) ) + 1;

				Voxel voxel;

				voxel.x = vx;
				voxel.y = vy;
				voxel.z = vz;

				LineIterator lineIter1(n, voxel, mVolumeOrigin, mVoxelStep);

				int frontCount = 0 , backCount = 0;

				long int zStep = mWidth * mHeight;
				long int yStep = mWidth;

				for (int ii = 0; ii < checkWidth; ii++)
				{
					Voxel nextVox = lineIter1.next();

					if (nextVox.x < 0 || nextVox.x >= mWidth - 1 || nextVox.y < 0 || nextVox.y >= mHeight - 1 || nextVox.z < 0 || nextVox.z >= mDepth - 1)
						continue;

					if (vData[zStep * nextVox.z + yStep * nextVox.y + nextVox.x] > mAirVoxelFilterVal)
					{
						frontCount++;
					}

				}


				voxel.x = (int)(vn(0) / mVoxelStep(0)) + 1;
				voxel.y = (int)(vn(1) / mVoxelStep(1)) + 1;
				voxel.z = (int)(vn(2) / mVoxelStep(2)) + 1;

				n *= -1;




				LineIterator lineIter2( n, voxel, mVolumeOrigin, mVoxelStep);


				for (int ii = 0; ii < checkWidth; ii++)
				{
					Voxel nextVox = lineIter2.next();

					if (nextVox.x < 0 || nextVox.x >= mWidth - 1 || nextVox.y < 0 || nextVox.y >= mHeight - 1 || nextVox.z < 0 || nextVox.z >= mDepth - 1)
						continue;

					if (vData[zStep * nextVox.z + yStep * nextVox.y + nextVox.x] > mAirVoxelFilterVal)
					{
						backCount++;
					}

				}

				if ( frontCount > backCount )
				{
					mVertexNormals[vv] *= -1;
				}
				
			}

		}



	}


}

#include "volumeinfo.moc"