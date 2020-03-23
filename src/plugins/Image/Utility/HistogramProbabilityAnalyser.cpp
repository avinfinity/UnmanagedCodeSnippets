//****************************************************************************
// (c) 2012 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
#include <openOR/Image/HistogramProbabilityAnalyser.hpp>

#include <openOR/cleanUpWindowsMacros.hpp>
#include <openOR/Math/constants.hpp>
#include <openOR/Math/comparison.hpp>

namespace openOR {
	namespace Image {


		HistogramProbabilityAnalyser::HistogramProbabilityAnalyser():
	m_bProbabilityBasedHighDensityMaterialSearch(true)
	{         
	}

	HistogramProbabilityAnalyser::~HistogramProbabilityAnalyser() {
	}

	void HistogramProbabilityAnalyser::operator()() const {
		// - create a copy of the Histogram 
		// - convert the image data into vector3ui data and copy it to a member
		// - calculate splitted normal distributions and copy them to m_pResults

		if (!m_pHistogram || !m_pRegions) {
			// TODO: error  ---> Fehlerausgabe mit Fehlerzeile 
			return;
		}

		//copy the member to local variable pHistogram
		size_t histSize = getWidth(m_pHistogram);
		boost::shared_array<long long> pHistogram = boost::shared_array<long long>(new long long[histSize]);
		for (size_t i = 0; i < histSize; i++) {
			pHistogram[i] = m_pHistogram->data()[i];
		}

		// convert image1D data to Vector3ui data
		std::vector<Math::Vector3ui> regions;
		size_t regionCount = getWidth(m_pRegions);
		regions.reserve(regionCount);
		for (size_t i = 0; i < regionCount; i++) {
			Triple<size_t> element = m_pRegions->data()[i];
			regions.push_back(Math::create<Math::Vector3ui>(static_cast<unsigned int>(element.first), static_cast<unsigned int>(element.second), static_cast<unsigned int>(element.third)));
		}

		//get Splitted Normal Distributions
		HistogramProbabilityAnalyserImpl impl;
		bool bProbabilityBasedHighDensityMaterialSearch = m_bProbabilityBasedHighDensityMaterialSearch;
		std::vector<Math::Vector4d> result = impl.getSplittedNormalDistributions(pHistogram, histSize, regions, bProbabilityBasedHighDensityMaterialSearch);
		m_pResult->setSize(result.size());

		for (size_t i = 0; i < result.size(); i++) {
			Math::Vector4d v = result[i];
			m_pResult->mutableData()[i].first = v[0];
			m_pResult->mutableData()[i].second = v[1];
			m_pResult->mutableData()[i].third = v[2];
			m_pResult->mutableData()[i].fourth = v[3];
		}

		m_pRegions->setSize(regions.size());
		for (size_t i = 0; i < regions.size(); i++) {
			m_pRegions->mutableData()[i].first = regions.at(i)[0];
			m_pRegions->mutableData()[i].second = regions.at(i)[1];
			m_pRegions->mutableData()[i].third = regions.at(i)[2];
		}
	}



	void HistogramProbabilityAnalyser::setData(const AnyPtr& data, const std::string& name) {
		// - set the matching member according to the type of data input

		std::tr1::shared_ptr<openOR::Image::Image1DDataUI64> pHistogram = interface_cast<openOR::Image::Image1DDataUI64>(data);
		if (pHistogram) {
			m_pHistogram = pHistogram;
		}
		std::tr1::shared_ptr<openOR::Image::Image1DData<openOR::Triple<size_t> > > pRegions = interface_cast<openOR::Image::Image1DData<openOR::Triple<size_t> > >(data);
		if (pRegions) {
			m_pRegions = pRegions;
		}
		std::tr1::shared_ptr<openOR::Image::Image1DData<openOR::Quad<double> > > pResult = interface_cast<openOR::Image::Image1DData<openOR::Quad<double> > >(data);
		if (pResult) {
			m_pResult = pResult;
		}
	}


