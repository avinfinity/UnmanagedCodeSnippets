#pragma once

#include <Inventor/SbLinear.h>
#include <Inventor/STL/map>
#include <Inventor/STL/list>

#include <Inventor/nodes/SoNode.h>
#include <Inventor/fields/SoSubField.h>

#include <Inventor/fields/SoSFNode.h>
#include <Inventor/fields/SoSFInt32.h>
#include <Inventor/fields/SoSFUInt32.h>
#include <Inventor/fields/SoSFEnum.h>
#include <Inventor/fields/SoSFFilePathString.h>
#include <Inventor/threads/SbThread.h>
#include <Inventor/threads/SbThreadMutex.h>
#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/readers/SoVolumeReader.h>
#include <VolumeViz/readers/SoVRLdmFileReader.h>
#include "volumetopooctree.h"


		class CustomTiledReader : public SoVolumeReader //SoVRLdmFileReader//
		{
			SO_FIELDCONTAINER_HEADER(CustomTiledReader);

			struct TilesHeader
			{
				int _Major, _Minor;
				int _TileDim;
				int headerSize;
				int mirrorZ;
				int numBitsPerVoxel;
				int volumeSizeX;
				int volumeSizeY;
				int volumeSizeZ;
				double voxelSizeX;
				double voxelSizeY;
				double voxelSizeZ;
				double resultBitDepthMinVal;
				double resultBitDepthmaxVal;
				double tubePosX;
				double tubePosY;
				double tubePosZ;
				int tubeCurrent;
				int tubeVoltage;
				double rtPosX;
				double rtPosY;
				double rtPosZ;
				double detectorIntegrationTime; //int mili seconds
				double detectorGain;
				double detectorXPos;
				double detectorYPos;
				double detectorZPos;
				float detectorPixelWidth;
				float detectorPixelHeight;
				int imageBitDepth;
				int detectorWidth;
				int detectorHeight;
				int detectorImageWidth;
				int detectorImageHeight;
				int numberOfProjections;
				int roiX;
				int roiY;
				int roiWidth;
				int roiHeight;
				double noiseSuppressionFilter;
				double voxelreductionFactor;
				float gain;
				float binningMode;
				char prefilterData[128];
				double volumeStartPosX;
				double volumeStartPosY;
				double volumeStartPosZ;
				float minValue;
				float maxValue;
				float volumeDefinitionAngle;
				bool volumeMerge;
			};


		public:

			/** Default Constructor. */
			CustomTiledReader();

			/** set use file name */
			virtual int setFilename(const SbString& filename);


			/** Standard reader stuf redefine */
			virtual SoVolumeReader::ReadError getDataChar(SbBox3f &size, SoVolumeData::DataType &type, SbVec3i32 &dim);
			virtual int getBorderFlag();
			virtual SbBool getTileSize(SbVec3i32 &size);
			virtual SbBool getMinMax(int & min, int & max);
			virtual SbBool getMinMax(double & min, double & max);
			virtual SbBool getMinMax(int64_t & min, int64_t & max);
			virtual SoBufferObject* readTile(int index, const SbBox3i32& tilePosition);
			virtual void getSubSlice(const SbBox2i32& subSlice, int sliceNumber, void * data);
			//virtual void getSubSlice(const SbBox2i32& subSlice, int sliceNumber, SoBufferObject * data);
			virtual SbBool isThreadSafe();
			virtual SbBool getHistogram(std::vector<int64_t>& histogram);

			const imt::volume::VolumeTopoOctree* topoTree() const;

			~CustomTiledReader();

		protected:

			SoLDMTopoOctree* m_topo;
			imt::volume::VolumeTopoOctree *_TopoTree;
			TilesHeader _Header;


			size_t _HeaderPos;

			SbBox3f m_size;
			SoVolumeData::DataType m_type;
			SbVec3i32 m_dim;
			SbVec3i32 m_tileDim;
			double m_min;
			double m_max;
			int m_overlap;
			size_t m_tileBytes;

			int64_t _Histogram[USHRT_MAX + 1];

			FILE* m_fp;

			std::vector< int > _LogicalToDiskMap;


		};




