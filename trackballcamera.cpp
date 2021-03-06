
#include "openglincludes.h"
#include "openglhelper.h"
#include "trackballcamera.h"
#include "cmath"


#include "iostream"




TrackBallCamera::TrackBallCamera()
{
	mStartTrackBall = false;
	mLastPointOK = false;
	
	mCurrentKey = -1;
	mCurrentMouseButton = -1;

	mEModelView.setIdentity();
}


void TrackBallCamera::registerMouseMove ( const QPoint& pos, const Qt::MouseButtons& buttons  )
{

	mCurrentMousePos = pos;

	if( mStartTrackBall && mCurrentKey == -1 )
	{
		track( pos , buttons ); 

		mLastPoint = pos;
	}

}


void TrackBallCamera::registerMousePress ( const QPoint& pos, const Qt::MouseButtons& buttons  )
{
	mStartTrackBall = true;

	mCurrentMousePos = pos;

	track( pos , buttons );
	
	mCurrentMouseButton = buttons;

	mLastPoint = pos;
}

  
void TrackBallCamera::registerMouseRelease ( const QPoint& pos )
{
	mStartTrackBall = false;

	mLastPointOK = false;
	
	mCurrentMouseButton = -1;
}


void TrackBallCamera::registerMousePos( const QPoint& pos )
{
  
   mCurrentMousePos = pos;
}

void TrackBallCamera::registerMouseWheel( int delta )
{
    float d = -(float)delta / 360.0 * 0.2 * mRadius ;
    translate( Eigen::Vector3f( 0.0 , 0.0 , d ) );
}

void TrackBallCamera::registerKeyPress( int key )
{
  mCurrentKey = key;
}

void TrackBallCamera::registerKeyRelease( int key )
{
  
  
  mCurrentKey = -1;
}


QMatrix4x4 TrackBallCamera::getModelViewMatrix()
{
	return mModelView;
}


QMatrix4x4 TrackBallCamera::getModelViewProjectionMatrix()
{
  
  return ( mProjectionMatrix * mModelView ) ;
  
}


QMatrix4x4 TrackBallCamera::getProjectionMatrix( int viewPortW, int viewPortH)
{
	QMatrix4x4 projectionMatrix;

	projectionMatrix.setToIdentity();
	projectionMatrix.perspective(45.0, (float)viewPortW / (float)viewPortH, 0.01 * mRadius, 100.0 * mRadius);

	return projectionMatrix;

}

QMatrix4x4 TrackBallCamera::getModelViewProjectionMatrix(int viewPortW, int viewPortH)
{
	return (getProjectionMatrix( viewPortW , viewPortH ) * mModelView);
}

QMatrix3x3 TrackBallCamera::getRotationMatrix()
{
//    Eigen::Matrix4f r;

//    r.setIdentity();

//    for( int rr = 0; rr < 3 ; rr++ )
//        for( int cc = 0; cc < 3 ; cc++ )
//        {
//           r( rr , cc ) = rotation( rr , cc );
//        }


    QMatrix3x3 r;

    for( int ii = 0; ii < 3 ; ii++ )
        for( int jj = 0; jj < 3 ; jj++ )
        {

            r( ii , jj ) = mModelView( ii , jj );
        }


    return r;

}

