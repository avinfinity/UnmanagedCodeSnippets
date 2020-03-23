#include "slicerendermanager.h"
#include "openglhelper.h"
#include "QMatrix4x4"
#include "iostream"
#include "QDebug"
#include "atomic"


namespace imt{

	namespace volume{



		SliceRenderManager::SliceRenderManager(imt::volume::VolumeInfo *volume , PLANETYPE planeType) : mVolume( volume ) , mPlaneType(planeType)
		{
			mIsInitialized = false;

			mHasNewData = false;

			mWte = 0;
		}

		void SliceRenderManager::setInfo( GLint surfaceSliceProgram, int nSurfaceVBOs, GLuint *surfaceVBOs, GLuint surfaceVAO, int nTris )
		{
			//mSurfaceSliceProgram = surfaceSliceProgram;

			//mSurfaceVBOs = surfaceVBOs;

			//mNumSurfaceVBOs = nSurfaceVBOs;

			//mSurfaceVAO = surfaceVAO;

			//mNumTriangles = nTris;

			//GL_CHECK(glBindVertexArray(mSurfaceVAO));
			//GL_CHECK(glBindVertexArray(0));

			
		}


		void SliceRenderManager::setSurface(  std::vector< Eigen::Vector3f >& vertices, std::vector< unsigned int >& faces )
		{
			std::cout << " setting surface data " << std::endl;

			mSurfaceVertices = vertices;

			mSurfaceIndices = faces;

			mHasNewData = true;
			
		}

		void SliceRenderManager::updateSurface()
		{

			mVertexRenderData.resize(mVolume->mVertices.size());

			int nVertices = mVolume->mVertices.size();

			if ( mVolume->mWallthickness.size() != mVolume->mVertices.size() )
			{
				mVolume->mWallthickness.resize( mVolume->mVertices.size() );

				std::fill( mVolume->mWallthickness.begin() , mVolume->mWallthickness.end() , -1 );
			} 

			if (mVolume->mVertexColors.size() != mVolume->mVertices.size())
			{
				mVolume->mVertexColors.resize(mVolume->mVertices.size());

				std::fill(mVolume->mVertexColors.begin(), mVolume->mVertexColors.end(), Eigen::Vector3f(1, 1, 1));
			}

			for (int vv = 0; vv < nVertices; vv++)
			{
				mVertexRenderData[vv].mVertex = mVolume->mVertices[vv];
				mVertexRenderData[vv].mVertexNormal = mVolume->mVertexNormals[vv];
				mVertexRenderData[vv].mVertexColor = mVolume->mVertexColors[vv];
				mVertexRenderData[vv].mWallThickness = mVolume->mWallthickness[vv];

				if (vv > 10000 && vv < 10010)
				{
					std::cout << vv << "  --  " << mVertexRenderData[vv].mVertexColor.transpose() << " " << mVertexRenderData[vv].mWallThickness << std::endl;
				}
			}

			mSurfaceIndices = mVolume->mFaceIndices;

			mHasNewData = true;
		}


		void SliceRenderManager::setOrthoProjectionMatrix( const QMatrix4x4& orthoProjMat )
		{
			mOrthoProjMat = orthoProjMat;
		}


		void SliceRenderManager::setCamera( TrackBallCamera *camera )
		{
			mCamera = camera;
		}


		void SliceRenderManager::setPlaneType(int planeType, Eigen::Vector3f& planeCoeff, int w, int h)
		{
			
			mCoeff = planeCoeff;

			//std::cout << mCoeff << std::endl;

			if (mWidth != w || mHeight != mHeight || planeType != mPlaneType)
			{
				mPlaneType = planeType;

				mWidth = w;
				mHeight = h;


				Eigen::Vector2f xLimits, yLimits, zLimits;

				xLimits(0) = mVolume->mVolumeOrigin(0);
				yLimits(0) = mVolume->mVolumeOrigin(1);
				zLimits(0) = mVolume->mVolumeOrigin(2);

				xLimits(1) = mVolume->mVolumeOrigin(0) + mVolume->mVoxelStep( 0 ) * mVolume->mWidth;
				yLimits(1) = mVolume->mVolumeOrigin(1) + mVolume->mVoxelStep( 1 ) * mVolume->mHeight;
				zLimits(1) = mVolume->mVolumeOrigin(2) + mVolume->mVoxelStep( 2 ) * mVolume->mDepth;

				buildProjectionmatrix( xLimits , yLimits , zLimits );
			}


		}


