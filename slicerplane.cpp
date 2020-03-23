#include "slicerplane.h"
#include "openglhelper.h"
#include "iostream"

namespace imt{

	namespace volume{


		SlicerPlane::SlicerPlane()
		{
			mCageChanged = false;
			mPlaneChanged = false;
			mRenderCage = false;
			mRenderPlane = false;

			mCamera = 0;

			mDefaultFBO = 0;

			mPlaneType = 0;
		}


		void SlicerPlane::setDimension( const Eigen::Vector3f& minLimits, const Eigen::Vector3f& maxLimits )
		{
			mMinLimits = minLimits;
			mMaxLimits = maxLimits;

			mCageChanged = true;
		}


		void SlicerPlane::setCageVisible(bool visib)
		{
			mRenderCage = visib;
		}

		void SlicerPlane::setPlaneVisible(bool visib)
		{
			mRenderPlane = visib;
		}

		void SlicerPlane::setPlaneType(int type , int h)
		{
			mPlaneType = type;

			mPlaneChanged = true;

			mHeight = h;
		}


		void SlicerPlane::setProgressReporter(imt::volume::ProgressReporter *progressReporter)
		{
			mProgressReporter = progressReporter;
		}



		void SlicerPlane::setCamera(TrackBallCamera *camera)
		{
			mCamera = camera;
		}


		void SlicerPlane::setDfaultFBO( int defaultFBO )
		{
			mDefaultFBO = defaultFBO;
		}


		void SlicerPlane::init()
		{
			initializeOpenGLFunctions();

			GL_CHECK( glGenVertexArrays(2, mVAOs) );
			GL_CHECK( glGenBuffers(5, mVBOs) );

			mBoundingBoxIndicesBuffer = mVBOs[0];
			mBoundingBoxVertexBuffer = mVBOs[1];
			mPlaneBoundaryBuffer = mVBOs[2];
			mPlaneIndicesBuffer = mVBOs[3];
			mPlaneVertexBuffer = mVBOs[4];


			unsigned int boxFrameIndices[] = { 0 , 1 , 1 , 2 , 2 , 3 , 
				                               3 , 0 , 4 , 5 , 5 , 6 , 
									           6 , 7 , 7 , 4 , 0 , 4 , 
									           1 , 5 , 2 , 6 , 3 , 7 };


			unsigned int planeFrameIndices[] = { 0 , 1 , 1 , 2 , 2 , 3 , 3 , 0 };

			unsigned int planeIndices[] = { 0, 1, 2, 0 , 2 , 3 };

			GL_CHECK(glBindVertexArray(mVAOs[0]));

			GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER, mBoundingBoxVertexBuffer ) );
			