	std::vector<Math::Vector4d> HistogramProbabilityAnalyser::HistogramProbabilityAnalyserImpl::getSplittedNormalDistributions(boost::shared_array<long long> Histogram, uint nHistogramDataSize,std::vector<Math::Vector3ui>& vecDensityIntervals,bool bProbabilityBasedHighDensityMaterialSearch,bool bLog10Normalization,uint nNumOfRefinements)
	{
		// - compute a splitted normal distribution (splitted: left and right from the peak)
		// - search for a missing material (if bProbabilityBasedHighDensityMaterialSearch is set)
		// - refine the normal distributions 

		boost::shared_array<long long> vecDummyData = boost::shared_array<long long>(new long long[nHistogramDataSize]);
		std::vector <Math::Vector4d> vecResults;
		std::vector <Math::Vector4d> vecNewResults;
		std::vector <Math::Vector4d> vecMergedResults;
		Math::Vector4d result;
		uint nRightBorder,nLeftBorder;

		//get initial splitted normal distribution
		for (uint i=0; i<vecDensityIntervals.size();++i)
		{
			//set border for probability calculation
			nLeftBorder = vecDensityIntervals.at(i)(0) + 0.75*(vecDensityIntervals.at(i)(1)-vecDensityIntervals.at(i)(0));
			nRightBorder = vecDensityIntervals.at(i)(2) -  + 0.75*(vecDensityIntervals.at(i)(2)-vecDensityIntervals.at(i)(1));

			result = getSplittedNormalDistributionOfAHistogram(Histogram,nHistogramDataSize,nLeftBorder,nRightBorder,vecDensityIntervals.at(i)(1),bLog10Normalization);
			vecResults.push_back(result);


		}

		//depending on bProbabilityBasedHighDensityMaterialSearch search for a missing material
		if (vecDensityIntervals.size()>0 && bProbabilityBasedHighDensityMaterialSearch)
		{
			Math::Vector4d secondResult = probabilityBasedHighDensityMaterialSearch(Histogram, nHistogramDataSize, nLeftBorder, nRightBorder, vecDensityIntervals.at(vecDensityIntervals.size()-1)(1), bLog10Normalization, vecDensityIntervals,result);

			if (Math::norm(secondResult)!=0) 
			{
				vecResults.push_back(secondResult);
			}

		}

		//refine normal distributions
		for (uint r=0; r<nNumOfRefinements;++r)
		{
			for (uint i=0; i<vecResults.size();++i)
			{
				Math::Vector4d vecPre = Math::create<Math::Vector4d>(0.0,0.0,0.0,0.0);
				Math::Vector4d vecCurrent=vecResults.at(i);
				Math::Vector4d vecPost = Math::create<Math::Vector4d>(0.0,nHistogramDataSize-1,0.0,0.0);
				if(i>0)vecPre=vecResults.at(i-1);
				if(i<vecDensityIntervals.size()-1)vecPost=vecResults.at(i+1);
				//refine current values
				vecNewResults.push_back(refineSplittedNormalDistributionOfAHistogram(Histogram,vecDummyData,nHistogramDataSize,vecPre,vecCurrent,vecPost, bLog10Normalization));
			}
			//check for probability merges TODO: doppelt (Codeoptimierung)
			vecMergedResults.clear();
			for (uint i=0; i<vecNewResults.size();++i)
			{
				if(!Math::isEqualTo(vecNewResults.at(i), Math::create<Math::Vector4d>(0.0, 0.0, 0.0, 0.0))) 
				{
					vecMergedResults.push_back(vecNewResults.at(i));
				}
			}
			vecResults.clear();
			vecResults=vecMergedResults;
		}
		//remove empty element TODO: doppelt
		vecMergedResults.clear();
		for (uint i=0; i<vecResults.size();++i)
		{
			if(!Math::isEqualTo(vecResults.at(i), Math::create<Math::Vector4d>(0.0, 0.0, 0.0, 0.0))) 
			{ 
				vecMergedResults.push_back(vecResults.at(i)); 
			}
		}
		vecResults.clear();
		vecResults=vecMergedResults;
		return vecResults;

	}

