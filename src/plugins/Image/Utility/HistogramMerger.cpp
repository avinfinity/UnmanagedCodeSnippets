//****************************************************************************
// (c) 2012 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
#include <openOR/Image/HistogramMerger.hpp>

#include <openOR/cleanUpWindowsMacros.hpp>

namespace openOR {
	namespace Image {

		HistogramMerger::HistogramMerger():m_shiftCorrections(new std::vector<int>())
		{
		}

		HistogramMerger::~HistogramMerger() {
		}

		std::vector<int> HistogramMerger::getShiftCorrections() const {
			return *m_shiftCorrections;
		}

		void HistogramMerger::operator()() const {
			// - create copies of m_pHistogram and m_pRegions
			// - merge density intervals
			// - write intervals to m_pResultRegions


			if (!m_pHistogram || m_pRegions.size() == 0 || !m_pResultRegions) {
				// TODO: error
				return;
			}

			//make local copies of m_pHistogram 
			size_t histSize = getWidth(m_pHistogram);
			boost::shared_array<long long> pHistogram = boost::shared_array<long long>(new long long[histSize]);
			for (int i = 0; i < (int)histSize; i++) {
				pHistogram[i] = m_pHistogram->data()[i];
			}

			HistogramMergerImpl impl;

			//create copy of m_pRegions
			std::vector<std::vector<Math::Vector3ui> > regions;
			for (int i = 0; i < (int)m_pRegions.size(); i++) {
				std::vector<Math::Vector3ui> element;
				for (int j = 0; j < (int)getWidth(m_pRegions[i]); j++) {
					Triple<size_t> val = m_pRegions[i]->data()[j];
					Math::Vector3ui newVal = Math::create<Math::Vector3ui>(val.first, val.second, val.third);
					element.push_back(newVal);
				}
				regions.push_back(element);
			}

			//merge density intervals and write them to regions
			std::vector<Math::Vector3ui> intervalls = impl.mergeDensityIntervals(pHistogram, histSize, regions);
			m_pResultRegions->setSize(intervalls.size());
			regions.push_back(intervalls);

			//calculate the interval shifts
			m_shiftCorrections->clear();
			*m_shiftCorrections = impl.computeDensityIntervalShifts(regions);

			for (int i = 0; i < (int)intervalls.size(); i++) {
				Math::Vector3ui v = intervalls[i]; //TODO: Variablen Namen
				m_pResultRegions->mutableData()[i].first = v[0];
				m_pResultRegions->mutableData()[i].second = v[1];
				m_pResultRegions->mutableData()[i].third = v[2];
			}
		}

		void HistogramMerger::setData(const AnyPtr& data, const std::string& name) {
			std::tr1::shared_ptr<openOR::Image::Image1DDataUI64> pHistogram = interface_cast<openOR::Image::Image1DDataUI64>(data);
			if (pHistogram) {
				m_pHistogram = pHistogram;
			}
			std::tr1::shared_ptr<openOR::Image::Image1DData<openOR::Triple<size_t> > > pRegions = interface_cast<openOR::Image::Image1DData<openOR::Triple<size_t> > >(data);
			if (pRegions) {
				if (name == "in") m_pRegions.push_back(pRegions);
				else m_pResultRegions = pRegions;
			}
		}

		HistogramMerger::HistogramMergerImpl::HistogramMergerImpl()
		{
			m_fMinNumOfVoxelInSection = 100 * 0.0125 * 0.0125 * 0.0125;
		}

		HistogramMerger::HistogramMergerImpl::~HistogramMergerImpl() {}


		std::vector<Math::Vector3ui> HistogramMerger::HistogramMergerImpl::mergeDensityIntervals(boost::shared_array<long long>& Histogram, uint nHistogramDataSize, std::vector<std::vector<Math::Vector3ui> >& vecDensityIntervals)
		{
			// - if all intervals are equal do no merging
			// - else merge the intervals and subsequently check if merging was successfull
			// - return the merged interval

			if (vecDensityIntervals.size()>1)
			{
				uint nThreshold = nHistogramDataSize / 200;
				bool bIntervalsSimilarityOK = true;
				for (uint i=1; i<vecDensityIntervals.size();++i)
				{
					// if all density intervals are equal then do nothing
					if (!areTwoDensityIntervalsEqual(vecDensityIntervals.at(i-1), vecDensityIntervals.at(i),nThreshold))
					{
						bIntervalsSimilarityOK = false;
						break;
					}  
				}

				if (bIntervalsSimilarityOK)
				{
					return vecDensityIntervals.at(0);
				} 
				else // if density intervals are not equal:
				{  
					try         {
						// merge density intervals
						std::vector<Math::Vector3ui> localResultDensityIntervals = vecDensityIntervals.at(vecDensityIntervals.size()-1);
						for (int i=vecDensityIntervals.size()-2; i>=0;--i)
						{
							if(!mergeTwoDensityIntervals(Histogram, nHistogramDataSize,vecDensityIntervals.at(i), localResultDensityIntervals, localResultDensityIntervals))
							{
								return vecDensityIntervals.at(0);
							}
						}
						//check plausibility
						bool bPlausible = wasMergingSuccessful(vecDensityIntervals,localResultDensityIntervals,nThreshold);
						if (bPlausible){
							return localResultDensityIntervals;
						}else{
							//std::cout << "RegionMerger: plausibility check failed" << std::endl; TODO: Fehlermeldung
						}
						return vecDensityIntervals.at(0);
					}
					catch (...){           
						return vecDensityIntervals.at(0);
					}                  
				}
			}
			else
			{
				if (vecDensityIntervals.size()>0) return vecDensityIntervals.at(0);
			}
			std::vector<Math::Vector3ui> vecResults;
			return vecResults;
		}

