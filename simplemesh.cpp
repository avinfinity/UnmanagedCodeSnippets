


#include "simplemesh.h"
//#include "TinyObj/tiny_obj_loader.h"
#include "openglhelper.h"
#include "iostream"



namespace vc{
  
  
	GLuint SimpleMesh::compileShaders( GLenum shaderType , const char *shaderSource )
	{
	   GLuint shader = glCreateShader( shaderType );

	   GL_CHECK( glShaderSource( shader , 1 , &shaderSource , NULL) );
         GL_CHECK( glCompileShader( shader ) );

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);


    if( status == GL_FALSE ) 
	{
        GLchar emsg[10024];
        glGetShaderInfoLog(shader, sizeof(emsg), 0, emsg);
        //fprintf(stderr, "Error compiling GLSL shader (%s): %s\n" , emsg );
		std::cout << "Error compiling GLSL shader : " << emsg << std::endl;
		//fprintf(stderr, "Section: %s\n", sdefine);
        //fprintf(stderr, "Defines: %s\n", define);
        //fprintf(stderr, "Source: %s\n", sources[2]); 

        QString errMsg = QString( "Error compiling GLSL shader (%s): %s\n" ) + emsg;

       qDebug()<<" error in compiling shaders , file : "<< __FILE__<<" : line : "<<__LINE__<< errMsg << endl;
 
        //exit(0);
    }

    return shader;
	
	}  
  

int SimpleMesh::addShaders( QString vertexShaderSrc , QString geometryShaderSrc , QString fragmentShaderSrc)
{
    ShaderSource shaderSource;

    shaderSource.mVertexShaderSrc = vertexShaderSrc;
    shaderSource.mFragmentShaderSrc = fragmentShaderSrc;
    shaderSource.mGeometryShaderSrc = geometryShaderSrc;

    shaderSource.mHasGeometryShaderSrc = true;

    mShaderSources.push_back( shaderSource );

    GLuint vertexShader = compileShaders( GL_VERTEX_SHADER , vertexShaderSrc.toStdString().c_str() );
    GLuint geometryShader = compileShaders( GL_GEOMETRY_SHADER, geometryShaderSrc.toStdString().c_str() );
    GLuint fragmentShader = compileShaders( GL_FRAGMENT_SHADER, fragmentShaderSrc.toStdString().c_str() );


    GLuint program = glCreateProgram();

    GL_CHECK( glAttachShader( program , vertexShader ) );
    GL_CHECK( glAttachShader( program , geometryShader) );
    GL_CHECK( glAttachShader( program , fragmentShader) );

    mShaderPrograms.push_back( program );

	GL_CHECK(glLinkProgram(program));

    return ( mShaderPrograms.size() - 1 );

}



int SimpleMesh::addShaders( QString vertexShaderSrc , QString fragmentShaderSrc )
{

    ShaderSource shaderSource;

    shaderSource.mVertexShaderSrc = vertexShaderSrc;
    shaderSource.mFragmentShaderSrc = fragmentShaderSrc;

    shaderSource.mHasGeometryShaderSrc = false;

    mShaderSources.push_back( shaderSource );

    GLuint vertexShader = compileShaders( GL_VERTEX_SHADER , vertexShaderSrc.toStdString().c_str() );
    GLuint fragmentShader = compileShaders( GL_FRAGMENT_SHADER, fragmentShaderSrc.toStdString().c_str() );


    GLuint program = glCreateProgram();

    GL_CHECK( glAttachShader( program , vertexShader ) );
    GL_CHECK( glAttachShader( program , fragmentShader) );

    mShaderPrograms.push_back( program );

    //qDebug() <<" shaders compiled "<<endl;

    return ( mShaderPrograms.size() - 1 );
}