	Math::Vector4d HistogramProbabilityAnalyser::HistogramProbabilityAnalyserImpl::getSplittedNormalDistributionOfAHistogram(boost::shared_array<long long> vecData, uint nHistogramDataSize, uint nBeginIndex, uint nEndIndex, uint nPeak,bool bLog10Normalization)
	{
		// - calculate a splitted normal distribution on the histogram
		// - split in left and right subinterval, compute std and mean
		// - return vector: (std left interval, peak, std right interval, scaled normal distribution at the peak value)


		Math::Vector4d vecResult = Math::create<Math::Vector4d>(0.0,nPeak,0.0,0.0);
		std::vector<long long> vecCurrentInterval;
		if (nBeginIndex>nPeak || nPeak>nEndIndex || (nPeak == 0 && nBeginIndex>0)) return vecResult;
		if (nEndIndex>nHistogramDataSize-1)nEndIndex=nHistogramDataSize-1;

		vecCurrentInterval.clear();
		vecCurrentInterval.reserve(2*(nPeak-nBeginIndex));

		//starting form the peak build left interval ...
		for (int i = nBeginIndex; i <= nPeak; ++i) {
			vecCurrentInterval.push_back(vecData[i]);
		}
		for (int i = nPeak - 1; i >= nBeginIndex && i >= 0; i--) {
			vecCurrentInterval.push_back(vecData[i]);     
		}
		// compute left mean and std
		Math::Vector2d leftmeanstd = getNormalDistributionOfAHistogram(vecCurrentInterval,bLog10Normalization);

		//and the right interval
		vecCurrentInterval.clear();
		vecCurrentInterval.reserve(2*(nEndIndex-nPeak));

		for (int i = nEndIndex; i > nPeak; --i) {
			vecCurrentInterval.push_back(vecData[i]);
		}
		for (int i = nPeak; i <= nEndIndex; ++i) {
			vecCurrentInterval.push_back(vecData[i]);
		}
		// compute right mean and std
		Math::Vector2d rightmeanstd = getNormalDistributionOfAHistogram(vecCurrentInterval,bLog10Normalization);

		vecResult(0)=leftmeanstd(1);
		vecResult(1)=nPeak;
		vecResult(2)=rightmeanstd(1);

		//findMax
		long long nMaxValue = 0;
		for (uint i = nBeginIndex; i <= nEndIndex; ++i) {
			nMaxValue = std::max(vecData[i],nMaxValue);
		}
		if(bLog10Normalization)
		{
			double fTmpValue = log10( static_cast<double>(nMaxValue) );
			if(fTmpValue<0.0) fTmpValue = 0;
			//fTmpValue = 1.0 / computeNormalDistributedProbability(vecResult(1),min(vecResult(0),vecResult(2)),vecResult(1));
			vecResult(3)=fTmpValue;//nMaxValue;
		}
		else
		{
			//
			vecResult(3)=static_cast<double>(nMaxValue)*computeNormalDistributedProbability(vecResult(1),std::min(vecResult(0),vecResult(2)),vecResult(1));
		}

		return vecResult; // = (std left interval, peak, std right interval, scaled normal distribution at the peak value)
	}

	Math::Vector4d HistogramProbabilityAnalyser::HistogramProbabilityAnalyserImpl::refineSplittedNormalDistributionOfAHistogram(boost::shared_array<long long> vecData, boost::shared_array<long long> vecDummyData, uint nHistogramDataSize, Math::Vector4d vecPre,Math::Vector4d vecCurrent,Math::Vector4d vecPost,bool bLog10Normalization)
	{	
		// - calculate a refined version of the normal distribution:
		// - compute total and current probabillity for each voxel from the spitted normal distributions
		// - copy the current amount of voxels corresponding to the computed probabilities to vecDummyData
		// - calculate the splitted normal distributions based on vecDumyData and return the result

		// build new temporary histogram for refinement
		memset((vecDummyData).get(),0,nHistogramDataSize*sizeof(long long));
		
		for(uint i=vecPre(1);i<=vecPost(1) && i<nHistogramDataSize;++i)
		{
			// calc total probability:
			double fTotalProbability = 0.0;
			if(i<= vecCurrent(1)) // if we are left from the peak, calculate total probability from left interval
			{
				fTotalProbability += vecCurrent(3)*computeNormalDistributedProbability(vecCurrent(1),vecCurrent(0),i); // use peak as the inputMean and the left interval std as inputStd
				fTotalProbability += vecPre(3)*computeNormalDistributedProbability(vecPre(1),vecPre(2),i);
			}
			else // we are right from the peak: calculate total probability from the right interval
			{
				fTotalProbability += vecCurrent(3)*computeNormalDistributedProbability(vecCurrent(1),vecCurrent(2),i); // use peak as the inputMean and the right interval std as inputStd
				fTotalProbability += vecPost(3)*computeNormalDistributedProbability(vecPost(1),vecPost(0),i);
			}

			// calculate the current probablility by the same procedure as above
			double fCurrentProbability;
			if(i<= vecCurrent(1))
			{
				fCurrentProbability = vecCurrent(3)*computeNormalDistributedProbability(vecCurrent(1),vecCurrent(0),i);
			}
			else
			{
				fCurrentProbability = vecCurrent(3)*computeNormalDistributedProbability(vecCurrent(1),vecCurrent(2),i);      
			}

			//copy the current amount of voxels corresponding to the computed probability
			if(fTotalProbability>0.0)
			{
				vecDummyData[i]=floor(static_cast<double>(vecData[i])*fCurrentProbability/fTotalProbability);      
			}
			else
			{
				vecDummyData[i]=0;
			}
		}
		Math::Vector4d vecResultDistribution = getSplittedNormalDistributionOfAHistogram(vecDummyData, nHistogramDataSize, vecPre(1), vecPost(1), vecCurrent(1),bLog10Normalization);
		return vecResultDistribution;
	}