void TrackBallCamera::setRotationMatrix( const QMatrix3x3& rotation )
{

//    for( int ii = 0; ii < 3 ; ii++ )
//        for( int jj = 0; jj < 3 ; jj++ )
//        {

//           mModelView( ii , jj ) = rotation( ii , jj );
//        }

//    Eigen::Matrix4f dMV , centerTranslate;

//    centerTranslate.setIdentity();
//    dMV.setIdentity();

//    Eigen::Vector4f eCenterVec(  mObjectCenter[ 0 ] , mObjectCenter[ 1 ] , mObjectCenter[ 2 ] , 1.0 );

//    Eigen::Vector4f tCenterVec = mEModelView * eCenterVec;

//    centerTranslate( 0 , 3 ) = -tCenterVec( 0 );
//    centerTranslate( 1 , 3 ) = -tCenterVec( 1 );
//    centerTranslate( 2 , 3 ) = -tCenterVec( 2 );

//    dMV( 0 , 3 ) += -tCenterVec( 0 );
//    dMV( 1 , 3 ) += -tCenterVec( 1 );
//    dMV( 2 , 3 ) += -tCenterVec( 2 );

//    Eigen::AngleAxisf angleAxis( angle * M_PI / 180.0 , Eigen::Vector3f( axis[ 0 ] , axis[ 1 ] , axis[ 2 ] ) );

//    Eigen::Quaternionf equatf( angleAxis );

//    Eigen::Matrix4f mat;

//    mat.setIdentity();

//    for( int ii = 0; ii < 3 ; ii++ )
//        for( int jj = 0; jj < 3 ; jj++ )
//        {
//            mat( ii , jj ) = rotation( ii , jj );
//        }

//   // mat.block( 0 , 0 , 3 , 3 ) = equatf.toRotationMatrix();

//    dMV = mat * dMV;

//    dMV = centerTranslate.inverse() * dMV;

//    mEModelView = dMV * mEModelView;

//    for( int rr = 0; rr < 4 ; rr++ )
//      for( int cc = 0 ; cc < 4 ; cc++ )
//      {
//        mModelView( rr , cc ) = mEModelView( rr , cc );
//      }

    Eigen::Matrix4f t1 ;

    Eigen::Matrix4f r;

    r.setIdentity();

    for( int rr = 0; rr < 3 ; rr++ )
        for( int cc = 0; cc < 3 ; cc++ )
        {
           r( rr , cc ) = rotation( rr , cc );
        }

    t1 = mEtranslation * r;

    mEModelView = t1;

    for( int rr = 0; rr < 4 ; rr++ )
     for( int cc = 0 ; cc < 4 ; cc++ )
     {
        mModelView( rr , cc ) = mEModelView( rr , cc );
     }

}


Eigen::Matrix3f TrackBallCamera::getNormalTransformationMatrix()
{

	Eigen::Matrix3f temp = mEModelView.block(0, 0, 3, 3);

	return temp.inverse().transpose();
  
}

void TrackBallCamera::setObjectCenter ( QVector3D objectCenter )
{
  mObjectCenter = objectCenter;
}


void TrackBallCamera::setRadius( float radius )
{
	mRadius = radius;
}


void TrackBallCamera::setViewPortDimension ( unsigned int w, unsigned int h )
{
   mViewPortWidth = w;
   mViewPortHeight = h;

   updateProjectionMatrix( w , h );
}


void TrackBallCamera::getDimensions(int& width, int& height)
{
  width = mViewPortWidth;
  height = mViewPortHeight;
}


void TrackBallCamera::getWinScale( float &wScale , float &hScale )
{
	wScale = mViewPortWidth;
	hScale = mViewPortHeight;

}


void TrackBallCamera::getModelViewMatrix( double *mv )
{

	double *m1 = mv;

	for( int cc = 0; cc < 4; cc++ )
		for( int rr = 0; rr < 4; rr++ )
		{
			*m1 = mModelView( rr , cc );

			m1++;
		}
}

void TrackBallCamera::getProjectionMatrix( double *p )
{
    double *m1 = p;

	for( int cc = 0; cc < 4; cc++ )
		for( int rr = 0; rr < 4; rr++ )
		{
			*m1 = mProjectionMatrix( rr , cc );

			m1++;
		}
}



