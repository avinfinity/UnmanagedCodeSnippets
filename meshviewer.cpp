#include "meshviewer.h"
#include "QMouseEvent"
#include "openglhelper.h"
#include "QApplication"
#include "QGLFormat"
#include "embreeincludes.h"
#include "volumeinfo.h"
#include "volumeutility.h"

namespace vc{


	int MESHViewer::viewMesh( const std::vector< Eigen::Vector3f > &vertices , const std::vector< unsigned int > &indices )
	{
		int argc = 1;
		char **argv = new char*[ 2 ];

		argv[ 0 ] = ".\\standaloneviewer.exe";

		QApplication app( argc , argv );

		QGLFormat format;

		format.setVersion( 3 , 3 );
		format.setProfile( QGLFormat::CoreProfile );

		MeshViewerWidget meshViewerWidget;

		meshViewerWidget.setMeshData( vertices , indices );

		meshViewerWidget.showMaximized();

		return app.exec();

	}

	int MESHViewer::viewMesh( const std::vector< Eigen::Vector3f > &vertices , const std::vector< Eigen::Vector3f > &normals , const std::vector< unsigned int > &indices )
	{
		int argc = 1;
		char **argv = new char*[ 2 ];

		argv[ 0 ] = ".\\standaloneviewer.exe";

		QApplication app( argc , argv );

		QGLFormat format;

		format.setVersion( 3 , 3 );
		format.setProfile( QGLFormat::CoreProfile );

		MeshViewerWidget meshViewerWidget;

		meshViewerWidget.setMeshData( vertices , normals , indices );

		meshViewerWidget.showMaximized();

		return app.exec();
	}



	int MESHViewer::viewMesh(const std::vector< Eigen::Vector3f > &vertices,  const std::vector< unsigned int > &indices , imt::volume::VolumeInfo* volume )
	{
		int argc = 1;
		char **argv = new char*[2];

		argv[0] = ".\\standaloneviewer.exe";

		QApplication app(argc, argv);

		QGLFormat format;

		format.setVersion(4, 5);
		format.setProfile(QGLFormat::CompatibilityProfile);

		MeshViewerWidget meshViewerWidget;

		meshViewerWidget.setMeshData(vertices,  indices , volume);

		meshViewerWidget.showMaximized();

		return app.exec();
	}
	
	int MESHViewer::viewMesh(const std::vector< Eigen::Vector3f > &vertices, const std::vector< Eigen::Vector3f > &normals,
		const std::vector< unsigned int > &indices, imt::volume::VolumeInfo* volume)
	{
		int argc = 1;
		char **argv = new char*[2];

		argv[0] = ".\\standaloneviewer.exe";

		QApplication app(argc, argv);

		QGLFormat format;

		format.setVersion(4, 5);
		format.setProfile(QGLFormat::CompatibilityProfile);

		MeshViewerWidget meshViewerWidget;

		meshViewerWidget.setMeshData(vertices , normals, indices, volume);

		meshViewerWidget.showMaximized();

		return app.exec();
	}


	
       int MESHViewer::viewPointCloud(const std::vector< Eigen::Vector3f >& vertices, std::vector< Eigen::Vector3f >& colors)
       {
          	int argc = 0;
		char **argv = new char*[ 2 ];

		argv[ 0 ] = "standaloneviewer";

		QApplication app( argc , argv );

		QGLFormat format;

		format.setVersion( 3 , 3 );
		format.setProfile( QGLFormat::CoreProfile );

		MeshViewerWidget meshViewerWidget;

		meshViewerWidget.setPointCloudData( vertices , colors );

		meshViewerWidget.showMaximized();

		return app.exec();
       }



