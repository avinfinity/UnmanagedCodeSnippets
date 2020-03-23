#include "histogramprocessor.h"
#include "limits"
#include "iostream"


namespace imt {

	namespace volume {

		HistogramProcessor::HistogramProcessor() 
		{



		}

		unsigned short HistogramProcessor::airThreshold(cv::Mat& histogram)
		{
			unsigned short airThreshold = 0;

			//Lets try median of two maxima first

			return airThreshold;
		}

		void HistogramProcessor::ApplyMedianFilter(cv::Mat& histogram, cv::Mat& filteredHistogram, int64_t halfSizeFilter)
		{

			int nRows = histogram.rows;
			int nCols = histogram.cols;

			cv::Mat histogramBeginEndWithZeros(nRows + 2 * halfSizeFilter, nCols, histogram.type());

			histogramBeginEndWithZeros.setTo(cv::Scalar(0));

			cv::Rect roi;

			roi.width = 1;
			roi.height = nRows;
			roi.x = 0;
			roi.y = halfSizeFilter;

			histogram.copyTo(histogramBeginEndWithZeros(roi));

			filteredHistogram.create(histogram.size(), histogram.type());

			//Mirror the end and the beginning of the new array...
			float *hbzData = (float*)histogramBeginEndWithZeros.data;
			float *fd = (float*)filteredHistogram.data;

			//Apply the median filter...

			std::vector< float > arr(2 * halfSizeFilter + 1, 0);

			for (int64_t i = 0; i < histogram.rows; i++)
			{
				//Collect the necessary informations...
				for (int k = -halfSizeFilter + i, jj = 0; k <= halfSizeFilter + i; k++, jj++)
				{
					//arrayForMedianCalculation[k] = histogramBeginEndWithZeros[i + k];

					arr[jj] = hbzData[k + halfSizeFilter];
				}

				std::sort(arr.begin(), arr.end());

				fd[i] = arr[halfSizeFilter];
			}

		}


		void HistogramProcessor::ApplyMedianFilter(std::vector<int64_t>& histogram, std::vector<int64_t>& filteredHistogram, int64_t halfSizeFilter)
		{
			//Mirror the end and the beginning of the new array...
			std::vector<int64_t> histogramBeginEndWithZeros(histogram.size() + 2 * halfSizeFilter);
			for (int64_t j = 0; j < halfSizeFilter; ++j)
			{
				histogramBeginEndWithZeros[j] = 0;
			}

			//histogram.CopyTo(histogramBeginEndWithZeros, halfSizeFilter);

			std::copy(histogram.begin(), histogram.end(), histogramBeginEndWithZeros.begin() + halfSizeFilter);

			for (int64_t j = halfSizeFilter; j < 2 * halfSizeFilter; ++j)
			{
				histogramBeginEndWithZeros[histogram.size() + j] = 0;
			}

			//Apply the median filter...
			std::vector< int64_t > histogramFiltered(histogram.size());
			std::vector< int64_t > arrayForMedianCalculation(2 * halfSizeFilter + 1);
			for (int64_t i = 0; i < histogram.size(); i++)
			{
				//Collect the necessary informations...
				for (int k = 0; k < arrayForMedianCalculation.size(); k++)
				{
					arrayForMedianCalculation[k] = histogramBeginEndWithZeros[i + k];
				}
				//System.Array.Sort(arrayForMedianCalculation);

				std::sort(arrayForMedianCalculation.begin(), arrayForMedianCalculation.end());
				histogramFiltered[i] = arrayForMedianCalculation[halfSizeFilter];
			}


			filteredHistogram = histogramFiltered;

		}




