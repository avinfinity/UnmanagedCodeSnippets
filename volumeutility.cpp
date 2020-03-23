#include "volumeutility.h"
#include "QFile"
#include "QDataStream"
#include "QTextStream"
#include "QFileInfo"
#include "QDebug"
#include "src/plugins/Application/ZeissViewer/SegmentationInterface/ZeissSegmentationInterface.hpp"
#include "src/plugins/Application/ZeissViewer/SeparationInterface/ZeissSeparationInterface.hpp"
#include "iostream"

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <map>
#include <algorithm>

//#include <Src/CmdLineParser.h>
#include "IsoSurfaceExtraction/Geometry.h"
#include "IsoSurfaceExtraction/Ply.h"
#include "IsoSurfaceExtraction/MarchingCubes.h"
#include "IsoSurfaceExtraction/Array.h"

#include <ipp.h>

namespace imt{
	
	namespace volume{



		void readUint16_SCV(QString filePath, int w, int h, int d, imt::volume::VolumeInfo& volume)
		{
			QFile file(filePath);

			file.open(QIODevice::ReadOnly);

			QDataStream reader(&file);

			reader.skipRawData(1024);

			volume.mWidth = w;
			volume.mHeight = h;
			volume.mDepth = d;

			volume.mVolumeData = (unsigned char*)(new unsigned short[w * h * d]);

			reader.readRawData((char*)volume.mVolumeData, w * h * d * 2);

			file.close();



		}


		void readVGI(QString filePath, int& w, int& h, int& d, float& voxelStep)
		{
			QFile file(filePath);

			file.open(QIODevice::ReadOnly);

			QTextStream reader(&file);

			qDebug() << reader.readLine() << endl;
			qDebug() << reader.readLine() << endl;
			//qDebug() << reader.readLine() << endl;

			QString title, c1, c2, c3, c4;


			reader >> title >> c1 >> c2 >> c3 >> c4;

			w = c2.toInt();
			h = c3.toInt();
			d = c4.toInt();

			qDebug() << title << " " << c2 << " " << c3 << " " << c4 << endl;

			for (int ii = 0; ii < 16; ii++)
			{
				reader.readLine();
			}

			reader >> title >> c1 >> c1 >> c2 >> c3 >> c4;

			qDebug() << title << " " << c2 << " " << c3 << " " << c4 << endl;

			voxelStep = c2.toDouble();

			file.close();
		}




		void VolumeUtility::loadVgiVolume(QString path, VolumeInfo& volume)
		{


			QFileInfo fileInfo(path);

			QString uint16SCVPath = fileInfo.completeBaseName() + ".uint16_scv";

			int w, h, d;

			float voxelStep;

			readVGI(path, w, h, d, voxelStep);
			readUint16_SCV(path, w, h, d, volume);

			volume.mVoxelStep = Eigen::Vector3f(voxelStep, voxelStep, voxelStep);

		}