//void SimpleMesh::loadMeshFromOBJFile( QString objFilePath )
//{
//
//    std::vector< tinyobj::shape_t > shape;
//    
//    tinyobj::LoadObj( shape , objFilePath.toStdString().c_str() );
//    
//    tinyobj::shape_t &shape0 = shape[ 0 ];
//    
//    std::vector< Eigen::Vector3f > vertices , normals;
//    std::vector< unsigned int > indices;
//    
//    int numNormals = shape0.mesh.normals.size() / 3;
//    int numVerts = shape0.mesh.positions.size() / 3;
//    int numIndices = shape0.mesh.indices.size();
//    
//    vertices.resize(numVerts);
//    indices.resize(numIndices);
//    
//    memcpy( vertices.data() , shape0.mesh.positions.data() , 3 * numVerts * sizeof( float ) );
//
//    memcpy( indices.data() , shape0.mesh.indices.data() , shape0.mesh.indices.size() * sizeof( unsigned int ) );
//    
//    if( numNormals == numVerts )
//    {
//        normals.resize( numNormals );
//
//        memcpy( normals.data() , shape0.mesh.normals.data() , 3 * numNormals * sizeof( float ) );
//
//        loadMesh( vertices , normals , indices );
//    }
//    else
//    {
//        loadMesh( vertices , indices );
//    }
//
//}


void SimpleMesh::loadMesh( const std::vector< Eigen::Vector3f > &vertices , const std::vector< Eigen::Vector3f > &normals , const std::vector< unsigned int > &indices , bool buildTracer)
{

    int numVertices = vertices.size();
    int numNormals = normals.size();
    int numTriangles = indices.size() / 3;


    float xMax = std::numeric_limits<float>::min() , xMin = std::numeric_limits<float>::max();
    float yMax = std::numeric_limits<float>::min() , yMin = std::numeric_limits<float>::max();
    float zMax = std::numeric_limits<float>::min() , zMin = std::numeric_limits<float>::max();

    mIndices = indices;

    mVertexData.resize( numVertices );

    if( numVertices == numNormals )
    {
        mVertexNormalPresent = true;
    }

    for( int ii = 0; ii < numVertices ; ii++ )
    {
        xMax = std::max( xMax , vertices[ ii ]( 0 ) );
                xMin = std::min( xMin , vertices[ ii ]( 0 ) );

                yMax = std::max( yMax , vertices[ ii ]( 1 ) );
                yMin = std::min( yMin , vertices[ ii ]( 1 ) );

                zMax = std::max( zMax , vertices[ ii ]( 2 ) );
                zMin = std::min( zMin , vertices[ ii ]( 2 ) );

                mVertexData[ ii ].mCoords = vertices[ ii ];

        if( numVertices == numNormals  )
        {
            mVertexData[ ii ].mVertexNormals = normals[ ii ];
        }

    }

    mRadius = ( xMax - xMin ) * ( xMax - xMin ) + ( yMax - yMin ) * ( yMax - yMin ) + ( zMax - zMin ) * ( zMax - zMin ) ;

    mRadius = sqrt( mRadius ) / 2;

    mObjectCenter( 0 ) = ( xMax + xMin ) / 2;
    mObjectCenter( 1 ) = ( yMax + yMin ) / 2;
    mObjectCenter( 2 ) = ( zMax + zMin ) / 2;

    mCage.resize( 8 );

    mCage[ 0 ] = Eigen::Vector3f( xMin , yMin , zMin );
    mCage[ 1 ] = Eigen::Vector3f( xMax , yMin , zMin );
    mCage[ 2 ] = Eigen::Vector3f( xMax , yMax , zMin );
    mCage[ 3 ] = Eigen::Vector3f( xMin , yMax , zMin );

    mCage[ 4 ] = Eigen::Vector3f( xMin , yMin , zMax );
    mCage[ 5 ] = Eigen::Vector3f( xMax , yMin , zMax );
    mCage[ 6 ] = Eigen::Vector3f( xMax , yMax , zMax );
    mCage[ 7 ] = Eigen::Vector3f( xMin , yMax , zMax );

	if (buildTracer)
	initTracer();
}