		bool HistogramMerger::HistogramMergerImpl::mergeTwoDensityIntervals(boost::shared_array<long long>& Histogram, uint nHistogramDataSize,std::vector<Math::Vector3ui> vecFirstDensityInterval, std::vector<Math::Vector3ui> vecSecondDensityInterval,std::vector<Math::Vector3ui>& resultDensityIntervals)
		{
			// - merge first and second density interval and copy them to the result vector:
			//		. find equal regions in the intervals
			//		- compare the regionsb regarding the parameters: peak, begin index and end index
			//		- if any of those parameters in the two intervals differ more than the threshold, merge the intervals according to the different merging functions 
			//			(explanation below in the merging functions part)
			//		- add the remaining intervals


			// if one interval is empty we are done
			if (vecFirstDensityInterval.size()==0)
			{
				resultDensityIntervals = vecFirstDensityInterval;
				return true;
			}
			if (vecSecondDensityInterval.size()==0)
			{
				resultDensityIntervals = vecSecondDensityInterval;
				return true;
			}

			//calc histogram scaling
			double fShift = 0;
			long long nNumOfVoxelThreshold = getNumOfVoxel(Histogram,nHistogramDataSize) * m_fMinNumOfVoxelInSection;
			int nShiftCorrectionThreshold = nHistogramDataSize / 100;//TODO: an empirical value...
			int nMinPeakToPeakDistance = nHistogramDataSize/75;         
			//int nMinPeakToPeakDistance = nHistogramDataSize/150;

			//if both intervals are equal we are done
			resultDensityIntervals.clear();
			if (areTwoDensityIntervalsEqual(vecFirstDensityInterval,vecSecondDensityInterval,nShiftCorrectionThreshold))
			{
				resultDensityIntervals = vecFirstDensityInterval;
				return true;
			}
			else
			{
				// compute max scaling shift
				fShift = computeDensityIntervalShift(vecFirstDensityInterval, vecSecondDensityInterval, nShiftCorrectionThreshold);
				//nShiftThreshold = std::max<int>(nShiftThreshold - abs(fShift),0);
				correctIntervalShift(vecSecondDensityInterval,fShift);
				int nShiftThreshold = nHistogramDataSize / 200;//TODO: an empirical value...

				// find equal regions and copy them into the result vector
				uint nFirstIndex = 0; 
				uint nSecondIndex = 0;
				while(nFirstIndex <vecFirstDensityInterval.size() && nSecondIndex <vecSecondDensityInterval.size())
				{
					if(!hasRegionEnoughVoxels(Histogram, nHistogramDataSize,vecSecondDensityInterval.at(nSecondIndex),nNumOfVoxelThreshold))
					{
						++nSecondIndex;
						continue;
					}
					if(!hasRegionEnoughVoxels(Histogram, nHistogramDataSize,vecFirstDensityInterval.at(nFirstIndex),nNumOfVoxelThreshold))
					{
						++nFirstIndex;
						continue;
					}
					//peak minimum distances check
					//increase the index as long as the distance between first (second) interval peak and the last resultInterval peak is smaller than a factor 
					if (resultDensityIntervals.size()>0 && abs(static_cast<int>(vecFirstDensityInterval.at(nFirstIndex)(1))-static_cast<int>(resultDensityIntervals.at(resultDensityIntervals.size()-1)(1)))<nMinPeakToPeakDistance){
						nFirstIndex++;
						continue;
					}
					if (resultDensityIntervals.size()>0 && abs(static_cast<int>(vecSecondDensityInterval.at(nSecondIndex)(1))-static_cast<int>(resultDensityIntervals.at(resultDensityIntervals.size()-1)(1)))<nMinPeakToPeakDistance){
						nSecondIndex++;
						continue;
					}

					// if interval begin indices are relativly the same:
					if (abs(static_cast<int>(vecFirstDensityInterval.at(nFirstIndex)(0))-static_cast<int>(vecSecondDensityInterval.at(nSecondIndex)(0)))<nShiftThreshold)
					{
						// if peaks are relatively the same:
						if (abs(static_cast<int>(vecFirstDensityInterval.at(nFirstIndex)(1))-static_cast<int>(vecSecondDensityInterval.at(nSecondIndex)(1)))<nShiftThreshold)
						{
							//if the end index of the intervals are relatively the same
							if (abs(static_cast<int>(vecFirstDensityInterval.at(nFirstIndex)(2))-static_cast<int>(vecSecondDensityInterval.at(nSecondIndex)(2)))<nShiftThreshold){
								// all region properties are equal enough => copy into the result vector
								mergeTwoDensityIntervals_XYZ(Histogram,nHistogramDataSize,vecFirstDensityInterval,vecSecondDensityInterval,resultDensityIntervals,nFirstIndex,nSecondIndex,nShiftThreshold); continue;                  
							}
							else // the end index seems to be different
							{                  
								mergeTwoDensityIntervals_XYnZ(Histogram,nHistogramDataSize,vecFirstDensityInterval,vecSecondDensityInterval,resultDensityIntervals,nFirstIndex,nSecondIndex,nShiftThreshold); continue;                  
							}
						}
						// peaks seem to have different heights
						else
						{
							// simple peak mismatch?
							if (abs(static_cast<int>(vecFirstDensityInterval.at(nFirstIndex)(2))-static_cast<int>(vecSecondDensityInterval.at(nSecondIndex)(2)))<nShiftThreshold){
								// all region properties are equal enough => copy into the result vector
								mergeTwoDensityIntervals_XnYZ(Histogram,nHistogramDataSize,vecFirstDensityInterval,vecSecondDensityInterval,resultDensityIntervals,nFirstIndex,nSecondIndex,nShiftThreshold);continue;
							}
							//or total mismatch?
							else
							{
								mergeTwoDensityIntervals_XnYnZ(Histogram,nHistogramDataSize,vecFirstDensityInterval,vecSecondDensityInterval,resultDensityIntervals,nFirstIndex,nSecondIndex,nShiftThreshold);continue;
							}
						}
					}
					// interval begin indices seem to be different:
					else
					{
						// if peaks are relatively the same height:
						if (abs(static_cast<int>(vecFirstDensityInterval.at(nFirstIndex)(1))-static_cast<int>(vecSecondDensityInterval.at(nSecondIndex)(1)))<nShiftThreshold)
						{
							//if the end indices of the intervals are relatively the same
							if (abs(static_cast<int>(vecFirstDensityInterval.at(nFirstIndex)(2))-static_cast<int>(vecSecondDensityInterval.at(nSecondIndex)(2)))<nShiftThreshold)
							{
								mergeTwoDensityIntervals_nXYZ(Histogram,nHistogramDataSize,vecFirstDensityInterval,vecSecondDensityInterval,resultDensityIntervals,nFirstIndex,nSecondIndex,nShiftThreshold);continue;
							}
							else // end indices seem to be different
							{
								mergeTwoDensityIntervals_nXYnZ(Histogram,nHistogramDataSize,vecFirstDensityInterval,vecSecondDensityInterval,resultDensityIntervals,nFirstIndex,nSecondIndex,nShiftThreshold);continue;
							}
						}
						else // peaks seem to be different
						{
							//if end indices are relatively the same
							if (abs(static_cast<int>(vecFirstDensityInterval.at(nFirstIndex)(2))-static_cast<int>(vecSecondDensityInterval.at(nSecondIndex)(2)))<nShiftThreshold)
							{
								mergeTwoDensityIntervals_nXnYZ(Histogram,nHistogramDataSize,vecFirstDensityInterval,vecSecondDensityInterval,resultDensityIntervals,nFirstIndex,nSecondIndex,nShiftThreshold);continue;
							}
							else // end indices seem to be different
							{
								mergeTwoDensityIntervals_nXnYnZ(Histogram,nHistogramDataSize,vecFirstDensityInterval,vecSecondDensityInterval,resultDensityIntervals,nFirstIndex,nSecondIndex,nShiftThreshold);continue;
							}
						}
					}
				}
				// add the remaining intervals for the first interval
				if (nFirstIndex<=vecFirstDensityInterval.size()-1)
				{
					bool bFirstDensityIntervalEnd = resultDensityIntervals.at(resultDensityIntervals.size()-1)(2)-nShiftThreshold-abs(fShift)<=vecFirstDensityInterval.at(nFirstIndex)(1);         
					if (bFirstDensityIntervalEnd)
					{
						for (uint n = nFirstIndex;n<vecFirstDensityInterval.size();++n)
						{
							if(hasRegionEnoughVoxels(Histogram, nHistogramDataSize,vecFirstDensityInterval.at(n),nNumOfVoxelThreshold))
							{
								resultDensityIntervals.push_back(vecFirstDensityInterval.at(n));
							}
						}
					}      
				}
				// add the remaining intervals for the second interval
				if (nSecondIndex<=vecSecondDensityInterval.size()-1)
				{
					bool bSecondDensityIntervalEnd = resultDensityIntervals.at(resultDensityIntervals.size()-1)(2)-nShiftThreshold-abs(fShift)<=vecSecondDensityInterval.at(nSecondIndex)(1);      
					if (bSecondDensityIntervalEnd)
					{
						for (uint n = nSecondIndex;n<vecSecondDensityInterval.size();++n)
						{
							if(hasRegionEnoughVoxels(Histogram, nHistogramDataSize,vecSecondDensityInterval.at(n),nNumOfVoxelThreshold))
							{
								resultDensityIntervals.push_back(vecSecondDensityInterval.at(n));
							}
						}
					}

				}    
				return true;
			}
			return false;
		}