void TrackBallCamera::getRay( QPointF pos, Eigen::Vector3f& origin, Eigen::Vector3f& dir )
{
    GLint viewport[ ] = { 0 , 0 , mViewPortWidth , mViewPortHeight };
    
    double mv[ 16 ] , p[ 16 ];
    
    float *d1 = mEModelView.data();
    
    for( int ii = 0; ii < 4; ii++ )
        for( int jj = 0; jj < 4; jj++ )
        {
            mv[ 4 * ii + jj ] = mEModelView( jj , ii );
            p[ 4 * ii + jj ] = mProjectionMatrix( jj , ii );
        }
  
    double nearPlaneLocation[3];
    
    Eigen::Vector4d nearV , farV;
    
    gluUnProject( pos.x(), mViewPortHeight - pos.y() , 0 , mv , p ,
                  viewport, &nearPlaneLocation[0], &nearPlaneLocation[1], 
                  &nearPlaneLocation[2] );

	Eigen::Vector3f ptp(pos.x(), mViewPortHeight - pos.y(), 0);
	Eigen::Vector4i eviewPort(viewport[0], viewport[1], viewport[2], viewport[3]);
	Eigen::Vector3f unprojPoint = unProject(ptp, mEModelView , mEProjectionMatrix, eviewPort);



	//std::cout << " match unprojection 1 : " << unprojPoint.transpose() << std::endl;
	//std::cout << " match unprojection 2 : " << nearPlaneLocation[0] << " " << nearPlaneLocation[1] << " " << nearPlaneLocation[2] << std::endl;
    


    double farPlaneLocation[3];

    gluUnProject( pos.x(), viewport[ 3 ] - pos.y() , 1 , mv , p ,
                  viewport, &farPlaneLocation[0], &farPlaneLocation[1], 
                  &farPlaneLocation[2] );
    
    
    origin( 0 ) = nearPlaneLocation[ 0 ];
    origin( 1 ) = nearPlaneLocation[ 1 ];
    origin( 2 ) = nearPlaneLocation[ 2 ];
    
    dir( 0 ) = farPlaneLocation[ 0 ] - nearPlaneLocation[ 0 ];
    dir( 1 ) = farPlaneLocation[ 1 ] - nearPlaneLocation[ 1 ];
    dir( 2 ) = farPlaneLocation[ 2 ] - nearPlaneLocation[ 2 ];
    
    dir.normalize();


	ptp = Eigen::Vector3f(pos.x(), mViewPortHeight - pos.y(), 1);
	eviewPort = Eigen::Vector4i(viewport[0], viewport[1], viewport[2], viewport[3]);
	unprojPoint = unProject(ptp, mEModelView, mEProjectionMatrix, eviewPort);


	//std::cout << " match unprojection 11 : " << unprojPoint.transpose() << std::endl;
	//std::cout << " match unprojection 22 : " << nearPlaneLocation[0] << " " << nearPlaneLocation[1] << " " << nearPlaneLocation[2] << std::endl;
  
}


Eigen::Vector3f TrackBallCamera::unProject(Eigen::Vector3f winPoint, Eigen::Matrix4f& modelMat, Eigen::Matrix4f& projectionMat, Eigen::Vector4i& viewPort)
{
	//template <typename T, typename U, precision P>
	//GLM_FUNC_QUALIFIER tvec3<T, P> unProject
	//	(
	//	tvec3<T, P> const & win,
	//	tmat4x4<T, P> const & model,
	//	tmat4x4<T, P> const & proj,
	//	tvec4<U, P> const & viewport
	//	)
	//{
	Eigen::Matrix4f  inverse = (projectionMat * modelMat).inverse();

	Eigen::Vector4f tmp( winPoint(0) , winPoint(0) , winPoint(2) , 1 );
	tmp.x() = (tmp.x() - ((float)viewPort(0))) / ((float)viewPort(2));
	tmp.y() = ( tmp.y() - ( (float)viewPort(1) )) / ( (float)viewPort(3) );

#if 0// GLM_DEPTH_CLIP_SPACE == GLM_DEPTH_ZERO_TO_ONE
	tmp.x() = tmp.x() * 2 - 1;
	tmp.y() = tmp.y() * 2 - 1;
#else
	tmp = tmp * 2 - Eigen::Vector4f::Ones();
#endif
	 
	Eigen::Vector4f obj = inverse * tmp;
	obj /= obj.w();

	return obj.block(0, 0, 3, 1);
}


void TrackBallCamera::getRay( Eigen::Vector3f &origin , Eigen::Vector3f &dir )
{
  
  getRay( mCurrentMousePos , origin , dir );
  
}