		void SliceRenderManager::setViewPort(int w, int h)
		{
			mWidth = w;
			mHeight = h;
		}


		void SliceRenderManager::setNumTriangles(int numTris)
		{
			mNumTriangles = numTris;
		}


		void SliceRenderManager::init()
		{
			initializeOpenGLFunctions();

			mIsInitialized = true;

			GL_CHECK(glGenBuffers(3, mSurfaceVBOs));
			GL_CHECK(glGenBuffers(2, mSliceVBO));
			GL_CHECK(glGenVertexArrays(2, mSurfaceVAO));

			mFeedbackBuffer = mSurfaceVBOs[2];

			GL_CHECK(glGenQueries(1, &mFeedBackQuery));



			initializeShaders();
		}


		bool SliceRenderManager::isInitialized()
		{
			return mIsInitialized;
		}


		void SliceRenderManager::setDefaultFBO(GLint fbo)
		{
			mDefaultFBO = fbo;
		}


		void SliceRenderManager::initializeShaders()
		{
			QFile file1(":/crosssectionvert.glsl"), file2(":/crosssectionfbgeom.glsl"), file3(":/crosssectionfrag.glsl");

			if (!file1.open(QIODevice::ReadOnly))
			{

				qDebug() << "Cannot open file for reading: ( mesh cross section vertex ) "
					<< qPrintable(file1.errorString()) << endl;

				return;
			}

			if (!file2.open(QIODevice::ReadOnly))
			{

				qDebug() << "Cannot open file for reading: ( mesh cross section geometry ) "
					<< qPrintable(file2.errorString()) << endl;

				return;
			}

			if (!file3.open(QIODevice::ReadOnly))
			{

				qDebug() << "Cannot open file for reading: ( mesh cross section fragment ) "
					<< qPrintable(file3.errorString()) << endl;

				return;
			}
			QString vertShaderSrc = file1.readAll();
			QString geomShaderSrc = file2.readAll();
			QString fragShaderSrc = file3.readAll();

			GLuint vertexShader = compileShaders(GL_VERTEX_SHADER, vertShaderSrc.toStdString().c_str());
			GLuint geomShader = compileShaders(GL_GEOMETRY_SHADER, geomShaderSrc.toStdString().c_str());
			GLuint fragmentShader = compileShaders(GL_FRAGMENT_SHADER, fragShaderSrc.toStdString().c_str());

			GLint program = glCreateProgram();


			GL_CHECK(glBindAttribLocation(program, 0, "vert"));
			GL_CHECK(glBindAttribLocation(program, 1, "normal"));
			GL_CHECK(glBindAttribLocation(program, 2, "color"));
			GL_CHECK(glBindAttribLocation(program, 3, "wallThickness"));

			GL_CHECK(glAttachShader(program, vertexShader));
			GL_CHECK(glAttachShader(program, geomShader));
			GL_CHECK(glAttachShader(program, fragmentShader));


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

			mSurfaceSliceProgram = program;



			QFile file4(":/crosssectionvert.glsl"), file5(":/crosssectionfbgeom.glsl");

			if (!file4.open(QIODevice::ReadOnly))
			{

				qDebug() << "Cannot open file for reading: ( mesh cross section vertex ) "
					<< qPrintable(file4.errorString()) << endl;

				return;
			}

			if (!file5.open(QIODevice::ReadOnly))
			{

				qDebug() << "Cannot open file for reading: ( mesh cross section geometry ) "
					<< qPrintable(file5.errorString()) << endl;

				return;
			}

			vertShaderSrc = file4.readAll();
			geomShaderSrc = file5.readAll();

			program = glCreateProgram();


			//qDebug() << geomShaderSrc << endl;

			vertexShader = compileShaders(GL_VERTEX_SHADER, vertShaderSrc.toStdString().c_str());
			geomShader = compileShaders(GL_GEOMETRY_SHADER, geomShaderSrc.toStdString().c_str());

			GL_CHECK(glBindAttribLocation(program, 0, "vert"));
			GL_CHECK(glBindAttribLocation(program, 1, "normal"));
			GL_CHECK(glBindAttribLocation(program, 2, "color"));
			GL_CHECK(glBindAttribLocation(program, 3, "wallThickness"));

			GL_CHECK(glAttachShader(program, vertexShader));
			GL_CHECK(glAttachShader(program, geomShader));
			

			GL_CHECK(glLinkProgram(program));

			status;

			GL_CHECK(glGetProgramiv(program, GL_LINK_STATUS, &status));

			if ( status == GL_FALSE )
			{
				GLchar emsg[1024];
				GL_CHECK(glGetProgramInfoLog(program, sizeof(emsg), 0, emsg));
				fprintf(stderr, "Error linking GLSL program : %s\n", emsg);
				return;
			}

			mVolumeSliceTFProgram = program;

			//declare transform feedback varying name
			static const char *const vars[] = {
				
				"intersectionPoint", "intersectionNormal", "intersectionColor"
			};

			std::cout << " variable count : " << sizeof(vars) / sizeof(vars[0]) << std::endl;

			 
			GL_CHECK( glTransformFeedbackVaryings(mVolumeSliceTFProgram, 3, vars, GL_INTERLEAVED_ATTRIBS) );

			GL_CHECK( glLinkProgram(mVolumeSliceTFProgram) );



		}