	MeshViewerWidget::MeshViewerWidget ( QWidget* parent, Qt::WindowFlags f ) : QOpenGLWidget( parent , f )
	{

		mIsCameraInitialized = false;

		mShaderInitialized = false;

		mMeshWithNormals = false;

		mCamera = new TrackBallCamera();

		mMesh = new SimpleMesh();

		mArrow = new SimpleMesh();

		mMesh->setCamera( mCamera );

		mArrow->setCamera(mCamera);
		
		mMeshDataLoaded = false;

		mDataLoadedToOpenGL = false;

		mVisualizeArrow = false;

		mShaderSetId = -1;  
		
		mVertWNSrcs =         "#version 450\n"
			                  "layout (location=0) in vec3 position;\n"
			                  "layout (location=1) in vec3 vertexColor;\n"
			                  "layout (location=2) in vec3 vertexNormal;\n"
			                  "out vec4 vPosition;\n"
			                  "out vec3 vColor;\n";
			                  "out vec3 vNormal;\n"
                              "void main()\n"
                              "{\n"
                              "vPosition = vec4( position , 1 );\n"
                              "vColor = vertexColor;\n"
                              "vNormal = vertexNormal;\n"
                              "}\n";
		
		mGeomWNSrcs = "#version 450\n"
                              "layout(triangles) in;\n"
                              "layout(triangle_strip, max_vertices = 3) out;\n"
			                  "uniform mat4 mvMatrix;\n"
			                  "uniform mat4 mvpMatrix;\n"
			                  "uniform mat3 normalMatrix;\n"
                              "uniform vec2 WIN_SCALE;\n"
                              "in vec4[ 3 ] vPosition;\n"
			                  "in vec3[ 3 ] vNormal;\n"
                              "in vec3[ 3 ] vColor;\n"
                              "out vec3 dist;\n"  
                              "out vec4 fPosition;\n" 
			                  "out vec3 surfacePos;\n"
                              "out vec3 fColor;\n"
			                  "out vec3 fNormal;"
                              "void main()\n"
                              "{\n"
                              "vec4 v0 = mvpMatrix * vPosition[0];\n"
                              "vec4 v1 = mvpMatrix * vPosition[1];\n"
                              "vec4 v2 = mvpMatrix * vPosition[2];\n"
                              "vec2 p0 = WIN_SCALE * v0.xy / v0.w;\n"
                              "vec2 p1 = WIN_SCALE * v1.xy / v1.w;\n"
                              "vec2 p2 = WIN_SCALE * v2.xy / v2.w;\n"
                              "vec2 vect0 = p2 - p1;\n"
                              "vec2 vect1 = p2 - p0;\n"
                              "vec2 vect2 = p1 - p0;\n"
                              "float area = abs( vect1.x * vect2.y - vect1.y * vect2.x);\n"
                              "dist = vec3( area / length( vect0 ) , 0 , 0 );\n"
                              "fColor = vertColor[ 0 ];\n"
                              "fPosition = vPosition[0];\n"
			                  "surfacePos = vec3(mvMatrix * vPosition[0]);\n"
			                  "fNormal = normalMatrix * vNormal[0];\n"
                              "gl_Position = v0;\n"
                              "EmitVertex();\n"
                              "dist = vec3( 0 , area / length( vect1 ) , 0 );\n"
                              "fColor = vertColor[ 1 ];\n"
                              "fPosition = vPosition[1];\n"
			                  "surfacePos = vec3(mvMatrix * vPosition[1]);\n"
			                  "fNormal = normalMatrix * vNormal[1];\n"
			                  "gl_Position = v1;\n"
                              "EmitVertex();\n"
                              "dist = vec3( 0 , 0 , area / length( vect2 ) );\n"
                              "fColor = vertColor[ 2 ];\n"
                              "fPosition = vPosition[2];\n"
			                  "fNormal = normalMatrix * vNormal[2];\n"
			                  "surfacePos = vec3(mvMatrix * vPosition[2]);\n"
                              "gl_Position = v2;\n"
                              "EmitVertex();\n"
                              "EndPrimitive();\n"
                              "}\n";
		
		mFragWNSrcs = "#version 450\n"
                               "layout (location = 0) out vec4 color;\n"
                               "in vec4 fPosition;\n"
                               "in vec3 fNormal;\n"
                               "in vec3 surfacePos;\n"
                               "in vec3 dist;\n"
                               "uniform vec3 ambient;\n"
                               "uniform vec3 lightColor;\n"
                               "uniform vec3 lightPosition;\n"
                               "uniform vec3 halfVector;// surface orientation of shiniest spot\n"
                               "uniform float shininess; // exponent of sharping highlights\n"
                               "uniform float strength; //extra factor to adjust shininess \n"
                               "uniform float enableWireframe;\n"
                               "uniform vec3 WIRE_COL;\n"
                               "uniform vec3 FILL_COL;\n" 
			                   "uniform vec3 pickedPosition;"
			                   "uniform float highlightRadius;"
                               "void main()\n"
                               "{\n"
                               " float nearD = min( min( dist[0] , dist[ 1 ] ) , dist[ 2 ] );\n"
                               " float I = exp2( -2.0 * nearD * nearD );\n"
			                   " vec3 highlightColor = vec3(0,1,0);"
                               " vec3 fragColor;\n"
                               " if( enableWireframe > 0.5 )\n"
                               " {\n"
                               "   fragColor = I * WIRE_COL + (1.0 - I) * FILL_COL;//\n"
                               " }\n"
                               " else\n"
                               " {\n"
                               "   fragColor = FILL_COL;\n"
                               " }\n"
			                   "float dist = length( pickedPosition - vertex.xyz);\n"
			                   "if ( dist < highlightRadius )\n"
			                   "{ \n"
							   "   fragColor = highlightColor;\n" 
							   "} \n"
                               " vec3 surfaceToLight = normalize( lightPosition - surfacePos );\n"
			                   " float brightness = abs(dot( tNormal, surfaceToLight) / (length(surfaceToLight) * length( tNormal ) ));\n"
                               " if( brightness > 0 )\n"
                               " {\n" 
                               "   brightness = clamp( brightness , 0 , 1 );\n"
                               "   vec3 scatteredLight = lightColor * brightness;\n"
                               "   vec3 rgb = min( fragColor * scatteredLight  , vec3( 1.0 ) );  //+ reflectedLight\n"
                               "   color.xyz = rgb;\n"
                               " }\n"
                               " else\n"
                               " {\n"
                               "   color.xyz = fragColor;\n"
                               " }\n"
			                   "color.w = 0.5;"
			"}";
		
		
		mVertWONSrcs = "#version 450\n"
                              "layout (location=0) in vec3 position;\n"
                              "layout (location=1) in vec3 vertexColor;\n"
                              "out vec4 vPosition;\n"
                              "void main()\n"
                              "{\n"
                              "vPosition = vec4( position , 1 );\n"
			"}\n";
		mGeomWONSrcs = "#version 450\n"
		"layout(triangles) in;\n"
		"layout(triangle_strip, max_vertices = 3) out;\n"
		"uniform mat4 mvMatrix;\n"
		"uniform mat4 mvpMatrix;\n"
		"uniform mat3 normalMatrix;\n"
		"uniform vec2 WIN_SCALE;\n"
		"in vec4[ 3 ] vPosition;\n"
		"out vec3 dist;\n"
		"out vec4 vertex;\n" 
                "out vec3 surfacePos;\n"
                "out vec3 tNormal;\n"
                "void main()\n"
                "{\n"
                "vec4 v0 = mvpMatrix * vPosition[0];\n"
                "vec4 v1 = mvpMatrix * vPosition[1];\n"
                "vec4 v2 = mvpMatrix * vPosition[2];\n"
                "vec4 sp0 = mvMatrix * vPosition[0];\n"
                "vec4 sp1 = mvMatrix * vPosition[1];\n"
                "vec4 sp2 = mvMatrix * vPosition[2];\n"
	        "vec3 vector1 = ( sp1.xyz - sp0.xyz );\n"
	        "vec3 vector2 = ( sp2.xyz - sp1.xyz );\n"
                "vec3 n = normalize( cross( vector1 , vector2 ) );\n"
                "vec2 p0 = WIN_SCALE * v0.xy / v0.w;\n"
                "vec2 p1 = WIN_SCALE * v1.xy / v1.w;\n"
                "vec2 p2 = WIN_SCALE * v2.xy / v2.w;\n"
                "vec2 vect0 = p2 - p1;\n"
                "vec2 vect1 = p2 - p0;\n"
                "vec2 vect2 = p1 - p0;\n"
                "float area = abs( vect1.x * vect2.y - vect1.y * vect2.x);\n"
                "dist = vec3( area / length( vect0 ) , 0 , 0 );\n"
                "vertex = vPosition[0];\n"
                "gl_Position = v0;\n"
                "tNormal = n;\n"
                "surfacePos = vec3( sp0 );\n"
                "EmitVertex();\n"
                "dist = vec3( 0 , area / length( vect1 ) , 0 );\n"
                "vertex = vPosition[1];\n"
                "gl_Position = v1;\n"
                "tNormal = n;\n"
	        "surfacePos = vec3( sp1 );\n"
                "EmitVertex();\n"
                "dist = vec3( 0 , 0 , area / length( vect2 ) );\n"
                "vertex = vPosition[2];\n"
                "gl_Position = v2;\n"
                "tNormal = n;\n"
	        "surfacePos = vec3( sp2 );\n"
                "EmitVertex();\n"
                "EndPrimitive();\n"
                "}\n";
		
		
		mFragWONSrcs = "#version 450\n"
                               "layout (location = 0) out vec4 color;\n"
                               "in vec4 vertex;\n"
                               "in vec3 tNormal;\n"
                               "in vec3 surfacePos;\n"
                               "in vec3 dist;\n"
                               "uniform vec3 ambient;\n"
                               "uniform vec3 lightColor;\n"
                               "uniform vec3 lightPosition;\n"
                               "uniform vec3 halfVector;// surface orientation of shiniest spot\n"
                               "uniform float shininess; // exponent of sharping highlights\n"
                               "uniform float strength; //extra factor to adjust shininess \n"
                               "uniform float enableWireframe;\n"
                               "uniform vec3 WIRE_COL;\n"
                               "uniform vec3 FILL_COL;\n" 
			                   "uniform vec3 pickedPosition;"
			                   "uniform float highlightRadius;"
                               "void main()\n"
                               "{\n"
                               " float nearD = min( min( dist[0] , dist[ 1 ] ) , dist[ 2 ] );\n"
                               " float I = exp2( -2.0 * nearD * nearD );\n"
			                   " vec3 highlightColor = vec3(0,1,0);"
                               " vec3 fragColor;\n"
                               " if( enableWireframe > 0.5 )\n"
                               " {\n"
                               "   fragColor = I * WIRE_COL + (1.0 - I) * FILL_COL;//\n"
                               " }\n"
                               " else\n"
                               " {\n"
                               "   fragColor = FILL_COL;\n"
                               " }\n"
			                   "float dist = length( pickedPosition - vertex.xyz);\n"
			                   "if ( dist < highlightRadius )\n"
			                   "{ \n"
							   "   fragColor = highlightColor;\n" 
							   "} \n"
                               " vec3 surfaceToLight = normalize( lightPosition - surfacePos );\n"
			                   " float brightness = abs(dot( tNormal, surfaceToLight) / (length(surfaceToLight) * length( tNormal ) ));\n"
                               " if( brightness > 0 )\n"
                               " {\n" 
                               "   brightness = clamp( brightness , 0 , 1 );\n"
                               "   vec3 scatteredLight = lightColor * brightness;\n"
                               "   vec3 rgb = min( fragColor * scatteredLight  , vec3( 1.0 ) );  //+ reflectedLight\n"
                               "   color.xyz = rgb;\n"
                               " }\n"
                               " else\n"
                               " {\n"
                               "   color.xyz = fragColor;\n"
                               " }\n"
			                   "color.w = 1.0;"
                               "}";
		
		
		mPointCloudVertSrcs = "#version 450\n"
                                      "layout (location=0) in vec3 position;\n"
                                      "layout (location=1) in vec3 color;\n"
                                      "uniform mat4 mvpMatrix;\n"
                                      "out vec3 vColor;\n"
                                      "void main()\n"
                                      "{\n"
                                      "gl_Position = mvpMatrix * vec4( position , 1 );\n"
                                      "vColor = color;\n"
                                      "}\n";
		
		mPointCloudFragSrcs = "#version 450\n"
                                      "in vec3 vColor;\n"
                                      "layout (location=0) out vec3 outColor;\n"
                                      "void main()\n"
                                      "{\n"
                                      "outColor = vColor;\n"
                                      "}";



		mArrowVertSrcs = mVertWONSrcs;
		mArrowGeomSrcs = mGeomWONSrcs;
		mArrowFragSrcs = mFragWONSrcs;
		
	}