		bool HistogramMerger::HistogramMergerImpl::areTwoDensityIntervalsEqual(std::vector<Math::Vector3ui> vecFirstDensityInterval, std::vector<Math::Vector3ui> vecSecondDensityInterval,uint nThreshold)
		{
			// - Check if intervals have the same size and start and begin indices are the same


			// have the vectors the same size
			if (vecFirstDensityInterval.size()!=vecSecondDensityInterval.size())
			{
				return false;
			}
			// are the intervals even?
			for (uint k=0; k<vecFirstDensityInterval.size();++k)
			{
				if (abs(static_cast<int>(vecFirstDensityInterval.at(k)(0))-static_cast<int>(vecSecondDensityInterval.at(k)(0)))>nThreshold)
				{
					return false;
				}
				if (abs(static_cast<int>(vecFirstDensityInterval.at(k)(2))-static_cast<int>(vecSecondDensityInterval.at(k)(2)))>nThreshold)
				{
					return false;
				}
			}
			return true;
		}

		bool HistogramMerger::HistogramMergerImpl::hasRegionEnoughVoxels(boost::shared_array<long long>& Histogram, uint nHistogramDataSize,Math::Vector3ui& vecRegion, long long nThreshold)
		{
			// - check if the interval has more than nThreshold Voxels

			if(vecRegion(2)>(nHistogramDataSize-1)) return false;
			long long nNumVoxel = 0;
			for (uint i=vecRegion(0);i<=vecRegion(2);++i)
			{
				nNumVoxel += Histogram[i];
			}
			if (nNumVoxel>nThreshold)
			{
				return true;
			}
			else
			{   
				return false;
			}
		}

		double HistogramMerger::HistogramMergerImpl::computeDensityIntervalShift(std::vector<Math::Vector3ui>& vecFirstDensityInterval, std::vector<Math::Vector3ui>& vecSecondDensityInterval,uint nThreshold)
		{
			// - shift both intervls until their peaks match in the best way
			// - if we cant find matching peaks the shift is set to zero
			// - return the shift value

			double fShift = std::numeric_limits<double>::max();
			int nDiff;
			bool bBegin;
			bool bEnd;

			// if there is only one or two intervals no threshold is needed, because there mustn't be a shift!
			if (vecFirstDensityInterval.size()<=2 || vecSecondDensityInterval.size() <= 2){
				nThreshold = std::numeric_limits<uint>::max();
			}

			for (uint k=0; k<vecFirstDensityInterval.size();++k)
			{
				for (uint j=0; j<vecSecondDensityInterval.size();++j)
				{
					//shift both intervals until both peaks are relaively equally high 
					bBegin = static_cast<int>(vecSecondDensityInterval.at(j)(0))-static_cast<int>(vecFirstDensityInterval.at(k)(0)) < nThreshold;
					bEnd = static_cast<int>(vecSecondDensityInterval.at(j)(2))-static_cast<int>(vecFirstDensityInterval.at(k)(2)) < nThreshold;
					nDiff = static_cast<int>(vecSecondDensityInterval.at(j)(1))-static_cast<int>(vecFirstDensityInterval.at(k)(1));
					if (abs(nDiff)<nThreshold && (bBegin ||bEnd ) )
					{
						if (abs(nDiff)<abs(fShift))
						{
							fShift = nDiff;
						}
						//fShift = std::min<double>(fShift,nDiff);
						//break;
					}
				}
			} 
			// if we didnt't find matching peaks that are equally high the shift is set to zero
			if (fShift == std::numeric_limits<double>::max())
			{
				fShift = 0;
			}
			return fShift;

		}

