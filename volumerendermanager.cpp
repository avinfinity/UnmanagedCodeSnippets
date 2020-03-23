#include "volumerendermanager.h"
#include "openglhelper.h"
#include "iostream"
#include "fstream"
#include "opencvincludes.h"
#include <vtkSmartPointer.h>
#include <vtkMarchingCubes.h>
#include <vtkImageData.h>
#include "vtkVolume.h"
#include "voxel.h"
#include "lineiterator.h"
#include "QMatrix4x4"


namespace imt{
	
	namespace volume{
		
		
	VolumeRenderManager::VolumeRenderManager(int defaultFBO) : mDefaultFBO( defaultFBO )
	{
		mRenderWidth = 640;
		mRenderHeight = 480;

		mRenderMode = VOLUME;//SURFACE; //

		mSurfaceDataChanged = false;
		mSphereSurfaceDataChanged = false;
		mHasNewData = false;
		mHeatMapViewingEnabled = true;
		mViewPortEnabled = false;


		_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
		_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

		//rtcInit(NULL);

		mScene = 0;

		mDevice = rtcNewDevice(NULL);

		mPickingEnabled = true;

		mProgressReporter = 0;

		//mCloud = pcl::PointCloud<pcl::PointXYZ>::Ptr( new pcl::PointCloud<pcl::PointXYZ>() );

		mVolumeInfo = 0;

		mIsInitialized = false;

		mTracerBuilt = false;




	}

	bool VolumeRenderManager::isInitialized()
	{
		return mIsInitialized;
	}

