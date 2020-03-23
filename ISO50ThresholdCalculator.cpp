#include "stdafx.h"
#include "ISO50ThresholdCalculator.h"
#include "src/plugins/Application/ZeissViewer/SegmentationInterface/ZeissSegmentationInterface.hpp"
#include "src/plugins/Application/ZeissViewer/SeparationInterface/ZeissSeparationInterface.hpp"
#include "limits"
#include "iostream"

#include "algorithm"


		ISO50ThresholdCalculator::ISO50ThresholdCalculator() 
		{



		}




		void ISO50ThresholdCalculator::ApplyMedianFilter(std::vector<int64_t>& histogram, std::vector<int64_t>& filteredHistogram, int64_t halfSizeFilter)
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
			std::vector< int64_t > ISO50ThresholdCalculatored(histogram.size());
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
				ISO50ThresholdCalculatored[i] = arrayForMedianCalculation[halfSizeFilter];
			}


			filteredHistogram = ISO50ThresholdCalculatored;

		}


		int64_t ISO50ThresholdCalculator::DetectMaximum(std::vector< int64_t >& histogram, int64_t step, int64_t startValue)
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

				double diff = currentGrayValue - startGrayValue;

				if (std::abs(diff) < (0.001 * highestGrayValue))
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



		bool ISO50ThresholdCalculator::CheckGradient(std::vector<int64_t>& histogram, int64_t step, int64_t& currentValueIndex, int64_t& lastGrayValue, double& lastGradient, bool& maximumReached)
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


		bool ISO50ThresholdCalculator::HasNextGradientSameSign(std::vector<int64_t>& histogram, double& currentGradient, int64_t& lastGrayValue, int64_t& currentValueIndex, int64_t step)
		{
			if (currentValueIndex + step / 10 < histogram.size())
			{
				double nextGradient = (1.0 * histogram[currentValueIndex + step / 10] - lastGrayValue) / step;
				if (nextGradient < 0.0 && currentGradient > 0.0 || nextGradient > 0.0 && currentGradient < 0.0)
					return false;
			}

			return true;
		}


		int64_t ISO50ThresholdCalculator::DetectLocalMaximum(std::vector<int64_t>& histogram, int64_t step, int64_t maximum)
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


		int64_t ISO50ThresholdCalculator::ISO50Threshold(std::vector<int64_t>& histogram)
		{
			if (histogram.size() == 0)
				return 22000;

			//First remove outliers (in height) by median filter
			std::vector< int64_t > ISO50ThresholdCalculatored;

			ApplyMedianFilter(histogram, ISO50ThresholdCalculatored, 3);

			//plotHistogram(ISO50ThresholdCalculatored);

			//Then filter the histogram with a gaussian kernel of 10 and sigma = 1.2            
			ISO50ThresholdCalculatored = ApplyGaussianFilter(ISO50ThresholdCalculatored, 10, 1.4, 10);

			//plotHistogram(ISO50ThresholdCalculatored);

			const int64_t step = 100;

			int64_t airPeak = DetectMaximum(ISO50ThresholdCalculatored, step);

			//std::cout << " air peak " << airPeak << std::endl;

			airPeak = DetectLocalMaximum(ISO50ThresholdCalculatored, step, airPeak);

			int64_t firstMaterialPeak = DetectMaximum(ISO50ThresholdCalculatored, step, airPeak);
			firstMaterialPeak = DetectLocalMaximum(ISO50ThresholdCalculatored, step, firstMaterialPeak);

			// weitere Untersuchungen für separierte Teile (künstlich mit Luft == 0 aufgefüllt):
			// weitere Suche nache einem Peak links von dem gefundenem "Luftpeak": 
			// Dafür wird nur noch 3/4 des Histogramms vom Luftpeak untersucht, dieser Peak soll mind. 1/5 so hoch sein als der gefundene Luftpeak
			int64_t partOfHistogram = airPeak / 4 * 3;
			if (partOfHistogram > 5000 && (histogram[0] > 1000000))
			{
				std::vector< int64_t > histogramToAirPeak(partOfHistogram);

				//Array.Copy(ISO50ThresholdCalculatored, 0, histogramToAirPeak, 0, partOfHistogram);

				std::copy(ISO50ThresholdCalculatored.begin(), ISO50ThresholdCalculatored.begin() + partOfHistogram, histogramToAirPeak.begin());


				int64_t peakLeft = DetectMaximum(histogramToAirPeak, step);
				peakLeft = DetectLocalMaximum(histogramToAirPeak, step, peakLeft);
				if (ISO50ThresholdCalculatored[peakLeft] > ISO50ThresholdCalculatored[airPeak] / 5)
				{
					firstMaterialPeak = airPeak;
					airPeak = peakLeft;
				}
			}


			return airPeak + (firstMaterialPeak - airPeak) / 2;
		}


		int64_t ISO50ThresholdCalculator::fraunhoufferThreshold( int w, int h, int d, double vx , double vy , double vz  , unsigned short* volume )
		{
			Volume vol;

			vol.size[0] = w;
			vol.size[1] = h;
			vol.size[2] = d;

			vol.voxel_size[0] = vx;
			vol.voxel_size[1] = vy;
			vol.voxel_size[2] = vz;

			vol.data = (uint16_t*)volume;

			ZeissSegmentationInterface segmenter;

			SegmentationParameter param;
			param.max_memory = static_cast<size_t>(2048) * 1024 * 1024;
			param.output_material_index = false;
			segmenter.setParameter(param);

			segmenter.setInputVolume(vol);

			Materials materials = segmenter.getMaterialRegions();

			return materials.regions[1].lower_bound;
		
		}


		/// <summary>
		/// Applies the Gaussian filter on a <paramref name="histogram"/> and returns the filtered histogram.
		/// </summary>
		/// <param name="histogram">the histogram to filter.</param>
		/// <param name="k">parameter k. Length of filtermask is 2k + 1.</param>
		/// <param name="sigma">sigma of the gaussian function</param>
		/// <param name="times">How many times should the filter be applied</param>
		/// <returns>the filtered histogram.</returns>
		std::vector< int64_t > ISO50ThresholdCalculator::ApplyGaussianFilter(std::vector<int64_t>& histogram, int64_t k, double sigma, int64_t times)
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

			std::vector< int64_t > ISO50ThresholdCalculatored = histogram;

			for (int64_t i = 0; i < times; i++)
			{
				ISO50ThresholdCalculatored = ApplyFilter(ISO50ThresholdCalculatored, filterMask);
			}

			return ISO50ThresholdCalculatored;
		}



		std::vector< int64_t > ISO50ThresholdCalculator::ApplyFilter(std::vector< int64_t >& histogram, std::vector< double >& filterMask)
		{
			//if (histogram == null)
			//	return null;

			std::vector< int64_t > ISO50ThresholdCalculatored(histogram.size());
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
				ISO50ThresholdCalculatored[i] = (int64_t)d;
			}

			return ISO50ThresholdCalculatored;
		}




		void ISO50ThresholdCalculator::createColorMap(unsigned int isoValue, std::vector< float >& colormap)
		{

			std::vector< ISO50ThresholdCalculator::MaterialRegion > materialRegions;
			std::vector<ISO50ThresholdCalculator::ColorWithOffset> colors;

			generateControlPoints(isoValue, 65535, materialRegions);
			GenerateTransferFunction(materialRegions, colors);

			colormap = CreateInterpolatedColorMap(colors);

		}


		void ISO50ThresholdCalculator::generateControlPoints(unsigned int isoValue, unsigned endValue, std::vector< MaterialRegion >& materialRegions)
		{

			MaterialRegion region1, region2, region3;

			region1._ControlPoints.push_back(ISO50ThresholdCalculator::ControlPoint(0, 0));
			region1._ControlPoints.push_back(ISO50ThresholdCalculator::ControlPoint(isoValue, 0));
			region1._Color.A = 0;
			region1._Color.R = 0;
			region1._Color.G = 0;
			region1._Color.B = 0;
			region1._Transparency = 0;

			region2._ControlPoints.push_back(ISO50ThresholdCalculator::ControlPoint(isoValue, 0));
			region2._ControlPoints.push_back(ISO50ThresholdCalculator::ControlPoint(endValue, 65535));
			region2._Color.A = 255;
			region2._Color.R = 255;
			region2._Color.G = 255;
			region2._Color.B = 255;
			region2._Transparency = 0;

			region3._ControlPoints.push_back(ISO50ThresholdCalculator::ControlPoint(isoValue, 0));
			region3._ControlPoints.push_back(ISO50ThresholdCalculator::ControlPoint(endValue, 65535));
			region3._Color.A = 255;
			region3._Color.R = 255;
			region3._Color.G = 255;
			region3._Color.B = 255;
			region3._Transparency = 0;

			materialRegions.push_back(region1);
			materialRegions.push_back(region2);
			materialRegions.push_back(region3);

		}




		void ISO50ThresholdCalculator::GenerateTransferFunction(std::vector< ISO50ThresholdCalculator::MaterialRegion >& materialRegions, std::vector< ISO50ThresholdCalculator::ColorWithOffset >& colors)
		{
			int numMaterials = materialRegions.size();

			for (int i = 0; i < numMaterials; i++)
			{
				auto& currentRegion = materialRegions[i];

				for (int j = 0; j < (currentRegion._ControlPoints.size() - 1); j++)
				{

					//std::cout << " i and j " << i << " " << j << std::endl;

					ISO50ThresholdCalculator::ControlPoint& currentControlPoint = currentRegion._ControlPoints[j];
					ISO50ThresholdCalculator::ControlPoint& nextControlPoint = currentRegion._ControlPoints[j + 1];

					double startPointOffset = (double)(currentControlPoint.X) / MAX_GRAYSCALE_VALUE;
					double endPointOffset = (double)(nextControlPoint.X) / MAX_GRAYSCALE_VALUE;

					double startOpacityValue = (double)(currentControlPoint.Y) / MAX_OPACITY_VALUE;
					double endOpacityValue = (double)(nextControlPoint.Y) / MAX_OPACITY_VALUE;

					double startAlphaValue = (currentRegion._Transparency > 0.0) ? 0.1*(1.0f - ((float)currentRegion._Transparency / 100.0))*startOpacityValue : startOpacityValue;
					double endAlphaValue = (currentRegion._Transparency > 0.0) ? 0.1*(1.0f - ((float)currentRegion._Transparency / 100.0))*endOpacityValue : endOpacityValue;

					unsigned char startR = (unsigned char)(int)(currentRegion._Color.R * startOpacityValue);
					unsigned char startG = (unsigned char)(int)(currentRegion._Color.G * startOpacityValue);
					unsigned char startB = (unsigned char)(int)(currentRegion._Color.B * startOpacityValue);
					unsigned char startAplha = (unsigned char)(int)(startAlphaValue*(COLOR_MAP_ENTRIES - 1));

					unsigned char endR = (unsigned char)(int)(currentRegion._Color.R * endOpacityValue);
					unsigned char endG = (unsigned char)(int)(currentRegion._Color.G * endOpacityValue);
					unsigned char endB = (unsigned char)(int)(currentRegion._Color.B * endOpacityValue);
					unsigned char endAplha = (unsigned char)(int)(endAlphaValue*(COLOR_MAP_ENTRIES - 1));

					ISO50ThresholdCalculator::ColorWithOffset cwo, cwoEnd;

					cwo.Offset = startPointOffset;
					cwo.R = startR;
					cwo.G = startG;
					cwo.B = startB;
					cwo.A = startAplha;

					cwoEnd.Offset = endPointOffset;
					cwoEnd.R = endR;
					cwoEnd.G = endG;
					cwoEnd.B = endB;
					cwoEnd.A = endAplha;

					colors.push_back(cwo);
					colors.push_back(cwoEnd);

				}
			}

		}



		std::vector<float> ISO50ThresholdCalculator::CreateInterpolatedColorMap(std::vector< ISO50ThresholdCalculator::ColorWithOffset >& colorList)
		{
			std::vector < float > colorMap(4 * COLOR_MAP_ENTRIES);
			//float colorMap = new float[4 * COLOR_MAP_ENTRIES];
			int numberOfColors = colorList.size();

			for (int index = 0; index < numberOfColors - 1; index++)
			{
				ISO50ThresholdCalculator::ColorWithOffset& colorThis = colorList[index];
				ISO50ThresholdCalculator::ColorWithOffset& colorNext = colorList[index + 1];

				int beginIndex = (int)(colorList[index].Offset * COLOR_MAP_ENTRIES);
				int endIndex = (int)(colorList[index + 1].Offset * COLOR_MAP_ENTRIES);

				for (int i = beginIndex; i < endIndex; i++)
				{
					double stepwidth = (i - beginIndex) / (double)(endIndex - beginIndex);
					colorMap[i * 4 + 0] = (float)((colorThis.R + (colorNext.R - colorThis.R) * stepwidth) / 255.0);
					colorMap[i * 4 + 1] = (float)((colorThis.G + (colorNext.G - colorThis.G) * stepwidth) / 255.0);
					colorMap[i * 4 + 2] = (float)((colorThis.B + (colorNext.B - colorThis.B) * stepwidth) / 255.0);
					colorMap[i * 4 + 3] = (float)((colorThis.A + (colorNext.A - colorThis.A) * stepwidth) / 255.0);
				}
			}

			return colorMap;
		}



