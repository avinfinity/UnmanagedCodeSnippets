#include "segmentationbenchmark.h"
#include "openglhelper.h"



namespace imt{

	namespace volume{


		SegmentationBenchmark::SegmentationBenchmark(imt::volume::VolumeInfo *volumeInfo) : mVolumeInfo(volumeInfo)
		{
			mHasNewReferenceData = false;
			mHasNewActualData = false;
		}

		void SegmentationBenchmark::init()
		{
			initializeOpenGLFunctions();

			GL_CHECK( glGenBuffers( 4 , mVBOs ) );
			GL_CHECK( glGenVertexArrays( 2 , mVAOs) );

			initializeShaders();

		}

		void SegmentationBenchmark::render()
		{


		}


		void SegmentationBenchmark::updateBenchMarkData()
		{
			buildTracer();

			compareMeshes();

		}

		void SegmentationBenchmark::renderReferenceData()
		{

			GL_CHECK(glBindVertexArray(mVAOs[0]));

			if ( mHasNewReferenceData )
			{
				GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, mVBOs[0] ) );
				GL_CHECK( glBufferData( GL_ARRAY_BUFFER, mRefVertData.size() * sizeof(VertexData),
					                    (void *)mRefVertData.data(), GL_STATIC_DRAW));

				//update points
				GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVBOs[1]));
				GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, mRefIndices.size() * sizeof(GLuint),
					mRefIndices.data(), GL_STATIC_DRAW));

				//update faces
				GL_CHECK(glEnableVertexAttribArray(0));
				GL_CHECK(glEnableVertexAttribArray(1));
				GL_CHECK(glEnableVertexAttribArray(2));

				GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), 0));

				int offset = 3 * sizeof(float);

				GL_CHECK(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offset));

				offset += 3 * sizeof(float);

				GL_CHECK(glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offset));

				mHasNewReferenceData = false;

			}


			GL_CHECK(glUseProgram(mSurfaceDisplayProgram));

			GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mDefaultFBO));
			GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

			GLint mvpMatrixLoc = glGetUniformLocation(mSurfaceDisplayProgram, "mvpMatrix");
			GLint rtLoc = glGetUniformLocation(mSurfaceDisplayProgram, "rT");


			GL_CHECK( glUniformMatrix4fv( mvpMatrixLoc, 1, false, mCamera->getModelViewProjectionMatrix().data() ) );

			GL_CHECK(glDrawElements( GL_LINES , mRefIndices.size() , GL_UNSIGNED_INT , 0 ) );

			GL_CHECK(glUseProgram(0));

			GL_CHECK(glBindVertexArray(0));

		}

		void SegmentationBenchmark::renderActualData()
		{

			GL_CHECK(glBindVertexArray(mVAOs[1]));

			if (mHasNewActualData)
			{
				GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, mVBOs[2]));
				GL_CHECK(glBufferData(GL_ARRAY_BUFFER, mActualVertData.size() * sizeof(VertexData),
					(void *)mActualVertData.data(), GL_STATIC_DRAW));

				//update points
				GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVBOs[3]));
				GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, mActualIndices.size() * sizeof(GLuint),
					mActualIndices.data(), GL_STATIC_DRAW));

				//update faces
				GL_CHECK(glEnableVertexAttribArray(0));
				GL_CHECK(glEnableVertexAttribArray(1));
				GL_CHECK(glEnableVertexAttribArray(2));

				GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), 0));

				int offset = 3 * sizeof(float);

				GL_CHECK(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offset));

				offset += 3 * sizeof(float);

				GL_CHECK(glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offset));

				mHasNewActualData = false;
			}

			GL_CHECK(glUseProgram(mSurfaceDisplayProgram));

			GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mDefaultFBO));
			GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

			GLint mvpMatrixLoc = glGetUniformLocation(mSurfaceDisplayProgram, "mvpMatrix");
			GLint rtLoc = glGetUniformLocation(mSurfaceDisplayProgram, "rT");


			GL_CHECK(glUniformMatrix4fv(mvpMatrixLoc, 1, false, mCamera->getModelViewProjectionMatrix().data()));

			GL_CHECK(glDrawElements(GL_LINES, mActualIndices.size(), GL_UNSIGNED_INT, 0));

			GL_CHECK(glUseProgram(0));

			GL_CHECK(glBindVertexArray(0));

		}


		void SegmentationBenchmark::buildTracer()
		{

#if 0
			_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
			_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

			rtcInit(NULL);

			unsigned int geomID;

			mScene = 0;

			mDevice = rtcNewDevice(NULL);

			mScene = rtcDeviceNewScene(mDevice, RTC_SCENE_DYNAMIC, RTC_INTERSECT8);

			long int numTriangles = mVolumeInfo->mSBRefIndices.size() / 3;
			long int numVertices = mVolumeInfo->mSBRefPoints.size();


			mGeomID = rtcNewTriangleMesh(mScene, RTC_GEOMETRY_STATIC, numTriangles, numVertices, 1);

			EmbreeTriangle* triangles = (EmbreeTriangle*)rtcMapBuffer(mScene, mGeomID, RTC_INDEX_BUFFER);

			for (int tt = 0; tt < numTriangles; tt++)
			{
				triangles[tt].v0 = mVolumeInfo->mSBRefIndices[3 * tt];
				triangles[tt].v1 = mVolumeInfo->mSBRefIndices[3 * tt + 1];
				triangles[tt].v2 = mVolumeInfo->mSBRefIndices[3 * tt + 2];
			}

			rtcUnmapBuffer(mScene, mGeomID, RTC_INDEX_BUFFER);

			EmbreeVertex* vertices = (EmbreeVertex*)rtcMapBuffer(mScene, mGeomID, RTC_VERTEX_BUFFER);

			int numBasePoints = numVertices;

			for (int pp = 0; pp < numBasePoints; pp++)
			{
				vertices[pp].x = mVolumeInfo->mSBRefPoints[pp](0);
				vertices[pp].y = mVolumeInfo->mSBRefPoints[pp](1);
				vertices[pp].z = mVolumeInfo->mSBRefPoints[pp](2);
				vertices[pp].a = 1.0;
			}

			rtcUnmapBuffer(mScene, mGeomID, RTC_VERTEX_BUFFER);

			rtcCommit(mScene);
#endif

		}


		void SegmentationBenchmark::compareMeshes()
		{

#if 0

			std::vector< float > closestSurfaceDistances( mVolumeInfo->mSBDataPoints.size());

			std::fill(closestSurfaceDistances.begin(), closestSurfaceDistances.end(), 1000);

			std::vector< bool > oppositeEndFound(mVolumeInfo->mSBDataPoints.size(), false);

			float nearV = 0;
			float farV = FLT_MAX;

			int numVertices = mVolumeInfo->mSBDataPoints.size();

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

					ray.orgx[ii] = mVolumeInfo->mSBDataPoints[id](0);
					ray.orgy[ii] = mVolumeInfo->mSBDataPoints[id](1);
					ray.orgz[ii] = mVolumeInfo->mSBDataPoints[id](2);
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

					ray.dirx[ii] = mVolumeInfo->mSBDataNormals[id](0);
					ray.diry[ii] = mVolumeInfo->mSBDataNormals[id](1);
					ray.dirz[ii] = mVolumeInfo->mSBDataNormals[id](2);

					ray.geomID[ii] = RTC_INVALID_GEOMETRY_ID;
					ray.instID[ii] = RTC_INVALID_GEOMETRY_ID;
					ray.primID[ii] = RTC_INVALID_GEOMETRY_ID;

					ray.tnear[ii] = nearV;
					ray.tfar[ii] = farV;

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

					ray.orgx[ii] = mVolumeInfo->mSBDataPoints[id](0);
					ray.orgy[ii] = mVolumeInfo->mSBDataPoints[id](1);
					ray.orgz[ii] = mVolumeInfo->mSBDataPoints[id](2);
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

					ray.dirx[ii] = -mVolumeInfo->mSBDataNormals[id](0);
					ray.diry[ii] = -mVolumeInfo->mSBDataNormals[id](1);
					ray.dirz[ii] = -mVolumeInfo->mSBDataNormals[id](2);

					ray.geomID[ii] = RTC_INVALID_GEOMETRY_ID;
					ray.instID[ii] = RTC_INVALID_GEOMETRY_ID;
					ray.primID[ii] = RTC_INVALID_GEOMETRY_ID;

					ray.tnear[ii] = nearV;
					ray.tfar[ii] = farV;

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


			int nPoints = numVertices;

			std::vector< Eigen::Vector3f > vertexColors(nPoints, Eigen::Vector3f(1, 1, 1));

			double thickness = 0;
			int count = 0;

			float coeff = 2;

			float th1 = 0.02 * coeff;
			float th2 = 0.03 * coeff;

			for (int vv = 0; vv < nPoints; vv++)
			{
				if (!oppositeEndFound[vv])
					continue;

				if (closestSurfaceDistances[vv] < th1)
				{
					vertexColors[vv](0) = 1 - closestSurfaceDistances[vv] / th1;
					vertexColors[vv](1) = closestSurfaceDistances[vv] / th1;
					vertexColors[vv](2) = 0;


				}
				else if (closestSurfaceDistances[vv] > th1  && closestSurfaceDistances[vv] < th2)
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
#endif


		}


		void SegmentationBenchmark::initializeShaders()
		{
			initializeOpenGLFunctions();

			QFile file1(":/simplevert.glsl"), file2(":/simplefrag.glsl");

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
			GL_CHECK(glBindAttribLocation(program, 1, "normal"));

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

			mSurfaceDisplayProgram = program;

		}


		GLuint SegmentationBenchmark::compileShaders(GLenum shaderType, const char *shaderSource)
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

		void SegmentationBenchmark::setCamera(TrackBallCamera *camera)
		{
			mCamera = camera;
		}


	}


}