void SimpleMesh::initTracer()
{


    int numVertices = mVertexData.size();
	int numTriangles = mIndices.size() / 3;


	_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
	_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

	rtcInit(NULL);

	scene = 0;

#if 1

	device = rtcNewDevice(NULL);

	scene = rtcDeviceNewScene(device, RTC_SCENE_STATIC, RTC_INTERSECT1);

	geomID = rtcNewTriangleMesh(scene, RTC_GEOMETRY_STATIC, numTriangles, numVertices, 1);

	EmbreeTriangle* triangles = (EmbreeTriangle*)rtcMapBuffer(scene, geomID, RTC_INDEX_BUFFER);

	rtcUnmapBuffer(scene, geomID, RTC_INDEX_BUFFER);

	EmbreeVertex* evertices = (EmbreeVertex*)rtcMapBuffer(scene, geomID, RTC_VERTEX_BUFFER);

	for (int tt = 0; tt < numTriangles; tt++)
	{
		triangles[tt].v0 = mIndices[3 * tt];
		triangles[tt].v1 = mIndices[3 * tt + 1];
		triangles[tt].v2 = mIndices[3 * tt + 2];
	}

	for (int pp = 0; pp < numVertices; pp++)
	{
		evertices[pp].x = mVertexData[pp].mCoords(0);
		evertices[pp].y = mVertexData[pp].mCoords(1);
		evertices[pp].z = mVertexData[pp].mCoords(2);
		evertices[pp].a = 1.0;
	}

	rtcUnmapBuffer(scene, geomID, RTC_VERTEX_BUFFER);

	rtcCommit(scene);

#endif

}

void SimpleMesh::loadMesh( const std::vector< Eigen::Vector3f > &vertices ,  const  std::vector< unsigned int > &indices, bool buildTracer)
{

    int numVertices = vertices.size();
    int numTriangles = indices.size() / 3;


    float xMax = std::numeric_limits<float>::min() , xMin = std::numeric_limits<float>::max();
    float yMax = std::numeric_limits<float>::min() , yMin = std::numeric_limits<float>::max();
    float zMax = std::numeric_limits<float>::min() , zMin = std::numeric_limits<float>::max();

    mIndices = indices;

    mVertexData.resize( numVertices );

    mVertexNormalPresent = false;

    for( int ii = 0; ii < numVertices ; ii++ )
    {
        xMax = std::max( xMax , vertices[ ii ]( 0 ) );
                xMin = std::min( xMin , vertices[ ii ]( 0 ) );

                yMax = std::max( yMax , vertices[ ii ]( 1 ) );
                yMin = std::min( yMin , vertices[ ii ]( 1 ) );

                zMax = std::max( zMax , vertices[ ii ]( 2 ) );
                zMin = std::min( zMin , vertices[ ii ]( 2 ) );

                mVertexData[ ii ].mCoords = vertices[ ii ];

				//mVertexData[ii].mVertexNormals = Eigen::Vector3f(1, 1, 1);

    }

    mRadius = ( xMax - xMin ) * ( xMax - xMin ) + ( yMax - yMin ) * ( yMax - yMin ) + ( zMax - zMin ) * ( zMax - zMin ) ;

    mRadius = sqrt( mRadius ) / 2;

    mObjectCenter( 0 ) = ( xMax + xMin ) / 2;
    mObjectCenter( 1 ) = ( yMax + yMin ) / 2;
    mObjectCenter( 2 ) = ( zMax + zMin ) / 2;

    mCage.resize( 8 );

    mCage[ 0 ] = Eigen::Vector3f( xMin , yMin , zMin );
    mCage[ 1 ] = Eigen::Vector3f( xMax , yMin , zMin );
    mCage[ 2 ] = Eigen::Vector3f( xMax , yMax , zMin );
    mCage[ 3 ] = Eigen::Vector3f( xMin , yMax , zMin );

    mCage[ 4 ] = Eigen::Vector3f( xMin , yMin , zMax );
    mCage[ 5 ] = Eigen::Vector3f( xMax , yMin , zMax );
    mCage[ 6 ] = Eigen::Vector3f( xMax , yMax , zMax );
    mCage[ 7 ] = Eigen::Vector3f( xMin , yMax , zMax );

	if( buildTracer )
	initTracer();
}