		bool HistogramMerger::HistogramMergerImpl::correctIntervalShift(std::vector<Math::Vector3ui>& vecDensityInterval,float fShift)
		{
			// - subtract the shift from vecDensity interval

			int nMax = std::numeric_limits<uint16>::max();
			for (uint k=0; k<vecDensityInterval.size();++k)
			{
				vecDensityInterval.at(k)(0)=std::min(std::max<int>((static_cast<int>(vecDensityInterval.at(k)(0))-fShift),0),nMax);
				vecDensityInterval.at(k)(1)=std::min(std::max<int>((static_cast<int>(vecDensityInterval.at(k)(1))-fShift),0),nMax);
				vecDensityInterval.at(k)(2)=std::min(std::max<int>((static_cast<int>(vecDensityInterval.at(k)(2))-fShift),0),nMax);
			} 
			return true;
		}

		// ------------------------ MERGING FUNCTIONS: -------------------------------------
		// - Function Name explenation:
		//		mergeTwoDensityIntervals_XYZ: all Parameters are the same: X = Interval Begin Index, Y = Interval Peak, Z = Interval End Index
		//		mergeTwoDensityIntervals_XYnZ: Z (Interval End Index) differs more than the threshold 
		//		mergeTwoDensityIntervals_XnYZ: Y (Interval Peak) differs more than the throshold

		bool HistogramMerger::HistogramMergerImpl::mergeTwoDensityIntervals_XYZ(boost::shared_array<long long>& Histogram, uint nHistogramDataSize, std::vector<Math::Vector3ui>& vecFirstDensityInterval,std::vector<Math::Vector3ui>& vecSecondDensityInterval,std::vector<Math::Vector3ui>& resultDensityIntervals,uint& nFirstIndex, uint&nSecondIndex,uint nShiftThreshold)
		{	// begin, end and peak are equal

			if(resultDensityIntervals.size()>0){
				resultDensityIntervals.at(resultDensityIntervals.size()-1)(2)=vecFirstDensityInterval.at(nFirstIndex)(0)-1;
			}
			// write the first interval to the result
			resultDensityIntervals.push_back(vecFirstDensityInterval.at(nFirstIndex));
			++nFirstIndex;
			++nSecondIndex;
			return true;
		}

		bool HistogramMerger::HistogramMergerImpl::mergeTwoDensityIntervals_XYnZ(boost::shared_array<long long>& Histogram, uint nHistogramDataSize, std::vector<Math::Vector3ui>& vecFirstDensityInterval,std::vector<Math::Vector3ui>& vecSecondDensityInterval,std::vector<Math::Vector3ui>& resultDensityIntervals,uint& nFirstIndex, uint&nSecondIndex,uint nShiftThreshold)
		{
			if(nFirstIndex+1 < vecFirstDensityInterval.size() && nSecondIndex+1 <vecSecondDensityInterval.size()){
				// begin and peak are equal but the end is not. so maybe only the border between two intervals has moved
				if (abs(static_cast<int>(vecFirstDensityInterval.at(nFirstIndex+1)(1))-static_cast<int>(vecSecondDensityInterval.at(nSecondIndex+1)(1)))<nShiftThreshold)
				{
					// compute new optimal interval border
					double fFirst_FirstInvervalQuotient = static_cast<double>(Histogram[vecFirstDensityInterval.at(nFirstIndex)(1)])/Histogram[vecFirstDensityInterval.at(nFirstIndex)(2)];
					double fFirst_SecondInvervalQuotient = static_cast<double>(Histogram[vecFirstDensityInterval.at(nFirstIndex+1)(1)])/Histogram[vecFirstDensityInterval.at(nFirstIndex+1)(0)];
					double fSecond_FirstInvervalQuotient = static_cast<double>(Histogram[vecSecondDensityInterval.at(nSecondIndex)(1)])/Histogram[vecSecondDensityInterval.at(nSecondIndex)(2)];
					double fSecond_SecondInvervalQuotient = static_cast<double>(Histogram[vecSecondDensityInterval.at(nSecondIndex+1)(1)])/Histogram[vecSecondDensityInterval.at(nSecondIndex+1)(0)];
					double fBestValue = std::max(std::max(fFirst_FirstInvervalQuotient,fFirst_SecondInvervalQuotient),std::max(fSecond_FirstInvervalQuotient,fSecond_SecondInvervalQuotient)); 
					uint nNewIndex;
					if(fBestValue==fFirst_FirstInvervalQuotient || fBestValue==fFirst_SecondInvervalQuotient) nNewIndex = vecFirstDensityInterval.at(nFirstIndex)(2);
					if(fBestValue==fSecond_FirstInvervalQuotient || fBestValue==fSecond_SecondInvervalQuotient) nNewIndex = vecSecondDensityInterval.at(nSecondIndex)(2);
					//findMinMax(const std::vector<double>& refBlockedHistogram,const uint nStartIndex,const uint nEndIndex, int& nMinIndex, double& fMinValue, int& nMaxIndex, double& fMaxValue) const
					// set the interval borders (end index) to the same values
					vecFirstDensityInterval.at(nFirstIndex)(2) = vecFirstDensityInterval.at(nFirstIndex+1)(0) = nNewIndex;
					vecSecondDensityInterval.at(nSecondIndex)(2) = vecSecondDensityInterval.at(nSecondIndex+1)(0) = nNewIndex;
					resultDensityIntervals.push_back(vecFirstDensityInterval.at(nFirstIndex));
					++nFirstIndex;
					++nSecondIndex;
					return true;
				}
				// or there is a new interval to insert?!?
				else
				{
					if (vecFirstDensityInterval.at(nFirstIndex)(1)<vecSecondDensityInterval.at(nSecondIndex)(1))
					{
						resultDensityIntervals.push_back(vecFirstDensityInterval.at(nFirstIndex));
						++nFirstIndex;
						++nSecondIndex;
						return true;
					} 
					else
					{
						resultDensityIntervals.push_back(vecSecondDensityInterval.at(nSecondIndex));
						++nFirstIndex;
						++nSecondIndex;
						return true;
					}                    
				}
			}
			//end of the one of the intervals
			else
			{
				if (nFirstIndex+1<vecFirstDensityInterval.size())
				{
					if(Histogram[vecSecondDensityInterval.at(nSecondIndex)(1)]>Histogram[vecFirstDensityInterval.at(nFirstIndex)(1)])
						shiftCorrectionForFollowingIntervals(vecFirstDensityInterval,nFirstIndex,static_cast<int>(vecSecondDensityInterval.at(nSecondIndex)(1))-vecFirstDensityInterval.at(nFirstIndex)(1));

					resultDensityIntervals.push_back(vecFirstDensityInterval.at(nFirstIndex));
					++nFirstIndex;
				}
				else if (nSecondIndex+1<vecSecondDensityInterval.size())
				{
					if(Histogram[vecSecondDensityInterval.at(nSecondIndex)(1)]<Histogram[vecFirstDensityInterval.at(nFirstIndex)(1)])
						shiftCorrectionForFollowingIntervals(vecSecondDensityInterval,nSecondIndex,static_cast<int>(vecFirstDensityInterval.at(nFirstIndex)(1))-vecSecondDensityInterval.at(nSecondIndex)(1));

					resultDensityIntervals.push_back(vecSecondDensityInterval.at(nSecondIndex));
					++nSecondIndex;
				}
				else
				{
					// both are at the end => take the top level histogram regions
					resultDensityIntervals.push_back(vecFirstDensityInterval.at(nFirstIndex));
					++nFirstIndex;
					++nSecondIndex;
				}
			}
			return true;
		}