			GL_CHECK( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mBoundingBoxIndicesBuffer ) );

			GL_CHECK( glBufferData( GL_ELEMENT_ARRAY_BUFFER, 24 * sizeof(GLuint),
				                    boxFrameIndices, GL_STATIC_DRAW));

			GL_CHECK(glEnableVertexAttribArray(0));

			int offset = 0;

			GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Eigen::Vector3f), 0));
			
			GL_CHECK(glBindVertexArray(mVAOs[1]));
			
			
			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, mPlaneVertexBuffer));
			
			GL_CHECK( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mPlaneIndicesBuffer ) );

			GL_CHECK( glBufferData( GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint),
				                    planeIndices, GL_STATIC_DRAW));


			GL_CHECK( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mPlaneBoundaryBuffer ) );

			GL_CHECK( glBufferData( GL_ELEMENT_ARRAY_BUFFER, 8 * sizeof(GLuint),
				                    planeFrameIndices, GL_STATIC_DRAW ) );

			GL_CHECK(glEnableVertexAttribArray(0));

			GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Eigen::Vector3f), 0));

			GL_CHECK(glBindVertexArray(0));


			initShaders();


		}


		void SlicerPlane::setCoeff(const Eigen::Vector3f& coeff)
		{
			mPlaneCoeffs = coeff;
		}


		GLuint SlicerPlane::compileShaders(GLenum shaderType, const char *shaderSource)
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



		void SlicerPlane::initShaders()
		{

			QFile file1(":/slicerplanefrag.glsl"), file2(":/cagefrag.glsl"), file3(":/basicvert.glsl");

			file1.open(QIODevice::ReadOnly);
			file2.open(QIODevice::ReadOnly);
			file3.open(QIODevice::ReadOnly);


			QString vertShaderSrc = file3.readAll();

			QString cageFragShaderSrc = file2.readAll();

			QString slicerPlaneFragSrc = file1.readAll();


			GLuint vertexShader = compileShaders(GL_VERTEX_SHADER, vertShaderSrc.toStdString().c_str());
			GLuint fragmentShader = compileShaders(GL_FRAGMENT_SHADER, cageFragShaderSrc.toStdString().c_str());

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

			mWireBoxProgram = program;
			
			vertexShader = compileShaders(GL_VERTEX_SHADER, vertShaderSrc.toStdString().c_str());
			fragmentShader = compileShaders(GL_FRAGMENT_SHADER, slicerPlaneFragSrc.toStdString().c_str());

			program = glCreateProgram();

			GL_CHECK(glBindAttribLocation(program, 0, "vert"));

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

			mSlicerPlaneProgram = program;

			std::cout << " slicer plane shaders compiled successfully " << std::endl;

		}


		void SlicerPlane::renderCage()
		{
		

			GL_CHECK( glUseProgram(mWireBoxProgram) );

			GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mDefaultFBO));


			//std::cout << " default fbo : " << mDefaultFBO << std::endl;

			GL_CHECK( glBindVertexArray(mVAOs[0]));

			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, mBoundingBoxVertexBuffer));

			GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBoundingBoxIndicesBuffer));

			GLint mvpMatrix = glGetUniformLocation(mWireBoxProgram, "mvpMatrix");
			QMatrix4x4 mvpMat = mCamera->getModelViewProjectionMatrix();

			GLint colorLoc = glGetUniformLocation( mWireBoxProgram , "color" );

			Eigen::Vector3f color(1, 0, 0);

			//std::cout << " mvp and color locations : " << mvpMatrix << " " << colorLoc << std::endl;

			GL_CHECK( glUniformMatrix4fv(mvpMatrix, 1, false, mvpMat.data()));
			GL_CHECK( glUniform3f( colorLoc, 1 , 0 , 0 ));


			GL_CHECK( glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0) );

			GL_CHECK( glBindVertexArray(0) );

			GL_CHECK( glUseProgram(0) );

		}


		void SlicerPlane::updateVertexData()
		{
			if (mCageChanged)
			{

				std::vector< Eigen::Vector3f > cageVertices(8);

				cageVertices[0] = mMinLimits;
				cageVertices[1] = Eigen::Vector3f(mMinLimits(0), mMaxLimits(1), mMinLimits(2));
				cageVertices[2] = Eigen::Vector3f(mMaxLimits(0), mMaxLimits(1), mMinLimits(2));
				cageVertices[3] = Eigen::Vector3f(mMaxLimits(0), mMinLimits(1), mMinLimits(2));

				cageVertices[4] = Eigen::Vector3f(mMinLimits(0), mMinLimits(1), mMaxLimits(2));
				cageVertices[5] = Eigen::Vector3f(mMinLimits(0), mMaxLimits(1), mMaxLimits(2));
				cageVertices[6] = Eigen::Vector3f(mMaxLimits(0), mMaxLimits(1), mMaxLimits(2));
				cageVertices[7] = Eigen::Vector3f(mMaxLimits(0), mMinLimits(1), mMaxLimits(2));

				mPlaneCoeffs(0) = (mMinLimits(0) + mMaxLimits(0)) * 0.5;
				mPlaneCoeffs(1) = (mMinLimits(1) + mMaxLimits(1)) * 0.5;
				mPlaneCoeffs(2) = (mMinLimits(2) + mMaxLimits(2)) * 0.5;


				std::cout << " updating cage : " << mMinLimits.transpose() << " " << mMaxLimits.transpose() << std::endl;

				GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, mBoundingBoxVertexBuffer));

				GL_CHECK(glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(Eigen::Vector3f),
					                  cageVertices.data(), GL_STATIC_DRAW));

				GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));

				mCageChanged = false;

				mPlaneChanged = true;
			}




			if (mPlaneChanged)
			{
				std::vector< Eigen::Vector3f > planeVertices( 4 );

				mRenderPlane = true;

				if ( mPlaneType == 0 )
				{


					planeVertices[0] = Eigen::Vector3f(mPlaneCoeffs(0), mMinLimits(1), mMinLimits(2));
					planeVertices[1] = Eigen::Vector3f(mPlaneCoeffs(0), mMinLimits(1), mMaxLimits(2));
					planeVertices[2] = Eigen::Vector3f(mPlaneCoeffs(0), mMaxLimits(1), mMaxLimits(2));
					planeVertices[3] = Eigen::Vector3f(mPlaneCoeffs(0), mMaxLimits(1), mMinLimits(2));
				}

				if ( mPlaneType == 1 )
				{
					planeVertices[0] = Eigen::Vector3f( mMinLimits(0), mPlaneCoeffs(1), mMinLimits(2));
					planeVertices[1] = Eigen::Vector3f( mMinLimits(0), mPlaneCoeffs(1), mMaxLimits(2));
					planeVertices[2] = Eigen::Vector3f( mMaxLimits(0), mPlaneCoeffs(1), mMaxLimits(2));
					planeVertices[3] = Eigen::Vector3f( mMaxLimits(0), mPlaneCoeffs(1), mMinLimits(2));
				}

				if ( mPlaneType == 2 )
				{

					planeVertices[0] = Eigen::Vector3f(mMinLimits(0), mMinLimits(1), mPlaneCoeffs(2));
					planeVertices[1] = Eigen::Vector3f(mMinLimits(0), mMaxLimits(1), mPlaneCoeffs(2));
					planeVertices[2] = Eigen::Vector3f(mMaxLimits(0), mMaxLimits(1), mPlaneCoeffs(2));
					planeVertices[3] = Eigen::Vector3f(mMaxLimits(0), mMinLimits(1), mPlaneCoeffs(2));
				}
				else
				{
					mRenderPlane = false;
				}

				GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, mPlaneVertexBuffer));

				GL_CHECK(glBufferData( GL_ARRAY_BUFFER, 4 * sizeof(Eigen::Vector3f),
					                   planeVertices.data(), GL_STATIC_DRAW));

				GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));

				mPlaneChanged = false;
			}
			else
			{
				//std::cout << " plane not changed " << std::endl;
			}

		}


		void SlicerPlane::renderPlane()
		{
			
			//GL_CHECK(glUseProgram(mWireBoxProgram));

			//GL_CHECK(glBindVertexArray(mVAOs[1]));

			//GL_CHECK(glDrawElements(GL_LINES, 8, GL_UNSIGNED_INT, 0));

			//GL_CHECK(glBindVertexArray(0));

			//GL_CHECK(glUseProgram(0));

			GL_CHECK(glDisable(GL_CULL_FACE));
			GL_CHECK( glEnable(GL_BLEND) );
			GL_CHECK( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) );

			GL_CHECK( glUseProgram( mSlicerPlaneProgram ) );

			GL_CHECK( glBindVertexArray(mVAOs[1]));

			GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mDefaultFBO));

			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, mPlaneVertexBuffer));

			GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mPlaneIndicesBuffer));

			GLint mvpMatrix = glGetUniformLocation(mSlicerPlaneProgram, "mvpMatrix");
			QMatrix4x4 mvpMat = mCamera->getModelViewProjectionMatrix();

			GLint colorLoc = glGetUniformLocation(mSlicerPlaneProgram, "planeColor");

		//	Eigen::Vector3f color( 0 , 1 , 0 , 0.5 );

			//std::cout << " mvp and color locations : " << mvpMatrix << " " << colorLoc << std::endl;

			GL_CHECK( glUniformMatrix4fv( mvpMatrix , 1 , false , mvpMat.data() ) );
			GL_CHECK( glUniform4f( colorLoc , 0, 1, 0, 0.5 ) );

			GL_CHECK( glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));

			GL_CHECK( glBindVertexArray(0));

			GL_CHECK( glUseProgram(0));

			GL_CHECK( glDisable(GL_BLEND));

			GL_CHECK(glEnable(GL_CULL_FACE));


		}


		void SlicerPlane::render()
		{

			updateVertexData();

			if ( mRenderCage )
			{
				renderCage();

				renderPlane();
			}


			if ( mRenderPlane )
			{
				renderPlane();
			}

		}


		void SlicerPlane::registerMouseMoveEvent(QMouseEvent* event)
		{
		
			if ( mRightButtonPressed )
			{

				if (mPlaneType > 2)
					return;

				mPreviousPos = mCurrentPos;

				mCurrentPos = event->pos();

				float delY = mCurrentPos.y() - mPreviousPos.y();

				float fraction = delY / mHeight;

				float shift = (mMaxLimits(mPlaneType) - mMinLimits(mPlaneType)) * fraction;

				mPlaneCoeffs(mPlaneType) += shift;

				mPlaneCoeffs(mPlaneType) = std::min(mPlaneCoeffs(mPlaneType), mMaxLimits(mPlaneType));
				mPlaneCoeffs(mPlaneType) = std::max(mPlaneCoeffs(mPlaneType), mMinLimits(mPlaneType));

				mPlaneChanged = true;

				mRenderPlane = true;

			}
		


		}

		void SlicerPlane::registerMousePressEvent(QMouseEvent* event)
		{
			
			if ( event->buttons() == Qt::RightButton )
			{
				mPreviousPos = event->pos();

				mCurrentPos = event->pos();

				mRightButtonPressed = true;
			}

		}


		void SlicerPlane::registerMouseReleaseEvent( QMouseEvent* event )
		{

			mRightButtonPressed = false;
		}


		Eigen::Vector3f& SlicerPlane::getPlaneSlicerCoeffs()
		{
			return mPlaneCoeffs;
		}


	}


}