bool SimpleMesh::activateShader( int shaderId )
{
    int program = mShaderPrograms[ shaderId ];

    //GL_CHECK( glBindBuffer(GL_ARRAY_BUFFER, mVBO[0]) );
    //GL_CHECK( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVBO[1] ) );

    if( mVertexNormalPresent )
    {
        GL_CHECK( glBindAttribLocation( program , 0 , "position") );
        GL_CHECK( glBindAttribLocation( program , 1 , "normal") );
        GL_CHECK( glBindAttribLocation( program , 2 , "texCoords") );
    }
    else
    {
        GL_CHECK( glBindAttribLocation( program , 0 , "position") );
        GL_CHECK( glBindAttribLocation( program , 1 , "texCoords") );
    }

    GL_CHECK( glLinkProgram( program ) );

    //     GL_CHECK( glBindProgram( program ) );

    GLint status;


    GL_CHECK( glGetProgramiv( program, GL_LINK_STATUS, &status ) );

    if( status == GL_FALSE )
    {
        GLchar emsg[1024];
        GL_CHECK( glGetProgramInfoLog( program , sizeof( emsg ) , 0 , emsg ) );
        fprintf(stderr, "Error linking GLSL program : %s\n", emsg );
        return false;
    }

    mCurrentProgram = program;

    return true;

}


bool SimpleMesh::pickMesh( Eigen::Vector3f& pickedPosition , Eigen::Vector3f& pickedDir )
{
#if 1
	Eigen::Vector3f pos;
	Eigen::Vector3f dir;

	mCamera->getRay(pos, dir);

	RTCRay ray;

	ray.org[0] = pos(0);
	ray.org[1] = pos(1);
	ray.org[2] = pos(2);

	ray.dir[0] = dir(0);
	ray.dir[1] = dir(1);
	ray.dir[2] = dir(2);

	ray.geomID = RTC_INVALID_GEOMETRY_ID;
	ray.instID = RTC_INVALID_GEOMETRY_ID;
	ray.primID = RTC_INVALID_GEOMETRY_ID;

	ray.tnear = 0;
	ray.tfar = FLT_MAX;

	ray.mask = 0xFFFFFFFF;
	ray.time = 0.f;

	rtcIntersect(scene, ray);

	if (ray.primID != RTC_INVALID_GEOMETRY_ID)
	{
		pickedPosition = pos + dir * ray.tfar;

		pickedDir = -dir;

		return true;
	}

#endif

	return false;
}


bool SimpleMesh::pickMesh( Eigen::Vector3f& pickedPosition, Eigen::Vector3f& pickedDir, int& triangleId)
{
#if 1
	Eigen::Vector3f pos;
	Eigen::Vector3f dir;

	mCamera->getRay(pos, dir);

	RTCRay ray;

	ray.org[0] = pos(0);
	ray.org[1] = pos(1);
	ray.org[2] = pos(2);

	ray.dir[0] = dir(0);
	ray.dir[1] = dir(1);
	ray.dir[2] = dir(2);

	ray.geomID = RTC_INVALID_GEOMETRY_ID;
	ray.instID = RTC_INVALID_GEOMETRY_ID;
	ray.primID = RTC_INVALID_GEOMETRY_ID;

	ray.tnear = 0;
	ray.tfar = FLT_MAX;

	ray.mask = 0xFFFFFFFF;
	ray.time = 0.f;

	rtcIntersect(scene, ray);

	if (ray.primID != RTC_INVALID_GEOMETRY_ID)
	{
		pickedPosition = pos + dir * ray.tfar;

		pickedDir = -dir;

		triangleId = ray.primID;

		return true;
	}
	else
	{
		triangleId = -1;
	}

#endif

	return false;
}





void SimpleMesh::setPickPosition(Eigen::Vector3f& pickedPosition)
{
	_PickedPosition = pickedPosition;
}