		GLuint SliceRenderManager::compileShaders(GLenum shaderType, const char *shaderSource)
		{
			GLuint shader = glCreateShader(shaderType);

			GL_CHECK(glShaderSource(shader, 1, &shaderSource, NULL));

			GL_CHECK(glCompileShader(shader));

			GLint status;

			glGetShaderiv(shader, GL_COMPILE_STATUS, &status);


			if (status == GL_FALSE)
			{
				GLchar emsg[10024];

				glGetShaderInfoLog(shader, sizeof(emsg), 0, emsg);

				QString errMsg = QString("Error compiling GLSL shader (%s): %s\n") + emsg;

				qDebug() << " error in compiling shaders , file : " << __FILE__ << " : line : " << __LINE__ << errMsg << endl;
			}

			return shader;
		}

		void SliceRenderManager::setChangedData()
		{
			updateSurface();
		}


		void SliceRenderManager::render()
		{
			if (!mIsInitialized)
				return;

			GL_CHECK( glBindVertexArray( mSurfaceVAO[0] ) );

			if ( mHasNewData )
			{

				GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, mSurfaceVBOs[0]));
				GL_CHECK(glBufferData(GL_ARRAY_BUFFER, mVertexRenderData.size() * sizeof(VertexData),
					                 (void *)mVertexRenderData.data(), GL_STATIC_DRAW));

				//update points
				GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mSurfaceVBOs[1]));
				GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, mSurfaceIndices.size() * sizeof(GLuint),
					                  mSurfaceIndices.data(), GL_STATIC_DRAW));

				//update faces
				GL_CHECK(glEnableVertexAttribArray(0));
				GL_CHECK(glEnableVertexAttribArray(1));
				GL_CHECK(glEnableVertexAttribArray(2));
				GL_CHECK(glEnableVertexAttribArray(3));

				GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), 0));

				int offset = 3 * sizeof(float);

				GL_CHECK(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offset));

				offset += 3 * sizeof(float);

				GL_CHECK(glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offset));

				offset += 3 * sizeof(float);

				GL_CHECK(glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offset));

				mHasNewData = false;

				GL_CHECK(glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, mFeedbackBuffer));

				GL_CHECK(glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, 1024 * 1024 * 100, NULL, GL_DYNAMIC_COPY));

				GL_CHECK(glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mFeedbackBuffer, 0, 1024 * 1024 * 100));
			}


			computeSlice();

			renderSlice();

		}

		void computeOppositeEndsOnPlane( std::vector< Eigen::Vector3f >& edgeEnds, std::vector< Eigen::Vector3f >& edgeNormals, std::vector< Eigen::Vector3f >& colors ,
			                             std::vector< SliceRenderManager::VertexData >& vertexData ,
			                             RTCDevice& device, RTCScene& scene, float vstep)
		{
#if 0

			vertexData.resize(edgeEnds.size());

			float maxLength = FLT_MAX;

			float nearV = vstep * 0.01;

			int numVertices = edgeEnds.size();

			std::atomic<int > nValidIntersection;

			nValidIntersection = 0;

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

					ray.orgx[ii] = edgeEnds[id](0);
					ray.orgy[ii] = edgeEnds[id](1);
					ray.orgz[ii] = edgeEnds[id](2);
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

					ray.dirx[ii] = -edgeNormals[id](0);
					ray.diry[ii] = -edgeNormals[id](1);
					ray.dirz[ii] = -edgeNormals[id](2);

					ray.geomID[ii] = RTC_INVALID_GEOMETRY_ID;
					ray.instID[ii] = RTC_INVALID_GEOMETRY_ID;
					ray.primID[ii] = RTC_INVALID_GEOMETRY_ID;

					ray.tnear[ii] = nearV;
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
						vertexData[vv + ii].mVertex(0) = ray.orgx[ii];
						vertexData[vv + ii].mVertex(1) = ray.orgy[ii];
						vertexData[vv + ii].mVertex(2) = ray.orgz[ii];
						vertexData[vv + ii].mWallThickness = ray.tfar[ii];
						vertexData[vv + ii].mVertexNormal(0) = ray.dirx[ii];
						vertexData[vv + ii].mVertexNormal(1) = ray.diry[ii];
						vertexData[vv + ii].mVertexNormal(2) = ray.dirz[ii];
						vertexData[vv + ii].mVertexColor = colors[vv + ii];
						
						nValidIntersection++;

						if (vertexData[vv + ii].mWallThickness > vstep * 100)
						{
							vertexData[vv + ii].mWallThickness = -1;

							vertexData[vv + ii].mVertexColor = Eigen::Vector3f(1, 1, 1);
						}
							
					}
					else
					{
						vertexData[vv + ii].mWallThickness = -1;

						vertexData[vv + ii].mVertexColor = Eigen::Vector3f(1, 1, 1);
					}
				}

			}

			std::cout << " num valid intersections : " << nValidIntersection << std::endl;
