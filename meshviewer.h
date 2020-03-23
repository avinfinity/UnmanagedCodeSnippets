#ifndef __MESH_VIEWER_H__
#define __MESH_VIEWER_H__

#include "openglincludes.h"
#include "eigenincludes.h"

#include "trackballcamera.h"
#include "simplemesh.h"
//#include "RenderObjects/pointset.h"
#include "QString"

//#include "opengl/shadersourcemanager.h"

//#ifdef VC_QOPENGL_FUNCTIONS

#include "QOpenGLFunctions_3_3_Core"
#include "QGLFormat"
#include "QOpenGLWidget"
#include "opencvincludes.h"

//#endif

namespace imt {

	namespace volume {


		class VolumeInfo;

	}

}


namespace vc{


	void createArrow(const Eigen::Vector3f& origin, const Eigen::Vector3f dir, float length, float shaftRadius,
		float arrowRadius, int shaftResolution, std::vector< Eigen::Vector3f >& points, std::vector< unsigned int >& faceIndices);


class MESHViewer
{



public:

	static int viewMesh(const std::vector< Eigen::Vector3f > &vertices, const std::vector< unsigned int > &indices, imt::volume::VolumeInfo* volume);
	static int viewMesh( const std::vector< Eigen::Vector3f > &vertices, const std::vector< Eigen::Vector3f > &normals, 
		                 const std::vector< unsigned int > &indices, imt::volume::VolumeInfo* volume);

	static int viewMesh( const std::vector< Eigen::Vector3f > &vertices , const std::vector< unsigned int > &indices );
	static int viewMesh( const std::vector< Eigen::Vector3f > &vertices , const std::vector< Eigen::Vector3f > &normals , 
			             const std::vector< unsigned int > &indices );
	
	static int viewPointCloud( const std::vector< Eigen::Vector3f > &vertices , std::vector< Eigen::Vector3f >& colors );


};
    
    


class MeshViewerWidget : public QOpenGLWidget , public QOpenGLFunctions_3_3_Core
{


  Q_OBJECT

  TrackBallCamera *mCamera;

  bool mIsCameraInitialized , mShaderInitialized;

  SimpleMesh *mMesh , *mArrow;

  bool mVisualizeArrow;
  
  QString mVertWONSrcs , mGeomWONSrcs , mFragWONSrcs , mVertWNSrcs  , 
	      mGeomWNSrcs , mFragWNSrcs , mPointCloudVertSrcs , mPointCloudFragSrcs , 
	      mArrowVertSrcs , mArrowGeomSrcs, mArrowFragSrcs;
  
  QString mVertexShaderSrc , mGeometryShaderSrc , mFragmentShaderSrc;
  
  QString mPSVShaderSrc , mPSFShaderSrc;
  
  bool mMeshDataLoaded , mMeshWithNormals , mDataLoadedToOpenGL;
  
  int mShaderSetId;
  int mPointCloudProgram;

  GLuint mVAO[ 2 ];
  
  float mArrowLength;


public:
  
  enum ViewMode{ MESH = 0 , POINT_CLOUD  };

  explicit MeshViewerWidget(  QWidget* parent = 0,  Qt::WindowFlags f = 0 );

  void setMeshData( const std::vector< Eigen::Vector3f > &vertices , const std::vector< Eigen::Vector3f > &normals , 
		            const std::vector< unsigned int > &indices );
  void setMeshData( const std::vector< Eigen::Vector3f > &vertices , const std::vector< unsigned int > &indices );

  void setMeshData(const std::vector< Eigen::Vector3f > &vertices, const std::vector< unsigned int > &indices , imt::volume::VolumeInfo* volume);

  void setMeshData(const std::vector< Eigen::Vector3f > &vertices, const std::vector< Eigen::Vector3f > &normals,const std::vector< unsigned int > &indices, imt::volume::VolumeInfo* volume);

  
  void setPointCloudData( const std::vector< Eigen::Vector3f > &vertices , const std::vector< Eigen::Vector3f > &colors );
  
  virtual void initializeGL();
  virtual void resizeGL( int w , int h );
  virtual void paintGL();  


  virtual void keyPressEvent(QKeyEvent* e);
  
  virtual void mousePressEvent(QMouseEvent *event);
  virtual void mouseReleaseEvent( QMouseEvent *event );
  virtual void mouseMoveEvent(QMouseEvent *event);     

  virtual void	wheelEvent( QWheelEvent * event );
  
  ~MeshViewerWidget();


protected:

	bool pickMesh(Eigen::Vector3f& pickedPoint);
  
  
protected:

  ViewMode mViewMode;  

  imt::volume::VolumeInfo *mVolume;

};

}


#endif