void SimpleMesh::setHighlightCircleRadius(float radius)
{

   _HighlightCircleRadius = radius;
}



void SimpleMesh::init()
{

// #ifdef VC_QOPENGL_FUNCTIONS
    initializeOpenGLFunctions();
// #endif
    
    std::cout<<" VAO : "<<mVAO<<std::endl;
    
    GL_CHECK( glGenVertexArrays( 1 , &mVAO ) );

    GL_CHECK( glBindVertexArray(mVAO) );

    GL_CHECK( glGenBuffers( 2 , mVBO ) );

    GL_CHECK( glGenTextures( 2 , mTextures ) );

    GL_CHECK( glBindVertexArray(0) );

    mBuffersInitialized;

    qDebug() << " buffers initialized "<< endl;

}


void SimpleMesh::loadTexture( cv::Mat& texture )
{
    if( texture.cols == 0 || texture.rows == 0 )
        return;

    if( texture.type() != CV_8UC3 )
    {
        qDebug()<<" Only thre channel 8 bit images are supported for texture "<<endl;

        return;
    }

    texture.copyTo( mTextureImage );

    mTexturePresent = true;

}


void SimpleMesh::setViewPort(int w, int h)
{
    GL_CHECK( glViewport( 0 , 0 , w , h ) );
}


void SimpleMesh::setCamera( TrackBallCamera *camera )
{
    mCamera = camera;
}

void SimpleMesh::render()
{

    GL_CHECK( glUseProgram( mCurrentProgram ) );


    GL_CHECK( glBindVertexArray(mVAO) );

    //qDebug() << " current program : "<<mCurrentProgram << endl;

    GL_CHECK( glBindBuffer(GL_ARRAY_BUFFER, mVBO[0]) );
    GL_CHECK( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVBO[1] ) );

    if( mUseTexture )
    {

        GL_CHECK( glActiveTexture( GL_TEXTURE0 ) );

        GL_CHECK( glBindTexture( GL_TEXTURE_2D , mTextures[ 0 ] ) );

        int texLoc = glGetUniformLocation( mCurrentProgram , "ReferenceTexture" );

        GL_CHECK( glUniform1i( texLoc , 0 ) );

    }


    int numIndices = mIndices.size() ;

    QMatrix4x4 mv = mCamera->getModelViewMatrix();
    QMatrix4x4 mat1 = mCamera->getModelViewProjectionMatrix();
    Eigen::Matrix3f mat2 = mCamera->getNormalTransformationMatrix();

    GLint mvpMatrix = glGetUniformLocation( mCurrentProgram , "mvpMatrix");
    GLint mvMatrix = glGetUniformLocation( mCurrentProgram , "mvMatrix");
    GLint normalMatrix = glGetUniformLocation( mCurrentProgram , "normalMatrix");
    GLint wireFrame = glGetUniformLocation( mCurrentProgram , "enableWireframe");

    GLint WIRE_COL = glGetUniformLocation( mCurrentProgram , "WIRE_COL");
    GLint FILL_COL = glGetUniformLocation( mCurrentProgram , "FILL_COL");
    GLint WIN_SCALE = glGetUniformLocation( mCurrentProgram , "WIN_SCALE");

    GLint ambient = glGetUniformLocation( mCurrentProgram , "ambient" );
    GLint lightColor = glGetUniformLocation( mCurrentProgram , "lightColor" );
    GLint lightPosition = glGetUniformLocation( mCurrentProgram , "lightPosition" );
    GLint halfVector = glGetUniformLocation( mCurrentProgram , "halfVector" );
    GLint shininess = glGetUniformLocation( mCurrentProgram , "shininess" );
    GLint strength = glGetUniformLocation( mCurrentProgram , "strength" );

	GLint pickedPositionLoc = glGetUniformLocation(mCurrentProgram, "pickedPosition");
	GLint hightlightRadiusLoc = glGetUniformLocation(mCurrentProgram, "highlightRadius");

	GL_CHECK(glUniform3f(pickedPositionLoc, _PickedPosition(0), _PickedPosition(1), _PickedPosition(2)));
	GL_CHECK(glUniform1f(hightlightRadiusLoc, _HighlightCircleRadius));


    float wScale , hScale;

    mCamera->getWinScale( wScale , hScale );

    GL_CHECK( glUniformMatrix4fv( mvMatrix , 1 , false , mv.data() ) );
    GL_CHECK( glUniformMatrix4fv( mvpMatrix , 1 , false , mat1.data() ) );
    GL_CHECK( glUniformMatrix3fv( normalMatrix , 1 , false , mat2.data() ) );
    GL_CHECK( glUniform3f( WIRE_COL , 0 , 0 , 0 ) );
    GL_CHECK( glUniform3f( FILL_COL , 1.0 , 1 , 1 ) );
    GL_CHECK( glUniform2f( WIN_SCALE , wScale , hScale  ) );
    
    if( mEnableWireframe )
    {
      GL_CHECK( glUniform1f( wireFrame , 1.0 ) );
    }
    else
    {
      GL_CHECK( glUniform1f( wireFrame , 0.0 ) );
    }

    GL_CHECK( glUniform3f( ambient , mLightingData.mLightSource[ 0 ].ambient( 0 ) ,
              mLightingData.mLightSource[ 0 ].ambient( 1 ) ,
            mLightingData.mLightSource[ 0 ].ambient( 2 ) ) );

    Eigen::Vector3f vec(  0.5,  0.2f, 1.0f );

    vec.normalize();

    GL_CHECK( glUniform3f( lightColor , 1.0 , 1.0 , 1.0 ) );
    GL_CHECK( glUniform3f( lightPosition , 0,  0, 0 ) );
    GL_CHECK( glUniform3f( halfVector , vec( 0 ),  vec( 1 ), vec( 2 ) ) );
    GL_CHECK( glUniform1f( shininess , 20.0 ) );
    GL_CHECK( glUniform1f( strength , 10.0 ) );

    //qDebug() << " win scale : "<<wScale<<" "<<hScale<< endl;
    //qDebug() << mv << endl;
    //qDebug() << mat1 << endl ;

    GL_CHECK( glDrawElements( GL_TRIANGLES , mIndices.size() , GL_UNSIGNED_INT , 0 ) );

    GL_CHECK( glBindBuffer(GL_ARRAY_BUFFER, 0 ) );
    GL_CHECK( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0 ) );

    GL_CHECK( glBindVertexArray(0) );
    GL_CHECK( glUseProgram( 0 ) );

}