#endif

		}

		void SliceRenderManager::computeSlice()
		{
			GL_CHECK(glUseProgram(mVolumeSliceTFProgram));
			
#if 1

			if (mSurfaceIndices.size() > 0 )
			{

				GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, mSurfaceVBOs[0]));

				// Bind transform feedback and target buffer
				GL_CHECK( glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0) );

				GL_CHECK(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mFeedbackBuffer));

				GL_CHECK( glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) );

				GLint planeNormalLoc = glGetUniformLocation( mVolumeSliceTFProgram, "planeNormal");
				GLint planeCoeffLoc = glGetUniformLocation( mVolumeSliceTFProgram, "planeCoeff");
				GLint planeTypeLoc = glGetUniformLocation( mVolumeSliceTFProgram, "pType");

				GLint mvpMatrix = glGetUniformLocation(mVolumeSliceTFProgram, "mvpMatrix");

				GL_CHECK( glUniform3f(planeCoeffLoc, mCoeff(0), mCoeff(1), mCoeff(2)));

				GL_CHECK( glUniform1i(planeTypeLoc, mPlaneType));

				GL_CHECK(glUniformMatrix4fv(mvpMatrix, 1, false, mCamera->getModelViewProjectionMatrix().data()));

				// Begin transform feedback
				GL_CHECK( glBeginQueryIndexed( GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN,0 , mFeedBackQuery) );

				GL_CHECK( glBeginTransformFeedback(GL_LINES));

				GL_CHECK( glDrawElements( GL_TRIANGLES , mSurfaceIndices.size() , GL_UNSIGNED_INT , (GLuint *)NULL ) );

				GL_CHECK( glEndTransformFeedback() );

				GL_CHECK( glEndQueryIndexed(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN , 0) );

				GLuint numWrittenPrimitives;

				GL_CHECK( glGetQueryObjectuiv(mFeedBackQuery, GL_QUERY_RESULT, &numWrittenPrimitives ) );

				numWrittenPrimitives *= 3;

			//	std::cout << " num preimitives : " << numWrittenPrimitives << std::endl;

				std::vector< Eigen::Vector3f > vertices(numWrittenPrimitives), normals(numWrittenPrimitives), colors(numWrittenPrimitives);

				Eigen::Vector3f* vdata = ( Eigen::Vector3f* ) glMapBuffer( GL_TRANSFORM_FEEDBACK_BUFFER , GL_READ_ONLY );

				for ( int pp = 0; pp < numWrittenPrimitives; pp++)
				{
					vertices[pp] = vdata[3 * pp];
					normals[pp] = vdata[3 * pp + 1];
					colors[pp] = vdata[3 * pp + 2];
				}

				GL_CHECK( glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER) );

				if ( numWrittenPrimitives > 0 )
				{
					std::vector< Eigen::Vector3f > otherEnds(numWrittenPrimitives), otherEndNormals(numWrittenPrimitives);

					std::vector< bool > oppositeEndFound( numWrittenPrimitives , false );

					mSliceVertexData.resize(numWrittenPrimitives);

					computeOppositeEndsOnPlane( vertices, normals, colors , mSliceVertexData , mWte->mDevice, mWte->mScene, mVolume->mVoxelStep(0) );
				}

			}