		void VolumeUtility::computeISOThreshold( VolumeInfo& volume, int& isoThreshold  , std::vector< __int64 >& histogram , int& minVal , int& maxVal)
		{

			
			ZeissSegmentationInterface segmenter;

			

			Volume vol;

			std::cout << volume.mWidth << " " << volume.mHeight << " " << volume.mDepth << std::endl;

			vol.size[0] = volume.mWidth;
			vol.size[1] = volume.mHeight;
			vol.size[2] = volume.mDepth;

			vol.voxel_size[0] = volume.mVoxelStep(0);
			vol.voxel_size[1] = volume.mVoxelStep(1);
			vol.voxel_size[2] = volume.mVoxelStep(2);

			vol.data = (uint16_t*)volume.mVolumeData;

			SegmentationParameter param;
			
			param.max_memory = static_cast<size_t>(2048) * 1024 * 1024;
			param.output_material_index = false;
			segmenter.setParameter(param);

			segmenter.setInputVolume(vol);

			Materials materials = segmenter.getMaterialRegions();

			isoThreshold = materials.regions[1].lower_bound;

			std::cout << " iso threshold " << isoThreshold << std::endl;

			int nMaterials = materials.regions.size();

			std::cout << " num materials : " << nMaterials << " " << USHRT_MAX << std::endl;

			minVal = materials.regions[1].lower_bound;
			maxVal = materials.regions[nMaterials - 1].upper_bound;



			

			//auto multiReshistograms = segmenter.getMultiResolutionHistograms();
			//return;
			histogram.resize(USHRT_MAX + 1);

			std::fill(histogram.begin(), histogram.end(), 0);

			unsigned short *vdata = ( unsigned short* )volume.mVolumeData;

			for (int zz = 0; zz < volume.mDepth; zz++)
				for (int yy = 0; yy < volume.mHeight; yy++)
					for (int xx = 0; xx < volume.mWidth; xx++)
					{
						//if (*vdata >= 0 && *vdata <= USHRT_MAX)
						{
							histogram[*vdata]++;
						}

						vdata++;
					}
		

			//for (int ii = 0; ii < USHRT_MAX; ii++)
			//{
			//	histogram[ii] = multiReshistograms.first_histogram_data[ii];
			//}


		}


		void VolumeUtility::computeMaterials( SoVolumeData *volumeData , std::vector< Material >& materials )
		{

			auto& ldmDataAccess =  volumeData->getLdmDataAccess();

			//ldmDataAccess.getData(  )

			//first we need to pick optimal resolution

			int optimalResolution = 0;

			int wHigh, hHigh, dHigh;

			auto volumeDimensions = volumeData->getDimension();

			wHigh = volumeDimensions[0];
			hHigh = volumeDimensions[1];
			dHigh = volumeDimensions[2];

			std::cout << " highest resolution dimensions : " << wHigh << " " << hHigh << " " << dHigh << std::endl;


		}




		float  LinearInterpolant( float x1 , float x2 , float isoValue )
		{ 
			return (isoValue - x1) / (x2 - x1); 
		}
		
		float QuadraticInterpolant(float x0, float x1, float x2, float x3, float isoValue)
		{
			// Adjust so that we are looking for a zero-crossing
			x0 -= isoValue, x1 -= isoValue, x2 -= isoValue, x3 -= isoValue;
			// Estimate the derivatives at x1 and x2
			float dx1 = (x2 - x0) / 2.f, dx2 = (x3 - x1) / 2.f;
			// Solve for the quadratic polynomial:
			//		P(x) = a x^2 + b x + c 
			// such that:
			//		P(0) = x1 , P(1) = x2 , and minimizing || P'(0) - dx1 ||^2 + || P'(1) - dx2 ||^2
			//	=>  c = x1 , a = x2 - x1 - b , and minimizing || b - dx1 ||^2 + || 2*a + b - dx2 ||^2
			//	=>  c = x1 , a = x2 - x1 - b , and minimizing || b - dx1 ||^2 + || 2*x2 - 2*x1 - b - dx2 ||^2
			//	=>  c = x1 , a = x2 - x1 - b , and minimizing || b - dx1 ||^2 + || b - ( 2*x2 - 2*x1 - dx2 ) ||^2
			//	=>  c = x1 , b = ( 2*x2 - 2*x1 - dx2 + dx1 ) / 2 , a = x2 - x1 - b
			//	=>  c = x1 , b = ( x2 - x1 ) - ( dx2 - dx1 ) / 2 , a = ( dx2 - dx1 ) / 2

			double a = (dx2 - dx1) / 2.f, b = (dx1 - dx2) / 2.f + x2 - x1, c = x1;
			if (!a)
			{
				// Solve b * x + c = 0
				return (float)(-c / b);
			}
			else
			{
				// Solve a x^2 + b x + c = 0
				b /= a, c /= a;
				double disc = b*b - 4.*c;
				if (disc<0) fprintf(stderr, "[ERROR] Negative discriminant: %g\n", disc), exit(0);
				disc = sqrt(disc);
				double r1 = (-b - disc) / 2., r2 = (-b + disc) / 2.;
				if (r2<0 || r1>1) fprintf(stderr, "[ERROR] Roots out of bounds: %g %g\n", r1, r2), exit(0);
				if (r2>1) return (float)r1;
				else       return (float)r2;
			}
		}