void SimpleMesh::loadGeometry()
{
    GL_CHECK( glBindVertexArray(mVAO) );

    GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER , mVBO[ 0 ] ) );
    GL_CHECK( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER , mVBO[ 1 ] ) );

    //qDebug() << mVBO[ 0 ]<<" "<<mVBO[ 1 ]<<endl;

    GL_CHECK( glBufferData( GL_ARRAY_BUFFER , mVertexData.size() * sizeof( VertexData ) , mVertexData.data() , GL_STATIC_DRAW ) );
    GL_CHECK( glBufferData( GL_ELEMENT_ARRAY_BUFFER , mIndices.size() * sizeof( GLuint ) , mIndices.data() , GL_STATIC_DRAW ) );

    GL_CHECK( glEnableVertexAttribArray(0) );
    GL_CHECK( glEnableVertexAttribArray(1) );

    if( mVertexNormalPresent )
    {
        GL_CHECK( glEnableVertexAttribArray(2) );
    }

    GL_CHECK( glVertexAttribPointer( 0 , 3, GL_FLOAT, GL_FALSE, sizeof( VertexData )  , 0) );


    if( mVertexNormalPresent )
    {
        int offset = 3 * sizeof( GLfloat );

        GL_CHECK( glVertexAttribPointer( 1 , 3 , GL_FLOAT, GL_FALSE, sizeof (  VertexData  ) , ( float * )offset ) );

        offset += 3 * sizeof( GLfloat );

        GL_CHECK( glVertexAttribPointer( 2 , 2 , GL_FLOAT, GL_FALSE, sizeof (  VertexData  ) , ( float * )offset ) );
    }
    else
    {
        int offset = 6 * sizeof( GLfloat );

        GL_CHECK( glVertexAttribPointer( 1 , 2 , GL_FLOAT, GL_FALSE, sizeof (  VertexData  ) , ( float * )offset ) );
    }


    GL_CHECK( glBindBuffer( GL_ARRAY_BUFFER , 0 ) );
    GL_CHECK( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER , 0 ) );

    GL_CHECK( glBindVertexArray(0) );
}