	void MeshViewerWidget::setMeshData( const std::vector< Eigen::Vector3f > &vertices , const std::vector< Eigen::Vector3f > &normals ,
					    const std::vector< unsigned int > &indices )
	{

		mMesh->loadMesh( vertices , normals , indices , true );

		mMeshDataLoaded = true;

		mMeshWithNormals = true;

		mVolume = 0;

	}


	void MeshViewerWidget::setMeshData( const std::vector< Eigen::Vector3f > &vertices , const std::vector< unsigned int > &indices )
	{

	   mMesh->loadMesh( vertices , indices , true );

	   std::vector< Eigen::Vector3f > arrowVertices;
	   std::vector< unsigned int > arrowFaces;

	   mArrowLength = 0;

	   unsigned int numFaces = indices.size()/3;

	   for (int ff = 0; ff < numFaces; ff++)
	   {
		   int id1 = indices[3 * ff ];
		   int id2 = indices[3 * ff + 1];
		   int id3 = indices[3 * ff + 2];

		   mArrowLength += (vertices[id1] - vertices[id2]).norm() + (vertices[id2] - vertices[id3]).norm() + (vertices[id3] - vertices[id1]).norm();
	   }

	   mArrowLength /= (numFaces);

	   Eigen::Vector3f defaultDir(1, 0, 0);

	   createArrow(vertices[0], defaultDir, mArrowLength, 0.2 * mArrowLength, 0.3 * mArrowLength, 10, arrowVertices, arrowFaces);

	   mArrow->loadMesh(arrowVertices, arrowFaces);

	   mVisualizeArrow = true;

	   mMeshDataLoaded = true;

	   mMeshWithNormals = false;

	   mVolume = 0;
	}


