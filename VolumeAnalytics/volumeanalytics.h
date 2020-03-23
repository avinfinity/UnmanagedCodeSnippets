#ifndef __IMT_VOLUME_VOLUMEANALYTICS_H__
#define __IMT_VOLUME_VOLUMEANALYTICS_H__

#include <string>
#include <vector>


#define VolumeAnalyticsExport   __declspec( dllexport )

namespace imt {

	namespace volume {


		class VolumeAnalyticsExport VolumeAnalytics
		{

		public:
			struct VolumeData
			{

				int64_t _Width, _Height, _Depth;

				double _VoxelSizeX, _VoxelSizeY, _VoxelSizeZ;

				unsigned short* _VolumeBuffer;

			};


			struct SurfaceMesh {

				std::vector<unsigned int> mSurfaceIndices;
				std::vector<float> mVertices, mVertexNormals;

			};

		protected:

			void resizeVolume_Uint16C1(unsigned short *inputVolume, int iWidth, int iHeight, int iDepth, unsigned short *outputVolume, int oWidth,
				int oHeight, int oDepth, double resizeRatio);


			void resizeVolume_Uint16C1_MT(unsigned short *inputVolume, int iWidth, int iHeight, int iDepth, unsigned short *outputVolume, int oWidth,
				int oHeight, int oDepth, double resizeRatio);

		public:

			VolumeAnalytics();

			void readVolumeData(std::string& path, VolumeData& volume);

			unsigned short computeIsoThreshold(VolumeData& volume);

			void generateMesh(const VolumeData& volume, int isoThreshold, SurfaceMesh& surfaceMesh);

			void reduceVolume(const VolumeData& inputVolume, VolumeData& targetVolume, double reductionCoeff);//each side is reduced by this coefficient




		};




	}



}





#endif