	double HistogramProbabilityAnalyser::HistogramProbabilityAnalyserImpl::computeNormalDistributedProbability(double mean,double stddev,double x)
	{
		// - calculate normal distribution for a given mean, std and x

		if(stddev == 0)
		{
			if(x==mean) return 1.0;
			else return 0.0;
		}
		double result;
		result = (x-mean) / stddev;
		result = -0.5 * result * result;
		result = exp(result) / (stddev * sqrt(2.0 * Math::PI));
		return result;

	}

	Math::Vector2d HistogramProbabilityAnalyser::HistogramProbabilityAnalyserImpl::getNormalDistributionOfAHistogram(std::vector<long long>& vecData, bool bLog10Normalization)
	{
		// - compute mean and standard deviation for the histogram
		// - with or without log 10 normalization
		// - return (Mean, Std)

		if (!vecData.size()) return Math::create<Math::Vector2d>(0.0,0.0);

		double fMean = 0.0;
		long long nTotalNumber = 0;
		double fStd = 0.0;
		double fTmpValue;

		//compute with log 10 normalization
		if (bLog10Normalization)
		{
			//compute mean for a histogram
			for (uint i=0; i<vecData.size(); ++i)
			{
				fTmpValue = log10( static_cast<double>(vecData.at(i)) );
				if(fTmpValue<0.0) fTmpValue = 0;
				fMean += fTmpValue * i;
				nTotalNumber += fTmpValue;
			}
			if(nTotalNumber>0) fMean = fMean / nTotalNumber;
			//standard deviation of a histogram
			for (uint i=0; i<vecData.size(); ++i)
			{
				fTmpValue = log10( static_cast<double>(vecData.at(i)) );
				if(fTmpValue<0.0) fTmpValue = 0;
				fStd +=  pow((static_cast<double>(i)-fMean),2.0) * fTmpValue;
			}
		}
		else //without log 10 normalization
		{
			//compute mean for a histogram
			for (uint i=0; i<vecData.size(); ++i)
			{
				fMean += static_cast<double>(vecData.at(i)) * i;
				nTotalNumber += vecData.at(i);
			}
			if(nTotalNumber>0) fMean = fMean / nTotalNumber;
			//standard deviation of a histogram
			for (uint i=0; i<vecData.size(); ++i)
			{
				fStd +=  pow((static_cast<double>(i)-fMean),2.0)*vecData.at(i);
			}
		}

		if(nTotalNumber>1) fStd = sqrt(fStd / (nTotalNumber - 1)); 
		return Math::create<Math::Vector2d>(fMean,fStd);
	}