	void VolumeRenderManager::setChangedData()
	{

		std::cout << " setting changed data " << std::endl;

		mVolumeInfo->mVertexColors.resize( mVolumeInfo->mVertices.size(), Eigen::Vector3f(1, 1, 1) );
		mVolumeInfo->mRayVertexColors.resize( mVolumeInfo->mVertices.size(), Eigen::Vector3f(1, 1, 1) );
		mVolumeInfo->mSphereVertexColors.resize( mVolumeInfo->mVertices.size(), Eigen::Vector3f(1, 1, 1) );

		std::fill( mVolumeInfo->mRayVertexColors.begin(), mVolumeInfo->mRayVertexColors.end(), Eigen::Vector3f(1, 1, 1) );
		std::fill( mVolumeInfo->mSphereVertexColors.begin(), mVolumeInfo->mSphereVertexColors.end(), Eigen::Vector3f(1, 1, 1) );

		loadSurface( mVolumeInfo->mVertices, mVolumeInfo->mRayVertexColors, mVolumeInfo->mSphereVertexColors,
			         mVolumeInfo->mVertexNormals, mVolumeInfo->mFaceIndices);

	}


		
	void VolumeRenderManager::init()
	{
		initializeOpenGLFunctions();

		GL_CHECK( glGenBuffers( 2 , mVolumeVBO ) );
		GL_CHECK( glGenBuffers( 3 , mSurfaceVBO ) );
		GL_CHECK( glGenVertexArrays( 1, &mVolumeVAO ) );
		GL_CHECK(glGenVertexArrays(1, &mSurfaceVAO)); 
		GL_CHECK(glGenVertexArrays(1, &mSphereVAO));
		GL_CHECK( glGenTextures( 1 , &mVolumeTex ) );
		GL_CHECK( glGenTextures( 1 , &mColorTex ) );
		GL_CHECK( glGenTextures( 1, &mIntersectionsTex ) );
		GL_CHECK( glGenFramebuffers( 1 , &mIntersectionsFBO ) );

		

		GL_CHECK( glBindTexture(GL_TEXTURE_3D, mVolumeTex) );

		GL_CHECK( glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR) );
		GL_CHECK( glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR) );
		GL_CHECK( glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT) );
		GL_CHECK( glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT) );
		GL_CHECK( glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT) );

		GL_CHECK(glTexImage3D(GL_TEXTURE_3D, 0, GL_R16, 10,
			10 , 10, 0, GL_RED, GL_UNSIGNED_SHORT, 0));

		GL_CHECK(glBindTexture(GL_TEXTURE_3D, 0));

		GL_CHECK( glBindTexture(GL_TEXTURE_2D, mIntersectionsTex) );
		GL_CHECK( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		GL_CHECK( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GL_CHECK( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		GL_CHECK( glTexImage2D( GL_TEXTURE_2D , 0 , GL_RGB32F , mRenderWidth ,
			                    mRenderHeight , 0 , GL_RGB , GL_FLOAT , 0 ));
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));




		const int MAX_CNT = 65535 * 3;
		GLubyte *tff = (GLubyte *)calloc(MAX_CNT, sizeof(float));

		std::string fileName = "C:/Data/ZxoData/separated_part_7_colormap.dat";//":/tff.dat";

		FILE *file = fopen( fileName.c_str(), "rb" );

		// read in the user defined data of transfer function
		//std::ifstream inFile(fileName, std::ifstream::in);
		
		/*if (!inFile)
		{
			std::cerr << "Error openning file: " << fileName << std::endl;
			exit(EXIT_FAILURE);
		}*/

		//inFile.read( reinterpret_cast<char *>(tff), 4096 * 3 * sizeof(float) );
		
		fread(tff, sizeof(float), MAX_CNT, file);

		/*if (inFile.eof())
		{
			size_t bytecnt = inFile.gcount();
			*(tff + bytecnt) = '\0';
			std::cout << "bytecnt " << bytecnt << std::endl;
		}
		else if (inFile.fail())
		{
			std::cout << fileName << "read failed " << std::endl;
		}
		else
		{
			std::cout << fileName << "is too large" << std::endl;
		}*/

		
		GL_CHECK(glBindTexture(GL_TEXTURE_1D, mColorTex));
		GL_CHECK(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		GL_CHECK(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GL_CHECK(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
        GL_CHECK(glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, 8192, 0, GL_RGBA, GL_FLOAT, tff));
        GL_CHECK(glBindTexture(GL_TEXTURE_1D, 0));




		GL_CHECK( glBindFramebuffer( GL_DRAW_FRAMEBUFFER, mIntersectionsFBO ) );
		GL_CHECK(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			                              GL_TEXTURE_2D, mIntersectionsTex, 0 ) );
		GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));


		// draw the six faces of the boundbox by drawwing triangles
		// draw it conter-clockwise
		// front: 1 5 7 3
		// back: 0 2 6 4
		// left：0 1 3 2
		// right:7 5 4 6    
		// up: 2 3 7 6
		// down: 1 0 4 5
		GLuint indices[36] = {
			1, 5, 7,
			7, 3, 1,
			0, 2, 6,
			6, 4, 0,
			0, 1, 3,
			3, 2, 0,
			7, 5, 4,
			4, 6, 7,
			2, 3, 7,
			7, 6, 2,
			1, 0, 4,
			4, 5, 1
		};


		mVertices.resize( 8 );
		mIndices.resize( 36 );

		memcpy(mIndices.data(), indices, 36 * sizeof(GLuint));
		initializeShaders();

		mHasNewData = true;

		mIsInitialized = true;
	}



	void VolumeRenderManager::setWallthicknessEstimator( imt::volume::WallthicknessEstimator *wte )
	{
		mWte = wte;
	}

	imt::volume::WallthicknessEstimator *VolumeRenderManager::wallthicknessEstimator()
	{
		return mWte;
	}


	void VolumeRenderManager::setPlaneType(int planeType, Eigen::Vector3f& planeCoeff, int w, int h)
	{
		mPlaneType = planeType;

		mPlaneCoeff = planeCoeff;

		mSliceWindowWidth = w;
		mSliceWindowHeight = h;

	}
	

	void VolumeRenderManager::buildOrthodProjectionMatrix()
	{

		//float leftV, rightV, bottomV, topV, nearV, farV;

		//Eigen::Vector3f xLimits, yLimits, zLimits;

		//xLimits(0) = 0;
		////yLim


		//if (mPlaneType == XY)
		//{
		//	leftV = xLimits(0);
		//	rightV = xLimits(1);
		//	bottomV = yLimits(0);
		//	topV = yLimits(1);
		//	nearV = zLimits(0);
		//	farV = zLimits(1);

		//}
		//else if (mPlaneType == YZ)
		//{
		//	leftV = yLimits(0);
		//	rightV = yLimits(1);
		//	bottomV = zLimits(0);
		//	topV = zLimits(1);
		//	nearV = xLimits(0);
		//	farV = xLimits(1);
		//}
		//else
		//{
		//	leftV = zLimits(0);
		//	rightV = zLimits(1);
		//	bottomV = xLimits(0);
		//	topV = xLimits(1);
		//	nearV = yLimits(0);
		//	farV = yLimits(1);
		//}



		//QMatrix4x4 orthoProjMat;

		//orthoProjMat.frustum(leftV, rightV, bottomV, topV, nearV, farV);




	}





	void VolumeRenderManager::setCamera(TrackBallCamera *camera)
	{
		mCamera = camera;
	}
		

	void VolumeRenderManager::setDefaultFrameBuffer(GLuint defaultFBO)
	{
		mDefaultFBO = defaultFBO;
	}


	void VolumeRenderManager::setRenderMode(RENDER_MODE mode)
	{
		mRenderMode = mode;
	}


	void VolumeRenderManager::setVolumeInfo(VolumeInfo *volumeInfo)
	{
		mVolumeInfo = volumeInfo;
	}
		
		
	void VolumeRenderManager::render()
	{


		if (!mIsInitialized)
			return;

		GL_CHECK(glEnable(GL_BLEND));
		GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

		

		GL_CHECK(glBindVertexArray(mVolumeVAO));

		int w, h;

		mCamera->getDimensions(w, h);

		if (mRenderWidth != w || mRenderHeight != h)
		{
			mRenderWidth = w;
			mRenderHeight = h;

			GL_CHECK( glBindTexture( GL_TEXTURE_2D, mIntersectionsTex ));

			GL_CHECK( glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F, mRenderWidth,
				                    mRenderHeight, 0, GL_RGB, GL_FLOAT, 0 ) );

			GL_CHECK( glBindTexture( GL_TEXTURE_2D , 0 ) );
		}

		if (mHasNewData)
		{
			GLfloat vertices[24] =
			{
				0.0, 0.0, 0.0,
				0.0, 0.0, mDepth * mVolumeStep(2),
				0.0, mHeight* mVolumeStep(1), 0.0,
				0.0, mHeight* mVolumeStep(1), mDepth* mVolumeStep(2),
				mWidth* mVolumeStep(0), 0.0, 0.0,
				mWidth* mVolumeStep(0), 0.0, mDepth* mVolumeStep(2),
				mWidth* mVolumeStep(0), mHeight* mVolumeStep(1), 0.0,
				mWidth* mVolumeStep(0), mHeight* mVolumeStep(1), mDepth* mVolumeStep(2)
			};

			memcpy( mVertices.data() , vertices , 8 * sizeof(Eigen::Vector3f) );

			GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER , mVolumeVBO[0] ) );
			GL_CHECK( glBufferData( GL_ARRAY_BUFFER , mVertices.size() * sizeof(Eigen::Vector3f),
				                    (void *)mVertices.data(), GL_STATIC_DRAW));

			std::cout << " setting cube indices " << std::endl;

			//update points
			GL_CHECK(glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mVolumeVBO[1] ) );
			GL_CHECK(glBufferData( GL_ELEMENT_ARRAY_BUFFER, mIndices.size() * sizeof(GLuint),
				                   mIndices.data(), GL_STATIC_DRAW ) );

			//update faces
			GL_CHECK(glEnableVertexAttribArray(0));

			GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Eigen::Vector3f), 0));

			mHasNewData = false;

			Eigen::Vector3f center(0, 0, 0);

			center(0) = mWidth / 2.0 * mVolumeStep(0);// +mVolumeOrigin(0);
			center(1) = mHeight / 2.0 * mVolumeStep(1);// +mVolumeOrigin(1);
			center(2) = mDepth / 2.0 * mVolumeStep(2);// +mVolumeOrigin(2);

			float radius = (center - mVolumeOrigin).norm();

			mCamera->setObjectCenter(QVector3D(center(0), center(1), center(2)));
			mCamera->setRadius(radius);

			mCamera->init();

		}

		GL_CHECK(glBindVertexArray(0));

		if (mRenderMode == VOLUME)
		{
		    renderIntersections();
			renderVolume();
		}
		else
		{
			if ( mProgressReporter->mShowSideBySideRS )
			{
				renderRay();
				renderSphere();
			}
			else 
			{
				renderSurface();
			}
			
		}


	}
		

	void VolumeRenderManager::renderIntersections()
	{

		GL_CHECK(glUseProgram(mVolumeVizBackFaceProgram));

		std::cout << " back face intersection program " << mVolumeVizBackFaceProgram << std::endl;

		GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mIntersectionsFBO));//mIntersectionsFBO

		GL_CHECK(glCullFace(GL_FRONT));

			GL_CHECK(glBindVertexArray(mVolumeVAO));
			GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

			GLint mvpMatrix = glGetUniformLocation(mVolumeVizBackFaceProgram, "mvpMatrix");

			//GLint planeNormalLoc = glGetUniformLocation(mMeshCrossSectionProgram, "planeNormal");
			//GLint planeCoeffLoc = glGetUniformLocation(mMeshCrossSectionProgram, "planeCoeff");
			//GLint planeTypeLoc = glGetUniformLocation(mMeshCrossSectionProgram, "pType");


			QMatrix4x4 mvpMat = mCamera->getModelViewProjectionMatrix();

			GL_CHECK(glUniformMatrix4fv(mvpMatrix, 1, false, mvpMat.data())); //mvpMat.data()

			//GL_CHECK(glUniform3f(planeCoeffLoc, mCoeff(0), mCoeff(1), mCoeff(2))); //mvpMat.data()

			GL_CHECK(glDrawElements(GL_TRIANGLES, mIndices.size(), GL_UNSIGNED_INT , 0));//, (GLuint *)NULL)

			GL_CHECK(glBindVertexArray(0));

			GL_CHECK(glUseProgram(0));



		

	}

	void VolumeRenderManager::renderVolume()
	{
		//std::cout << " default fbo : " << mDefaultFBO << std::endl;

		GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mDefaultFBO));

		GL_CHECK(glBindVertexArray(mVolumeVAO));
		GL_CHECK(glUseProgram(mVolumeVizProgram));
		//update faces
		//GL_CHECK(glEnableVertexAttribArray(0));

		GL_CHECK(glCullFace(GL_BACK));
		
		GLint mvpMatrix = glGetUniformLocation(mVolumeVizProgram, "mvpMatrix");
		QMatrix4x4 mvpMat = mCamera->getModelViewProjectionMatrix();

		GL_CHECK(glUniformMatrix4fv(mvpMatrix, 1, false, mvpMat.data()));

		GLint transferFuncLoc = glGetUniformLocation(mVolumeVizProgram, "TransferFunc");

		if ( transferFuncLoc >= 0 )
		{
			std::cout << " transfer function location " << transferFuncLoc << std::endl;

			GL_CHECK(glActiveTexture(GL_TEXTURE0));
			GL_CHECK(glBindTexture(GL_TEXTURE_1D, mColorTex));
			GL_CHECK(glUniform1i(transferFuncLoc, 0));
		} //StepSize

		GLint exitPointsLoc = glGetUniformLocation(mVolumeVizProgram, "ExitPoints");

		if ( exitPointsLoc >= 0 )
		{
			std::cout << " setting intersection texture : " << exitPointsLoc << " " << GL_INVALID_VALUE << std::endl;

			GL_CHECK( glActiveTexture( GL_TEXTURE1 ) );
			GL_CHECK( glBindTexture( GL_TEXTURE_2D , mIntersectionsTex ) );
			GL_CHECK( glUniform1i( exitPointsLoc , 1 ) );
		} 

		GLint stepSizeLoc = glGetUniformLocation(mVolumeVizProgram, "StepSize");

		GL_CHECK(glUniform1f(stepSizeLoc, mVolumeStep(0)));

		GLint screenSizeLoc = glGetUniformLocation(mVolumeVizProgram, "ScreenSize");

		//std::cout << " screen size : " << mRenderWidth << " " << mRenderHeight << std::endl;

		GL_CHECK( glUniform2f(screenSizeLoc, mRenderWidth, mRenderHeight) );

		GLint volumeLoc = glGetUniformLocation(mVolumeVizProgram, "VolumeTex");

		if (volumeLoc >= 0)
		{
			//std::cout << " binding volume tex " << std::endl;

			GL_CHECK( glActiveTexture(GL_TEXTURE2) );
			GL_CHECK( glBindTexture(GL_TEXTURE_3D, mVolumeTex) );
			GL_CHECK( glUniform1i(volumeLoc, 2) );
		}

		GLint wLoc = glGetUniformLocation(mVolumeVizProgram, "w");
		GL_CHECK(glUniform1f(wLoc, mWidth * mVolumeStep(0)));

		GLint hLoc = glGetUniformLocation(mVolumeVizProgram, "h");
		GL_CHECK(glUniform1f(hLoc, mHeight * mVolumeStep(0)));
		
		GLint dLoc = glGetUniformLocation(mVolumeVizProgram, "d");
		GL_CHECK(glUniform1f(dLoc, mDepth * mVolumeStep(0)));


		GL_CHECK( glDrawElements( GL_TRIANGLES , 36 , GL_UNSIGNED_INT , (GLuint *)NULL ) );

		GL_CHECK(glBindVertexArray(0));
		GL_CHECK(glUseProgram(0));


	}


	int VolumeRenderManager::sliceRenderProgram()
	{
		return mMeshCrossSectionProgram;
	}


	void VolumeRenderManager::renderSurface()
	{


#if 0
		

		int w, h;

		mCamera->getDimensions(w, h);

		if ( mViewPortEnabled )
		{
			GL_CHECK( glViewport(0, 0, w / 2, h) );
		}
		

		GL_CHECK(glUseProgram(mSurfaceProgram));

		GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mDefaultFBO));

		GL_CHECK(glBindVertexArray(mSurfaceVAO));
		GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		GL_CHECK(glCullFace(GL_BACK));

		

		if (mSurfaceDataChanged) 
		{
			std::cout << " loading surface data "<< mSurfaceVertices.size() << " " << mSurfaceIndices.size() / 3 <<" "<<mDefaultFBO<< std::endl;

			GL_CHECK(glBindBuffer( GL_ARRAY_BUFFER, mSurfaceVBO[0] ));
			GL_CHECK(glBufferData( GL_ARRAY_BUFFER, mSurfaceVertices.size() * sizeof(VertexData),
				                   (void *)mSurfaceVertices.data(), GL_STATIC_DRAW));

			//update points
			GL_CHECK(glBindBuffer( GL_ELEMENT_ARRAY_BUFFER , mSurfaceVBO[1]) );
			GL_CHECK(glBufferData( GL_ELEMENT_ARRAY_BUFFER , mSurfaceIndices.size() * sizeof(GLuint),
				                   mSurfaceIndices.data()  , GL_STATIC_DRAW ) );

			//update faces
			GL_CHECK(glEnableVertexAttribArray(0));
			GL_CHECK(glEnableVertexAttribArray(1));
			GL_CHECK(glEnableVertexAttribArray(2));

			int offset = 0;

			GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), 0));

			offset += sizeof(Eigen::Vector3f);

			GL_CHECK(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offset ));

			offset += sizeof(Eigen::Vector3f);

			GL_CHECK(glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offset ));

			mSurfaceDataChanged = false;

		}

		

		GLint mvpMatrix = glGetUniformLocation(mSurfaceProgram, "mvpMatrix");
		QMatrix4x4 mvpMat = mCamera->getModelViewProjectionMatrix();

		if (mViewPortEnabled)
		{
			mvpMat = mCamera->getModelViewProjectionMatrix( w / 2, h);
		}

		GLint mvMatrix = glGetUniformLocation(mSurfaceProgram, "mvMatrix");
		QMatrix4x4 mvMat = mCamera->getModelViewMatrix();

		GLint rTLoc = glGetUniformLocation(mSurfaceProgram, "rT");
		Eigen::Matrix3f rT = mCamera->getNormalTransformationMatrix();



		GL_CHECK(glUniformMatrix4fv(mvpMatrix, 1, false, mvpMat.data()));
		GL_CHECK(glUniformMatrix4fv(mvMatrix, 1, false, mvMat.data()));
		GL_CHECK(glUniformMatrix3fv(rTLoc, 1, false, rT.data()));


		if ( mPickingEnabled && mTracerBuilt)
		{
			RTCRay ray;

			Eigen::Vector3f origin , dir;

			mCamera->getRay( origin , dir );

			ray.org[0] = origin(0);
			ray.org[1] = origin(1);
			ray.org[2] = origin(2);

			ray.dir[0] = dir(0);
			ray.dir[1] = dir(1);
			ray.dir[2] = dir(2);

			ray.tnear = 0.0f;
			ray.tfar = FLT_MAX;
			ray.instID = RTC_INVALID_GEOMETRY_ID;
			ray.geomID = RTC_INVALID_GEOMETRY_ID;
			ray.primID = RTC_INVALID_GEOMETRY_ID;
			ray.mask = 0xFFFFFFFF;
			ray.time = 0.0f;

			rtcIntersect( mScene , ray );

			float disp = mVolumeStep.norm() * 0.05;

			if ( ray.primID != RTC_INVALID_GEOMETRY_ID )
			{

				//valid intersection
				Eigen::Vector3f coord = origin + ray.tfar * dir;

				float thickness = 0;

				int vid1 = mSurfaceIndices[3 * ray.primID];
				int vid2 = mSurfaceIndices[3 * ray.primID + 1];
				int vid3 = mSurfaceIndices[3 * ray.primID + 2];

				Eigen::Vector3f vec1 = mSurfaceVertices[vid2].mPoint - mSurfaceVertices[vid1].mPoint;
				Eigen::Vector3f vec2 = mSurfaceVertices[vid3].mPoint - mSurfaceVertices[vid2].mPoint;

				Eigen::Vector3f n = -vec1.cross(vec2);

				n.normalize();

				if (1)//( mProgressReporter->mShowRay  )
				{

					//std::cout << " rendering ray method " << std::endl;

					ray.org[0] = coord(0);
					ray.org[1] = coord(1);
					ray.org[2] = coord(2);

					ray.dir[0] = n(0);
					ray.dir[1] = n(1);
					ray.dir[2] = n(2);

					ray.tnear = disp;
					ray.tfar = FLT_MAX;
					ray.instID = RTC_INVALID_GEOMETRY_ID;
					ray.geomID = RTC_INVALID_GEOMETRY_ID;
					ray.primID = RTC_INVALID_GEOMETRY_ID;
					ray.mask = 0xFFFFFFFF;
					ray.time = 0.0f;

					rtcIntersect(mScene, ray);

					if (ray.primID != RTC_INVALID_GEOMETRY_ID)
					{
						thickness = ray.tfar;
					}

					if (mProgressReporter)
					{
						Eigen::Vector3f dispCoord;

						dispCoord(0) = (int)(coord(0) / mVolumeStep(0));
						dispCoord(1) = (int)(coord(1) / mVolumeStep(1));
						dispCoord(2) = (int)(coord(2) / mVolumeStep(2));
						mProgressReporter->setUnserCursorInfo(dispCoord, thickness);
					}
				}
				else if (mProgressReporter->mShowSphere && mVolumeInfo && mVolumeInfo->mHasDistanceTransformData )
				{

					std::cout << " rendering sphere method " << std::endl;

					Eigen::Vector3f dispCoord;

					dispCoord(0) = (int)(coord(0) / mVolumeStep(0));
					dispCoord(1) = (int)(coord(1) / mVolumeStep(1));
					dispCoord(2) = (int)(coord(2) / mVolumeStep(2));

					Voxel initVox;

					initVox.x = (coord(0) - mVolumeOrigin(0)) / mVolumeStep(0); //+ dir(0) * step * 0.05
					initVox.y = (coord(1) - mVolumeOrigin(1)) / mVolumeStep(1); //+ dir(1) * step * 0.05
					initVox.z = (coord(2) - mVolumeOrigin(2)) / mVolumeStep(2); //+ dir(2) * step * 0.05

					LineIterator it(dir, initVox, mVolumeOrigin, mVolumeStep);

					long int zStep = mWidth * mHeight;
					long int yStep = mWidth;

					float initDist = mVolumeInfo->mDistanceTransform[zStep * initVox.z + yStep * initVox.y + initVox.x];

					float prevDist = initDist;
					float currentDist = initDist;

					bool medialAxisFound = false;

					while (true)
					{
						Voxel vox = it.next();

						prevDist = currentDist;

						if (vox.x < 0 || vox.x >= mVolumeInfo->mWidth ||
							vox.y < 0 || vox.y >= mVolumeInfo->mHeight ||
							vox.z < 0 || vox.z >= mVolumeInfo->mDepth)
						{
							break;
						}

						float currentDist = mVolumeInfo->mDistanceTransform[zStep * vox.z + yStep * vox.y + vox.x];

						if (currentDist < 0)
						{
							continue;
						}


						if (currentDist < prevDist)
						{
							medialAxisFound = true;

							break;
						}

					}


					if (medialAxisFound && prevDist > 0)
					{
						thickness = 2 * prevDist / mVolumeInfo->mVoxelStep(0);
					}

					mProgressReporter->setUnserCursorInfo(dispCoord, thickness);
				}
				else
				{
					//std::cout << " in else block " << std::endl;
				}


				
			}

			dir = Eigen::Vector3f(1, 0, 0);


			if (0)//(mProgressReporter->mNewPinnedData)
			{
				//std::cout << " computing pinned intersection " << std::endl;

				Eigen::Vector3f coord;// = mProgressReporter->mPinnedCoords;

				coord(0) = mProgressReporter->mPinnedCoords(0) * mVolumeStep(0);
				coord(1) = mProgressReporter->mPinnedCoords(1) * mVolumeStep(1);
				coord(2) = mProgressReporter->mPinnedCoords(2) * mVolumeStep(2);

				mProgressReporter->mNewPinnedData = false;

				int incidentTri = findIncidentTriangle(coord);
				
				if ( incidentTri > 0 )
				{
					float thickness = 0;

					int vid1 = mSurfaceIndices[3 * incidentTri];
					int vid2 = mSurfaceIndices[3 * incidentTri + 1];
					int vid3 = mSurfaceIndices[3 * incidentTri + 2];

					Eigen::Vector3f vec1 = mSurfaceVertices[vid2].mPoint - mSurfaceVertices[vid1].mPoint;
					Eigen::Vector3f vec2 = mSurfaceVertices[vid3].mPoint - mSurfaceVertices[vid2].mPoint;

					Eigen::Vector3f n = -vec1.cross(vec2);

					n.normalize();

					ray.org[0] = coord(0);
					ray.org[1] = coord(1);
					ray.org[2] = coord(2);

					ray.dir[0] = n(0);
					ray.dir[1] = n(1);
					ray.dir[2] = n(2);

					ray.tnear = disp;
					ray.tfar = FLT_MAX;
					ray.instID = RTC_INVALID_GEOMETRY_ID;
					ray.geomID = RTC_INVALID_GEOMETRY_ID;
					ray.primID = RTC_INVALID_GEOMETRY_ID;
					ray.mask = 0xFFFFFFFF;
					ray.time = 0.0f;

					rtcIntersect(mScene, ray);

					if (ray.primID != RTC_INVALID_GEOMETRY_ID)
					{
						thickness = ray.tfar;
					}

					if (mProgressReporter)
					{
						mProgressReporter->setPinnedPointInfo(coord, thickness);
					}
				}

			}

		}

		GLint lightColor = glGetUniformLocation(mSurfaceProgram, "lightColor");
		GLint lightPosition = glGetUniformLocation(mSurfaceProgram, "lightPosition");
		GLint halfVector = glGetUniformLocation(mSurfaceProgram, "halfVector");
		GLint shininess = glGetUniformLocation(mSurfaceProgram, "shininess");
		GLint strength = glGetUniformLocation(mSurfaceProgram, "strength");
		GLint heatMapFlagLoc = glGetUniformLocation(mSurfaceProgram, "enableHeatMap");


		GL_CHECK(glUniform3f(lightColor, 1.0, 1.0, 1.0));
		GL_CHECK(glUniform3f(lightPosition, 0, 0, 0));
		GL_CHECK(glUniform3f(halfVector, 0.5, 0.2f, 1.0f));
		GL_CHECK(glUniform1f(shininess, 20.0));
		GL_CHECK(glUniform1f(strength, 10.0));

		if (mProgressReporter->mShowHeatMap)
		{
			GL_CHECK( glUniform1f( heatMapFlagLoc , 1.0 ) );
		}
		else
		{
			GL_CHECK( glUniform1f( heatMapFlagLoc , 0.0 ) );
		}


		GL_CHECK(glDrawElements(GL_TRIANGLES, mSurfaceIndices.size(), GL_UNSIGNED_INT, (GLuint *)NULL));

		GL_CHECK(glBindVertexArray(0));
		GL_CHECK(glUseProgram(0));
