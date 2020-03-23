
#include "iostream"

//#include "overlappedvoxelsegmentation.h"
#include "marchingcubesmultimaterial.h"
#include "volumeutility.h"
#include "rawvolumedataio.h"
#include "histogramfilter.h"
#include "display3droutines.h"

int main( int argc , char **argv )
{

	QString filePath = "C:/projects/Wallthickness/data/bottle/bottle.wtadat";//"C:/projects/Wallthickness/data/fanuc_engine_cover/engine_cover.wtadat";//	

	QString fileName = "G:\\Data\\Multimaterial\\T2_R2_BSC_M800_BHC_02_03";
		//"G:\\Projects\\Wallthickness\\data\\CT multi\\Hemanth_ Sugar free 2016-6-6 11-33"; //"C:/Projects/Wallthickness/data/separated_part_7";
	// "C:/Data/ZxoData/T2_R2_BSC_M800_BHC_02_03";//"C:/Data/WallthicknessData/10.03.17/Mobile cover_Static 2017-3-10 12-44";//"C:/Data/WallthicknessData/11.1.17/Bizerba_2 2016-8-25 17-50-31";//"C:/Data/WallthicknessData/11.1.17/Blister_Single 2016-10-19 8-55";// "C:/projects/Wallthickness/data/CT multi/Calliper Assy 2016-6-9 10-12";
	"G:/projects/Wallthickness/data/Datasets for Multi-material/MultiMaterial/MAR_Ref 2012-8-30 15-13"; 
	//"C:/projects/Wallthickness/data/Mobile charger/Wall Thick_RnD_1 2016-10-13 15-10_sep1";
	//"C:/projects/Wallthickness/data/Mobile charger/Wall Thick_RnD_1 2016-10-13 15-10_sep1"; 
	////""C:/projects/datasets/Bitron/separated_part_0";
	//"C:/projects/Wallthickness/data/CT multi/Hemanth_ Sugar free 2016-6-6 11-33";
	"G:/projects/Wallthickness/data/CT multi/Fuel_filter_0km_PR_1 2016-5-30 12-54";//;
	"C:/projects/Wallthickness/data/Mobile charger/Wall Thick_RnD_1 2016-10-13 15-10_sep1";
	//"C:/projects/datasets/Bitron/separated_part_7";
	//"C:/projects/datasets/bug_38950/Sample H wo frame_1 2012-9-10 6-27";//
	QString scvFilePath = fileName + ".uint16_scv";
	QString vgiFilePath = fileName + ".vgi";
	QString stlFilePath = "C:/projects/Wallthickness/data/Mobile _ Charger.stl";

	//viewSTLFile(stlFilePath);

	//return 0;

	int w, h, d;

	float voxelStep;

	imt::volume::VolumeInfo volume;

	//readVGI(vgiFilePath , w , h , d , voxelStep );



	//readUint16_SCV(scvFilePath , w , h , d , volume );

	imt::volume::RawVolumeDataIO::readUint16SCV(scvFilePath, volume);

	std::cout << volume.mWidth << " " << volume.mHeight << " " << volume.mDepth << " " << voxelStep << std::endl;

	//imt::volume::OverlappedVoxelSegmentation segmentation(volume);

	imt::volume::HistogramFilter hf(&volume);

	std::vector< long > histogram(std::numeric_limits< unsigned short >::max(), 0);

	unsigned short *vData = (unsigned short*)volume.mVolumeData;

	for (int zz = 0; zz <volume.mDepth; zz++)
		for (int yy = 0; yy < volume.mHeight; yy++)
			for (int xx = 0; xx < volume.mWidth; xx++)
			{
				histogram[vData[zz * volume.mWidth * volume.mHeight + yy * volume.mWidth + xx]] += 1;
			}

	auto thresholds = hf.fraunhoufferThresholds(volume.mWidth, volume.mHeight, volume.mDepth, volume.mVoxelStep(0),
		                     volume.mVoxelStep(1), volume.mVoxelStep(2), (unsigned short*)volume.mVolumeData);

	std::vector< std::pair< int, int> > isoThresholds;

	//segmentation.compute(isoThresholds);
	
	//std::cout << " iso labeling completed " << std::endl;

	std::cout << "fraunhoffer threshold : " << thresholds[1].first << std::endl;

	for (int ii = 0; ii < thresholds.size(); ii++)
	{
		std::cout << thresholds[ii].first << " " << thresholds[ii].second << std::endl;
	}


	//thresholds[1].first = (thresholds[1].first + thresholds[1].second) / 2;

	//tr::Display3DRoutines::displayHistogram(histogram, 0, 30000);

	imt::volume::MarchingCubesMultiMaterial mcmm(volume, thresholds);

	mcmm.compute(3);

	return 0;
}  