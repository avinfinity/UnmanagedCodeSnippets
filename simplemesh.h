#ifndef __SIMPLE_MESH_H__
#define __SIMPLE_MESH_H__

#ifdef VC_WINDOWS
#include "windows.h"
#endif

#include "openglincludes.h"
#include "opencvincludes.h"
//#include "visualization3D/buffermanager.h"
#include <QString>
#include "eigenincludes.h"
#include "trackballcamera.h"
#include "embreeincludes.h"


namespace vc{
  
  
struct VertexData
{
  Eigen::Vector3f mCoords;
  Eigen::Vector3f mVertexNormals;
  Eigen::Vector2f mTexCoords;
  
};





  
class SimpleMesh : public QOpenGLFunctions_3_3_Core
{


public:

	struct ShaderSource{
  
  QString mVertexShaderSrc , mFragmentShaderSrc , mGeometryShaderSrc;
  
  bool mHasGeometryShaderSrc;
  
};  


	struct EmbreeVertex
	{
		float x, y, z, a;
	};

	struct EmbreeTriangle { int v0, v1, v2; };



//protected:

public:
  
  std::vector< unsigned int > mShaderPrograms;
  
  std::vector< ShaderSource > mShaderSources;
  
  std::vector< VertexData > mVertexData;
  
  std::vector< unsigned int > mIndices; 
  
  unsigned int mTextures[ 2 ] , mVBO[ 2 ] , mCurrentProgram , mVAO;
  
  bool mBuffersInitialized;
  
  cv::Mat mTextureImage;
  
  float mRadius;
  
  Eigen::Vector3f mObjectCenter;
  
  bool mTexturePresent , mVertexNormalPresent , mEnableWireframe ;

  TrackBallCamera *mCamera;

  bool mUseTexture , m;

  std::vector< Eigen::Vector3f > mCage;

  
public:


struct Lighting 
{
   struct Light 
   {
            Eigen::Vector3f position;
            Eigen::Vector3f ambient;
            Eigen::Vector3f diffuse;
            Eigen::Vector3f specular;
   } mLightSource[2];

} mLightingData;


  
  SimpleMesh();
  
  //void loadMeshFromOBJFile( QString objFilePath );
  void loadMesh(const std::vector< Eigen::Vector3f > &vertices, const std::vector< Eigen::Vector3f > &normals, const std::vector< unsigned int > &indices, bool buildTracer = false);
  void loadMesh(const std::vector< Eigen::Vector3f > &vertices, const  std::vector< unsigned int > &indices, bool buildTracer = false);
  void loadTexture( cv::Mat &texture );
  
  GLuint compileShaders( GLenum shaderType , const char *shaderSource );
  int addShaders( QString vertexShaderSrc , QString geometryShaderSrc , QString fragmentShaderSrc );
  int addShaders( QString vertexShaderSrc , QString fragmentShaderSrc );
  
  bool activateShader( int shaderId );

  bool pickMesh( Eigen::Vector3f& pickedPosition, Eigen::Vector3f& pickedDir);

  bool pickMesh(Eigen::Vector3f& pickedPosition, Eigen::Vector3f& pickedDir , int& triangleId);

  void setPickPosition(Eigen::Vector3f& pickedPosition);

  void setHighlightCircleRadius(float radius);
  
  void init();
  
  void render();
  
  void setViewPort( int w , int h );
  
  void enableWireframe();
  void disableWireframe();

  void setCamera( TrackBallCamera *camera );

  void loadGeometry();
  void loadTexture();

  void useTexture();
  void unuseTexture();
  
  Eigen::Vector3f getObjectCenter();
  float getRadius();
  void getCage( std::vector< Eigen::Vector3f > &cage );

  void setVAO( unsigned int vao );
  void generateVAO();

  int getNumVertices();
  int getNumFaces();
  
  ~SimpleMesh();


protected:

	void initTracer();



protected:

  RTCScene scene;
  unsigned int geomID;

  RTCDevice device;

  float _HighlightCircleRadius;
  Eigen::Vector3f _PickedPosition;
  
};



}



#endif