#endif


	}


	void VolumeRenderManager::renderRay()
	{
#if 0
		int w, h;

		mCamera->getDimensions(w, h);

		if (mProgressReporter->mShowSideBySideRS)
		{
			GL_CHECK(glViewport(0, 0, w / 2, h));
		}


		GL_CHECK(glUseProgram(mSurfaceProgram));

		GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mDefaultFBO));

		GL_CHECK(glBindVertexArray(mSurfaceVAO));
		GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		GL_CHECK(glCullFace(GL_BACK));

		if (mSurfaceDataChanged)
		{
			std::cout << " loading surface data " << mSurfaceVertices.size() << " " << mSurfaceIndices.size() / 3 << std::endl;

			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, mSurfaceVBO[0]));
			GL_CHECK(glBufferData(GL_ARRAY_BUFFER, mSurfaceVertices.size() * sizeof(VertexData),
				(void *)mSurfaceVertices.data(), GL_STATIC_DRAW));

			//update points
			GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mSurfaceVBO[1]));
			GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, mSurfaceIndices.size() * sizeof(GLuint),
				mSurfaceIndices.data(), GL_STATIC_DRAW));

			//update faces
			GL_CHECK(glEnableVertexAttribArray(0));
			GL_CHECK(glEnableVertexAttribArray(1));
			GL_CHECK(glEnableVertexAttribArray(2));

			int offset = 0;

			GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), 0));

			offset += sizeof(Eigen::Vector3f);

			GL_CHECK(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offset));

			offset += sizeof(Eigen::Vector3f);

			GL_CHECK(glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offset));

			mSurfaceDataChanged = false;

		}

		GLint mvpMatrix = glGetUniformLocation(mSurfaceProgram, "mvpMatrix");
		QMatrix4x4 mvpMat = mCamera->getModelViewProjectionMatrix();

		if (mProgressReporter->mShowSideBySideRS)
		{
			mvpMat = mCamera->getModelViewProjectionMatrix(w / 2, h);
		}

		GLint mvMatrix = glGetUniformLocation(mSurfaceProgram, "mvMatrix");
		QMatrix4x4 mvMat = mCamera->getModelViewMatrix();

		GLint rTLoc = glGetUniformLocation(mSurfaceProgram, "rT");
		Eigen::Matrix3f rT = mCamera->getNormalTransformationMatrix();



		GL_CHECK(glUniformMatrix4fv(mvpMatrix, 1, false, mvpMat.data()));
		GL_CHECK(glUniformMatrix4fv(mvMatrix, 1, false, mvMat.data()));
		GL_CHECK(glUniformMatrix3fv(rTLoc, 1, false, rT.data()));


		if (mPickingEnabled)
		{
			RTCRay ray;

			Eigen::Vector3f origin, dir;

			//mCamera->getRay(origin, dir);
			if (mProgressReporter->mShowSideBySideRS)
			{
				mCamera->getRay(origin, dir, mvpMat, 0 , 0 , w /2 , h );
			}
			else
			{
				mCamera->getRay(origin, dir);
			}
			

			ray.org[0] = origin(0);
			ray.org[1] = origin(1);
			ray.org[2] = origin(2);

			ray.dir[0] = dir(0);
			ray.dir[1] = dir(1);
			ray.dir[2] = dir(2);

			ray.tnear = 0.0f;
			ray.tfar = FLT_MAX;
			ray.instID = RTC_INVALID_GEOMETRY_ID;
			ray.geomID = RTC_INVALID_GEOMETRY_ID;
			ray.primID = RTC_INVALID_GEOMETRY_ID;
			ray.mask = 0xFFFFFFFF;
			ray.time = 0.0f;

			rtcIntersect(mScene, ray);

			float disp = mVolumeStep.norm() * 0.05;

			if (ray.primID != RTC_INVALID_GEOMETRY_ID)
			{
				//valid intersection
				Eigen::Vector3f coord = origin + ray.tfar * dir;

				float thickness = 0;

				int vid1 = mSurfaceIndices[3 * ray.primID];
				int vid2 = mSurfaceIndices[3 * ray.primID + 1];
				int vid3 = mSurfaceIndices[3 * ray.primID + 2];

				Eigen::Vector3f vec1 = mSurfaceVertices[vid2].mPoint - mSurfaceVertices[vid1].mPoint;
				Eigen::Vector3f vec2 = mSurfaceVertices[vid3].mPoint - mSurfaceVertices[vid2].mPoint;

				Eigen::Vector3f n = -vec1.cross(vec2);

				n.normalize();



				ray.org[0] = coord(0);
				ray.org[1] = coord(1);
				ray.org[2] = coord(2);

				ray.dir[0] = n(0);
				ray.dir[1] = n(1);
				ray.dir[2] = n(2);

				ray.tnear = disp;
				ray.tfar = FLT_MAX;
				ray.instID = RTC_INVALID_GEOMETRY_ID;
				ray.geomID = RTC_INVALID_GEOMETRY_ID;
				ray.primID = RTC_INVALID_GEOMETRY_ID;
				ray.mask = 0xFFFFFFFF;
				ray.time = 0.0f;

				rtcIntersect(mScene, ray);

				if (ray.primID != RTC_INVALID_GEOMETRY_ID)
				{
					thickness = ray.tfar;
				}

				if (mProgressReporter)
				{
					Eigen::Vector3f dispCoord;

					dispCoord(0) = (int)(coord(0) / mVolumeStep(0));
					dispCoord(1) = (int)(coord(1) / mVolumeStep(1));
					dispCoord(2) = (int)(coord(2) / mVolumeStep(2));
					mProgressReporter->setUnserCursorInfo(dispCoord, thickness);
				}

			}
			else
			{
				//std::cout << " invalid intersection " << ray.primID << " " << origin.transpose() << " " << dir.transpose() << std::endl;
			}


			dir = Eigen::Vector3f(1, 0, 0);


			if (0)//(mProgressReporter->mNewPinnedData)
			{
				//std::cout << " computing pinned intersection " << std::endl;

				Eigen::Vector3f coord;// = mProgressReporter->mPinnedCoords;

				coord(0) = mProgressReporter->mPinnedCoords(0) * mVolumeStep(0);
				coord(1) = mProgressReporter->mPinnedCoords(1) * mVolumeStep(1);
				coord(2) = mProgressReporter->mPinnedCoords(2) * mVolumeStep(2);

				mProgressReporter->mNewPinnedData = false;

				int incidentTri = findIncidentTriangle(coord);

				if (incidentTri > 0)
				{
					//valid intersection


					float thickness = 0;

					int vid1 = mSurfaceIndices[3 * incidentTri];
					int vid2 = mSurfaceIndices[3 * incidentTri + 1];
					int vid3 = mSurfaceIndices[3 * incidentTri + 2];

					Eigen::Vector3f vec1 = mSurfaceVertices[vid2].mPoint - mSurfaceVertices[vid1].mPoint;
					Eigen::Vector3f vec2 = mSurfaceVertices[vid3].mPoint - mSurfaceVertices[vid2].mPoint;

					Eigen::Vector3f n = -vec1.cross(vec2);

					n.normalize();

					ray.org[0] = coord(0);
					ray.org[1] = coord(1);
					ray.org[2] = coord(2);

					ray.dir[0] = n(0);
					ray.dir[1] = n(1);
					ray.dir[2] = n(2);

					ray.tnear = disp;
					ray.tfar = FLT_MAX;
					ray.instID = RTC_INVALID_GEOMETRY_ID;
					ray.geomID = RTC_INVALID_GEOMETRY_ID;
					ray.primID = RTC_INVALID_GEOMETRY_ID;
					ray.mask = 0xFFFFFFFF;
					ray.time = 0.0f;

					rtcIntersect(mScene, ray);

					if (ray.primID != RTC_INVALID_GEOMETRY_ID)
					{
						thickness = ray.tfar;
					}

					if (mProgressReporter)
					{
						mProgressReporter->setPinnedPointInfo(coord, thickness);
					}
				}

			}

		}

		GLint lightColor = glGetUniformLocation(mSurfaceProgram, "lightColor");
		GLint lightPosition = glGetUniformLocation(mSurfaceProgram, "lightPosition");
		GLint halfVector = glGetUniformLocation(mSurfaceProgram, "halfVector");
		GLint shininess = glGetUniformLocation(mSurfaceProgram, "shininess");
		GLint strength = glGetUniformLocation(mSurfaceProgram, "strength");
		GLint heatMapFlagLoc = glGetUniformLocation(mSurfaceProgram, "enableHeatMap");


		GL_CHECK(glUniform3f(lightColor, 1.0, 1.0, 1.0));
		GL_CHECK(glUniform3f(lightPosition, 0, 0, 0));
		GL_CHECK(glUniform3f(halfVector, 0.5, 0.2f, 1.0f));
		GL_CHECK(glUniform1f(shininess, 20.0));
		GL_CHECK(glUniform1f(strength, 10.0));

		if (mProgressReporter->mShowHeatMap)
		{
			GL_CHECK(glUniform1f(heatMapFlagLoc, 1.0));
		}
		else
		{
			GL_CHECK(glUniform1f(heatMapFlagLoc, 0.0));
		}


		GL_CHECK(glDrawElements(GL_TRIANGLES, mSurfaceIndices.size(), GL_UNSIGNED_INT, (GLuint *)NULL));

		GL_CHECK(glBindVertexArray(0));
		GL_CHECK(glUseProgram(0));
