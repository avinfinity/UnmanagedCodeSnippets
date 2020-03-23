
#include "volumeinfo.h"
#include "opencvincludes.h"


namespace imt{
	
	namespace volume{
		
		
		
class HistogramFilter
{
	
	
	public:


	HistogramFilter( VolumeInfo *volumeInfo );

	void apply();

	std::vector< int64_t > ApplyGaussianFilter(std::vector<int64_t>& histogram, int64_t k, double sigma, int64_t times = 1);

	std::vector< int64_t > ApplyFilter(std::vector< int64_t >& histogram, std::vector< double >& filterMask);


	int64_t OtsuThreshold(std::vector<int64_t>& histogram);

	int64_t ISO50Threshold(std::vector<int64_t>& histogram);

	void plotHistogram(std::vector< int64_t >& histogram);

	void plotHistogramVTK(std::vector< int64_t >& histogram);

	int64_t fraunhoufferThreshold(int w, int h, int d, double vx, double vy, double vz, unsigned short* volume);
	
	std::vector<std::pair<int, int>> fraunhoufferThresholds(int w, int h, int d, double vx, double vy, double vz, unsigned short* volume);

protected:
	
	void plotHistogram( cv::Mat& histogram );

	unsigned short airThreshold( cv::Mat& histogram );

	void ApplyMedianFilter( cv::Mat& histogram , cv::Mat& filteredHistogram, int64_t halfSizeFilter );

	int64_t OtsuThreshold( cv::Mat& histogram );

	void ApplyMedianFilter(std::vector<int64_t>& histogram, std::vector<int64_t>& filteredHistogram, int64_t halfSizeFilter);

	bool HasNextGradientSameSign(std::vector<int64_t>& histogram, double& currentGradient, int64_t& lastGrayValue, int64_t& currentValueIndex, int64_t step);

	bool CheckGradient(std::vector<int64_t>& histogram, int64_t step, int64_t& currentValueIndex, int64_t& lastGrayValue, double& lastGradient, bool& maximumReached);

	int64_t DetectMaximum(std::vector< int64_t >& histogram, int64_t step, int64_t startValue = 0);

	int64_t DetectLocalMaximum(std::vector<int64_t>& histogram, int64_t step, int64_t maximum);

	

	protected:
	
	
	VolumeInfo *mVolumeInfo;
	
	
	
};




}


}