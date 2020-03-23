#ifndef __RAWVOLUMEDATAIO_H__
#define __RAWVOLUMEDATAIO_H__

#include "string"
#include "volumeinfo.h"
#include "CPUBuffer.h"



using namespace Zeiss::IMT::NG::NeoInsights::Volume::DataProcessing;

namespace imt{

	namespace volume{




class RawVolumeDataIO
{
	
	public:
	
	RawVolumeDataIO();

	
	bool readHeader( std::string fileName , VolumeInfo& info );

	bool writeHeader(std::string fileName, VolumeInfo& info);

	bool readData( std::string fileName ,  unsigned char*& volumeData );

	static void readUint16SCV(QString filePath, VolumeInfo& info);

	static void readUint16SCV(QString filePath, VolumeInfo& info ,CPUBuffer& cpuBuffer );

	static void readUint16ScvHeader(QString filePath, VolumeInfo& info);


protected:

	VolumeInfo mVolumeInfo;
	
};



	}


}




#endif