void TrackBallCamera::getRay(Eigen::Vector3f &origin, Eigen::Vector3f &dir , QMatrix4x4 projMat, int origX, int origY , int viewPortW , int viewPortH )
{

	QPointF pt;

	pt.rx() = mCurrentMousePos.x();
	pt.ry() = mCurrentMousePos.y();

	GLint viewport[] = { origX, origY, viewPortW, viewPortH };

	double mv[16], p[16];

	float *d1 = mEModelView.data();

	for (int ii = 0; ii < 4; ii++)
		for (int jj = 0; jj < 4; jj++)
		{
			mv[4 * ii + jj] = mEModelView(jj, ii);
			p[4 * ii + jj] = projMat(jj, ii);
		}

	double nearPlaneLocation[3];

	Eigen::Vector4d nearV, farV;

	gluUnProject(pt.x(), mViewPortHeight - pt.y(), 0, mv, p,
		viewport, &nearPlaneLocation[0], &nearPlaneLocation[1],
		&nearPlaneLocation[2]);


	double farPlaneLocation[3];

	gluUnProject(pt.x(), viewport[3] - pt.y(), 1, mv, p,
		viewport, &farPlaneLocation[0], &farPlaneLocation[1],
		&farPlaneLocation[2]);


	origin(0) = nearPlaneLocation[0];
	origin(1) = nearPlaneLocation[1];
	origin(2) = nearPlaneLocation[2];

	dir(0) = farPlaneLocation[0] - nearPlaneLocation[0];
	dir(1) = farPlaneLocation[1] - nearPlaneLocation[1];
	dir(2) = farPlaneLocation[2] - nearPlaneLocation[2];

	dir.normalize();


	//getRay(pt, origin, dir);
}

void TrackBallCamera::project(Eigen::Vector3f& pos, Eigen::Vector2f& projection)
{
  double x = pos( 0 ) , y = pos( 1 ) , z = pos( 2 );
  
  double mv[ 16 ] , p[ 16 ];
      
  GLint viewport[ ] = { 0 , 0 , mViewPortWidth , mViewPortHeight };
    
  float *d1 = mEModelView.data();
    
  for( int ii = 0; ii < 4; ii++ )
   for( int jj = 0; jj < 4; jj++ )
   {
      mv[ 4 * ii + jj ] = mEModelView( jj , ii );
      p[ 4 * ii + jj ] = mProjectionMatrix( jj , ii );
   }
        
        
  double winX , winY , winZ;      
  
  gluProject( x , y , z , mv , p , viewport , &winX , & winY , &winZ );
  
  projection( 0 ) = winX;
  projection( 1 ) = mViewPortHeight - winY;
  
}



float TrackBallCamera::radius( float R, Eigen::Vector3f& intersection )
{
    GLdouble winCoords[ 3 ] , vCircle[ 3 ];
    
    Eigen::Vector4f pickedPoint( intersection( 0 ) , intersection( 1 ) , intersection( 2 ) , 1.0f );
    
    GLint viewport[ ] = { 0 , 0 , mViewPortWidth , mViewPortHeight };
    
    double mv[ 16 ] , p[ 16 ];
    
    float *d1 = mEModelView.data();
    float *d2 = mEProjectionMatrix.data();
    
    for( int ii = 0; ii < 4; ii++ )
        for( int jj = 0; jj < 4; jj++ )
        {
            mv[ 4 * ii + jj ] = mEModelView( jj , ii );
            p[ 4 * ii + jj ] = mProjectionMatrix( jj , ii );
        }
    
    gluProject( intersection.x(), intersection.y() , intersection( 2 ) , mv , p ,
                viewport , &winCoords[0] , &winCoords[1] , 
                &winCoords[2]  );
    
    gluUnProject( mCurrentMousePos.x() + R  , ( viewport[ 3 ] - mCurrentMousePos.y() ) , winCoords[2] , mv , p ,
                  viewport , &vCircle[0] , &vCircle[1] , &vCircle[ 2 ] );

		      
        
    float radiusSq = ( intersection( 0 ) - vCircle[0] ) * ( intersection( 0 ) - vCircle[0] ) + 
                     ( intersection( 1 ) - vCircle[1] ) * ( intersection( 1 ) - vCircle[1] ) +
		             ( intersection( 2 ) - vCircle[2] ) * ( intersection( 2 ) - vCircle[2] ) ;
			      
			      
    return radiusSq;
}




