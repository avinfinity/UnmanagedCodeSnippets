

#include "iostream"
#include "rawvolumedataio.h"
#include "volumeinfo.h"


int main( int argc , char **argv )
{
	

	QString dataPath = "C:/projects/Wallthickness/data/separated_part_7.uint16_scv";// "C:/projects/Wallthickness/data/CT multi/Calliper Assy 2016-6-9 10-12.uint16_scv"; //;

	imt::volume::RawVolumeDataIO io;

	imt::volume::VolumeInfo volInfo;

	//io.readHeader(dataPath, volInfo);

	io.readUint16SCV( dataPath, volInfo);

	std::cout << volInfo.mWidth << " " << volInfo.mHeight << " " << volInfo.mHeight << std::endl;

	std::cout << " voxel step : " << volInfo.mVoxelStep.transpose() << std::endl;


	
	return 0;
}