	Math::Vector4d HistogramProbabilityAnalyser::HistogramProbabilityAnalyserImpl::probabilityBasedHighDensityMaterialSearch(boost::shared_array<long long> pHistogram, uint nHistogramDataSize, uint nLeftBorder, uint nRightBorder, uint nPeak,bool bLog10Normalization,std::vector<Math::Vector3ui>& vecDensityIntervals,Math::Vector4d& vecCurrentLastMaterial)
	{
		// - check if there might be a missing material
		// - if so, find new peak, compute new probabilities and update the interval vector 

		// sum up total voxels
		long long nTotalVoxels = 0;
		for (uint i=0; i<nHistogramDataSize; i++ )
			nTotalVoxels += pHistogram[i];

		//constants (empirical values) TODO
		double fRelativeVoxelThresholdFactor = 0.035*0.035*0.035; // old value 0.025*0.025*0.025;
		float nStdDevFactor = 5.0; //old value 5.0
		float nStdDevFactorForRecalculation = 0.66 * nStdDevFactor; // 1.0
		double  fStdDevThresholdForHDMats = 750.0; //2500
		uint nMaxSearchArea = 65535.0 - 2.0 * nStdDevFactor * fStdDevThresholdForHDMats;
		//double  fVolumsizeNormalization = log10((double)nTotalVoxels / (128.0*128.0*128.0));
		//double  fStdDevThresholdForHDMats = std::min<double>(std::max<double>(1250.0 * fVolumsizenormalization, 1250.0), 3500.0);
		double  fPeakNormalization = 1.5 * 65535.0 / vecCurrentLastMaterial(1);
		fStdDevThresholdForHDMats = std::min<double>(std::max<double>(fStdDevThresholdForHDMats * fPeakNormalization, 750.0), 3500.0);

		//check if there might be a missing material
		bool bMissingMat = false;
		// if the last peak plus the right std doesn't reach the end of the histogram:
		if ((nPeak+vecCurrentLastMaterial(2)*nStdDevFactor+1) < nHistogramDataSize - 100)
		{

			// set threshold number of voxels
			nTotalVoxels *= fRelativeVoxelThresholdFactor;

			// sum up voxels in the area after 4*std
			long long nVoxelsInOuterArea = 0;
			for (uint i=(nPeak+vecCurrentLastMaterial(2)*nStdDevFactor+1); i<nHistogramDataSize; i++ )
				nVoxelsInOuterArea += pHistogram[i];
			// check for voxel sum and std dev from last material (a high std indicates an already detected high density material)
			if (nVoxelsInOuterArea > nTotalVoxels){
				if(vecCurrentLastMaterial(2) < fStdDevThresholdForHDMats){
					if ((vecCurrentLastMaterial(2) * nStdDevFactor + vecCurrentLastMaterial(1)) < nMaxSearchArea){
						bMissingMat = true;
					}
				}
			}

		}
		//search for the missing material
		if (bMissingMat)
		{
			// search new peak value
			nRightBorder = nPeak+vecCurrentLastMaterial(2) * nStdDevFactorForRecalculation;

			long long nNewPeakValue = 0;
			uint nNewPeak = 0;
			for (uint i=nRightBorder; i < nHistogramDataSize; i++ ){
				if (pHistogram[i]>nNewPeakValue){
					nNewPeakValue = pHistogram[i];
					nNewPeak = i;
				}         
			}
			uint nNewLeftBorder = nNewPeak - 0.5*(nNewPeak - (nRightBorder + 1));    
			uint nNewRightBorder = nHistogramDataSize-1;

			// compute new probabilities
			nRightBorder = nPeak+vecCurrentLastMaterial(2)*nStdDevFactor;     
			Math::Vector4d  refinedOldResult = getSplittedNormalDistributionOfAHistogram(pHistogram,nHistogramDataSize,nLeftBorder,nRightBorder,nPeak,bLog10Normalization);
			vecCurrentLastMaterial = refinedOldResult;
			Math::Vector4d  newResult = getSplittedNormalDistributionOfAHistogram(pHistogram,nHistogramDataSize,nNewLeftBorder,nNewRightBorder,nNewPeak,bLog10Normalization);

			// update interval vector
			nRightBorder = nPeak+vecCurrentLastMaterial(2) * nStdDevFactorForRecalculation;
			Math::Vector3ui newInterval = Math::create<Math::Vector3ui>( (nRightBorder + 1) , nNewPeak , (nHistogramDataSize-1) );
			vecDensityIntervals.at(vecDensityIntervals.size()-1)(2)=nRightBorder;
			vecDensityIntervals.push_back(newInterval);

			return newResult;
		}else{
			// no new material found
			return Math::create<Math::Vector4d>(0.0,0.0,0.0,0.0);
		}
	}
	}
}