		struct IsoVertex
		{
			int dir, idx[3];
			Point3D< float > p;
			IsoVertex(Point3D< float > p, int dir, int x, int y, int z){ this->p = p, this->dir = dir, idx[0] = x, idx[1] = y, idx[2] = z; }
#define _ABS_( a ) ( (a)<0 ? -(a) : (a) )
			static bool CoFacial(const IsoVertex& t1, const IsoVertex& t2)
			{
				int d[] = { _ABS_(t1.idx[0] - t2.idx[0]), _ABS_(t1.idx[1] - t2.idx[1]), _ABS_(t1.idx[2] - t2.idx[2]) };
				
				if ( t1.dir == t2.dir ) 
					return d[t1.dir] == 0 && ((d[(t1.dir + 1) % 3] == 0 && d[(t1.dir + 2) % 3] <= 1) || (d[(t1.dir + 2) % 3] == 0 && d[(t1.dir + 1) % 3] <= 1));
				else                  
					return d[3 - t1.dir - t2.dir] == 0 && d[t1.dir] <= 1 && d[t2.dir] <= 1;
			}
#undef _ABS_
		};

		void ExtractIsoSurface( int resX, int resY, int resZ, ConstPointer(float) values, float isoValue, std::vector< IsoVertex >& vertices, 
			                    std::vector< std::vector< int > >& polygons, bool fullCaseTable, bool quadratic, bool flip)
		{
#define INDEX( x , y , z ) ( std::min< int >( resX-1 , std::max< int >( 0 , (x) ) ) + std::min< int >( resY-1 , std::max< int >( 0 , (y) ) )*resX + std::min< int >( resZ-1 , std::max< int >( 0 , (z) ) )*resX*resY )
			std::map< long long, int > isoVertexMap[3];
			Pointer(unsigned char) flags = NewPointer< unsigned char >(resX*resY*resZ);

			// Mark the voxels that are larger than the iso value
#pragma omp parallel for
			for (int i = 0; i<resX*resY*resZ; i++) flags[i] = MarchingCubes::ValueLabel(values[i], isoValue);

			// Get the zero-crossings along the x-edges
#pragma omp parallel for
			for (int i = 0; i<resX - 1; i++) for (int j = 0; j<resY; j++) for (int k = 0; k<resZ; k++)
			{
				int idx0 = INDEX(i, j, k), idx1 = INDEX(i + 1, j, k);
				if (flags[idx0] != flags[idx1])
				{
					float iso;
					if (quadratic)
					{
						int _idx0 = INDEX(i - 1, j, k), _idx1 = INDEX(i + 2, j, k);
						iso = QuadraticInterpolant(values[_idx0], values[idx0], values[idx1], values[_idx1], isoValue);
					}
					else iso = LinearInterpolant(values[idx0], values[idx1], isoValue);
					Point3D< float > p = Point3D< float >((float)i + iso, (float)j, (float)k);
					long long key = i + j*(resX)+k*(resX*resY);
#pragma omp critical
					{
						isoVertexMap[0][key] = (int)vertices.size();
						vertices.push_back(IsoVertex(p, 0, i, j, k));
					}
				}
			}

			// Get the zero-crossings along the y-edges
#pragma omp parallel for
			for (int i = 0; i<resX; i++) for (int j = 0; j<resY - 1; j++) for (int k = 0; k<resZ; k++)
			{
				int idx0 = INDEX(i, j, k), idx1 = INDEX(i, j + 1, k);
				if (flags[idx0] != flags[idx1])
				{
					float iso;
					if (quadratic)
					{
						int _idx0 = INDEX(i, j - 1, k), _idx1 = INDEX(i, j + 2, k);
						iso = QuadraticInterpolant(values[_idx0], values[idx0], values[idx1], values[_idx1], isoValue);
					}
					else iso = LinearInterpolant(values[idx0], values[idx1], isoValue);
					Point3D< float > p = Point3D< float >((float)i, (float)j + iso, (float)k);
					long long key = i + j*(resX)+k*(resX*resY);
#pragma omp critical
					{
						isoVertexMap[1][key] = (int)vertices.size();
						vertices.push_back(IsoVertex(p, 1, i, j, k));
					}
				}
			}

			// Get the zero-crossings along the z-edges
#pragma omp parallel for
			for (int i = 0; i<resX; i++) for (int j = 0; j<resY; j++) for (int k = 0; k<resZ - 1; k++)
			{
				int idx0 = INDEX(i, j, k), idx1 = INDEX(i, j, k + 1);
				if (flags[idx0] != flags[idx1])
				{
					float iso;
					if (quadratic)
					{
						int _idx0 = INDEX(i, j, k - 1), _idx1 = INDEX(i, j, k + 2);
						iso = QuadraticInterpolant(values[_idx0], values[idx0], values[idx1], values[_idx1], isoValue);
					}
					else iso = LinearInterpolant(values[idx0], values[idx1], isoValue);
					Point3D< float > p = Point3D< float >((float)i, (float)j, (float)k + iso);
					long long key = i + j*(resX)+k*(resX*resY);
#pragma omp critical
					{
						isoVertexMap[2][key] = (int)vertices.size();
						vertices.push_back(IsoVertex(p, 2, i, j, k));
					}
				}
			}

			// Iterate over the cubes and get the polygons
			if (fullCaseTable) MarchingCubes::SetFullCaseTable();
			else                MarchingCubes::SetCaseTable();

#pragma omp parallel for
			for (int i = 0; i<resX - 1; i++) for (int j = 0; j<resY - 1; j++) for (int k = 0; k<resZ - 1; k++)
			{
				float _values[Cube::CORNERS];
				for (int cx = 0; cx<2; cx++) for (int cy = 0; cy<2; cy++) for (int cz = 0; cz<2; cz++) _values[Cube::CornerIndex(cx, cy, cz)] = values[(i + cx) + (j + cy)*resX + (k + cz)*resX*resY];
				int mcIndex = fullCaseTable ? MarchingCubes::GetFullIndex(_values, isoValue) : MarchingCubes::GetIndex(_values, isoValue);
				const std::vector< std::vector< int > >& isoPolygons = MarchingCubes::caseTable(mcIndex, fullCaseTable);
				for (int p = 0; p<isoPolygons.size(); p++)
				{
					const std::vector< int >& isoPolygon = isoPolygons[p];
					std::vector< int > polygon(isoPolygon.size());
					for (int v = 0; v<isoPolygon.size(); v++)
					{
						int orientation, i1, i2;
						Cube::FactorEdgeIndex(isoPolygon[v], orientation, i1, i2);
						long long key;
						switch (orientation)
						{
						case 0: key = (i)+(j + i1)*resX + (k + i2)*resX*resY; break;
						case 1: key = (i + i1) + (j)*resX + (k + i2)*resX*resY; break;
						case 2: key = (i + i1) + (j + i2)*resX + (k)*resX*resY; break;
						}
						std::map< long long, int >::const_iterator iter = isoVertexMap[orientation].find(key);
						if (iter == isoVertexMap[orientation].end()) fprintf(stderr, "[ERROR] Couldn't find iso-vertex in map.\n"), exit(0);
						if (flip) polygon[polygon.size() - 1 - v] = iter->second;
						else       polygon[v] = iter->second;
					}
#pragma omp critical
					polygons.push_back(polygon);
				}
			}
			DeletePointer(flags);
#undef INDEX
		}