bool TrackBallCamera::mapToSphere ( QPoint pos , QVector3D& mappedPoint )
{

#if 0
  if ((pos.x() >= 0) && (pos.x() <= int( mViewPortWidth )) &&
      (pos.y() >= 0) && (pos.y() <= int( mViewPortHeight )) )
  {
    double x  = ( double )( pos.x() - 0.5 * mViewPortWidth )  / ( double ) mViewPortWidth;
    double y  = ( double )( 0.5 * mViewPortHeight - pos.y() ) / ( double ) mViewPortHeight;
    
    double sinx         = sin( M_PI * x * 0.5 );
    double siny         = sin( M_PI * y * 0.5 );
    
    double sinx2siny2   = sinx * sinx + siny * siny;

    mappedPoint[ 0 ] = sinx;
    mappedPoint[ 1 ] = siny;
    mappedPoint[ 2 ] = sinx2siny2 < 1.0 ? sqrt( 1.0 - sinx2siny2 ) : 0.0 ;

    return true;
  }
  else
    return false;


#elif 1
	if (!((pos.x() >= 0) && (pos.x() <= int( mViewPortWidth )) &&
      (pos.y() >= 0) && (pos.y() <= int( mViewPortHeight )) ) )
  {
	  return false;
}

	 // This is actually doing the Sphere/Hyperbolic sheet hybrid thing,
    // based on Ken Shoemake's ArcBall in Graphics Gems IV, 1993.
	double x =  (2.0 * pos.x() - mViewPortWidth )/ mViewPortWidth;
    
	double y = -( 2.0 * pos.y() - mViewPortHeight ) / mViewPortHeight ;
    
    double xval = x;
    
    double yval = y;
    
    double x2y2 = xval * xval + yval * yval;

	const double rsqr = 0.6 * 0.6;//mRadius * mRadius;
    
    mappedPoint[ 0 ] = xval;
    mappedPoint[ 1 ] = yval;
    
    if ( x2y2 < 0.5 * rsqr ) 
    {
        mappedPoint[ 2 ] = sqrt(rsqr - x2y2);
    } 
    else 
    {
        mappedPoint[ 2 ] = 0.5 * rsqr /sqrt(x2y2);
    }
    
    return true;

#endif


}


void TrackBallCamera::track( const QPoint& pos, const Qt::MouseButtons& buttons)
{
  
  QVector3D newPoint3D;
  
  bool newPointOk = mapToSphere( pos , newPoint3D );

  if ( mLastPointOK && newPointOk )
  {

	  float dx = pos.x() - mLastPoint.x();
	  float dy = pos.y() - mLastPoint.y();

#if 0
    
    QVector3D axis = QVector3D::crossProduct( mLastPoint3D , newPoint3D ).normalized();
    
    float cos_angle = QVector3D::dotProduct( mLastPoint3D , newPoint3D );
    
    if ( fabsf( cos_angle ) < 1.0 )
    {
       float angle = 2.0 * acos( cos_angle );
      
       QQuaternion quat = QQuaternion::fromAxisAndAngle( axis , angle );
      
       mModelView.rotate( quat );
      
    }
#else
	  if( buttons == Qt::LeftButton )
	  {
         QVector3D axis = QVector3D::crossProduct( mLastPoint3D , newPoint3D );
        
         if ( axis.lengthSquared() < 1e-7 ) 
         {
               axis = QVector3D( 1 , 0 , 0 );
         } 
         else 
         {
               axis.normalize();
         }

         // find the amount of rotation
         QVector3D d = mLastPoint3D - newPoint3D;
            
		 float t = 0.5 * d.length() / 0.6;
        
	     if (t < -1.0)
               t = -1.0;
         else if (t > 1.0)
               t = 1.0;
	    
         float phi = 2.0 * asin(t);
         float angle = phi * 180.0 / M_PI;

		 rotate( angle , axis );
	  }
	  else if( buttons == Qt::RightButton )
	  {
		  Eigen::Vector4f center( mObjectCenter[ 0 ] , mObjectCenter[ 1 ] , mObjectCenter[ 2 ] , 1.0 );

		  Eigen::Vector4f mappedCenter = mEModelView * center;

		  float z = -mappedCenter( 2 ) / mappedCenter( 3 ); 

		  float fovy = 45.0f;

		  float aspect     = ( float )mWidth / mHeight;
		  float near_plane = 0.01 * mRadius;
          float top        = tan( fovy / 2.0f * M_PI /180.0f ) * near_plane;
          float right      = aspect*top;

		  Eigen::Vector3f translation;

		  translation( 0 ) = 2.0 * dx / mWidth * right / near_plane * z;
		  translation( 1 ) = -2.0 * dy / mHeight * top / near_plane * z;
		  translation( 2 ) = 0.0;

		  translate( translation );
	  
	  }

#endif
    
  }

  mLastPoint3D = newPoint3D;
  mLastPointOK = newPointOk;
}


