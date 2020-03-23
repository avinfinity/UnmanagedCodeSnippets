#ifndef __TRACKBALL_CAMERA_H__
#define __TRACKBALL_CAMERA_H__

#include "openglincludes.h"

#include "QMatrix4x4"
#include "eigenincludes.h"


class TrackBallCamera
{
  
  QMatrix4x4 mModelView , mProjectionMatrix , mRotation , mTranslation;

  Eigen::Matrix4f mEModelView , mEProjectionMatrix , mERotation , mEtranslation;
  
  QVector3D mObjectCenter , mLastPoint3D;
  
  float mRadius;
  
  unsigned int mViewPortWidth , mViewPortHeight;
  
  bool mLastPointOK;
  
  int mWidth , mHeight;

  bool mStartTrackBall;

  QVector3D mRotationTarget;

  std::vector< Eigen::Vector3f > mCorners;

  QPoint mLastPoint;
  
  QPointF mCurrentMousePos;
  
  int mCurrentKey , mCurrentMouseButton;

  float mZFar, mZNear;
  
  
protected:

 bool mapToSphere( QPoint pos , QVector3D &mappedPoint );  

 void viewAll();

 void intersectWithTrackBallSphere( QVector3D &origin , QVector3D &ray , QVector3D &intersectionPoint );

 void rotate( float angle , QVector3D axis );

 void translate( const Eigen::Vector3f &translationVector );

 void checkCenter();

    
  
public:
  
  TrackBallCamera();
  
  
  QMatrix4x4 getModelViewMatrix();
  QMatrix4x4 getModelViewProjectionMatrix();  
  QMatrix3x3 getRotationMatrix();

  QMatrix4x4 getProjectionMatrix(int viewPortW, int viewPortH);

  QMatrix4x4 getModelViewProjectionMatrix(int viewPortW, int viewPortH);

  void setRotationMatrix( const QMatrix3x3& rotation );

  Eigen::Matrix3f getNormalTransformationMatrix();

  void updateProjectionMatrix( int w , int h);
  
  void registerMousePress( const QPoint& pos  , const Qt::MouseButtons &buttons  );
  void registerMouseMove( const QPoint& pos  , const Qt::MouseButtons &buttons );
  void registerMouseRelease( const QPoint& pos );
  
  void registerMousePos( const QPoint& pos );

  void registerMouseWheel( int delta );
  
  void registerKeyPress( int key );
  void registerKeyRelease( int key );
  
  void setObjectCenter( QVector3D objectCenter );

  void setRadius( float radius );
  
  void setViewPortDimension( unsigned int w , unsigned int h );

  void getWinScale( float &wScale , float &hScale );

  virtual void getModelViewMatrix( double *mv );
  virtual void getProjectionMatrix( double *p );

  virtual void getRay( QPointF pos , Eigen::Vector3f &origin , Eigen::Vector3f &dir );


  Eigen::Vector3f TrackBallCamera::unProject(Eigen::Vector3f winPoint, Eigen::Matrix4f& modelMat, Eigen::Matrix4f& projectionMat, Eigen::Vector4i& viewPort);
  
  
  void getRay( Eigen::Vector3f &origin , Eigen::Vector3f &dir );

  void getRay(Eigen::Vector3f &origin, Eigen::Vector3f &dir, QMatrix4x4 projMat, int origX, int origY, int viewPortW, int viewPortH);
  
  void project( Eigen::Vector3f& pos , Eigen::Vector2f& projection );

  void unProject();
  
  float radius( float R, Eigen::Vector3f& intersection );
  
  void track( const QPoint& pos , const Qt::MouseButtons &buttons );

  void init();


  void testSetCageCorners( std::vector< Eigen::Vector3f > &corners ); 
  
  void getDimensions( int& width , int& height );
  
  int getCurrentKey();
  int getCurrentMouseButton();

  float nearPlane();
  float farPlane();

  int viewportWidth();
  int viewportHeight();
  
  
};


#endif