#endif
			GL_CHECK(glUseProgram(0));
		}


		void SliceRenderManager::renderSlice()
		{

			if (mSliceVertexData.size() == 0)
				return;

			GL_CHECK(glUseProgram(mSurfaceSliceProgram));

			GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mDefaultFBO));

			//GL_CHECK(glEnable(GL_LINE_SMOOTH));

			GL_CHECK( glDisable(GL_CULL_FACE) );

			GL_CHECK(glBindVertexArray(mSurfaceVAO[1]));

				GL_CHECK( glBindBuffer(GL_ARRAY_BUFFER, mSliceVBO[0]));
				GL_CHECK( glBufferData( GL_ARRAY_BUFFER, mSliceVertexData.size() * sizeof(VertexData),
					                    (void *)mSliceVertexData.data(), GL_DYNAMIC_DRAW ) );

				std::vector< unsigned int > indexArr(mSliceVertexData.size());

				for (int ii = 0; ii < indexArr.size(); ii++)
				{
					indexArr[ii] = ii;
				}

				GL_CHECK( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mSliceVBO[1]));
				GL_CHECK( glBufferData( GL_ELEMENT_ARRAY_BUFFER, indexArr.size() * sizeof(unsigned int),
					(void *)indexArr.data(), GL_DYNAMIC_DRAW));
				

				//update faces
				GL_CHECK(glEnableVertexAttribArray(0));
				GL_CHECK(glEnableVertexAttribArray(1));
				GL_CHECK(glEnableVertexAttribArray(2));
				GL_CHECK(glEnableVertexAttribArray(3));

				GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), 0));

				int offset = 3 * sizeof(float);

				GL_CHECK(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offset));

				offset += 3 * sizeof(float);

				GL_CHECK(glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offset));

				offset += 3 * sizeof(float);

				GL_CHECK(glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offset));

#if 1
				GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

				GLint mvpMatrix = glGetUniformLocation(mSurfaceSliceProgram, "mvpMatrix");


				GL_CHECK(glUniformMatrix4fv(mvpMatrix, 1, false, mCamera->getModelViewProjectionMatrix().data()));


				GL_CHECK( glDrawElements( GL_LINES ,   mSliceVertexData.size() , GL_UNSIGNED_INT , 0 ) );

#endif
			GL_CHECK(glUseProgram(0));

			GL_CHECK(glBindVertexArray(0));

		}

		void SliceRenderManager::setWallthicknessEstimator( imt::volume::WallthicknessEstimator *wte )
		{
			mWte = wte;
		}


		void SliceRenderManager::registerKeyPressEvent( QKeyEvent* e )
		{


		}
		void SliceRenderManager::registerKeyReleaseEvent( QKeyEvent* e)
		{


		}

		void SliceRenderManager::registerMousePressEvent( QMouseEvent *event )
		{

		}

		void SliceRenderManager::registerMouseReleaseEvent( QMouseEvent *event )
		{


		}

		void SliceRenderManager::registerMouseMoveEvent( QMouseEvent *event )
		{



		}


		void SliceRenderManager::registerWheelEvent( QWheelEvent * event )
		{


		}

		void SliceRenderManager::buildProjectionmatrix( const Eigen::Vector2f& xLimits , const Eigen::Vector2f& yLimits , const Eigen::Vector2f& zLimits)
		{

			float leftV, rightV, bottomV, topV, nearV, farV;

			if (mPlaneType == XY)
			{
				leftV = xLimits(0);
				rightV = xLimits(1);
				bottomV = yLimits(0);
				topV = yLimits(1);
				nearV = zLimits(0);
				farV = zLimits(1);

			}
			else if (mPlaneType == YZ)
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

			orthoProjMat.frustum( leftV , rightV , bottomV , topV , nearV , farV );


			mOrthoProjMat = orthoProjMat;


		}





		void SliceRenderManager::setLimits( const Eigen::Vector3f& minLimits , const Eigen::Vector3f& maxLimits )
		{
			mMinLimits = minLimits;

			mMaxLimits = maxLimits;

			Eigen::Vector2f xLimits(minLimits(0), maxLimits(0)), yLimits(minLimits(1), maxLimits(1)), zLimits(minLimits(2), maxLimits(2));

			buildProjectionmatrix(xLimits, yLimits, zLimits);
		}




		
	}

}