	void MeshViewerWidget::setMeshData(const std::vector< Eigen::Vector3f > &vertices, const std::vector< unsigned int > &indices, imt::volume::VolumeInfo* volume)
	{


		mMesh->loadMesh(vertices, indices, true);

		std::vector< Eigen::Vector3f > arrowVertices;
		std::vector< unsigned int > arrowFaces;

		mArrowLength = 0;

		unsigned int numFaces = indices.size() / 3;

		for (int ff = 0; ff < numFaces; ff++)
		{
			int id1 = indices[3 * ff];
			int id2 = indices[3 * ff + 1];
			int id3 = indices[3 * ff + 2];

			mArrowLength += (vertices[id1] - vertices[id2]).norm() + (vertices[id2] - vertices[id3]).norm() + (vertices[id3] - vertices[id1]).norm();
		}

		mArrowLength /= (numFaces);

		Eigen::Vector3f defaultDir(1, 0, 0);

		createArrow(vertices[0], defaultDir, mArrowLength, 0.2 * mArrowLength, 0.3 * mArrowLength, 10, arrowVertices, arrowFaces);

		mArrow->loadMesh(arrowVertices, arrowFaces);

		mVisualizeArrow = true;

		mMeshDataLoaded = true;

		mMeshWithNormals = false;

		mVolume = volume;

	}
	