		int64_t HistogramProcessor::OtsuThreshold(cv::Mat& histogram)
		{
			if (histogram.cols == 0)
				return 22000;

			cv::Mat filteredHistogram;

			//plotHistogram(histogram);

			ApplyMedianFilter(histogram, filteredHistogram, 3);

			//plotHistogram(filteredHistogram);

			//filteredHistogram = histogram;

			//int64_t *data = ApplyMedianFilter(histogram, 1);
			//int64_t[] maxGrayValueIndices;
			//int64_t[] minGrayValueIndices;
			//BuildClasses(histogram, countClasses, out data, out maxGrayValueIndices, out minGrayValueIndices);

			// Otsu's threshold algorithm
			// 
			// 
			int64_t kStar;  // k = the current threshold; kStar = optimal threshold
			int64_t N1, N;    // N1 = # points with intensity <=k; N = total number of points
			double BCV, BCVmax; // The current Between Class Variance and maximum BCV
			double num, denom;  // temporary bookeeping
			int64_t Sk;  // The total intensity for all histogram points <=k
			int64_t S; // The total intensity of the image

			float *fd = (float*)filteredHistogram.data;

			// Initialize values:
			S = N = 0;
			for (int k = 0; k < histogram.rows; k++)
			{
				S += k * fd[k];	// Total histogram intensity
				N += fd[k];		// Total number of data points
			}

			Sk = 0;
			N1 = fd[0]; // The entry for zero intensity
			BCV = 0;
			BCVmax = 0;
			kStar = 0;

			// Look at each possible threshold value,
			// calculate the between-class variance, and decide if it's a max
			for (int k = 1; k < histogram.rows - 1; k++)
			{
				// No need to check endpoints k = 0 or k = L-1
				Sk += k * fd[k];
				N1 += fd[k];

				// The float casting here is to avoid compiler warning about loss of precision and
				// will prevent overflow in the case of large saturated images
				denom = (double)(N1)* (N - N1); // Maximum value of denom is (N^2)/4 =  approx. 3E10

				if (denom != 0)
				{
					// Float here is to avoid loss of precision when dividing
					num = ((double)N1 / N) * S - Sk; 	// Maximum value of num =  255*N = approx 8E7
					BCV = (num * num) / denom;
				}
				else
					BCV = 0;

				if (BCV >= BCVmax)
				{
					// Assign the best threshold found so far
					BCVmax = BCV;
					kStar = k;
				}
			}
			// kStar += 1;	// Use QTI convention that intensity -> 1 if intensity >= k
			// (the algorithm was developed for I-> 1 if I <= k.)

			//return minGrayValueIndices[kStar];
			return kStar;
		}


		int64_t HistogramProcessor::DetectMaximum(std::vector< int64_t >& histogram, int64_t step, int64_t startValue)
		{
			if (startValue >= histogram.size())
				return startValue;

			std::vector< int64_t > clonedHistogram(histogram.size());
			//Array.Copy(histogram, clonedHistogram, histogram.Length);

			clonedHistogram = histogram;
			//Array.Sort(clonedHistogram);

			std::sort(clonedHistogram.begin(), clonedHistogram.end());

			int64_t highestGrayValue = clonedHistogram[clonedHistogram.size() - 1];

			//std::cout << " highest gray value : " << highestGrayValue << std::endl;

			int64_t maxGrayValueIndex = startValue;
			int64_t maxGrayValue = 0;
			int64_t startGrayValue = histogram[startValue];
			int64_t lastGrayValue = -1;
			double lastGradient = -1.0;
			for (int64_t nGrayValue = startValue; nGrayValue < histogram.size(); nGrayValue += step)
			{
				int64_t currentGrayValue = histogram[nGrayValue];

				if (std::abs(currentGrayValue - startGrayValue) < (0.001 * highestGrayValue))
					continue;



				bool maximumReached;
				if (!CheckGradient(histogram, step, nGrayValue, lastGrayValue, lastGradient, maximumReached))
				{
					if (maximumReached)
						return nGrayValue - step / 2;

					continue;
				}

				//std::cout << " after gradient " << std::endl;

				if (maxGrayValue >= currentGrayValue)
					continue;

				//std::cout << nGrayValue << std::endl;

				maxGrayValue = currentGrayValue;
				maxGrayValueIndex = nGrayValue;
			}

			//std::cout << " max gray value index : " << maxGrayValueIndex << std::endl;

			return maxGrayValueIndex;
		}



		bool HistogramProcessor::CheckGradient(std::vector<int64_t>& histogram, int64_t step, int64_t& currentValueIndex, int64_t& lastGrayValue, double& lastGradient, bool& maximumReached)
		{

			//std::cout << " current value index "<<currentValueIndex << std::endl;

			maximumReached = false;
			int64_t currentGrayValue = histogram[currentValueIndex];
			double gradient = -1.0;

			//std::cout << " last gray value : " << lastGrayValue << std::endl;
			if (lastGrayValue > 0)
			{
				gradient = (1.0 * currentGrayValue - lastGrayValue) / step;

				//std::cout << " gradient : " << gradient << std::endl;

				if (!HasNextGradientSameSign(histogram, gradient, lastGrayValue, currentValueIndex, step))
					return false;

				if (gradient < -1.0 && lastGradient > 1.0)
				{
					maximumReached = true;
					return false;
				}

				lastGradient = gradient;
			}

			lastGrayValue = currentGrayValue;



			if (gradient < 0.0)
			{
				//std::cout << " negative gradient : " << gradient << std::endl;

				return false;
			}

			return true;
		}