void TrackBallCamera::viewAll()
{
  
    qDebug() << " object center : "<<mObjectCenter<<endl;
  
    QVector4D vec( mObjectCenter[ 0 ] , mObjectCenter[ 1 ] , mObjectCenter[ 2 ] , 1.0 );
    
    QVector4D mappedVec = mModelView.map( vec );
    
    QVector3D transVec;
    
	transVec[ 0 ] = -mappedVec[ 0 ];
    transVec[ 1 ] = -mappedVec[ 1 ];
    transVec[ 2 ] = -mappedVec[ 2 ] -  3.0 * mRadius;

	mTranslation.translate( transVec );

	mEtranslation( 0 , 3 ) += transVec[ 0 ];
    mEtranslation( 1 , 3 ) += transVec[ 1 ];
	mEtranslation( 2 , 3 ) += transVec[ 2 ];

	mEModelView = mEtranslation * mEModelView;

	for( int rr = 0; rr < 4 ; rr++ )
	  for( int cc = 0 ; cc < 4 ; cc++ )
	  {
		mModelView( rr , cc ) = mEModelView( rr , cc );
	  }


	mZNear = 0;
	mZFar = 3 * mRadius;
}


void TrackBallCamera::updateProjectionMatrix( int w , int h)
{
	mWidth = w;
	mHeight = h;

	mViewPortWidth = w;
	mViewPortHeight = h;

    mProjectionMatrix.setToIdentity();    
    mProjectionMatrix.perspective( 45.0 , ( float ) w / ( float ) h , 0.01 * mRadius , 100.0 * mRadius ); 

	for (int rr = 0; rr < 4; rr++)
		for (int cc = 0; cc < 4; cc++)
		{
			mEProjectionMatrix(rr, cc) = mProjectionMatrix(rr, cc);
		}
}


void TrackBallCamera::intersectWithTrackBallSphere( QVector3D &origin , QVector3D &ray , QVector3D &intersectionPoint )
{

  QVector3D v = ( origin - mObjectCenter );

  float A = 1.0;

  float B = 2 * QVector3D::dotProduct( v , ray );

  float C = QVector3D::dotProduct( v , v ) - mRadius * mRadius;

  float val = sqrtf( B * B - 4 * A * C );

  float t = ( -B - val ) / ( 2 * A ) ;

  intersectionPoint = ( origin + t * ray - mObjectCenter );

}


