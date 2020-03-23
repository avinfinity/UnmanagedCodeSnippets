
#include "iostream"
#include "wallthicknessestimator.h"
#include "QFile"
#include "QByteArray"
#include "volumeinfo.h"
#include "display3droutines.h"
#include "fstream"


void writeMeshData(QString filePath, std::vector< Eigen::Vector3f >& vertices, std::vector< Eigen::Vector3f >& normals, std::vector< unsigned int >& indices);

void readMeshData(std::string filePath, std::vector< Eigen::Vector3f >& vertices, std::vector< Eigen::Vector3f >& normals, std::vector< unsigned int >& indices);


int main( int argc , char **argv )
{
	
	//QFile file("C:/projects/Wallthickness/data/Engine.raw");

	//file.open(QIODevice::ReadOnly);

	//QByteArray data = file.readAll();
	
	imt::volume::VolumeInfo volInfo;


	//std::cout << data.length() << " " << 256 * 256 * 110 << std::endl;

	//volInfo.mVolumeData = (unsigned char*)data.data();
	//
	//volInfo.mWidth = 256;
	//volInfo.mHeight = 256;
	//volInfo.mDepth = 110;

	//volInfo.mVolumeElementComponents = 1;
	//volInfo.mVolumeElementSize = 1;

	QString wtFilepath = "G:/projects/Wallthickness/data/bottle/bottle.wtadat";//"C:/projects/Wallthickness/data/Mobile charger/mc.wtadat";//

	volInfo.loadVolume(wtFilepath);

	std::cout << volInfo.mVoxelStep.transpose() << std::endl;

	imt::volume::WallthicknessEstimator estimator( volInfo );

	estimator.computeBrepRayTraceThicknessMin( volInfo );

	size_t volumeSize = volInfo.mWidth * volInfo.mHeight * volInfo.mDepth;

	unsigned short *thicknessVolume = new unsigned short[volumeSize];

	estimator.fillRayTraceVolume(thicknessVolume);

	cv::Mat sliceImage;

	estimator.getWallthicknessSliceImage(sliceImage, thicknessVolume);

	//tr::Display3DRoutines::displayMesh( volInfo.mVertices , volInfo.mRayVertexColors , volInfo.mFaceIndices );

	return 0;

	//estimator.aabbtreeDemo();

	//estimator.normalEstimationDemo();

	//estimator.rayTracingDemo();

	//estimator.lineVoxelIteratorDemo();

	

	estimator.computeBrepRayTraceThicknessMin(volInfo);

	writeMeshData("C:/projects/Wallthickness/data/mc.meshdata", volInfo.mVertices, volInfo.mVertexNormals, volInfo.mFaceIndices);

	readMeshData("C:/projects/Wallthickness/data/mc.meshdata", volInfo.mVertices, volInfo.mVertexNormals, volInfo.mFaceIndices);

	tr::Display3DRoutines::displayMesh(volInfo.mVertices, volInfo.mRayVertexColors, volInfo.mFaceIndices);

	return 0;
}

void writeMeshData(QString filePath, std::vector< Eigen::Vector3f >& vertices, std::vector< Eigen::Vector3f >& normals, std::vector< unsigned int >& indices)
{

	std::fstream writer;

	writer.open(filePath.toStdString(), std::ios::binary | std::ios::out);

	int numVertices = vertices.size();
	int numFaces = indices.size() / 3;

	writer.write((char*)&numVertices, sizeof(int));
	writer.write((char*)&numFaces, sizeof(int));

	writer.write((char*)vertices.data(), sizeof(Eigen::Vector3f) * numVertices);
	writer.write((char*)normals.data(), sizeof(Eigen::Vector3f) * numVertices);
	writer.write((char*)indices.data(), sizeof(unsigned int ) * 3 * numFaces);

	writer.close();
}

void readMeshData(std::string filePath, std::vector< float >& vertices, std::vector< float >& normals, std::vector< unsigned int >& indices)
{

	std::fstream reader;

	reader.open(filePath, std::ios::binary | std::ios::in);

	int numVertices = 0;
	int numFaces = 0;



	reader.read((char*)&numVertices, sizeof(int));
	reader.read((char*)&numFaces, sizeof(int));

	vertices.resize(numVertices * 3);
	normals.resize(numVertices * 3);
	indices.resize(numFaces * 3);

	reader.read((char*)vertices.data(), numVertices * 3 * sizeof(float));
	reader.read((char*)normals.data(), numVertices * 3 * sizeof(float));
	reader.read((char*)indices.data(), sizeof(unsigned int) * 3 * numFaces);

	reader.close();
}