	void MeshViewerWidget::setMeshData( const std::vector< Eigen::Vector3f > &vertices, const std::vector< Eigen::Vector3f > &normals,
		                                const std::vector< unsigned int > &indices, imt::volume::VolumeInfo* volume )
	{


		mMesh->loadMesh(vertices, normals, indices, true);

		std::vector< Eigen::Vector3f > arrowVertices;
		std::vector< unsigned int > arrowFaces;

		mArrowLength = 0;

		unsigned int numFaces = indices.size() / 3;

		for (int ff = 0; ff < numFaces; ff++)
		{
			int id1 = indices[3 * ff];
			int id2 = indices[3 * ff + 1];
			int id3 = indices[3 * ff + 2];

			mArrowLength += (vertices[id1] - vertices[id2]).norm() + (vertices[id2] - vertices[id3]).norm() + (vertices[id3] - vertices[id1]).norm();
		}

		mArrowLength /= (numFaces);

		Eigen::Vector3f defaultDir(1, 0, 0);

		createArrow(vertices[0], defaultDir, mArrowLength, 0.2 * mArrowLength, 0.3 * mArrowLength, 10, arrowVertices, arrowFaces);

		mArrow->loadMesh(arrowVertices, arrowFaces);

		mVisualizeArrow = true;

		mMeshDataLoaded = true;

		mMeshWithNormals = false;

		mVolume = volume;

	}




