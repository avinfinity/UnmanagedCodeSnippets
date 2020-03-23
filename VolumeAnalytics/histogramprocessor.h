
#include "opencvincludes.h"


namespace imt {

	namespace volume {



		class HistogramProcessor
		{


		public:


			HistogramProcessor();

			void apply();

			std::vector< int64_t > ApplyGaussianFilter(std::vector<int64_t>& histogram, int64_t k, double sigma, int64_t times = 1);

			std::vector< int64_t > ApplyFilter(std::vector< int64_t >& histogram, std::vector< double >& filterMask);


			int64_t OtsuThreshold(std::vector<int64_t>& histogram);

			int64_t ISO50Threshold(std::vector<int64_t>& histogram);

		protected:

			unsigned short airThreshold(cv::Mat& histogram);

			void ApplyMedianFilter(cv::Mat& histogram, cv::Mat& filteredHistogram, int64_t halfSizeFilter);

			int64_t OtsuThreshold(cv::Mat& histogram);

			void ApplyMedianFilter(std::vector<int64_t>& histogram, std::vector<int64_t>& filteredHistogram, int64_t halfSizeFilter);

			bool HasNextGradientSameSign(std::vector<int64_t>& histogram, double& currentGradient, int64_t& lastGrayValue, int64_t& currentValueIndex, int64_t step);

			bool CheckGradient(std::vector<int64_t>& histogram, int64_t step, int64_t& currentValueIndex, int64_t& lastGrayValue, double& lastGradient, bool& maximumReached);

			int64_t DetectMaximum(std::vector< int64_t >& histogram, int64_t step, int64_t startValue = 0);

			int64_t DetectLocalMaximum(std::vector<int64_t>& histogram, int64_t step, int64_t maximum);


		};




	}


}