		bool HistogramProcessor::HasNextGradientSameSign(std::vector<int64_t>& histogram, double& currentGradient, int64_t& lastGrayValue, int64_t& currentValueIndex, int64_t step)
		{
			if (currentValueIndex + step / 10 < histogram.size())
			{
				double nextGradient = (1.0 * histogram[currentValueIndex + step / 10] - lastGrayValue) / step;
				if (nextGradient < 0.0 && currentGradient > 0.0 || nextGradient > 0.0 && currentGradient < 0.0)
					return false;
			}

			return true;
		}


		int64_t HistogramProcessor::DetectLocalMaximum(std::vector<int64_t>& histogram, int64_t step, int64_t maximum)
		{
			int64_t startValue = std::max((int64_t)0, maximum - step);
			int64_t stopValue = std::min((int64_t)histogram.size(), maximum + step);
			int64_t maxGrayValueIndex = maximum;
			int64_t maxGrayValue = histogram[maximum];
			for (int64_t nGrayValue = startValue; nGrayValue < stopValue; nGrayValue++)
			{
				int64_t currentGrayValue = histogram[nGrayValue];
				if (maxGrayValue >= currentGrayValue)
					continue;

				maxGrayValue = currentGrayValue;
				maxGrayValueIndex = nGrayValue;
			}

			return maxGrayValueIndex;
		}


		int64_t HistogramProcessor::OtsuThreshold(std::vector<int64_t>& histogram)
		{
			if (histogram.size() == 0)
				return 22000;

			std::vector< int64_t > filteredHistogram;

			//plotHistogram(histogram);

			ApplyMedianFilter(histogram, filteredHistogram, 3);

			//plotHistogram(filteredHistogram);

			//filteredHistogram = histogram;

			//int64_t *data = ApplyMedianFilter(histogram, 1);
			//int64_t[] maxGrayValueIndices;
			//int64_t[] minGrayValueIndices;
			//BuildClasses(histogram, countClasses, out data, out maxGrayValueIndices, out minGrayValueIndices);

			// Otsu's threshold algorithm
			// 
			// 
			int64_t kStar;  // k = the current threshold; kStar = optimal threshold
			int64_t N1, N;    // N1 = # points with intensity <=k; N = total number of points
			double BCV, BCVmax; // The current Between Class Variance and maximum BCV
			double num, denom;  // temporary bookeeping
			int64_t Sk;  // The total intensity for all histogram points <=k
			int64_t S; // The total intensity of the image

					   //float *fd = (float*)filteredHistogram.data;

					   // Initialize values:
			S = N = 0;
			for (int k = 0; k < histogram.size(); k++)
			{
				S += k * filteredHistogram[k];	// Total histogram intensity
				N += filteredHistogram[k];		// Total number of data points
			}

			Sk = 0;
			N1 = filteredHistogram[0]; // The entry for zero intensity
			BCV = 0;
			BCVmax = 0;
			kStar = 0;

			// Look at each possible threshold value,
			// calculate the between-class variance, and decide if it's a max
			for (int k = 1; k < histogram.size() - 1; k++)
			{
				// No need to check endpoints k = 0 or k = L-1
				Sk += k * filteredHistogram[k];
				N1 += filteredHistogram[k];

				// The float casting here is to avoid compiler warning about loss of precision and
				// will prevent overflow in the case of large saturated images
				denom = (double)(N1)* (N - N1); // Maximum value of denom is (N^2)/4 =  approx. 3E10

				if (denom != 0)
				{
					// Float here is to avoid loss of precision when dividing
					num = ((double)N1 / N) * S - Sk; 	// Maximum value of num =  255*N = approx 8E7
					BCV = (num * num) / denom;
				}
				else
					BCV = 0;

				if (BCV >= BCVmax)
				{
					// Assign the best threshold found so far
					BCVmax = BCV;
					kStar = k;
				}
			}
			// kStar += 1;	// Use QTI convention that intensity -> 1 if intensity >= k
			// (the algorithm was developed for I-> 1 if I <= k.)

			//return minGrayValueIndices[kStar];
			return kStar;
		}