		bool HistogramMerger::HistogramMergerImpl::mergeTwoDensityIntervals_XnYZ(boost::shared_array<long long>& Histogram, uint nHistogramDataSize, std::vector<Math::Vector3ui>& vecFirstDensityInterval,std::vector<Math::Vector3ui>& vecSecondDensityInterval,std::vector<Math::Vector3ui>& resultDensityIntervals,uint& nFirstIndex, uint&nSecondIndex,uint nShiftThreshold)
		{ // begin and end are the same but the peak has moved
			if(Histogram[vecSecondDensityInterval.at(nSecondIndex)(1)]>Histogram[vecFirstDensityInterval.at(nFirstIndex)(1)])
				//take the second peak
				vecFirstDensityInterval.at(nFirstIndex)(1)=vecSecondDensityInterval.at(nSecondIndex)(1);
			resultDensityIntervals.push_back(vecFirstDensityInterval.at(nFirstIndex));
			++nFirstIndex;
			++nSecondIndex;
			return true;
		}

		bool HistogramMerger::HistogramMergerImpl::mergeTwoDensityIntervals_XnYnZ(boost::shared_array<long long>& Histogram, uint nHistogramDataSize, std::vector<Math::Vector3ui>& vecFirstDensityInterval,std::vector<Math::Vector3ui>& vecSecondDensityInterval,std::vector<Math::Vector3ui>& resultDensityIntervals,uint& nFirstIndex, uint&nSecondIndex,uint nShiftThreshold)
		{ // begin is the same but peak and and have moved
			//additional interval in first vector
			if (vecFirstDensityInterval.at(nFirstIndex)(1)<vecSecondDensityInterval.at(nSecondIndex)(2) && vecFirstDensityInterval.at(nFirstIndex)(2)<vecSecondDensityInterval.at(nSecondIndex)(2))
			{
				if (resultDensityIntervals.size()>0 && resultDensityIntervals.at(resultDensityIntervals.size()-1)(2)-nShiftThreshold>vecFirstDensityInterval.at(nFirstIndex)(1))
				{
					++nFirstIndex;
					return true;
				}
				Math::Vector3ui newVec = vecFirstDensityInterval.at(nFirstIndex);
				if (resultDensityIntervals.size()>0)
				{
					//newVec(0) = resultDensityIntervals.at(resultDensityIntervals.size()-1)(2)+1;
					resultDensityIntervals.at(resultDensityIntervals.size()-1)(2) = newVec(0)-1;
				}      
				resultDensityIntervals.push_back(newVec);
				++nFirstIndex;
				return true;
			}
			//additional interval in second vector
			if (vecSecondDensityInterval.at(nSecondIndex)(1)<vecFirstDensityInterval.at(nFirstIndex)(2) && vecSecondDensityInterval.at(nSecondIndex)(2)<vecFirstDensityInterval.at(nFirstIndex)(2))
			{
				if (resultDensityIntervals.size()>0 && resultDensityIntervals.at(resultDensityIntervals.size()-1)(2)-nShiftThreshold>vecSecondDensityInterval.at(nSecondIndex)(1))
				{
					++nSecondIndex;
					return true;
				}
				Math::Vector3ui newVec = vecSecondDensityInterval.at(nSecondIndex);
				if (resultDensityIntervals.size()>0)
				{
					//newVec(0) = resultDensityIntervals.at(resultDensityIntervals.size()-1)(2)+1;
					resultDensityIntervals.at(resultDensityIntervals.size()-1)(2) = newVec(0)-1;
				}
				resultDensityIntervals.push_back(newVec);
				++nSecondIndex;
				return true;
			}
			//create new interval
			uint nNewX = 0;
			if (resultDensityIntervals.size()>0) nNewX = resultDensityIntervals.at(resultDensityIntervals.size()-1)(2)+1;
			uint nNewY = std::min(vecFirstDensityInterval.at(nFirstIndex)(1),vecSecondDensityInterval.at(nSecondIndex)(1));
			uint nNewZ = std::min(vecFirstDensityInterval.at(nFirstIndex)(2),vecSecondDensityInterval.at(nSecondIndex)(2));
			resultDensityIntervals.push_back(Math::create<Math::Vector3ui>(nNewX,nNewY,nNewZ));
			++nFirstIndex;
			++nSecondIndex;
			return true;
		}