		void VolumeUtility::extractIsoSurface( VolumeInfo& volume , int& isoThreshold )
		{
			Pointer(float) voxelValues = NewPointer< float >( volume.mWidth * volume.mHeight * volume.mDepth );


#define INDEX( x , y , z ) ( (x) + (y)*volume.mWidth + (z)*volume.mWidth * volume.mHeight )

			float min, max;
			min = max = voxelValues[0];

			for (int x = 0; x<volume.mWidth; x++) 
				for (int y = 0; y<volume.mHeight; y++) 
					for (int z = 0; z < volume.mDepth; z++)
					{
						min = std::min< float >(min, voxelValues[INDEX(x, y, z)]), max = std::max< float >(max, voxelValues[INDEX(x, y, z)]);
					}


#undef INDEX

			std::vector< IsoVertex > vertices;
			std::vector< std::vector< int > > polygons;
			
			ExtractIsoSurface( volume.mWidth , volume.mHeight , volume.mDepth , voxelValues , isoThreshold , vertices , polygons , 1 , 1 , 0 );

			std::vector< PlyVertex< float > > _vertices(vertices.size());

			for (int i = 0; i < vertices.size(); i++)
				for (int d = 0; d < 3; d++)
					_vertices[i].point[d] = vertices[i].p[d] * volume.mVoxelStep(d); //Dimensions.values[d];
			
			//if (Polygons.set)
			//{
			//	PlyWritePolygons(Out.value, _vertices, polygons, PlyVertex< float >::WriteProperties, PlyVertex< float >::WriteComponents, PLY_BINARY_NATIVE);
			//	printf("Vertices / Polygons: %d / %d\n", (int)vertices.size(), (int)polygons.size());
			//}
			//else
			//{
				std::vector< TriangleIndex > triangles;
				MinimalAreaTriangulation< float > mat;

				for (int i = 0; i<polygons.size(); i++)
				{
					// To ensure that we have no more than two triangles adjacent on an edge,
					// we avoid creating a minimial area triangulation when it could introduce a new
					// edge that is on a face of a cube
					bool isCofacial = false;
					
					//if (!NonManifold.set)
					for ( int j = 0; j<(int)polygons[i].size(); j++) for (int k = 0; k<j; k++)
						  if ((j + 1) % polygons[i].size() != k && (k + 1) % polygons[i].size() != j)
							 if (IsoVertex::CoFacial(vertices[polygons[i][j]], vertices[polygons[i][k]])) isCofacial = true;

					if (isCofacial)
					{
						TriangleIndex triangle;
						PlyVertex< float > plyVertex;
						
						for (int j = 0; j<(int)polygons[i].size(); j++) 
							plyVertex.point += vertices[polygons[i][j]].p;
						
						plyVertex.point /= (float)polygons[i].size();
						
						int cIdx = (int)_vertices.size();
						
						for (int d = 0; d < 3; d++) 
							plyVertex.point[d] *= volume.mVoxelStep(d);//Dimensions.values[d];
						
						_vertices.push_back(plyVertex);
						
						for (int j = 0; j<(int)polygons[i].size(); j++)
						{
							triangle[0] = polygons[i][j];
							triangle[1] = polygons[i][(j + 1) % polygons[i].size()];
							triangle[2] = cIdx;
							triangles.push_back(triangle);
						}
					}
					else
					{
						std::vector< Point3D< float > > _polygon(polygons[i].size());
						std::vector< TriangleIndex > _triangles;
						for (int j = 0; j<polygons[i].size(); j++) _polygon[j] = vertices[polygons[i][j]].p;
						mat.GetTriangulation(_polygon, _triangles);
						for (int j = 0; j<_triangles.size(); j++)
						{
							TriangleIndex tri;
							for (int k = 0; k<3; k++) tri[k] = polygons[i][_triangles[j][k]];
							triangles.push_back(tri);
						}
					}
				}

				//PlyWriteTriangles(Out.value, _vertices, triangles, PlyVertex< float >::WriteProperties, PlyVertex< float >::WriteComponents, PLY_BINARY_NATIVE);
				//printf("Vertices / Triangles: %d / %d\n", (int)_vertices.size(), (int)triangles.size());
			//}
			
			DeletePointer(voxelValues);
				     

		}
		