void SimpleMesh::loadTexture()
{
    if( mTexturePresent )
    {
        GL_CHECK( glBindTexture( GL_TEXTURE_2D , mTextures[ 0 ] ) );

        //GL_CHECK( glTexStorage2D( GL_TEXTURE_2D , 1 , GL_RGB8 , mTextureImage.cols , mTextureImage.rows ) );

        GL_CHECK( glTexImage2D( GL_TEXTURE_2D ,  0 , 0 , 0  , mTextureImage.cols , mTextureImage.rows , GL_BGR , GL_UNSIGNED_BYTE, mTextureImage.data ) );

        GL_CHECK( glBindTexture( GL_TEXTURE_2D , 0 ) );
    }
}

void SimpleMesh::useTexture()
{
    mUseTexture = true;
}

void SimpleMesh::unuseTexture()
{
    mUseTexture = false;
}


SimpleMesh::SimpleMesh()
{
    mUseTexture = false;

    mBuffersInitialized = false;

    mVertexNormalPresent = false;
    
    mEnableWireframe = false;

    mLightingData.mLightSource[ 0 ].position = Eigen::Vector3f( 0.5,  0.2f, 1.0f );
    mLightingData.mLightSource[ 0 ].ambient = Eigen::Vector3f(  0.1f, 0.1f, 0.1f );
    mLightingData.mLightSource[ 0 ].diffuse = Eigen::Vector3f( 0.7f, 0.7f, 0.7f );
    mLightingData.mLightSource[ 0 ].specular = Eigen::Vector3f( 0.8f, 0.8f, 0.8f );

    mLightingData.mLightSource[ 1 ].position = Eigen::Vector3f( -0.8f, 0.4f, -1.0f );
    mLightingData.mLightSource[ 1 ].ambient = Eigen::Vector3f(  0.0f, 0.0f,  0.0f );
    mLightingData.mLightSource[ 1 ].diffuse = Eigen::Vector3f( 0.5f, 0.5f,  0.5f );
    mLightingData.mLightSource[ 1 ].specular = Eigen::Vector3f( 0.8f, 0.8f,  0.8f );

}


void SimpleMesh::enableWireframe()
{
  mEnableWireframe = true;
}

void SimpleMesh::disableWireframe()
{
  mEnableWireframe = false;
}



Eigen::Vector3f SimpleMesh::getObjectCenter()
{
    return mObjectCenter;
}

float SimpleMesh::getRadius()
{
    return mRadius;
}

void SimpleMesh::getCage( std::vector< Eigen::Vector3f > &cage )
{
    cage = mCage;
}

void SimpleMesh::setVAO( unsigned int vao )
{
  qDebug()<<" setting VAO : "<<vao<<endl;
    mVAO = vao;
}

void SimpleMesh::generateVAO()
{
  GL_CHECK( glGenVertexArrays( 1 , &mVAO )  );
}


int SimpleMesh::getNumVertices()
{
    return mVertexData.size();

}


int SimpleMesh::getNumFaces()
{
   return mIndices.size() / 3 ;
}


SimpleMesh::~SimpleMesh()
{

    if( mBuffersInitialized )
    {
        GL_CHECK( glBindVertexArray(mVAO) );
        GL_CHECK( glDeleteBuffers( 2 , mVBO ) );
        GL_CHECK( glDeleteTextures( 2 , mTextures ) );
        GL_CHECK( glBindVertexArray(0) );
    }

}

}
