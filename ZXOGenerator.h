#ifndef __ZXOGENERATOR_H__
#define __ZXOGENERATOR_H__


#include "string"
#include "vector"

#define API_ZXOGenerator __declspec(dllexport)

class API_ZXOGenerator ZXOGenerator
{


public:


	struct MetaData
	{
		enum DataTypes
		{
			UNSIGNED_CHAR = 0,
			UNSIGNED_SHORT,
			UNSIGNED_INT,
			FLOAT
		};


        //dimesion of volume
		unsigned int _VolumeDimX, _VolumeDimY, _VolumeDimZ;
		//lenth of one voxel
		float _VoxelSizeX, _VoxelSizeY, _VoxelSizeZ;
		//byte size of gray value stored at each voxel
		unsigned int _ByteSize;

		bool _IsSigned;
		//start position for volume
		float _VolumeStartPointX, _VolumeStartPointY, _VolumeStartPointZ;
		
		double  _VolumeDefiningAngle;

		std::string _PublishedVersion;

		//Data type of gray values
		DataTypes _Type;
	};




	ZXOGenerator();

	void setFileName(std::string fileName);

	//returns false in case filename is not set or the path is invalid
	bool writeMetaData(MetaData metaData);
	//returns false in case any od the chunk files are invalid or empty is not set
	bool writeData(const std::vector< std::string >& chunkPaths);
	
};



#endif