void TrackBallCamera::rotate( float angle , QVector3D axis )
{

	Eigen::Matrix4f dMV , centerTranslate;

	centerTranslate.setIdentity();
	dMV.setIdentity();

	Eigen::Vector4f eCenterVec(  mObjectCenter[ 0 ] , mObjectCenter[ 1 ] , mObjectCenter[ 2 ] , 1.0 );

	Eigen::Vector4f tCenterVec = mEModelView * eCenterVec;

	centerTranslate( 0 , 3 ) = -tCenterVec( 0 );
	centerTranslate( 1 , 3 ) = -tCenterVec( 1 );
	centerTranslate( 2 , 3 ) = -tCenterVec( 2 );

	dMV( 0 , 3 ) += -tCenterVec( 0 );
	dMV( 1 , 3 ) += -tCenterVec( 1 );
	dMV( 2 , 3 ) += -tCenterVec( 2 );

	Eigen::AngleAxisf angleAxis( angle * M_PI / 180.0 , Eigen::Vector3f( axis[ 0 ] , axis[ 1 ] , axis[ 2 ] ) );

	Eigen::Quaternionf equatf( angleAxis );

	Eigen::Matrix4f mat;

	mat.setIdentity();

	mat.block( 0 , 0 , 3 , 3 ) = equatf.toRotationMatrix();

	dMV = mat * dMV;

	dMV = centerTranslate.inverse() * dMV;

	mEModelView = dMV * mEModelView;

	for( int rr = 0; rr < 4 ; rr++ )
	  for( int cc = 0 ; cc < 4 ; cc++ )
	  {
		mModelView( rr , cc ) = mEModelView( rr , cc );
	  }

}


void TrackBallCamera::translate( const Eigen::Vector3f &translationVector )
{
	Eigen::Matrix4f translationMatrix;

	translationMatrix.setIdentity();

	translationMatrix( 0 , 3 ) = translationVector( 0 );
	translationMatrix( 1 , 3 ) = translationVector( 1 );
	translationMatrix( 2 , 3 ) = translationVector( 2 );

    mEtranslation = translationMatrix * mEtranslation;

	mEModelView = translationMatrix * mEModelView;

	for( int rr = 0; rr < 4 ; rr++ )
	 for( int cc = 0 ; cc < 4 ; cc++ )
	  {
		mModelView( rr , cc ) = mEModelView( rr , cc );
	  }
}

void TrackBallCamera::checkCenter()
{
	int numCorners = mCorners.size();

	Eigen::Vector3f objectCenter( 0 , 0 , 0 ) , objectCenter2( 0 , 0 , 0 );

	Eigen::Matrix4f mv;

	for( int rr = 0; rr < 4; rr++ )
		for( int cc = 9 ; cc < 4 ; cc++ )
		{
			mv( rr , cc ) = mModelView( rr , cc );
		}

	for( int cc = 0; cc < numCorners ; cc++ )
	{
		Eigen::Vector4f vec( mCorners[ cc ]( 0 ) , mCorners[ cc ]( 1 ) , mCorners[ cc ]( 2 ) , 1.0 );

		Eigen::Vector4f transformedVec = mv * vec;

		//transformedVec /= transformedVec( 3 ); 

		objectCenter += transformedVec.block( 0 , 0 , 3 , 1 );

		transformedVec = (  mERotation * mEModelView ) * vec ;

		objectCenter2 += transformedVec.block( 0 , 0 , 3 , 1 );

	}

	objectCenter /= numCorners;
	objectCenter2 /= numCorners;

	std::cout<<" object center : "<< objectCenter.transpose() <<" "<<numCorners<< std::endl;
	std::cout<<" object center2 : "<< objectCenter2.transpose() <<" "<<numCorners<< std::endl;
}

void TrackBallCamera::init()
{
	qDebug()<<" width and height : "<<mViewPortWidth<<" "<<mViewPortHeight<< endl;

	updateProjectionMatrix( mViewPortWidth , mViewPortHeight );

	mRotation.setToIdentity();
	mTranslation.setToIdentity();
	mModelView.setToIdentity();	


	mEModelView.setIdentity();
	mEProjectionMatrix.setIdentity();
	mERotation.setIdentity();
	mEtranslation.setIdentity();

	viewAll();
}


void TrackBallCamera::testSetCageCorners( std::vector< Eigen::Vector3f > &corners )
{
	mCorners = corners;

}


int TrackBallCamera::getCurrentKey()
{
  return mCurrentKey;
}


int TrackBallCamera::getCurrentMouseButton()
{
  return mCurrentMouseButton;
}



float TrackBallCamera::nearPlane()
{
	return mZNear;
}

float TrackBallCamera::farPlane()
{
	return mZFar;
}


int TrackBallCamera::viewportWidth()
{
	return mViewPortWidth;
}
int TrackBallCamera::viewportHeight()
{
	return mViewPortHeight;
}




