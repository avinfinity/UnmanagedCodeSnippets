#ifndef __IMT_VOLUMEINFO__
#define __IMT_VOLUMEINFO__

#include "eigenincludes.h"
#include "QString"
#include "QObject"

namespace imt{

	namespace volume{




class VolumeInfo : public QObject
{

	Q_OBJECT


public:

	VolumeInfo();

	void convertToByteArray( unsigned char background = 0 , unsigned char material = 40 , unsigned char border = 45 );

	void loadVolume( QString filePath );
	void saveVolume( QString filePath );

	void buildVertexIncidence();

	void fixNormals();

	void updateSurfaceData();

	void loadBenchmarkData();

	double valueAt(double x, double y, double z);



signals :

	void surfaceDataChangedS();




public:


	QString mSBRefDataPath, mSBActualDataPath;

	size_t mWidth, mHeight, mDepth;

	int64_t mZStep, mYStep;

	unsigned char *mVolumeData;
	unsigned short *mVolumeDataU16;

	std::vector< float > mDistanceTransform;

	int mVolumeElementSize, mVolumeElementComponents;

	Eigen::Vector3f mVolumeOrigin , mVoxelStep;

	unsigned int mAirVoxelFilterVal;

	std::vector< Eigen::Vector3f > mVertices , mVertexNormals , mOppVertices;
	std::vector< float > mWallthickness , mMinThickness , mMaxThickness;
	std::vector< Eigen::Vector3f > mVertexColors , mRayVertexColors , mSphereVertexColors;
	std::vector< unsigned int > mFaceIndices;

	std::vector<int> mOppositeFaceIndices;


	std::vector< std::vector<int> > mVertexIncidence;

	bool mWallthicknessDataChanged , mHasDistanceTransformData;

	Eigen::Vector3f mBeginColor, mMidColor, mEndColor;

	std::vector< Eigen::Vector3f > mSBRefPoints , mSBRefNormals, mSBDataPoints , mSBDataNormals;
	std::vector< unsigned int > mSBRefIndices, mSBDataIndices;

};

	}

}



#endif