        void MeshViewerWidget::setPointCloudData(const std::vector< Eigen::Vector3f >& vertices, const std::vector< Eigen::Vector3f >& colors)
        {
           //mPointSet->setData( vertices  , colors );
	   
	       mViewMode = vc::MeshViewerWidget::POINT_CLOUD;
	   
	       mMeshDataLoaded = true;
	   
	       mMeshWithNormals = false;
        }


	void MeshViewerWidget::initializeGL()
	{
	  
		initializeOpenGLFunctions();
		


//         mMesh->init();
        //mPointSet->init();

        if( mViewMode == POINT_CLOUD )
        {
            //mPointCloudProgram = mPointSet->addShaders( mPointCloudVertSrcs , mPointCloudFragSrcs );

            mShaderInitialized = true;
        }

		qDebug() << " OpenGL Version : " << QGLFormat::openGLVersionFlags() << endl;

		qDebug() << "Context valid: " << context()->isValid();

		QString versionString( QLatin1String(reinterpret_cast< const char* >( glGetString( GL_VERSION ) )) );

		qDebug() << "Driver Version String : "<< versionString << endl;

		setMouseTracking( true );

		GL_CHECK( glClearColor(0.1f, 0.1f, 0.1f, 1.0f) );
		GL_CHECK( glEnable(GL_DEPTH_TEST) );

		//mMesh->init();

	}


	void MeshViewerWidget::resizeGL( int w , int h )
	{
		GL_CHECK( glViewport( 0 , 0 , w , h ) );

		mCamera->setViewPortDimension( w , h );

	}

	void MeshViewerWidget::paintGL()
	{

// 		if( mMesh )
// 		{
		glEnable(GL_BLEND); 
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		
		if( mMeshDataLoaded && !mDataLoadedToOpenGL )
			{

 				mMesh->init();

 				mMesh->loadGeometry(); 

				mArrow->init();

				mArrow->loadGeometry();
				
// 				mPointSet->init();

				Eigen::Vector3f objectCenter = mMesh->getObjectCenter();

				mCamera->setObjectCenter( QVector3D( objectCenter( 0 ) , objectCenter( 1 ) , objectCenter( 2 ) ) );
				mCamera->setRadius(mMesh->getRadius() );

				mCamera->setViewPortDimension( width() , height() );

				mCamera->init();
				
				//mMesh->setCameraToInitialized();

				mDataLoadedToOpenGL = true;
			}

 
			if (!mShaderInitialized)
				{

					if (mMeshWithNormals)
					{
						mShaderSetId = mMesh->addShaders(mVertWNSrcs, mGeomWNSrcs, mFragWONSrcs);
					}
					else
					{
						mShaderSetId = mMesh->addShaders(mVertWONSrcs, mGeomWONSrcs, mFragWONSrcs);
					}

					mShaderInitialized = mMesh->activateShader(mShaderSetId);

					int shaderId = mArrow->addShaders(mVertWONSrcs, mGeomWONSrcs, mFragWONSrcs);
					
					mArrow->activateShader(shaderId);
				}

			GL_CHECK(  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) );

			mCamera->setViewPortDimension( width() , height() );
			