		bool HistogramMerger::HistogramMergerImpl::mergeTwoDensityIntervals_nXYZ(boost::shared_array<long long>& Histogram, uint nHistogramDataSize, std::vector<Math::Vector3ui>& vecFirstDensityInterval,std::vector<Math::Vector3ui>& vecSecondDensityInterval,std::vector<Math::Vector3ui>& resultDensityIntervals,uint& nFirstIndex, uint&nSecondIndex,uint nShiftThreshold)
		{
			// the begin index is different. peak and end index are the same
			int nDiffFirst = vecFirstDensityInterval.at(nFirstIndex)(0);
			if (resultDensityIntervals.size()>0) nDiffFirst = static_cast<int>(resultDensityIntervals.at(resultDensityIntervals.size()-1)(1))-vecFirstDensityInterval.at(nFirstIndex)(1);
			int nDiffSecond = vecSecondDensityInterval.at(nSecondIndex)(0);
			if (resultDensityIntervals.size()>0) nDiffSecond = static_cast<int>(resultDensityIntervals.at(resultDensityIntervals.size()-1)(1))-vecSecondDensityInterval.at(nSecondIndex)(1);

			if (abs(nDiffFirst)<=abs(nDiffSecond))
			{
				// correct the shift
				if(Histogram[vecSecondDensityInterval.at(nSecondIndex)(1)]>Histogram[vecFirstDensityInterval.at(nFirstIndex)(1)])
					shiftCorrectionForFollowingIntervals(vecFirstDensityInterval,nFirstIndex,static_cast<int>(vecSecondDensityInterval.at(nSecondIndex)(1))-vecFirstDensityInterval.at(nFirstIndex)(1));

				if (resultDensityIntervals.size()>0) resultDensityIntervals.at(resultDensityIntervals.size()-1)(2) = vecFirstDensityInterval.at(nFirstIndex)(0)-1;
				resultDensityIntervals.push_back(vecFirstDensityInterval.at(nFirstIndex));
				++nFirstIndex;
				++nSecondIndex;
				return true;
			} 
			else
			{
				if(Histogram[vecSecondDensityInterval.at(nSecondIndex)(1)]<Histogram[vecFirstDensityInterval.at(nFirstIndex)(1)])
					shiftCorrectionForFollowingIntervals(vecSecondDensityInterval,nSecondIndex,static_cast<int>(vecFirstDensityInterval.at(nFirstIndex)(1))-vecSecondDensityInterval.at(nSecondIndex)(1));

				if (resultDensityIntervals.size()>0) resultDensityIntervals.at(resultDensityIntervals.size()-1)(2) = vecSecondDensityInterval.at(nSecondIndex)(0)-1;
				resultDensityIntervals.push_back(vecSecondDensityInterval.at(nSecondIndex));
				++nFirstIndex;
				++nSecondIndex;
				return true;
			}
		}

		bool HistogramMerger::HistogramMergerImpl::mergeTwoDensityIntervals_nXYnZ(boost::shared_array<long long>& Histogram, uint nHistogramDataSize, std::vector<Math::Vector3ui>& vecFirstDensityInterval,std::vector<Math::Vector3ui>& vecSecondDensityInterval,std::vector<Math::Vector3ui>& resultDensityIntervals,uint& nFirstIndex, uint&nSecondIndex,uint nShiftThreshold)
		{
			// begin and end index are different, peak is the same
			// if the first end index is smaller than the second end index 
			if (vecFirstDensityInterval.at(nFirstIndex)(2)<vecSecondDensityInterval.at(nSecondIndex)(2))
			{
				// second peak higher than the first
				if(Histogram[vecSecondDensityInterval.at(nSecondIndex)(1)]>Histogram[vecFirstDensityInterval.at(nFirstIndex)(1)])
					shiftCorrectionForFollowingIntervals(vecFirstDensityInterval,nFirstIndex,static_cast<int>(vecSecondDensityInterval.at(nSecondIndex)(1))-vecFirstDensityInterval.at(nFirstIndex)(1));
				//vecFirstDensityInterval.at(nFirstIndex)(1)=vecSecondDensityInterval.at(nSecondIndex)(1);

				Math::Vector3ui newVec = vecFirstDensityInterval.at(nFirstIndex);
				if (resultDensityIntervals.size()>0) resultDensityIntervals.at(resultDensityIntervals.size()-1)(2) = newVec(0)-1;
				// newVec(0) = resultDensityIntervals.at(resultDensityIntervals.size()-1)(2)+1;
				resultDensityIntervals.push_back(newVec);
				++nFirstIndex;
				++nSecondIndex;
				return true;
			} 
			else
			{
				// second peak lower than first
				if(Histogram[vecSecondDensityInterval.at(nSecondIndex)(1)]<Histogram[vecFirstDensityInterval.at(nFirstIndex)(1)])
					shiftCorrectionForFollowingIntervals(vecSecondDensityInterval,nSecondIndex,static_cast<int>(vecFirstDensityInterval.at(nFirstIndex)(1))-vecSecondDensityInterval.at(nSecondIndex)(1));
				//vecSecondDensityInterval.at(nSecondIndex)(1)=vecFirstDensityInterval.at(nFirstIndex)(1);

				Math::Vector3ui newVec = vecSecondDensityInterval.at(nSecondIndex);
				if (resultDensityIntervals.size()>0)resultDensityIntervals.at(resultDensityIntervals.size()-1)(2) = newVec(0)-1;
				// newVec(0) = resultDensityIntervals.at(resultDensityIntervals.size()-1)(2)+1;
				resultDensityIntervals.push_back(newVec);
				++nFirstIndex;
				++nSecondIndex;
				return true;
			}
		}