		int64_t HistogramProcessor::ISO50Threshold(std::vector<int64_t>& histogram)
		{
			if (histogram.size() == 0)
				return 22000;

			//First remove outliers (in height) by median filter
			std::vector< int64_t > histogramFiltered;

			ApplyMedianFilter(histogram, histogramFiltered, 3);

			//plotHistogram(histogramFiltered);

			//Then filter the histogram with a gaussian kernel of 10 and sigma = 1.2            
			histogramFiltered = ApplyGaussianFilter(histogramFiltered, 10, 1.4, 10);

			//plotHistogram(histogramFiltered);

			const int64_t step = 100;

			int64_t airPeak = DetectMaximum(histogramFiltered, step);

			//std::cout << " air peak " << airPeak << std::endl;

			airPeak = DetectLocalMaximum(histogramFiltered, step, airPeak);

			int64_t firstMaterialPeak = DetectMaximum(histogramFiltered, step, airPeak);
			firstMaterialPeak = DetectLocalMaximum(histogramFiltered, step, firstMaterialPeak);

			// weitere Untersuchungen für separierte Teile (künstlich mit Luft == 0 aufgefüllt):
			// weitere Suche nache einem Peak links von dem gefundenem "Luftpeak": 
			// Dafür wird nur noch 3/4 des Histogramms vom Luftpeak untersucht, dieser Peak soll mind. 1/5 so hoch sein als der gefundene Luftpeak
			int64_t partOfHistogram = airPeak / 4 * 3;
			if (partOfHistogram > 5000 && (histogram[0] > 1000000))
			{
				std::vector< int64_t > histogramToAirPeak(partOfHistogram);

				//Array.Copy(histogramFiltered, 0, histogramToAirPeak, 0, partOfHistogram);

				std::copy(histogramFiltered.begin(), histogramFiltered.begin() + partOfHistogram, histogramToAirPeak.begin());


				int64_t peakLeft = DetectMaximum(histogramToAirPeak, step);
				peakLeft = DetectLocalMaximum(histogramToAirPeak, step, peakLeft);
				if (histogramFiltered[peakLeft] > histogramFiltered[airPeak] / 5)
				{
					firstMaterialPeak = airPeak;
					airPeak = peakLeft;
				}
			}


			return airPeak + (firstMaterialPeak - airPeak) / 2;
		}



		/// <summary>
		/// Applies the Gaussian filter on a <paramref name="histogram"/> and returns the filtered histogram.
		/// </summary>
		/// <param name="histogram">the histogram to filter.</param>
		/// <param name="k">parameter k. Length of filtermask is 2k + 1.</param>
		/// <param name="sigma">sigma of the gaussian function</param>
		/// <param name="times">How many times should the filter be applied</param>
		/// <returns>the filtered histogram.</returns>
		std::vector< int64_t > HistogramProcessor::ApplyGaussianFilter(std::vector<int64_t>& histogram, int64_t k, double sigma, int64_t times)
		{


			//if (times <= 0)
			//{
			//	return (int64_t[])histogram.Clone();
			//}

			std::vector< double > filterMask(2 * k + 1);
			filterMask[0] = filterMask[2 * k] = 1;
			double denominator = 0.5 / (sigma * sigma);

			int filterLength = 2 * k + 1;

			double g_k = std::exp(-k * k * denominator);

			for (int64_t i = 0; i < k; i++)
			{
				double g_i = std::exp(-i * i * denominator);
				filterMask[k + i] = filterMask[k - i] = (int64_t)(g_i / g_k);
			}
			double normFact = 0;
			for (int64_t i = 0; i < filterLength; i++)
			{
				normFact += filterMask[i];
			}

			for (int64_t i = 0; i < filterLength; i++)
			{
				filterMask[i] = filterMask[i] / normFact;
			}

			std::vector< int64_t > histogramFiltered = histogram;

			for (int64_t i = 0; i < times; i++)
			{
				histogramFiltered = ApplyFilter(histogramFiltered, filterMask);
			}

			return histogramFiltered;
		}



		std::vector< int64_t >HistogramProcessor::ApplyFilter(std::vector< int64_t >& histogram, std::vector< double >& filterMask)
		{
			//if (histogram == null)
			//	return null;

			std::vector< int64_t > histogramFiltered(histogram.size());
			int halfSizeFilter = (filterMask.size() - 1) / 2;
			std::vector< int64_t > histogramBeginEndMirrored(histogram.size() + 2 * halfSizeFilter);
			for (int j = 0; j < halfSizeFilter; ++j)
			{
				histogramBeginEndMirrored[j] = histogram[halfSizeFilter - j];
			}

			//histogram.CopyTo(histogramBeginEndMirrored, halfSizeFilter);

			std::copy(histogram.begin(), histogram.end(), histogramBeginEndMirrored.begin() + halfSizeFilter);

			for (int j = halfSizeFilter; j < 2 * halfSizeFilter; ++j)
			{
				histogramBeginEndMirrored[histogram.size() + j] = histogram[histogram.size() - j + halfSizeFilter - 1];
			}

			for (int j = halfSizeFilter, i = 0; i < histogram.size(); ++i, ++j)
			{
				double d = 0.0;
				for (int k = 0; k < filterMask.size(); ++k)
				{
					d += histogramBeginEndMirrored[j - halfSizeFilter + k] * filterMask[k];
				}
				histogramFiltered[i] = (int64_t)d;
			}

			return histogramFiltered;
		}


	}


}