#endif

	}


	void VolumeRenderManager::renderSphere()
	{

#if 0
		int w, h;

		mCamera->getDimensions(w, h);

		if (mProgressReporter->mShowSideBySideRS)
		{
			GL_CHECK(glViewport(w/2, 0, w / 2, h));
		}


		GL_CHECK(glUseProgram(mSphereProgram));

		GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mDefaultFBO));

		GL_CHECK(glBindVertexArray(mSphereVAO));
	//	GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		GL_CHECK(glCullFace(GL_BACK));

		if (mSphereSurfaceDataChanged)
		{
			std::cout << " loading surface data for Sphere method " << mSurfaceVertices.size() << " " << mSurfaceIndices.size() / 3 << std::endl;

			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, mSurfaceVBO[2]));
			GL_CHECK(glBufferData(GL_ARRAY_BUFFER, mSurfaceVerticesSphere.size() * sizeof(VertexData),
				(void *)mSurfaceVerticesSphere.data(), GL_STATIC_DRAW));

			//update points
			GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mSurfaceVBO[1]));
			GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, mSurfaceIndices.size() * sizeof(GLuint),
				mSurfaceIndices.data(), GL_STATIC_DRAW));

			//update faces
			GL_CHECK(glEnableVertexAttribArray(0));
			GL_CHECK(glEnableVertexAttribArray(1));
			GL_CHECK(glEnableVertexAttribArray(2));

			int offset = 0;

			GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), 0));

			offset += sizeof(Eigen::Vector3f);

			GL_CHECK(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offset));

			offset += sizeof(Eigen::Vector3f);

			GL_CHECK(glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offset));

			mSphereSurfaceDataChanged = false;

		}

		GLint mvpMatrix = glGetUniformLocation(mSurfaceProgram, "mvpMatrix");
		QMatrix4x4 mvpMat = mCamera->getModelViewProjectionMatrix();

		if (mProgressReporter->mShowSideBySideRS)
		{
			mvpMat = mCamera->getModelViewProjectionMatrix(w / 2, h);
		}

		GLint mvMatrix = glGetUniformLocation(mSurfaceProgram, "mvMatrix");
		QMatrix4x4 mvMat = mCamera->getModelViewMatrix();

		GLint rTLoc = glGetUniformLocation(mSurfaceProgram, "rT");
		Eigen::Matrix3f rT = mCamera->getNormalTransformationMatrix();



		GL_CHECK(glUniformMatrix4fv(mvpMatrix, 1, false, mvpMat.data()));
		GL_CHECK(glUniformMatrix4fv(mvMatrix, 1, false, mvMat.data()));
		GL_CHECK(glUniformMatrix3fv(rTLoc, 1, false, rT.data()));


		if (mPickingEnabled)
		{
			RTCRay ray;

			Eigen::Vector3f origin, dir;

			mCamera->getRay(origin, dir);

			ray.org[0] = origin(0);
			ray.org[1] = origin(1);
			ray.org[2] = origin(2);

			ray.dir[0] = dir(0);
			ray.dir[1] = dir(1);
			ray.dir[2] = dir(2);

			ray.tnear = 0.0f;
			ray.tfar = FLT_MAX;
			ray.instID = RTC_INVALID_GEOMETRY_ID;
			ray.geomID = RTC_INVALID_GEOMETRY_ID;
			ray.primID = RTC_INVALID_GEOMETRY_ID;
			ray.mask = 0xFFFFFFFF;
			ray.time = 0.0f;

			rtcIntersect(mScene, ray);

			float disp = mVolumeStep.norm() * 0.05;

			if (ray.primID != RTC_INVALID_GEOMETRY_ID)
			{
				//valid intersection
				Eigen::Vector3f coord = origin + ray.tfar * dir;

				float thickness = 0;


				if ( mProgressReporter && mVolumeInfo && mVolumeInfo->mHasDistanceTransformData )
				{

					

					Eigen::Vector3f dispCoord;

					dispCoord(0) = (int)(coord(0) / mVolumeStep(0));
					dispCoord(1) = (int)(coord(1) / mVolumeStep(1));
					dispCoord(2) = (int)(coord(2) / mVolumeStep(2));

					Voxel initVox;

					initVox.x = (coord(0) - mVolumeOrigin(0)) / mVolumeStep(0); //+ dir(0) * step * 0.05
					initVox.y = (coord(1) - mVolumeOrigin(1)) / mVolumeStep(1); //+ dir(1) * step * 0.05
					initVox.z = (coord(2) - mVolumeOrigin(2)) / mVolumeStep(2); //+ dir(2) * step * 0.05

					LineIterator it(dir, initVox, mVolumeOrigin, mVolumeStep );

					long int zStep = mWidth * mHeight;
					long int yStep = mWidth;

					float initDist = mVolumeInfo->mDistanceTransform[zStep * initVox.z + yStep * initVox.y + initVox.x];

					float prevDist = initDist;
					float currentDist = initDist;

					bool medialAxisFound = false;

					while (true)
					{
						Voxel vox = it.next();

						prevDist = currentDist;

						if (vox.x < 0 || vox.x >= mVolumeInfo->mWidth ||
							vox.y < 0 || vox.y >= mVolumeInfo->mHeight ||
							vox.z < 0 || vox.z >= mVolumeInfo->mDepth)
						{
							break;
						}

						float currentDist = mVolumeInfo->mDistanceTransform[zStep * vox.z + yStep * vox.y + vox.x];

						if (currentDist < 0)
						{
							continue;
						}


						if (currentDist < prevDist)
						{
							medialAxisFound = true;

							break;
						}

					}

					
					if (medialAxisFound && prevDist > 0)
					{
						thickness = 2 * prevDist / mVolumeInfo->mVoxelStep(0);
					}
					
					mProgressReporter->setUnserCursorInfo(dispCoord, thickness);
				}

			}
			else
			{
				//std::cout << " invalid intersection " << ray.primID << " " << origin.transpose() << " " << dir.transpose() << std::endl;
			}

		}

		GLint lightColor = glGetUniformLocation(mSurfaceProgram, "lightColor");
		GLint lightPosition = glGetUniformLocation(mSurfaceProgram, "lightPosition");
		GLint halfVector = glGetUniformLocation(mSurfaceProgram, "halfVector");
		GLint shininess = glGetUniformLocation(mSurfaceProgram, "shininess");
		GLint strength = glGetUniformLocation(mSurfaceProgram, "strength");
		GLint heatMapFlagLoc = glGetUniformLocation(mSurfaceProgram, "enableHeatMap");


		GL_CHECK(glUniform3f(lightColor, 1.0, 1.0, 1.0));
		GL_CHECK(glUniform3f(lightPosition, 0, 0, 0));
		GL_CHECK(glUniform3f(halfVector, 0.5, 0.2f, 1.0f));
		GL_CHECK(glUniform1f(shininess, 20.0));
		GL_CHECK(glUniform1f(strength, 10.0));

		if (mProgressReporter->mShowHeatMap)
		{
			GL_CHECK(glUniform1f(heatMapFlagLoc, 1.0));
		}
		else
		{
			GL_CHECK(glUniform1f(heatMapFlagLoc, 0.0));
		}


		GL_CHECK(glDrawElements(GL_TRIANGLES, mSurfaceIndices.size(), GL_UNSIGNED_INT, (GLuint *)NULL));

		GL_CHECK(glBindVertexArray(0));
		GL_CHECK(glUseProgram(0));