		bool HistogramMerger::HistogramMergerImpl::mergeTwoDensityIntervals_nXnYZ(boost::shared_array<long long>& Histogram, uint nHistogramDataSize, std::vector<Math::Vector3ui>& vecFirstDensityInterval,std::vector<Math::Vector3ui>& vecSecondDensityInterval,std::vector<Math::Vector3ui>& resultDensityIntervals,uint& nFirstIndex, uint&nSecondIndex,uint nShiftThreshold)
		{
			// begin and peak are different, end is the same
			int nDiffFirst = vecFirstDensityInterval.at(nFirstIndex)(0);
			if (resultDensityIntervals.size()>0) nDiffFirst = static_cast<int>(resultDensityIntervals.at(resultDensityIntervals.size()-1)(1))-vecFirstDensityInterval.at(nFirstIndex)(1);
			int nDiffSecond = vecSecondDensityInterval.at(nSecondIndex)(0);
			if (resultDensityIntervals.size()>0) nDiffSecond = static_cast<int>(resultDensityIntervals.at(resultDensityIntervals.size()-1)(1))-vecSecondDensityInterval.at(nSecondIndex)(1);

			if (abs(nDiffFirst)<=abs(nDiffSecond))
			{
				//if (resultDensityIntervals.size()>0) resultDensityIntervals.at(resultDensityIntervals.size()-1)(2) = vecFirstDensityInterval.at(nFirstIndex)(0)-1;
				if (resultDensityIntervals.size()>0){
					if (vecFirstDensityInterval.at(nFirstIndex)(0)-1 > resultDensityIntervals.at(resultDensityIntervals.size()-1)(1))
					{
						resultDensityIntervals.at(resultDensityIntervals.size()-1)(2) = vecFirstDensityInterval.at(nFirstIndex)(0)-1;
					}else{
						//new border between the two peaks
						resultDensityIntervals.at(resultDensityIntervals.size()-1)(2) = 0.5*(vecFirstDensityInterval.at(nFirstIndex)(1) + resultDensityIntervals.at(resultDensityIntervals.size()-1)(1));
						vecFirstDensityInterval.at(nFirstIndex)(0) = resultDensityIntervals.at(resultDensityIntervals.size()-1)(2) + 1;
					}            
				}
				resultDensityIntervals.push_back(vecFirstDensityInterval.at(nFirstIndex));
				++nFirstIndex;
				++nSecondIndex;
				return true;
			} 
			else
			{
				//if (resultDensityIntervals.size()>0) resultDensityIntervals.at(resultDensityIntervals.size()-1)(2) = vecSecondDensityInterval.at(nSecondIndex)(0)-1;
				if (resultDensityIntervals.size()>0){
					if (vecSecondDensityInterval.at(nSecondIndex)(0)-1 > resultDensityIntervals.at(resultDensityIntervals.size()-1)(1))
					{
						resultDensityIntervals.at(resultDensityIntervals.size()-1)(2) = vecSecondDensityInterval.at(nSecondIndex)(0)-1;
					}else{
						//new border between the two peaks
						resultDensityIntervals.at(resultDensityIntervals.size()-1)(2) = 0.5*(vecSecondDensityInterval.at(nSecondIndex)(1) + resultDensityIntervals.at(resultDensityIntervals.size()-1)(1));
						vecSecondDensityInterval.at(nSecondIndex)(0) = resultDensityIntervals.at(resultDensityIntervals.size()-1)(2) + 1;
					}            
				}
				resultDensityIntervals.push_back(vecSecondDensityInterval.at(nSecondIndex));
				++nFirstIndex;
				++nSecondIndex;
				return true;
			}
		}

		bool HistogramMerger::HistogramMergerImpl::mergeTwoDensityIntervals_nXnYnZ(boost::shared_array<long long>& Histogram, uint nHistogramDataSize, std::vector<Math::Vector3ui>& vecFirstDensityInterval,std::vector<Math::Vector3ui>& vecSecondDensityInterval,std::vector<Math::Vector3ui>& resultDensityIntervals,uint& nFirstIndex, uint&nSecondIndex,uint nShiftThreshold)
		{
			// begin, end and peak are different
			//additional interval in first vector
			if (vecFirstDensityInterval.at(nFirstIndex)(1)<vecSecondDensityInterval.at(nSecondIndex)(2) && vecFirstDensityInterval.at(nFirstIndex)(2)<vecSecondDensityInterval.at(nSecondIndex)(2))
			{
				if (resultDensityIntervals.size()>0 && resultDensityIntervals.at(resultDensityIntervals.size()-1)(2)-nShiftThreshold>vecFirstDensityInterval.at(nFirstIndex)(1))
				{
					++nFirstIndex;
					return true;
				}
				Math::Vector3ui newVec = vecFirstDensityInterval.at(nFirstIndex);
				if (resultDensityIntervals.size()>0)resultDensityIntervals.at(resultDensityIntervals.size()-1)(2) = newVec(0)-1;// newVec(0) = resultDensityIntervals.at(resultDensityIntervals.size()-1)(2)+1;
				resultDensityIntervals.push_back(newVec);
				++nFirstIndex;
				return true;
			}
			//additional interval in second vector
			if (vecSecondDensityInterval.at(nSecondIndex)(1)<vecFirstDensityInterval.at(nFirstIndex)(2) && vecSecondDensityInterval.at(nSecondIndex)(2)<vecFirstDensityInterval.at(nFirstIndex)(2))
			{   
				if (resultDensityIntervals.size()>0 && resultDensityIntervals.at(resultDensityIntervals.size()-1)(2)-nShiftThreshold>vecSecondDensityInterval.at(nSecondIndex)(1))
				{
					++nSecondIndex;
					return true;
				}
				Math::Vector3ui newVec = vecSecondDensityInterval.at(nSecondIndex);
				if (resultDensityIntervals.size()>0)resultDensityIntervals.at(resultDensityIntervals.size()-1)(2) = newVec(0)-1;// newVec(0) = resultDensityIntervals.at(resultDensityIntervals.size()-1)(2)+1;
				resultDensityIntervals.push_back(newVec);
				++nSecondIndex;
				return true;
			}
			//create new interval
			uint nNewX = 0;
			if (resultDensityIntervals.size()>0) nNewX = resultDensityIntervals.at(resultDensityIntervals.size()-1)(2)+1;
			uint nNewY = std::min(vecFirstDensityInterval.at(nFirstIndex)(1),vecSecondDensityInterval.at(nSecondIndex)(1));
			uint nNewZ = std::min(vecFirstDensityInterval.at(nFirstIndex)(2),vecSecondDensityInterval.at(nSecondIndex)(2));
			resultDensityIntervals.push_back(Math::create<Math::Vector3ui>(nNewX,nNewY,nNewZ));
			++nFirstIndex;
			++nSecondIndex;
			return true;
		}