			if( mViewMode == vc::MeshViewerWidget::POINT_CLOUD )
			{
			  //mPointSet->render(); 
			}
 			else
 			{
 			  mMesh->render();

			  if (mVisualizeArrow)
			  {
				  mArrow->loadGeometry();
				  mArrow->render();
			  }
 			}
// 		}

	}

	void MeshViewerWidget::keyPressEvent( QKeyEvent* e )
	{
		if( e->key() == Qt::Key_Escape )
		{
			close();
		}
		
		if( e->key() == Qt::Key_W )
		{
		  
		  mMesh->enableWireframe();

		  update();
		}
		
		if( e->key() == Qt::Key_S )
		{
		  
		  mMesh->disableWireframe();

		  update();
		}
		
	}

	void MeshViewerWidget::mousePressEvent( QMouseEvent *event )
	{
		mCamera->registerMousePress( event->pos() , event->buttons() );

		//qDebug()<<" start tracking "<<endl;

		if (event->buttons() == Qt::LeftButton)
		{
			Eigen::Vector3f pickPos, pickDir;

			int triangleId = -1;

			if (mMesh->pickMesh(pickPos, pickDir , triangleId))
			{

				Eigen::Vector3f gradient;
				imt::volume::VolumeUtility::volumeGradient(pickPos,*mVolume, gradient);
				
				std::vector< Eigen::Vector3f > arrowVertices;
				std::vector< unsigned int > arrowFaces;

				gradient.normalize();

				int id1 = mMesh->mIndices[3 * triangleId];
				int id2 = mMesh->mIndices[3 * triangleId + 1];
				int id3 = mMesh->mIndices[3 * triangleId + 2];

				Eigen::Vector3f averageNormal = mMesh->mVertexData[id1].mVertexNormals + mMesh->mVertexData[id2].mVertexNormals + mMesh->mVertexData[id3].mVertexNormals;

				averageNormal.normalize();

		
				if (gradient.dot(pickDir) < 0)
					gradient *= -1;

				createArrow(pickPos, averageNormal , mArrowLength, 0.2 * mArrowLength, 0.3 * mArrowLength, 10, arrowVertices, arrowFaces);

				mArrow->loadMesh(arrowVertices, arrowFaces);

				mMesh->setPickPosition(pickPos);
				
				if (mVolume)
				{
					float highlightRadius = mVolume->mVoxelStep(0) * 10;

					mMesh->setHighlightCircleRadius(highlightRadius);
				}
		


			}


		}

		
		update();
	}

	void MeshViewerWidget::mouseReleaseEvent( QMouseEvent *event )
	{
		mCamera->registerMouseRelease( event->pos() );
	}

	void MeshViewerWidget::mouseMoveEvent( QMouseEvent *event )
	{ 

		Qt::MouseButtons buttons = event->buttons();

		mCamera->registerMouseMove( event->pos() , event->buttons() );

		// qDebug() << " register mouse move " << event->pos() << endl;
		update();
	}

	void	MeshViewerWidget::wheelEvent ( QWheelEvent * event )
	{

		mCamera->registerMouseWheel( event->delta() );

		QWidget::wheelEvent(event);

		update();
	}


	MeshViewerWidget::~MeshViewerWidget()
	{
		delete mMesh;
		//delete mShaderSourceManager;
		delete mCamera;

// 		GL_CHECK( glDeleteVertexArrays( 2 , mVAO ) );
	}


	bool MeshViewerWidget::pickMesh(Eigen::Vector3f& pickedPoint)
	{
		bool pickSuccessfull = false;


		return pickSuccessfull;

	}



	void createArrow(const Eigen::Vector3f& origin, const Eigen::Vector3f dir, float length, float shaftRadius,
		float arrowRadius, int shaftResolution, std::vector< Eigen::Vector3f >& points, std::vector< unsigned int >& faceIndices)
	{

		float shaftLength = 0.7 * length;
		float pointerLength = 0.3 * length;


		Eigen::Vector3f xAxis, yAxis;

		Eigen::Vector3f mostDivergedAxis;

		if (std::abs(dir(0)) < std::abs(dir(1)) && std::abs(dir(0)) < std::abs(dir(2)))
		{
			mostDivergedAxis = Eigen::Vector3f(1, 0, 0);
		}
		else if (std::abs(dir(1)) < std::abs(dir(2)))
		{
			mostDivergedAxis = Eigen::Vector3f(0, 1, 0);
		}
		else
		{
			mostDivergedAxis = Eigen::Vector3f(0, 0, 1);
		}

		xAxis = dir.cross(mostDivergedAxis);

		xAxis.normalize();

		yAxis = dir.cross(xAxis);

		yAxis.normalize();

		float angleStep = 2 * M_PI / shaftResolution;

		points.push_back(origin);

		//bottom base 
		for (int ss = 0; ss < shaftResolution; ss++)
		{
			float currentAngle = angleStep * ss;

			Eigen::Vector3f currentPos = origin + (xAxis * cos(currentAngle) + yAxis * sin(currentAngle)) * shaftRadius;

			points.push_back(currentPos);

			if (ss > 0)
			{
				faceIndices.push_back(0);
				faceIndices.push_back(ss);
				faceIndices.push_back(ss + 1);
			}

		}

		faceIndices.push_back(0);
		faceIndices.push_back(shaftResolution);
		faceIndices.push_back(1);


		int id = 1;
		int id2 = 1 + shaftResolution;

		//upper base
		for (int ss = 0; ss < shaftResolution; ss++)
		{
			float currentAngle = angleStep * ss;

			Eigen::Vector3f currentPos = origin + dir * shaftLength + (xAxis * cos(currentAngle) + yAxis * sin(currentAngle)) * shaftRadius;

			points.push_back(currentPos);

			//id , id + 1 , id2 + 1 , id2
			if (ss < shaftResolution - 1)
			{
				faceIndices.push_back(id);
				faceIndices.push_back(id + 1);
				faceIndices.push_back(id2 + 1);

				faceIndices.push_back(id);
				faceIndices.push_back(id2 + 1);
				faceIndices.push_back(id2);
			}

			id++;
			id2++;
		}

		faceIndices.push_back(shaftResolution);
		faceIndices.push_back(1);
		faceIndices.push_back(shaftResolution + 1);

		faceIndices.push_back(shaftResolution);
		faceIndices.push_back(shaftResolution + 1);
		faceIndices.push_back(2 * shaftResolution);



		id = 1 + shaftResolution;
		id2 = 1 + 2 * shaftResolution;

		//arrow base

		for (int ss = 0; ss < shaftResolution; ss++)
		{
			float currentAngle = angleStep * ss;

			Eigen::Vector3f currentPos = origin + dir * shaftLength + (xAxis * cos(currentAngle) + yAxis * sin(currentAngle)) * arrowRadius;

			points.push_back(currentPos);

			if (ss < shaftResolution - 1)
			{
				faceIndices.push_back(id);
				faceIndices.push_back(id + 1);
				faceIndices.push_back(id2 + 1);

				faceIndices.push_back(id);
				faceIndices.push_back(id2 + 1);
				faceIndices.push_back(id2);
			}

			id++;
			id2++;

		}

		faceIndices.push_back(2 * shaftResolution);
		faceIndices.push_back(1 + shaftResolution);
		faceIndices.push_back(2 * shaftResolution + 1);

		faceIndices.push_back(2 * shaftResolution);
		faceIndices.push_back(2 * shaftResolution + 1);
		faceIndices.push_back(3 * shaftResolution);

		//arrow tip
		Eigen::Vector3f arrowTip = origin + dir * length;

		points.push_back(arrowTip);

		unsigned int arrowTipId = points.size() - 1;


		for (int ss = 0; ss < shaftResolution; ss++)
		{

			if (ss > 0)
			{
				faceIndices.push_back(arrowTipId);
				faceIndices.push_back(ss + 2 * shaftResolution);
				faceIndices.push_back(ss + 2 * shaftResolution + 1);

			}

		}

		faceIndices.push_back(arrowTipId);
		faceIndices.push_back(3 * shaftResolution);
		faceIndices.push_back(2 * shaftResolution + 1);

	}



}