#endif


	}


	void VolumeRenderManager::renderCrossSection()
	{
		GL_CHECK(glUseProgram(mMeshCrossSectionProgram));

		GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mDefaultFBO));

		GL_CHECK(glBindVertexArray(mSurfaceVAO));
		GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		GLint mvpMatrix = glGetUniformLocation(mMeshCrossSectionProgram, "mvpMatrix");

		GLint planeNormalLoc = glGetUniformLocation(mMeshCrossSectionProgram, "planeNormal");
		GLint planeCoeffLoc = glGetUniformLocation(mMeshCrossSectionProgram, "planeCoeff");
		GLint planeTypeLoc = glGetUniformLocation(mMeshCrossSectionProgram, "pType");


		QMatrix4x4 mvpMat = mCamera->getModelViewProjectionMatrix();

		GL_CHECK(glUniformMatrix4fv(mvpMatrix, 1, false, mOrthoProjMatData.data())); //mvpMat.data()

		GL_CHECK(glUniform3f(planeCoeffLoc, mCoeff(0), mCoeff(1), mCoeff(2))); //mvpMat.data()

		GL_CHECK(glDrawElements(GL_TRIANGLES, mSurfaceIndices.size(), GL_UNSIGNED_INT, (GLuint *)NULL));

		GL_CHECK(glBindVertexArray(0));

		GL_CHECK(glUseProgram(0));


	}


	void VolumeRenderManager::renderCrosssection( int planeType, Eigen::Vector3f planeCoeff , int rw , int rh )
	{

		if (!mVolumeInfo)
			return;

		float leftV, rightV, bottomV, topV, nearV, farV;

		Eigen::Vector2f xLimits, yLimits, zLimits;

		xLimits(0) = 0;
		yLimits(0) = 0;
		zLimits(0) = 0;

		xLimits(1) = mVolumeInfo->mVolumeOrigin( 0 ) + mVolumeInfo->mWidth * mVolumeInfo->mVoxelStep(0);
		yLimits(1) = mVolumeInfo->mVolumeOrigin(1) + mVolumeInfo->mHeight * mVolumeInfo->mVoxelStep(1);
		zLimits(1) = mVolumeInfo->mVolumeOrigin(2) + mVolumeInfo->mDepth * mVolumeInfo->mVoxelStep(2);

		if (mPlaneType == 0)
		{
			leftV = xLimits(0);
			rightV = xLimits(1);
			bottomV = yLimits(0);
			topV = yLimits(1);
			nearV = zLimits(0);
			farV = zLimits(1);

		}
		else if (mPlaneType == 1)
		{
			leftV = yLimits(0);
			rightV = yLimits(1);
			bottomV = zLimits(0);
			topV = zLimits(1);
			nearV = xLimits(0);
			farV = xLimits(1);
		}
		else
		{
			leftV = zLimits(0);
			rightV = zLimits(1);
			bottomV = xLimits(0);
			topV = xLimits(1);
			nearV = yLimits(0);
			farV = yLimits(1);
		}



		QMatrix4x4 orthoProjMat;

		orthoProjMat.frustum(leftV, rightV, bottomV, topV, nearV, farV);

		GL_CHECK(glViewport(0, 0, rw, rh));

		GL_CHECK(glUseProgram(mMeshCrossSectionProgram));

		//GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mDefaultFBO));

		GL_CHECK(glBindVertexArray(mSurfaceVAO));
		GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		GLint mvpMatrix = glGetUniformLocation(mMeshCrossSectionProgram, "mvpMatrix");

		GLint planeNormalLoc = glGetUniformLocation(mMeshCrossSectionProgram, "planeNormal");
		GLint planeCoeffLoc = glGetUniformLocation(mMeshCrossSectionProgram, "planeCoeff");
		GLint planeTypeLoc = glGetUniformLocation(mMeshCrossSectionProgram, "pType");
		GLint lineColorLoc = glGetUniformLocation(mMeshCrossSectionProgram, "lineColor");

		QMatrix4x4 mvpMat = mCamera->getModelViewProjectionMatrix();

		GL_CHECK(glUniformMatrix4fv(mvpMatrix, 1, false, orthoProjMat.data())); //mvpMat.data()

		GL_CHECK(glUniform3f(planeCoeffLoc, mCoeff(0), mCoeff(1), mCoeff(2))); //mvpMat.data()

		GL_CHECK(glUniform3f(lineColorLoc, 1, 0, 0)); //mvpMat.data()

		GL_CHECK(glDrawElements(GL_TRIANGLES, mSurfaceIndices.size(), GL_UNSIGNED_INT, (GLuint *)NULL));

		GL_CHECK(glBindVertexArray(0));

		GL_CHECK(glUseProgram(0));

	}


	void VolumeRenderManager::initializeShaders()
	{
		initializeOpenGLFunctions();

		QFile file1(":/raycastingvert.glsl"), file2(":/raycastingfrag.glsl");//file1(":/backfacevert.glsl"), file2(":/backfacefrag.glsl");//

		if (!file1.open(QIODevice::ReadOnly))
		{
			qDebug() << "Cannot open file for reading: ( ray cast volume vertex ) "
				<< qPrintable(file1.errorString()) << endl;
			return;
		}

		if (!file2.open(QIODevice::ReadOnly))
		{
			qDebug() << "Cannot open file for reading: ( ray cast volume fragment ) "
				<< qPrintable(file2.errorString()) << endl;
			return;
		}

		

		QString vertShaderSrc = file1.readAll();
		QString fragShaderSrc = file2.readAll();

		qDebug() << vertShaderSrc << endl;

		GLuint vertexShader = compileShaders(GL_VERTEX_SHADER, vertShaderSrc.toStdString().c_str());
		GLuint fragmentShader = compileShaders(GL_FRAGMENT_SHADER, fragShaderSrc.toStdString().c_str());

		GLuint program = glCreateProgram();

		GL_CHECK(glBindAttribLocation(program, 0, "vert"));

		GL_CHECK(glAttachShader(program, fragmentShader));
		GL_CHECK(glAttachShader(program, vertexShader));

		GL_CHECK(glLinkProgram(program));

		GLint status;

		GL_CHECK(glGetProgramiv(program, GL_LINK_STATUS, &status));

		if (status == GL_FALSE)
		{
			GLchar emsg[1024];
			GL_CHECK(glGetProgramInfoLog(program, sizeof(emsg), 0, emsg));
			fprintf(stderr, "Error linking GLSL program : %s\n", emsg);
			return;
		}

		mVolumeVizProgram = program;


		QFile file3(":/backfacevert.glsl"), file4(":/backfacefrag.glsl");

		if (!file3.open(QIODevice::ReadOnly))
		{
			qDebug() << "Cannot open file for reading: ( ray cast volume back face intersection vertex ) "
				<< qPrintable(file3.errorString()) << endl;
			return;
		}

		if (!file4.open(QIODevice::ReadOnly))
		{
			qDebug() << "Cannot open file for reading: ( ray cast volume backface intersection fragment ) "
				<< qPrintable(file4.errorString()) << endl;
			return;
		}


		vertShaderSrc = file3.readAll();
		fragShaderSrc = file4.readAll();

		vertexShader = compileShaders(GL_VERTEX_SHADER, vertShaderSrc.toStdString().c_str());
		fragmentShader = compileShaders(GL_FRAGMENT_SHADER, fragShaderSrc.toStdString().c_str());

		program = glCreateProgram();

		GL_CHECK( glBindAttribLocation(program, 0, "vert") );

		GL_CHECK( glAttachShader(program, fragmentShader) );
		GL_CHECK( glAttachShader(program, vertexShader) );

		GL_CHECK( glLinkProgram(program) );

		GL_CHECK( glGetProgramiv(program, GL_LINK_STATUS, &status) );

		if (status == GL_FALSE)
		{
			GLchar emsg[1024];
			GL_CHECK(glGetProgramInfoLog(program, sizeof(emsg), 0, emsg));
			fprintf(stderr, "Error linking GLSL program : %s\n", emsg);
			return;
		}

		mVolumeVizBackFaceProgram = program;



		QFile file5(":/surfacevert.glsl"), file6(":/surfacefrag.glsl");

		if ( !file5.open(QIODevice::ReadOnly) )
		{
			
			qDebug() << "Cannot open file for reading: ( ray cast volume back face intersection vertex ) "
				     << qPrintable(file5.errorString()) << endl;

			return;
		}

		if ( !file6.open(QIODevice::ReadOnly) )
		{
			
			qDebug() << "Cannot open file for reading: ( ray cast volume backface intersection fragment ) "
				     << qPrintable(file6.errorString()) << endl;
			
			return;
		}
		vertShaderSrc = file5.readAll();
		fragShaderSrc = file6.readAll();

		vertexShader = compileShaders(GL_VERTEX_SHADER, vertShaderSrc.toStdString().c_str());
		fragmentShader = compileShaders(GL_FRAGMENT_SHADER, fragShaderSrc.toStdString().c_str());

		program = glCreateProgram();

		GL_CHECK(glBindAttribLocation(program, 0, "vert"));
		GL_CHECK(glBindAttribLocation(program, 1, "normal"));
		GL_CHECK(glBindAttribLocation(program, 2, "color"));

		GL_CHECK(glAttachShader(program, fragmentShader));
		GL_CHECK(glAttachShader(program, vertexShader));

		GL_CHECK(glLinkProgram(program));

		GL_CHECK(glGetProgramiv(program, GL_LINK_STATUS, &status));

		if (status == GL_FALSE)
		{
			GLchar emsg[1024];
			GL_CHECK(glGetProgramInfoLog(program, sizeof(emsg), 0, emsg));
			fprintf(stderr, "Error linking GLSL program : %s\n", emsg);
			return;
		}

		mSurfaceProgram = program;

		mSphereProgram = program;


		std::cout << " initializing mesh cross section shaders " << std::endl;


		QFile file7(":/crosssectionvert.glsl"), file8(":/crosssectionfbgeom.glsl"), file9(":/crosssectionfrag.glsl");

		if (!file7.open(QIODevice::ReadOnly))
		{

			qDebug() << "Cannot open file for reading: ( mesh cross section vertex ) "
				<< qPrintable(file7.errorString()) << endl;

			return;
		}

		if (!file8.open(QIODevice::ReadOnly))
		{

			qDebug() << "Cannot open file for reading: ( mesh cross section geometry ) "
				<< qPrintable(file8.errorString()) << endl;

			return;
		}

		if (!file9.open(QIODevice::ReadOnly))
		{

			qDebug() << "Cannot open file for reading: ( mesh cross section fragment ) "
				<< qPrintable(file9.errorString()) << endl;

			return;
		}
		vertShaderSrc = file7.readAll();
		QString geomShaderSrc = file8.readAll();
		fragShaderSrc = file9.readAll();

		vertexShader = compileShaders(GL_VERTEX_SHADER, vertShaderSrc.toStdString().c_str());
		GLuint geomShader = compileShaders(GL_GEOMETRY_SHADER, geomShaderSrc.toStdString().c_str());
		fragmentShader = compileShaders(GL_FRAGMENT_SHADER, fragShaderSrc.toStdString().c_str());

		program = glCreateProgram();

		GL_CHECK(glBindAttribLocation(program, 0, "vert"));
	/*	GL_CHECK(glBindAttribLocation(program, 1, "normal"));
		GL_CHECK(glBindAttribLocation(program, 2, "color"));*/

		GL_CHECK(glAttachShader(program, vertexShader));
		GL_CHECK(glAttachShader(program, geomShader));
		GL_CHECK(glAttachShader(program, fragmentShader));
		

		GL_CHECK(glLinkProgram(program));

		GL_CHECK(glGetProgramiv(program, GL_LINK_STATUS, &status));

		if (status == GL_FALSE)
		{
			GLchar emsg[1024];
			GL_CHECK(glGetProgramInfoLog(program, sizeof(emsg), 0, emsg));
			fprintf(stderr, "Error linking GLSL program : %s\n", emsg);
			return;
		}

		mMeshCrossSectionProgram = program;


		std::cout << " mesh cross section shaders initialized " << mMeshCrossSectionProgram  << std::endl;

	}


	GLuint VolumeRenderManager::compileShaders(GLenum shaderType, const char *shaderSource)
	{
		GLuint shader = glCreateShader(shaderType); 

		GL_CHECK( glShaderSource(shader, 1, &shaderSource, NULL) );

		GL_CHECK( glCompileShader(shader) );

		GLint status;

		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);


		if ( status == GL_FALSE )
		{
			GLchar emsg[10024];
			
			glGetShaderInfoLog(shader, sizeof(emsg), 0, emsg);
			
			QString errMsg = QString("Error compiling GLSL shader (%s): %s\n") + emsg;

			qDebug() << " error in compiling shaders , file : " << __FILE__ << " : line : " << __LINE__ << errMsg << endl;
		}

		return shader;
	}


	void VolumeRenderManager::loadVolume(unsigned char* volumeData, int w, int h, int d, Eigen::Vector3f origin, Eigen::Vector3f voxelStep)
	{

		mWidth = w;
		mHeight = h;
		mDepth = d;

		std::cout << " width , height and depth : " << mWidth << " " << mHeight << " " << mDepth << std::endl;

		mVolumeOrigin = origin;
		mVolumeStep = voxelStep;

		mHasNewData = true;

		std::cout << " volume step " << mVolumeStep << std::endl;

		//std::cout << " voxel step : " << voxelStep.transpose() << std::endl;

		GL_CHECK(glBindTexture(GL_TEXTURE_3D, mVolumeTex));

		// pixel transfer happens here from client to OpenGL server
		//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		GL_CHECK(glTexImage3D(GL_TEXTURE_3D, 0, GL_R16, w, h, d, 0, GL_RED, GL_UNSIGNED_SHORT, volumeData));

		GL_CHECK(glBindTexture(GL_TEXTURE_3D,0));

		//long int nVoxels = 0;

		//for (int zz = 0; zz < d; zz++)
		//	for (int yy = 0; yy < h; yy++)
		//		for (int xx = 0; xx < w; xx++)
		//		{
		//			if ( volumeData[zz * w * h + yy * w + xx] > 30 )
		//			{
		//				nVoxels++;
		//			}
		//		}


		//std::cout << " number of filled voxels : " << nVoxels << std::endl;


	}


	void VolumeRenderManager::loadVolume(unsigned short* volumeData, int w, int h, int d, Eigen::Vector3f origin, Eigen::Vector3f voxelStep)
	{
		GL_CHECK(glBindTexture(GL_TEXTURE_3D, mVolumeTex));

		// pixel transfer happens here from client to OpenGL server
		//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, w, h, d, 0, GL_RED, GL_UNSIGNED_SHORT, volumeData);

		GL_CHECK(glBindTexture(GL_TEXTURE_3D, 0));
	}
		
		
	void VolumeRenderManager::loadSurface( std::vector< Eigen::Vector3f >& vertices , std::vector< Eigen::Vector3f >& colors , 
		                                   std::vector< Eigen::Vector3f >& normals  , std::vector< unsigned int >& faceIndices )
	{

		long int nVerts = vertices.size();

		mSurfaceVertices.resize(nVerts);

		mSurfaceVerticesSphere.resize(nVerts);

		mIncidentTriangles.resize(nVerts);

		for (int vv = 0; vv < nVerts; vv++)
		{
			mSurfaceVertices[vv].mPoint = vertices[vv];
			mSurfaceVertices[vv].mNormal = -normals[vv];
			mSurfaceVertices[vv].mColor = colors[vv];

			mSurfaceVerticesSphere[vv].mPoint = vertices[vv];
			mSurfaceVerticesSphere[vv].mNormal = -normals[vv];
			mSurfaceVerticesSphere[vv].mColor = colors[vv];
		}


		mSurfaceDataChanged = true;

		mSphereSurfaceDataChanged = true;

		mSurfaceIndices = faceIndices;

		

		mUsedTriangleMask.resize(faceIndices.size() / 3);

		std::fill(mUsedTriangleMask.begin(), mUsedTriangleMask.end(), 0);

		long int nTris = faceIndices.size() / 3;

		mPlaneCoeffs.resize(nTris);

		std::fill(mPlaneCoeffs.begin(), mPlaneCoeffs.end(), 0);

		mFaceNormals.resize(nTris);

		for (long int tt = 0; tt < nTris; tt++)
		{
			int vid1 = mSurfaceIndices[3 * tt];
			int vid2 = mSurfaceIndices[3 * tt + 1];
			int vid3 = mSurfaceIndices[3 * tt + 2];

			Eigen::Vector3f vec1 = mSurfaceVertices[vid2].mPoint - mSurfaceVertices[vid1].mPoint;
			Eigen::Vector3f vec2 = mSurfaceVertices[vid3].mPoint - mSurfaceVertices[vid2].mPoint;

			Eigen::Vector3f n = vec1.cross(vec2);

			n.normalize();

			mFaceNormals[tt] = n;

			mPlaneCoeffs[tt] = n.dot(mSurfaceVertices[vid1].mPoint);

			mIncidentTriangles[vid1].push_back(tt);
			mIncidentTriangles[vid2].push_back(tt);
			mIncidentTriangles[vid3].push_back(tt);

		}


		buildTracer();
	}


	void VolumeRenderManager::loadSurface(std::vector< Eigen::Vector3f >& vertices, std::vector< Eigen::Vector3f >& rayColors, std::vector< Eigen::Vector3f >& sphereColors,
		std::vector< Eigen::Vector3f >& normals, std::vector< unsigned int >& faceIndices)
	{

		long int nVerts = vertices.size();

		mSurfaceVertices.resize(nVerts);

		mSurfaceVerticesSphere.resize(nVerts);

		mIncidentTriangles.resize(nVerts);

		for (int vv = 0; vv < nVerts; vv++)
		{
			mSurfaceVertices[vv].mPoint = vertices[vv];
			mSurfaceVertices[vv].mNormal = -normals[vv];
			mSurfaceVertices[vv].mColor = rayColors[vv];

			mSurfaceVerticesSphere[vv].mPoint = vertices[vv];
			mSurfaceVerticesSphere[vv].mNormal = -normals[vv];
			mSurfaceVerticesSphere[vv].mColor = sphereColors[vv];
		}


		mSurfaceDataChanged = true;

		mSphereSurfaceDataChanged = true;

		mSurfaceIndices = faceIndices;



		mUsedTriangleMask.resize(faceIndices.size() / 3);

		std::fill(mUsedTriangleMask.begin(), mUsedTriangleMask.end(), 0);

		long int nTris = faceIndices.size() / 3;

		mPlaneCoeffs.resize(nTris);

		std::fill(mPlaneCoeffs.begin(), mPlaneCoeffs.end(), 0);

		mFaceNormals.resize(nTris);

		for (long int tt = 0; tt < nTris; tt++)
		{
			int vid1 = mSurfaceIndices[3 * tt];
			int vid2 = mSurfaceIndices[3 * tt + 1];
			int vid3 = mSurfaceIndices[3 * tt + 2];

			Eigen::Vector3f vec1 = mSurfaceVertices[vid2].mPoint - mSurfaceVertices[vid1].mPoint;
			Eigen::Vector3f vec2 = mSurfaceVertices[vid3].mPoint - mSurfaceVertices[vid2].mPoint;

			Eigen::Vector3f n = vec1.cross(vec2);

			n.normalize();

			mFaceNormals[tt] = n;

			mPlaneCoeffs[tt] = n.dot(mSurfaceVertices[vid1].mPoint);

			mIncidentTriangles[vid1].push_back(tt);
			mIncidentTriangles[vid2].push_back(tt);
			mIncidentTriangles[vid3].push_back(tt);

		}


		buildTracer();


	}



	void VolumeRenderManager::setProgressReporter(imt::volume::ProgressReporter *progressReporter)
	{
		mProgressReporter = progressReporter;
	}


	void VolumeRenderManager::buildTracer()
	{

#if 0

		mScene = rtcDeviceNewScene(mDevice, RTC_SCENE_STATIC, RTC_INTERSECT1);

		long int numTriangles = mSurfaceIndices.size() / 3;
		long int numVertices = mSurfaceVertices.size();


		mGeomID = rtcNewTriangleMesh(mScene, RTC_GEOMETRY_STATIC, numTriangles, numVertices, 1);

		EmbreeTriangle* triangles = (EmbreeTriangle*)rtcMapBuffer(mScene, mGeomID, RTC_INDEX_BUFFER);

		for (int tt = 0; tt < numTriangles; tt++)
		{
			triangles[tt].v0 = mSurfaceIndices[3 * tt];
			triangles[tt].v1 = mSurfaceIndices[3 * tt + 1];
			triangles[tt].v2 = mSurfaceIndices[3 * tt + 2];
		}

		rtcUnmapBuffer(mScene, mGeomID, RTC_INDEX_BUFFER);

		EmbreeVertex* vertices = (EmbreeVertex*)rtcMapBuffer(mScene, mGeomID, RTC_VERTEX_BUFFER);

		int numBasePoints = numVertices;

		for (int pp = 0; pp < numBasePoints; pp++)
		{
			vertices[pp].x = mSurfaceVertices[pp].mPoint(0);
			vertices[pp].y = mSurfaceVertices[pp].mPoint(1);
			vertices[pp].z = mSurfaceVertices[pp].mPoint(2);
			vertices[pp].a = 1.0;
        }

		rtcUnmapBuffer(mScene, mGeomID, RTC_VERTEX_BUFFER);

		rtcCommit(mScene);


		// Generate pointcloud data
		//mCloud->width = mSurfaceVertices.size();
		//mCloud->height = 1;
		//mCloud->points.resize(mCloud->width * mCloud->height);

		//for (size_t i = 0; i < mCloud->points.size(); ++i)
		//{
		//	mCloud->points[i].x = mSurfaceVertices[i].mPoint(0);
		//	mCloud->points[i].y = mSurfaceVertices[i].mPoint(1);
		//	mCloud->points[i].z = mSurfaceVertices[i].mPoint(2);
		//}

		//mKdtree.setInputCloud( mCloud );

		mTracerBuilt = true;

#endif

	}


	int VolumeRenderManager::findIncidentTriangle(Eigen::Vector3f point)
	{


		int incidentTri = -1;
#if 0
		std::vector< int > indices;
		std::vector< float > distances;
		std::vector< int > searchedIndices;

		std::vector< unsigned int > usedTris;

		int closestTriangleId = -1;

		usedTris.reserve(100);

		pcl::PointXYZ pt;

		pt.x = point(0);
		pt.y = point(1);
		pt.z = point(2);

		if ( mKdtree.nearestKSearch( pt , 30 , indices, distances ) )
		{
			int numIndices = indices.size();

			bool validTriangleFound = false;

			for (int ii = 0; ii < numIndices; ii++)
			{
				int numIncidentTris = mIncidentTriangles[indices[ii]].size();

				for (int it = 0; it < numIncidentTris; it++)
				{
					int tid = mIncidentTriangles[indices[ii]][it];

					//float dist = std::abs(mPlaneCoeffs[tid] - mFaceNormals[it].dot(mProgressReporter->mPinnedCoords));

					//if (dist < disp)
					//{

					//}

					float disp = mVolumeStep.norm() * 2;

					RTCRay ray;

					Eigen::Vector3f origin = point + mFaceNormals[ tid ] * disp , dir = -mFaceNormals[ tid ];

					ray.org[0] = origin(0);
					ray.org[1] = origin(1);
					ray.org[2] = origin(2);

					ray.dir[0] = dir(0);
					ray.dir[1] = dir(1);
					ray.dir[2] = dir(2);

					ray.tnear = 0.0f;
					ray.tfar = 2 * disp;
					ray.instID = RTC_INVALID_GEOMETRY_ID;
					ray.geomID = RTC_INVALID_GEOMETRY_ID;
					ray.primID = RTC_INVALID_GEOMETRY_ID;
					ray.mask = 0xFFFFFFFF;
					ray.time = 0.0f;

					rtcIntersect(mScene, ray);

					if ( ray.primID == tid )
					{
						validTriangleFound = true;

						incidentTri = tid;

						break;
					}

					
				}

				if ( validTriangleFound )
					break;
			}


			std::cout << " valid triangle found " <<" "<<validTriangleFound<< std::endl;
		}
		else
		{
			std::cout << " nearest neighbor search failed " << std::endl;
		}

#endif

		return incidentTri;

	}


	void VolumeRenderManager::enablePicking()
	{

		mPickingEnabled = true;

	}



	void VolumeRenderManager::disablePicking()
	{

		mPickingEnabled = false;

	}


	void VolumeRenderManager::enableHeatMap()
	{
		mHeatMapViewingEnabled = true;

	}


	void VolumeRenderManager::disableHeatMap()
	{

		mHeatMapViewingEnabled = false;

	}

	GLint VolumeRenderManager::surfaceSliceProgram()
	{
		return mMeshCrossSectionProgram;

	}


	GLuint VolumeRenderManager::surfaceVAO()
	{
		return mSurfaceVAO;
	}


	GLuint *VolumeRenderManager::surfaceVBO()
	{
		return mSurfaceVBO;
	}


	unsigned int VolumeRenderManager::numTriangles()
	{
		unsigned int nTris = mSurfaceIndices.size() / 3;

		return nTris;
	}



	Eigen::Matrix4f VolumeRenderManager::computeOrthoProjectionMatrix( int plane , Eigen::Vector3f minLimits , Eigen::Vector3f maxLimits )
	{
		
		if ( plane == 0 ) // xy plane 
		{

			//QMatrix4x4::ortho(minLimits(0), maxLimits(0), minLimits(1), maxLimits(1), minLimits(2), maxLimits(2));
		}

		if ( plane == 1 ) // yz plane
		{


		}

		if ( plane == 2 ) // zx plane
		{


		}

		return Eigen::Matrix4f();
	}



	}
	
}



#include "volumerendermanager.moc"