		VolumeUtility::SobelGradientOperator3x3::SobelGradientOperator3x3()
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



		void VolumeUtility::SobelGradientOperator3x3::init(size_t volumeW, size_t volumeH, size_t volumeD, double voxelSizeX,
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


		void VolumeUtility::SobelGradientOperator3x3::apply(Eigen::Vector3f& point, Eigen::Vector3f& gradient)
		{
			double sumx = 0, sumy = 0, sumz = 0;

			double x = point(0) / _VoxelSizeX;
			double y = point(1) / _VoxelSizeY;
			double z = point(2) / _VoxelSizeZ;

			size_t zStep = _VolumeW * _VolumeH;
			size_t yStep = _VolumeW;

			unsigned short *volumeData = _VolumeData;

			for (int mm = -1; mm <= 1; mm++)
				for (int nn = -1; nn <= 1; nn++)
					for (int kk = -1; kk <= 1; kk++)
					{
						int xx = x + mm;
						int yy = y + nn;
						int zz = z + kk;

						unsigned short gVal = grayValue(xx, yy, zz);

						sumx += _KernelGx[mm + 1][nn + 1][kk + 1] * gVal;
						sumy += _KernelGy[mm + 1][nn + 1][kk + 1] * gVal;
						sumz += _KernelGz[mm + 1][nn + 1][kk + 1] * gVal;
					}


			gradient(0) = sumx;
			gradient(1) = sumy;
			gradient(2) = sumz;
		}

		
		void VolumeUtility::volumeGradient(Eigen::Vector3f& position, imt::volume::VolumeInfo& volume, Eigen::Vector3f& gradient)
		{
			SobelGradientOperator3x3 gradientOp;

			gradientOp.init(volume.mWidth, volume.mHeight, volume.mDepth, volume.mVoxelStep(0), volume.mVoxelStep(1), volume.mVoxelStep(2), (unsigned short*)volume.mVolumeData);
		   
			gradientOp.apply(position, gradient);
		
		}


		void VolumeUtility::volumeGradient( std::vector<Eigen::Vector3f>& position, imt::volume::VolumeInfo& volume, std::vector<Eigen::Vector3f>& gradients )
		{
			SobelGradientOperator3x3 gradientOp;

			gradientOp.init(volume.mWidth, volume.mHeight, volume.mDepth, volume.mVoxelStep(0), volume.mVoxelStep(1), volume.mVoxelStep(2), (unsigned short*)volume.mVolumeData);

			int64_t numVertices = position.size();

			gradients.resize(numVertices);

			for (int64_t vv = 0; vv < numVertices; vv++)
			{
				gradientOp.apply(position[vv], gradients[vv]);
			}
			
		}




		// Type of interpolation, the following values are possible :

		// IPPI_INTER_NN - nearest neighbor interpolation,

		//	IPPI_INTER_LINEAR - trilinear interpolation,

		//	IPPI_INTER_CUBIC - tricubic interpolation,

		//	IPPI_INTER_CUBIC2P_BSPLINE - B - spline,

		//	IPPI_INTER_CUBIC2P_CATMULLROM - Catmull - Rom spline,

		//	IPPI_INTER_CUBIC2P_B05C03 - special two - parameters filter(1 / 2, 3 / 10).


		void VolumeUtility::resizeVolume_Uint16C1(unsigned short *inputVolume, int iWidth, int iHeight, int iDepth, unsigned short *outputVolume, int oWidth,
			int oHeight, int oDepth, double resizeRatio)
		{
			IpprVolume inputVolumeSize, outputVolumeSize;

			int srcStep = iWidth * sizeof(unsigned short);
			int srcPlaneStep = iWidth * iHeight * sizeof(unsigned short);
			IpprCuboid srcVoi;

			int dstStep = oWidth * sizeof(unsigned short);
			int dstPlaneStep = oWidth * oHeight * sizeof(unsigned short);
			IpprCuboid dstVoi;

			double xFactor = resizeRatio, yFactor = resizeRatio, zFactor = resizeRatio;
			double xShift = 0, yShift = 0, zShift = 0;

			int interpolation = IPPI_INTER_LINEAR;//IPPI_INTER_CUBIC2P_B05C03;//

												  // Type of interpolation, the following values are possible :

												  // IPPI_INTER_NN - nearest neighbor interpolation,

												  //	IPPI_INTER_LINEAR - trilinear interpolation,

												  //	IPPI_INTER_CUBIC - tricubic interpolation,

												  //	IPPI_INTER_CUBIC2P_BSPLINE - B - spline,

												  //	IPPI_INTER_CUBIC2P_CATMULLROM - Catmull - Rom spline,

												  //	IPPI_INTER_CUBIC2P_B05C03 - special two - parameters filter(1 / 2, 3 / 10).

			inputVolumeSize.width = iWidth;
			inputVolumeSize.height = iHeight;
			inputVolumeSize.depth = iDepth;

			srcVoi.x = 0;
			srcVoi.y = 0;
			srcVoi.z = 0;

			srcVoi.width = iWidth;
			srcVoi.height = iHeight;
			srcVoi.depth = iDepth;

			dstVoi.x = 0;
			dstVoi.y = 0;
			dstVoi.z = 0;

			dstVoi.width = oWidth;
			dstVoi.height = oHeight;
			dstVoi.depth = oDepth;

			Ipp8u *computationBuffer;

			int bufferSize = 0;



#if 0
			ipprResizeGetBufSize(srcVoi, dstVoi, 1, interpolation, &bufferSize);

			computationBuffer = new Ipp8u[bufferSize];

			ipprResize_16u_C1V(inputVolume, inputVolumeSize, srcStep, srcPlaneStep, srcVoi, outputVolume, dstStep,
				dstPlaneStep, dstVoi, xFactor, yFactor, zFactor, xShift, yShift, zShift, interpolation, computationBuffer);

			delete[] computationBuffer;
#endif
		}


		void VolumeUtility::resizeVolume_Uint16C1_MT(unsigned short *inputVolume, int iWidth, int iHeight, int iDepth, unsigned short *outputVolume, int oWidth,
			int oHeight, int oDepth, double resizeRatio)
		{
			int bandWidth = 32;

			int numBands = oDepth % 32 == 0 ? oDepth / bandWidth : oDepth / bandWidth + 1;

			int extension = (1.0 / resizeRatio) + 1;

			if (numBands > 1)
			{

				double progress = 0;

#pragma omp parallel for
				for (int bb = 0; bb < numBands; bb++)
				{
					IpprVolume inputVolumeSize, outputVolumeSize;

					int srcStep = iWidth * sizeof(unsigned short);
					int srcPlaneStep = iWidth * iHeight * sizeof(unsigned short);
					IpprCuboid srcVoi;

					int dstStep = oWidth * sizeof(unsigned short);
					int dstPlaneStep = oWidth * oHeight * sizeof(unsigned short);
					IpprCuboid dstVoi;

					double xFactor = resizeRatio, yFactor = resizeRatio, zFactor = resizeRatio;
					double xShift = 0, yShift = 0, zShift = 0;

					int interpolation = IPPI_INTER_LINEAR;

					int sourceBand = (int)(bandWidth / resizeRatio + 1);

					if (bb == 0 || bb == numBands - 1)
					{
						sourceBand += extension;
					}
					else
					{
						sourceBand += 2 * extension;
					}

					double destStartLine = bandWidth * bb;

					int destBandWidth = bandWidth;

					double sourceStartLine = destStartLine / resizeRatio;

					if (bb == numBands - 1)
					{
						destBandWidth = oDepth - bandWidth * bb;
					}

					dstVoi.x = 0;
					dstVoi.y = 0;
					dstVoi.z = 0;

					dstVoi.width = oWidth;
					dstVoi.height = oHeight;
					dstVoi.depth = destBandWidth;

					size_t sourceDataShift = 0;
					size_t destDataShift = 0;

					double sourceLineZ = bandWidth * bb / resizeRatio;

					int sourceStartZ = (int)(sourceLineZ - extension);

					if (bb == 0)
						sourceStartZ = 0;

					if (bb == numBands - 1)
					{
						sourceBand = iDepth - sourceStartZ;
					}

					srcVoi.x = 0;
					srcVoi.y = 0;
					srcVoi.z = 0;

					srcVoi.width = iWidth;
					srcVoi.height = iHeight;
					srcVoi.depth = sourceBand;

					inputVolumeSize.width = iWidth;
					inputVolumeSize.height = iHeight;
					inputVolumeSize.depth = sourceBand;

					sourceDataShift = (size_t)sourceStartZ * (size_t)iWidth * (size_t)iHeight;
					destDataShift = (size_t)bandWidth * (size_t)bb * (size_t)oWidth * (size_t)oHeight;

					Ipp8u *computationBuffer;

					zShift = -destStartLine + sourceStartZ * resizeRatio;

					int bufferSize = 0;

#if 0
					ipprResizeGetBufSize(srcVoi, dstVoi, 1, interpolation, &bufferSize);

					computationBuffer = new Ipp8u[bufferSize];

					ipprResize_16u_C1V(inputVolume + sourceDataShift, inputVolumeSize, srcStep, srcPlaneStep,
						srcVoi, outputVolume + destDataShift, dstStep, dstPlaneStep, dstVoi,
						xFactor, yFactor, zFactor, xShift, yShift, zShift, interpolation, computationBuffer);

					delete[] computationBuffer;
#endif

				}

			}
			else
			{
#if 0
				resizeVolume_Uint16C1(inputVolume, iWidth, iHeight, iDepth, outputVolume, oWidth, oHeight, oDepth, resizeRatio);
#endif
			}

		}




		void VolumeUtility::copyRoi(unsigned short *srcData, VolumeUtility::VolumeDim& srcDim, VolumeUtility::Roi& srcRoi, unsigned short *dstData,
			VolumeUtility::VolumeDim& dstDim, VolumePosition& dstStartPos)
		{

			size_t dstLineStep = dstDim._Width;
			size_t dstPlaneStep = dstDim._Width * dstDim._Height;

			size_t srcLineStep = srcDim._Width;
			size_t srcPlaneStep = srcDim._Width * srcDim._Height;

			//perform sanity check here 

			unsigned short *buffer = dstData + dstPlaneStep * dstStartPos.pos[2] + dstLineStep * dstStartPos.pos[1] + dstStartPos.pos[0];

			unsigned short *volumeBuffer = srcData + srcPlaneStep * srcRoi._Start[2] + srcLineStep * srcRoi._Start[1] + srcRoi._Start[0];


			size_t lineBufferSize = srcRoi._Dims[0] * sizeof(unsigned short);

			for (int zz = 0; zz < srcRoi._Dims[2]; zz++)
				for (int yy = 0; yy < srcRoi._Dims[1]; yy++)
				{
					memcpy(buffer, volumeBuffer, lineBufferSize);

					buffer += dstLineStep;
					volumeBuffer += srcLineStep;
				}
		}

		
		
		
	}
	
	
	
	
}