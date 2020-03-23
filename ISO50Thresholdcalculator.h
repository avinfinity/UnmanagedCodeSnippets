#ifndef __ISO50THRESHOLDCALCULATOR_H__
#define __ISO50THRESHOLDCALCULATOR_H__

#include "vector"

#define int64_t long long

#define MAX_GRAYSCALE_VALUE 65535
#define MAX_OPACITY_VALUE 65535
#define COLOR_MAP_ENTRIES 256

class ISO50ThresholdCalculator
{
	
	public:






		struct ColorWithOffset
		{

			float R, G, B, A;

			float Offset;

		};

		struct Color
		{
			float R, G, B, A;
		};


		struct ControlPoint
		{
			double X, Y;

			ControlPoint()
			{

				X = 0;
				Y = 0;
			}

			ControlPoint(double x, double y)
			{
				X = x;
				Y = y;
			}
		};


		struct MaterialRegion
		{
			std::vector< ControlPoint > _ControlPoints;

			double _Transparency;

			Color _Color;


		};

		void createColorMap(unsigned int isoValue, std::vector< float >& colormap);

		void generateControlPoints(unsigned int isoValue, unsigned endValue, std::vector< MaterialRegion >& materialRegions);

		void GenerateTransferFunction(std::vector< MaterialRegion >& materialRegions, std::vector< ColorWithOffset >& colors);

		std::vector<float> CreateInterpolatedColorMap(std::vector< ColorWithOffset >& colorList);


	
	ISO50ThresholdCalculator();
	
	long compute(std::vector<int64_t>& histogram);

	void ApplyMedianFilter(std::vector<int64_t>& histogram, std::vector<int64_t>& filteredHistogram, int64_t halfSizeFilter);

	int64_t DetectMaximum(std::vector< int64_t >& histogram, int64_t step, int64_t startValue = 0);

	bool CheckGradient(std::vector<int64_t>& histogram, int64_t step, int64_t& currentValueIndex, int64_t& lastGrayValue, double& lastGradient, bool& maximumReached);

	bool HasNextGradientSameSign(std::vector<int64_t>& histogram, double& currentGradient, int64_t& lastGrayValue, int64_t& currentValueIndex, int64_t step);

	int64_t DetectLocalMaximum(std::vector<int64_t>& histogram, int64_t step, int64_t maximum = 0);


	int64_t ISO50Threshold(std::vector<int64_t>& histogram);

	int64_t fraunhoufferThreshold(int w, int h, int d, double vx, double vy, double vz, unsigned short* volume);

	std::vector< int64_t > ApplyGaussianFilter(std::vector<int64_t>& histogram, int64_t k, double sigma, int64_t times);
	
	std::vector< int64_t > ApplyFilter(std::vector< int64_t >& histogram, std::vector< double >& filterMask);
	
	
};






#endif