#ifndef __IMT_VOLUMEUTILITY__
#define __IMT_VOLUMEUTILITY__

#include "QString"
#include "volumeinfo.h"
#include <VolumeViz/nodes/SoVolumeData.h>

namespace imt{
	
	namespace volume{


		struct Material{

			int mLowerBound, mUpperBound;
			float mTransparency;
			bool mIsVisible, mIsHighlighted;

		};


		
class VolumeUtility
{
	
	public:


		class SobelGradientOperator3x3
		{
			int _KernelGx[3][3][3], _KernelGy[3][3][3], _KernelGz[3][3][3];

		public:


			SobelGradientOperator3x3();

			void init(size_t volumeW, size_t volumeH, size_t volumeD, double voxelSizeX,
				double voxelSizeY, double voxelSizeZ, unsigned short *volumeData);

			void apply(Eigen::Vector3f& point, Eigen::Vector3f& gradient);

		protected:


			size_t _VolumeW, _VolumeH, _VolumeD;

			double _VoxelSizeX, _VoxelSizeY, _VoxelSizeZ;

			unsigned short *_VolumeData;
		};


		struct VolumeDim
		{
			size_t _Width, _Height, _Depth;
		};

		struct Roi
		{
			size_t _Start[3];
			size_t _Dims[3];
		};

		struct VolumePosition
		{
			size_t pos[3];
		};
	
	
	static void loadVgiVolume( QString path , VolumeInfo& volume );

	static void computeISOThreshold( VolumeInfo& volume, int& isoThreshold, std::vector< __int64 >& histogram, int& minVal, int& maxVal);

	static void computeMaterials(SoVolumeData *volumeData, std::vector< Material >& materials);

	static void extractIsoSurface(VolumeInfo& volume, int& isoThreshold);
	

	static void volumeGradient(Eigen::Vector3f& position, imt::volume::VolumeInfo& volume, Eigen::Vector3f& gradient);

	static void volumeGradient(  std::vector<Eigen::Vector3f>& position, imt::volume::VolumeInfo& volume, std::vector<Eigen::Vector3f>& gradients);

	static void resizeVolume_Uint16C1(unsigned short *inputVolume, int iWidth, int iHeight, int iDepth, unsigned short *outputVolume, int oWidth,
		int oHeight, int oDepth, double resizeRatio);

	static void resizeVolume_Uint16C1_MT(unsigned short *inputVolume, int iWidth, int iHeight, int iDepth, unsigned short *outputVolume, int oWidth,
		int oHeight, int oDepth, double resizeRatio);


	void copyRoi( unsigned short *srcData, VolumeUtility::VolumeDim& srcDim, VolumeUtility::Roi& srcRoi, unsigned short *dstData,
		          VolumeUtility::VolumeDim& dstDim, VolumePosition& dstStartPos);
	
};		
		
		
		
		
		
		
		
		
	}
	
	
	
	
	
}





#endif