		long long HistogramMerger::HistogramMergerImpl::getNumOfVoxel(const boost::shared_array<long long>& refHistogram,const uint nHistogramDataSize) const
		{
			long long nNumOfVoxels = 0;

			for (uint i=0; i<nHistogramDataSize; ++i)
			{
				nNumOfVoxels+=refHistogram[i];
			}

			return nNumOfVoxels;
		}

		bool HistogramMerger::HistogramMergerImpl::findMinMax(const std::vector<double>& refBlockedHistogram,const uint nStartIndex,const uint nEndIndex, int& nMinIndex, double& fMinValue, int& nMaxIndex, double& fMaxValue) const
		{
			// - iterate through refBlockHistogram and find min and Max in the interval given from StartIndex to EndIndex

			fMinValue = std::numeric_limits<double>::max();
			nMinIndex = -1;
			fMaxValue = std::numeric_limits<double>::min();
			nMaxIndex = -1;

			if (nEndIndex>refBlockedHistogram.size()-1)
			{
				return false;
			}

			for(uint k=nStartIndex;k<(nEndIndex+1);++k)
			{
				if(fMinValue>refBlockedHistogram.at(k))
				{
					fMinValue = refBlockedHistogram.at(k);
					nMinIndex = k;
				}
				if(fMaxValue<refBlockedHistogram.at(k))
				{
					fMaxValue = refBlockedHistogram.at(k);
					nMaxIndex = k;
				}
			} 
			return true;
		}
		bool HistogramMerger::HistogramMergerImpl::wasMergingSuccessful(std::vector<std::vector<Math::Vector3ui> >& vecDensityIntervals,std::vector<Math::Vector3ui>& vecResultInterval, uint nThreshold) const
		{
			//  - check if merging was successful by checking for merging size and overlapping 

			uint nMinSize = std::numeric_limits<uint>::max();
			uint nMaxSize = std::numeric_limits<uint>::min();
			// check merging size
			for (uint i=0;i<vecDensityIntervals.size();++i)
			{
				nMinSize = std::min<uint>(nMinSize,vecDensityIntervals.at(i).size());
				nMaxSize = std::max<uint>(nMaxSize,vecDensityIntervals.at(i).size());            
			}

			//for (uint i=0;i<vecDensityIntervals.size();++i){
			//   std::cout << "histogram " << i << " : " << vecDensityIntervals.at(i).size() << "regions :";
			//   for (uint j=0;j<vecDensityIntervals.at(i).size();++j){
			//      std::cout << "   y= " << vecDensityIntervals.at(i).at(j)(1);
			//   }
			//   std::cout << std::endl;
			//}

			//std::cout << "final histogram " << " : " << vecResultInterval.size() << "regions :";
			//for (uint j=0;j<vecResultInterval.size();++j){
			//   std::cout << "   y= " << vecResultInterval.at(j)(1);
			//}
			//std::cout << std::endl;

			if(vecResultInterval.size()>nMaxSize || vecResultInterval.size()<nMinSize){
				std::cout << "region increase after region merging detected : " << vecResultInterval.size() << " regions" << std::endl;            
				return false;
			}
			//check for overlapping
			for (uint i=0;i<vecResultInterval.size()-1;++i)
			{
				bool bOverlappingDetected = false;
				if(vecResultInterval.at(i)(2) > vecResultInterval.at(i+1)(0)+ nThreshold)  bOverlappingDetected = true;
				if(vecResultInterval.at(i)(1) > vecResultInterval.at(i+1)(0)+ nThreshold)  bOverlappingDetected = true;
				if(vecResultInterval.at(i)(2) > vecResultInterval.at(i+1)(2)+ nThreshold)  bOverlappingDetected = true;
				if(vecResultInterval.at(i)(1) > vecResultInterval.at(i+1)(1)+ nThreshold)  bOverlappingDetected = true;

				if (bOverlappingDetected == true){
					std::cout << "Overlap after region merging detected" << std::endl;
					return false;
				}
			}
			return true;
		}

		std::vector<int> HistogramMerger::HistogramMergerImpl::computeDensityIntervalShifts(std::vector<std::vector<Math::Vector3ui> >& vecDensityIntervals)
		{
			// - compute Density Intervalshifts for each interval in the given vector

			std::vector<int> vecResults;
			if(vecDensityIntervals.size()==0)return vecResults;
			float fLastShift = 0.0;
			float fCurrentShift;
			vecResults.push_back(0);
			uint nShiftThreshold=vecDensityIntervals.at(0).at(vecDensityIntervals.at(0).size()-1)(2)/100; //TODO: an empirical value... old value: 200;

			if(vecDensityIntervals.size()==1)return vecResults;

			for (uint i=1;i<vecDensityIntervals.size();++i)
			{
				fCurrentShift = computeDensityIntervalShift(vecDensityIntervals.at(i-1), vecDensityIntervals.at(i), nShiftThreshold);
				fLastShift += fCurrentShift;
				vecResults.push_back(fLastShift);
			}

			return vecResults;
		}
		bool HistogramMerger::HistogramMergerImpl::shiftCorrectionForFollowingIntervals(std::vector<Math::Vector3ui>& vecIntervals,uint& nIndex, int nShift) {
			int nMax = std::numeric_limits<uint16>::max();
			for (uint k=nIndex; k<vecIntervals.size();++k)
			{      
				vecIntervals.at(k)(1)=std::min(std::max<int>((static_cast<int>(vecIntervals.at(k)(1))+nShift),0),nMax);      
			} 
			return true;
		}
	}
}
