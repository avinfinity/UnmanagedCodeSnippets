//****************************************************************************
// (c) 2012 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
#include <openOR/Image/HistogramAnalyser.hpp>

#include <openOR/cleanUpWindowsMacros.hpp>

namespace openOR {
	namespace Image {

		HistogramAnalyser::HistogramAnalyser():
		m_pObjectThreshold(new uint32(0)),
		m_pBackground(new uint32(0)),
		m_background(true),
		m_carrierMaterial(true),
		m_searchforhighdensitycarriermaterial(true)
	{
	}

	HistogramAnalyser::~HistogramAnalyser() {
	}

	void HistogramAnalyser::operator()() const {
		// - make a copy of the histogram 
		// - compute the histpogram intervals and set the corresponding regions
		// - set object threshold and background grey value 

		if (!m_pHistogram || !m_pRegions) 
		{
			// TODO: error Fehlerausgabe mit Fehlerwert und Zeile
			return;
		}

		//make a copy of m_pHistogram
		size_t histSize = getWidth(m_pHistogram);
		boost::shared_array<long long> pHistogram = boost::shared_array<long long>(new long long[histSize]);

		for (int i = 0; i < (int)histSize; i++) 
		{
			pHistogram[i] = m_pHistogram->data()[i];
		}

		// compute the histogram density intervals 
		HistogramAnalyserImpl impl;
		impl.computeDensityIntervals(pHistogram, histSize, m_searchforhighdensitycarriermaterial);
		std::vector<Math::Vector3ui> intervals = impl.getDensityIntervals(m_background, m_carrierMaterial);

		//get regions form the intervals
		m_pRegions->setSize(intervals.size());
		for (int i = 0; i < (int)intervals.size(); i++) 
		{
			Math::Vector3ui v = intervals[i];
			m_pRegions->mutableData()[i].first = v[0];
			m_pRegions->mutableData()[i].second = v[1];
			m_pRegions->mutableData()[i].third = v[2];
		}
		//set objectThreshold and Background grey value
		*m_pObjectThreshold = impl.getObjectThreshold();
		*m_pBackground = impl.getBackgroundPeakGreyValue();
	}

	void HistogramAnalyser::setData(const AnyPtr& data, const std::string& name) {
		std::tr1::shared_ptr<openOR::Image::Image1DDataUI64> pHistogram = interface_cast<openOR::Image::Image1DDataUI64>(data);
		if (pHistogram) {
			m_pHistogram = pHistogram;
		}
		std::tr1::shared_ptr<openOR::Image::Image1DData<openOR::Triple<size_t> > > pRegions = interface_cast<openOR::Image::Image1DData<openOR::Triple<size_t> > >(data);
		if (pRegions) {
			m_pRegions = pRegions;
		}
	}


	uint32 HistogramAnalyser::backgroundPeak() const {
		return *m_pBackground;
	}
	uint32 HistogramAnalyser::objectThreshold() const {
		return *m_pObjectThreshold;
	}

	bool HistogramAnalyser::carrierMaterial() const {
		return m_carrierMaterial;
	}

	void HistogramAnalyser::setCarrierMaterial(bool newCarrierMaterial) {
		m_carrierMaterial = newCarrierMaterial;
	}

	bool HistogramAnalyser::background() const {
		return m_background;
	}

	void HistogramAnalyser::setBackground(bool newBackground) {
		m_background = newBackground;
	}

	bool HistogramAnalyser::searchforhighdensitycarriermaterial() const{
		return m_searchforhighdensitycarriermaterial;

	}
	void HistogramAnalyser::setSearchforhighdensitycarriermaterial(bool newValue){
		m_searchforhighdensitycarriermaterial = newValue;
	}


	HistogramAnalyser::HistogramAnalyserImpl::HistogramAnalyserImpl():
	m_nSectionBeginValue(1),
		m_nSectionMaxvalue(2)
	{
		init();

		// blocking size
		uint nBlockSize = 32; //32
		// filter length for the blocked histogram
		uint nBlockedHistogramFilterSize = 8;

		// filter length for first derivation
		uint nFirstDerivationFilterSize = 3;

		// filter length for second derivation
		uint nSecondDerivationFilterSize = 3;

		// mimimum voxel number for maxima detection in percent
		//float fHistThreshold = 0.01;
		float fHistThreshold = 100.0 * 1.0 / 1250000.0; // 100.0 * 1.0 / 1250000.0;

		// minimum second derivation height for maxima detection relative to number of voxels
		float fSecondDerivationThreshold = fHistThreshold / 50.0;

		// minimum difference between two sections in percent relative to voxel number (for example section begin/end<->peak)

		//float fAbsoluteSectionDifferenceThreshold = 0.02;//2.0;
		float fAbsoluteSectionDifferenceThreshold = 100.0 * (1.0 / 50000); //(100 * 1.0 / 100000)

		// minimum difference between two sections in percent relative to last extreme value (for example section begin/end<->peak)
		float fRelativePeakSectionDifferenceThreshold = -1;//40.0;

		//min number of voxel in section in percent to total voxel number
		float fMinNumOfVoxelInSection = 100 * 0.025 * 0.025 * 0.025;

		// max background peak to carrier material peak distance in percent of grey scale (2^16-1)
		float fCarrierMaterialDistance = 2.5;

		//minimum distance between two peaks in percent of total histogram size
		float fRelativePeakDistanceThreshold = 1.0;//0.5;

		// max background peak to high density carrier material peak distance in percent of grey scale (2^16-1)
		float fHighDensityCarrierMaterialDistance = 10.0;

		setParameter(nBlockSize, nBlockedHistogramFilterSize, nFirstDerivationFilterSize, nSecondDerivationFilterSize, fHistThreshold, fSecondDerivationThreshold, fAbsoluteSectionDifferenceThreshold, fRelativePeakSectionDifferenceThreshold, fMinNumOfVoxelInSection, fCarrierMaterialDistance, fRelativePeakDistanceThreshold, fHighDensityCarrierMaterialDistance);
	}

	HistogramAnalyser::HistogramAnalyserImpl::~HistogramAnalyserImpl()
	{
	}

	void HistogramAnalyser::HistogramAnalyserImpl::init()
	{
		m_vecBlockedHistogram.clear();
		m_vecFilteredHistogram.clear();
		m_vecFirstDerivation.clear();
		m_vecSecondDerivation.clear();
		m_vecTemp.clear();
		m_vecExtremeValues.clear();
		m_vecSections.clear();
		m_vecDensityIntervals.clear();
		m_vecDensityIntervalsWithoutBackground.clear();
		m_vecDensityIntervalsWithoutBackgroundAndCarrierMaterial.clear();
		m_nBackgroundPeakGreyValue = 0;
		m_nObjectThreshold = 0;
	}

	std::vector<Math::Vector3ui> HistogramAnalyser::HistogramAnalyserImpl::getDensityIntervals(bool bBackground,bool bCarrierMaterial)
	{
		if(bBackground)
		{
			if(bCarrierMaterial) return m_vecDensityIntervals;
			else
			{
				// TODO: Fehlermeldung? hier passiert zweimal das gleiche
				return m_vecDensityIntervals;
			}

		}
		else
		{
			if(bCarrierMaterial) return m_vecDensityIntervalsWithoutBackground;
			else                 return m_vecDensityIntervalsWithoutBackgroundAndCarrierMaterial;
		}
	}


	uint HistogramAnalyser::HistogramAnalyserImpl::getBackgroundPeakGreyValue()
	{
		return m_nBackgroundPeakGreyValue;
	}

	uint HistogramAnalyser::HistogramAnalyserImpl::getObjectThreshold()
	{
		return m_nObjectThreshold;
	}

	bool HistogramAnalyser::HistogramAnalyserImpl::computeDensityIntervals(boost::shared_array<long long>& refHistogram, uint nHistogramDataSize, bool bSearchforhighdensitycarriermaterial)
	{
		// - calculate histogram blocks
		// - smooth the histogram
		// - calculate first and second derivation an filter them
		// - calculate extrema and get sections between them
		// - form intervals form the sections
		// - check for saddle points and merge regions that were formed from saddle points
		// - rescale intervals and extract background as well as carrier material (material which holds the object that should be xrayed) 

		init();
		bool bResult;
		try			//TODO: Checken, ob die Funktionen mit Uebergabe von Membern abgeaendert werden koennen 
		{
			//number of voxels
			long long nNumOfVoxels = getNumOfVoxel(refHistogram, nHistogramDataSize);
			if (nNumOfVoxels == 0) throw "invalid histogram (zero voxels)";

			//blocking sums (histogram bins) 
			bResult = blockHistogram(refHistogram, nHistogramDataSize, m_vecBlockedHistogram, m_nBlockSize,m_nBackgroundPeakGreyValue);
			if(!bResult) throw "block histogram";

			m_vecMaskingPeak(0) = -1;
			m_vecMaskingPeak(1) = -1;
			//changed: bResult = modifyHistogram_MaskingGreyValue(m_vecBlockedHistogram,m_vecMaskingPeak,true,2.0,3.0);
			bResult = modifyHistogram_MaskingGreyValue(true,2.0,3.0);
			if(!bResult) throw "gray masking value";

			//smooth histogram
			bResult = filterHistogram(m_vecBlockedHistogram,m_vecFilteredHistogram,m_nBlockedHistogramFilterSize);	// kein Rausschmeissen der Member möglich
			if(!bResult) throw "filter histogram";

			//calc first derivation
			bResult = deriveHistogram(m_vecFilteredHistogram,m_vecTemp); // kein Rausschmeissen der Member möglich
			if(!bResult) throw "calc first derivation";

			//filter first derivation
			bResult = filterHistogram(m_vecTemp,m_vecFirstDerivation,m_nFirstDerivationFilterSize); // kein Rausschmeissen der Member möglich
			if(!bResult) throw "filter first derivation";

			//calc second derivation
			bResult = deriveHistogram(m_vecFirstDerivation,m_vecTemp); // kein Rausschmeissen der Member möglich
			if(!bResult) throw "calc second derivation";

			//filter second derivation
			bResult = filterHistogram(m_vecTemp,m_vecSecondDerivation,m_nSecondDerivationFilterSize); // kein Rausschmeissen der Member möglich
			if(!bResult) throw "filter second derivation";

			//compute extreme values
			bResult =  detectExtremeValues(m_vecExtremeValues,nNumOfVoxels,
				m_vecFilteredHistogram, m_nBlockedHistogramFilterSize,
				m_vecFirstDerivation, m_nFirstDerivationFilterSize,
				m_vecSecondDerivation, m_nSecondDerivationFilterSize,
				m_fHistThreshold, m_fSecondDerivationThreshold);  //TODO: Member rausschmeißen
			if(!bResult) throw "compute extreme value";

			//detect sections  (std::vector<int>& vecExtremeValues,std::vector<double>& BlockedHistogram,ulong nNumOfVoxels, uint nSectionMinimumHeightThreshold,std::vector<uint>& vecSections);
			bResult =  detectSections(m_vecExtremeValues, m_vecBlockedHistogram, nNumOfVoxels, m_fAbsoluteSectionDifferenceThreshold,m_fRelativePeakSectionDifferenceThreshold,m_vecSections,m_fRelativePeakDistanceThreshold,m_nBlockedHistogramFilterSize); //TODO: Member können theoretisch alle raus
			if(!bResult) throw "section detection";

			// build intervals from detected sections
			bResult =  buildIntervals(m_vecSections,m_vecDensityIntervals); //TODO Member können theoretisch alle raus
			if(!bResult) throw "interval building";

			////delete saddle points and merge regions
			bResult =  mergeIntervals(m_vecDensityIntervals,m_vecBlockedHistogram,nNumOfVoxels,m_fRelativePeakSectionDifferenceThreshold,m_fAbsoluteSectionDifferenceThreshold,m_nBlockedHistogramFilterSize,m_fMinNumOfVoxelInSection);
			if(!bResult) throw "interval merging";//TODO Member können theoretisch alle raus

			////delete saddle points and merge regions
			//bResult =  mergeIntervals(m_vecDensityIntervals,m_vecBlockedHistogram,nNumOfVoxels,m_fRelativePeakSectionDifferenceThreshold,m_fAbsoluteSectionDifferenceThreshold,m_nBlockedHistogramFilterSize);
			//if(!bResult) throw "interval merging";

			// find missing maxima in detected intervals
			bResult =  findMissingMaxima(m_vecBlockedHistogram,m_vecDensityIntervals,nNumOfVoxels,m_fAbsoluteSectionDifferenceThreshold,m_fRelativePeakSectionDifferenceThreshold);
			if(!bResult) throw "find missing maxima"; //TODO Member können theoretisch alle raus

			// rescale intervals to original interval
			bResult = rescaleIntervals(m_vecDensityIntervals,m_nBlockSize); //TODO Member können theoretisch alle raus
			if(!bResult) throw "interval rescaling";

			// extract background and carrier material intervals
			float fHighDensityCarrierMaterialDistance(m_fCarrierMaterialDistance);
			if (bSearchforhighdensitycarriermaterial) fHighDensityCarrierMaterialDistance =  m_fHighDensityCarrierMaterialDistance;            
			bResult =  analyzeDetectedIntervals(m_vecDensityIntervals,m_vecDensityIntervalsWithoutBackground,m_vecDensityIntervalsWithoutBackgroundAndCarrierMaterial,m_nBackgroundPeakGreyValue,m_fCarrierMaterialDistance,fHighDensityCarrierMaterialDistance,m_nBlockSize,m_vecBlockedHistogram,m_vecBlockedHistogram.size()*m_nBlockSize); //TODO: alle member raus, nur fHighDensityCarrierMaterialDistance bleibt
			if(!bResult) throw "interval building";

			// adds the removed masking gray value (restore single peak gray value from volume masking (because of projection geometry))
			//changed: bResult = modifyHistogram_MaskingGreyValue(m_vecBlockedHistogram,m_vecMaskingPeak,false);
			bResult = modifyHistogram_MaskingGreyValue(false);
			if(!bResult) throw "gray masking value";

		}
		catch (char * str )
		{
			std::cout << "Exception raised during: computeDensityIntervals - " << str << '\n';
			init();
			return false;
		}
		catch (...)
		{
			std::cout << "UNHANDLED Exception raised during: computeDensityIntervals" << std::endl;
			init();
			return false;
		}
		if(bResult) return true;
		else return false;
	}


	bool HistogramAnalyser::HistogramAnalyserImpl::blockHistogram(const boost::shared_array<long long>& refOriginalHistogram,const uint nHistogramDataSize, std::vector<double>& refBlockedHistogram,const uint nBlockSize,uint& nMaxValue) const
	{
		// - calculate histogram bins

		if(nHistogramDataSize==0)return false;

		//initialize refBlockedHisogram
		uint nBlockedHistogramDataSize = floor(static_cast<float>(nHistogramDataSize)/nBlockSize);
		refBlockedHistogram.clear();
		refBlockedHistogram.reserve(nBlockedHistogramDataSize);

		for(uint i = 0; i < nBlockedHistogramDataSize; i++) {
			refBlockedHistogram.push_back(0.0);
		}

		long long nCurrentMaxValue = 0;
		uint nCurrentMaxValueIndex = 0;
		long long nCurrentValue;
		// Group values within each block
		for(uint i = 0; i < nBlockedHistogramDataSize;i++) {
			for(uint j = 0; j < nBlockSize; j++) {
				nCurrentValue = refOriginalHistogram[(i*nBlockSize)+j];
				if(nCurrentMaxValue < nCurrentValue) {
					nCurrentMaxValue = nCurrentValue;
					nCurrentMaxValueIndex = (i*nBlockSize)+j;
				}
				refBlockedHistogram[i]+=nCurrentValue;
			}
		}
		nMaxValue = nCurrentMaxValueIndex;
		return true;
	}

	bool HistogramAnalyser::HistogramAnalyserImpl::filterHistogram(const std::vector<double>& refOriginalHistogram, std::vector<double>& refFilteredHistogram,const uint nFilterSize)const
	{
		// - smooth the histogram by moving average filter 

		if(refOriginalHistogram.size()<nFilterSize)return false;
		
		//basic implementation of a kind of a moving average filter
		uint nFilteredHistogramDataSize = refOriginalHistogram.size()-nFilterSize;

		refFilteredHistogram.clear();
		refFilteredHistogram.reserve(nFilteredHistogramDataSize);

		double fTemp;
		for(uint i = 0; i < nFilteredHistogramDataSize; ++i) {
			fTemp = 0.0;
			// use a asymmetrical filter
			for(uint k = 0; k < nFilterSize;++k) {
				fTemp += refOriginalHistogram[i+k];
			}
			// normalize and push
			refFilteredHistogram.push_back(fTemp/nFilterSize);
		}
		return true;
	}

	bool HistogramAnalyser::HistogramAnalyserImpl::deriveHistogram(const std::vector<double>& refOriginalHistogram, std::vector<double>& refDerivedHistogram) const {

		// - calculate the first derivation of the histogram

		if(refOriginalHistogram.size() < 2)return false;

		//implementation of the first derivation of the histogram
		uint nDerivedHistogramDataSize = refOriginalHistogram.size() - 1;

		refDerivedHistogram.clear();
		refDerivedHistogram.reserve(nDerivedHistogramDataSize);

		for(uint i = 0; i < nDerivedHistogramDataSize;i++) {
			refDerivedHistogram.push_back(refOriginalHistogram[i+1]-refOriginalHistogram[i]);
		}
		return true;
	}

	bool HistogramAnalyser::HistogramAnalyserImpl::detectExtremeValues(std::vector<int>& vecExtremeValues,long long nNumOfVoxels,std::vector<double>& refFilteredHistogram,uint nFilterSizeFilteredHistogram,std::vector<double>& refFirstDerivation,uint nFilterSizeFirstDerivation,std::vector<double>& refSecondDerivation,uint nFilterSizeSecondDerivation, float fHistThreshold, float fSecondDerivationThreshold)
	{
		// - search for extrema by looking at the first and second derivative of the histogram
		// - if extremum is found set the corresponding index in vecExtremeValues to 10 for a maximum and to -10 fo a minimum

		uint nSize = refFilteredHistogram.size();
		vecExtremeValues.clear();
		vecExtremeValues.reserve(nSize);

		// compute local thresholds
		//float fHistogramThreshold = pow(static_cast<float>(nNumOfVoxels),static_cast<float>(1.0/3.0)) * fHistThreshold;
		//float fSecondDevThreshold = pow(static_cast<float>(nNumOfVoxels),static_cast<float>(1.0/3.0)) * fSecondDerivationThreshold;
		float fHistogramThreshold = static_cast<float>(nNumOfVoxels) * fHistThreshold / 100.0;
		float fSecondDevThreshold = static_cast<float>(nNumOfVoxels) * fSecondDerivationThreshold / 100.0;

		// filtering related offsets
		float fFirst_Offset = static_cast<float>(nFilterSizeFirstDerivation - 1.0) / 2.0;
		float fSecond_Offset = static_cast<float>(nFilterSizeSecondDerivation - 1.0) / 2.0;

		int nFirst_Offset = floor(fFirst_Offset);
		int nSecond_Offset = floor(fSecond_Offset);

		//nFirst_Offset = 0;
		nSecond_Offset = 0;

		// init extreme values vector
		for (uint i=0; i<nSize+nFilterSizeFilteredHistogram; ++i)//nFilterSizeFilteredHistogram
		{
			vecExtremeValues.push_back(0);
		}

		// value for extreme values
		uint nExtremeValue = 10;

		uint nMaxIndex = refSecondDerivation.size() - nSecond_Offset;

		bool bCondA,bCondB,bCondC;

		//search for extreme values
		for (uint i=0; i<nMaxIndex; ++i)
		{
			//check minimum signal height
			if(refFilteredHistogram[i]>fHistogramThreshold)
			{
				bCondA = refFirstDerivation.at(i+nFirst_Offset)==0;
				bCondB = (refFirstDerivation.at(i+nFirst_Offset)>0 && refFirstDerivation.at(i+nFirst_Offset+1) <0);
				bCondC = (refFirstDerivation.at(i+nFirst_Offset)<0 && refFirstDerivation.at(i+nFirst_Offset+1) >0);
				if( bCondA || bCondB || bCondC )
				{
					// maximum?
					if(refSecondDerivation.at(i+nSecond_Offset) < -fSecondDevThreshold)
					{
						vecExtremeValues.at(i+nFilterSizeFilteredHistogram)= nExtremeValue;
					}
					// minimum?
					if (refSecondDerivation.at(i+nSecond_Offset) > fSecondDevThreshold)
					{
						vecExtremeValues.at(i+nFilterSizeFilteredHistogram)= -nExtremeValue;
					}
				}
			}
		}

		return true;
	}

	bool HistogramAnalyser::HistogramAnalyserImpl::detectSections(std::vector<int>& vecExtremeValues,std::vector<double>& vecBlockedHistogram,long long nNumOfVoxels,float fAbsoluteSectionDifferenceThreshold,float fRelativePeakSectionDifferenceThreshold,std::vector<uint>& vecSections,float fRelativePeakDistanceThreshold, uint nBlockedHistogramFilterSize)
	{
		// - devide the histogram into sections. the borders of the sections are located beween every two maxima. every maximum corresponds to a material.

		uint nSize = vecExtremeValues.size();  
		//long long nAbsoluteSectionDifferenceThreshold = floor(pow(static_cast<float>(nNumOfVoxels),static_cast<float>(1.0/3.0)) * fAbsoluteSectionDifferenceThreshold);
		long long nAbsoluteSectionDifferenceThreshold = floor(static_cast<float>(nNumOfVoxels) * fAbsoluteSectionDifferenceThreshold / 100.0);


		//init
		vecSections.clear();
		vecSections.reserve(nSize);
		// init sections vector
		for (uint i=0; i<nSize; ++i)
		{
			vecSections.push_back(0);
		}

		vecSections.at(1) = m_nSectionBeginValue;
		vecSections.at(nSize-1) = m_nSectionBeginValue;

		long long nLastExtremeValue = vecBlockedHistogram.at(0);
		uint nLastExtremeValueIndex = 0;
		bool bCondA,bCondB,bCondC,bCondD;
		long long nCurrentExtremeValue;
		double fCurrentValue;

		for (uint i=1;i<(nSize-nBlockedHistogramFilterSize/2);++i)
		{
			//there is a delay because of filtering => shift by filter size half to the right
			nCurrentExtremeValue = vecExtremeValues.at(i+nBlockedHistogramFilterSize/2);
			if (nCurrentExtremeValue != 0)
			{
				//check minimum signal height
				fCurrentValue = vecBlockedHistogram.at(i);
				//condA: difference between current and last extreme value is larger then section Threshold
				bCondA = abs(fCurrentValue-nLastExtremeValue) > nAbsoluteSectionDifferenceThreshold;
				//condB: difference is larger than relative factor times last extreme value
				if (fRelativePeakSectionDifferenceThreshold>0.0){
					bCondB = abs(fCurrentValue-nLastExtremeValue) > fRelativePeakSectionDifferenceThreshold/100.0 * nLastExtremeValue;
				}else{
					float fThreshold = 1.0 / std::max<float>( (log10(fCurrentValue)-1.0), 0.1);
					bCondB = abs(fCurrentValue-nLastExtremeValue) > fThreshold * nLastExtremeValue;
				}
				//condC:
				bCondC = (vecSections.at(i-1) == m_nSectionMaxvalue);
				//condD:
				bCondD = nCurrentExtremeValue>0;


				if(bCondD)
				{
					if(bCondC || (!bCondC && bCondA &&bCondB)) 
					{
						vecSections.at(i) = m_nSectionMaxvalue;
						nLastExtremeValue=fCurrentValue;
						nLastExtremeValueIndex=i;
					}              
				}
				else
				{
					vecSections.at(i) = m_nSectionBeginValue;
					nLastExtremeValue=fCurrentValue;
					nLastExtremeValueIndex=i;  
				}
			}
		}

		// add section borders between two maxima
		nLastExtremeValueIndex = 1;
		uint nMinPeakDist = (static_cast<float>(nSize) *fRelativePeakDistanceThreshold) / 100.0;  // todo => member parameter
		for (uint i=0;i<nSize;++i)
		{
			if (vecSections.at(i)== m_nSectionMaxvalue && vecSections.at(nLastExtremeValueIndex) == m_nSectionMaxvalue)
			{
				if (abs(static_cast<int>(nLastExtremeValueIndex-i))<nMinPeakDist)
				{
					if (vecBlockedHistogram.at(i) > vecBlockedHistogram.at(nLastExtremeValueIndex) )
					{
						vecSections.at(nLastExtremeValueIndex) = 0;
					} 
					else
					{
						vecSections.at(i) = 0;
					}
				}
				else
				{
					//find min between two maxima
					double fMinValue,fMaxValue;
					int nMinIndex,nMaxIndex;
					if(findMinMax(vecBlockedHistogram,nLastExtremeValueIndex,i,nMinIndex,fMinValue,nMaxIndex,fMaxValue))
					{
						vecSections.at(nMinIndex)=m_nSectionBeginValue;
					}
				}
			}
			if (vecSections.at(i) > 0.0)
			{
				nLastExtremeValueIndex = i;
			}
		}
		return true;
	}


	bool HistogramAnalyser::HistogramAnalyserImpl::buildIntervals(std::vector<uint>& vecSections,std::vector<Math::Vector3ui>&   vecDensityIntervals)
	{
		// - from the sections vector create vecDensityIntervals with begin-index, peak-value and end-index

		vecDensityIntervals.clear();

		//build section vectors
		uint nSize = vecSections.size();  
		uint nCurrentBeginIndex = 1;
		uint nLastSectionIndex = 0;
		while (nCurrentBeginIndex<nSize)
		{
			Math::Vector3ui vecNewSection = Math::create<Math::Vector3ui>(nLastSectionIndex,0,nSize); // nLastSectionIndex,0,0
			for (uint i=nCurrentBeginIndex; i<nSize; ++i)
			{  
				if(vecSections.at(i)==m_nSectionMaxvalue && vecNewSection(1)==0)
				{
					vecNewSection(1) = i;
				}
				if(vecSections.at(i)==m_nSectionBeginValue && vecNewSection(2)==nSize && i>nCurrentBeginIndex)
				{
					nLastSectionIndex=i;
					nCurrentBeginIndex=i+1;
					vecNewSection(2)=i;
					vecDensityIntervals.push_back(vecNewSection);
					break;
				}
			}
		}
		return true;
	}


	bool HistogramAnalyser::HistogramAnalyserImpl::findMissingMaxima(std::vector<double>& refBlockedHistogram,
		std::vector<Math::Vector3ui>&   vecDensityIntervals,
		long long nNumOfVoxels,
		float fAbsoluteSectionDifferenceThreshold,
		float fRelativePeakSectionDifferenceThreshold)
	{
	// - if we didn't find a maximum in a section (intensityIntervals.at(i) = 0):
	// - search for the maximum in the reference blocked Histogram

		for (uint i=0; i < vecDensityIntervals.size();++i)
		{
			// if there is no maximum maybe we missed it
			if(vecDensityIntervals.at(i)(1)==0)
			{
				uint nStartIndex = 0;
				if (vecDensityIntervals.at(i)(0)>0) nStartIndex = vecDensityIntervals.at(i)(0)+1;
				uint nEndIndex = vecDensityIntervals.at(i)(2)-1;
				double fBeginValue = refBlockedHistogram.at(nStartIndex);
				//special case => ideal or masked volume       
				double fEndValue = refBlockedHistogram.at(nEndIndex); 
				int nMinIndex;  double fMinValue; int nMaxIndex; double fMaxValue;
				findMinMax(refBlockedHistogram,nStartIndex,nEndIndex, nMinIndex, fMinValue, nMaxIndex, fMaxValue);

				bool bCondB;         
				if (fRelativePeakSectionDifferenceThreshold>0.0){
					bCondB = fMaxValue > (1.0 + fRelativePeakSectionDifferenceThreshold/100.0) * std::max(fBeginValue,fEndValue);
				}else{
					float fThreshold = 1.0 / std::max<float>( (log10(fMaxValue)-1.0), 0.1);
					bCondB = fMaxValue > (1.0 + fThreshold) * std::max<float>(fBeginValue,fEndValue);
				}

				//new maximum ?
				if(bCondB)
				{
					vecDensityIntervals.at(i)(1) = nMaxIndex;
				}
				else
				{
					nStartIndex = vecDensityIntervals.at(i)(0);
					nEndIndex = vecDensityIntervals.at(i)(2);
					findMinMax(refBlockedHistogram,nStartIndex,nEndIndex, nMinIndex, fMinValue, nMaxIndex, fMaxValue);

					vecDensityIntervals.at(i)(1) = nMaxIndex;
				}
			}
		}
		return true;
	}


	bool HistogramAnalyser::HistogramAnalyserImpl::rescaleIntervals(std::vector<Math::Vector3ui>& vecDensityIntervals,uint nBlockSize)
	{
		// - rescale the intervals by the block size
		Math::Vector3ui vecInterval;
		for (uint i = 0; i < vecDensityIntervals.size();++i)
		{
			vecInterval = vecDensityIntervals.at(i);
			vecDensityIntervals.at(i)(0) = nBlockSize*vecInterval(0);
			if(vecDensityIntervals.at(i)(1)!=0)
			{
				vecDensityIntervals.at(i)(1) = nBlockSize*vecInterval(1)+nBlockSize/2;
			}
			vecDensityIntervals.at(i)(2) = nBlockSize*vecInterval(2)-1;
		}
		return true;
	}

	bool HistogramAnalyser::HistogramAnalyserImpl::mergeIntervals(std::vector<Math::Vector3ui>& vecDensityIntervals,
		std::vector<double>& refBlockedHistogram,
		uint nNumOfVoxels,
		float fRelativePeakSectionDifferenceThreshold,
		float fAbsoluteSectionDifferenceThreshold,
		uint nBlockedHistogramFilterSize,
		float fMinNumOfVoxelInSection)
	{
		// - find saddle point intervals and wrong peaks
		// - merge the surrounding intervals

		long long nAbsoluteSectionDifferenceThreshold = floor(static_cast<float>(nNumOfVoxels) * fAbsoluteSectionDifferenceThreshold / 100.0);

		try
		{
			if(vecDensityIntervals.size()<3)
			{
				return true;
			}

			uint nUintMaxValue = std::numeric_limits<uint>::max();

			uint k=0; // TODO: VARIABLENNAMEN!
			while(k<vecDensityIntervals.size() && vecDensityIntervals.size() > 1)
			{        
				//find saddle point intervals and wrong peaks
				uint i=k;// TODO: VARIABLENNAMEN!
				long long nSectionBeginValue = refBlockedHistogram.at(vecDensityIntervals.at(i)(0));         
				long long nSectionPeakValue = refBlockedHistogram.at(vecDensityIntervals.at(i)(1));         
				long long nSectionEndValue = refBlockedHistogram.at(vecDensityIntervals.at(i)(2));
				//find wrong peaks
				if(vecDensityIntervals.at(i)(1) != 0)
				{
					int nMinIndex;
					double fMinValue;
					int nMaxIndex;
					double fMaxValue;

					//find real section begin value in filter interval
					findMinMax(refBlockedHistogram,std::max<int>(0, static_cast<int>(vecDensityIntervals.at(i)(0))-nBlockedHistogramFilterSize),
						std::min<int>(refBlockedHistogram.size()-1,
						static_cast<int>(vecDensityIntervals.at(i)(0)) + nBlockedHistogramFilterSize),
						nMinIndex, fMinValue, nMaxIndex, fMaxValue);

					if(std::abs(nMinIndex) != vecDensityIntervals.at(i)(0)) {
						nSectionBeginValue = floor(fMinValue + 0.5);
					}

					//find real section end value in filter interval
					findMinMax(refBlockedHistogram,std::max<int>(0,(static_cast<int>(vecDensityIntervals.at(i)(2))-nBlockedHistogramFilterSize)),
						std::min<int>(refBlockedHistogram.size()-1,
						(static_cast<int>(vecDensityIntervals.at(i)(2)) + nBlockedHistogramFilterSize)),
						nMinIndex, fMinValue, nMaxIndex, fMaxValue);

					if(std::abs(nMinIndex) != vecDensityIntervals.at(i)(2)) {
						nSectionEndValue = floor(fMinValue + 0.5);
					}

					//find real max value in filter interval          
					findMinMax(refBlockedHistogram,std::max(vecDensityIntervals.at(i)(0),(vecDensityIntervals.at(i)(1)-nBlockedHistogramFilterSize)),std::min(vecDensityIntervals.at(i)(2),vecDensityIntervals.at(i)(1)+nBlockedHistogramFilterSize), nMinIndex, fMinValue, nMaxIndex, fMaxValue);
					if(std::abs(nMaxIndex) != vecDensityIntervals.at(i)(1)) {
						vecDensityIntervals.at(i)(1) = nMaxIndex;
						nSectionPeakValue = floor(fMaxValue + 0.5);
					}        

					double fRelativeFactor;
					if (fRelativePeakSectionDifferenceThreshold>0.0){
						fRelativeFactor = 1.0 + (fRelativePeakSectionDifferenceThreshold/100.0);
					}else{
						float fThreshold = 1.0 / std::max<float>( (log10(fMaxValue)-1.0), 0.1);
						fRelativeFactor = 1.0 + fThreshold;
					}

					if (fMaxValue<nAbsoluteSectionDifferenceThreshold || (fRelativeFactor*nSectionBeginValue > nSectionPeakValue && i!=0) || fRelativeFactor*nSectionEndValue > nSectionPeakValue || (std::max(nSectionBeginValue,nSectionEndValue)+nAbsoluteSectionDifferenceThreshold)>nSectionPeakValue)
					{
						vecDensityIntervals.at(i)(1)=nUintMaxValue;
					}
				}
				//search for saddle point intervals
				//else
				if(true)
				{
					uint nSearchBeginIndex = vecDensityIntervals.at(i)(0);
					uint nSearchEndIndex = vecDensityIntervals.at(i)(2);
					//double fMinValue,fMaxValue;
					double fMinValue(-1.0);
					double fMaxValue(-1.0);
					int nMinIndex,nMaxIndex;
					if(findMinMax(refBlockedHistogram,nSearchBeginIndex,nSearchEndIndex,nMinIndex,fMinValue,nMaxIndex,fMaxValue))
					{
						double fRelativeFactor;
						if (fRelativePeakSectionDifferenceThreshold>0.0){
							fRelativeFactor = 1.0 + (fRelativePeakSectionDifferenceThreshold/100.0);
						}else{
							float fThreshold = 1.0 / std::max<float>((log10(fMaxValue)-1.0), 0.1);
							fRelativeFactor = 1.0 + fThreshold;
						}

						bool bCondA = (nMaxIndex == 0);
						bool bCondB = (i==vecDensityIntervals.size()-1);
						bool bCondC = fMaxValue > fRelativeFactor*nSectionBeginValue;
						bool bCondD = fMaxValue > fRelativeFactor*nSectionEndValue;
						bool bCondE = fMaxValue - nSectionBeginValue > nAbsoluteSectionDifferenceThreshold;
						bool bCondF = fMaxValue - nSectionEndValue > nAbsoluteSectionDifferenceThreshold;
						if(!(bCondC && bCondD && bCondE && bCondF) && !bCondA && !(bCondB && bCondC))// && bCondC && bCondF))
						{
							vecDensityIntervals.at(i)(1)=nUintMaxValue;
						}
					}
					else vecDensityIntervals.at(i)(1)=nUintMaxValue;

				}
				// has the interval enough voxels?
				if (!hasIntervalEnoughVoxels(refBlockedHistogram,vecDensityIntervals.at(i), nNumOfVoxels * fMinNumOfVoxelInSection))
				{
					vecDensityIntervals.at(i)(1)=nUintMaxValue;
				}

				//remove saddle point intervals and merge surrounding intervals
				if(vecDensityIntervals.at(k)(1)==nUintMaxValue)
				{
					//first element?
					if (k==0)
					{
						vecDensityIntervals.at(k+1)(0)=vecDensityIntervals.at(k)(0);//merge  
					}
					//last element?
					else if(k==vecDensityIntervals.size()-1)
					{
						vecDensityIntervals.at(k-1)(2)=vecDensityIntervals.at(k)(2);//merge  
					}
					//then it must be an interval somewhere between
					else
					{
						//find minimum a new minimum
						//get search area begin index
						uint nSearchBeginIndex = vecDensityIntervals.at(k)(0);
						if(vecDensityIntervals.at(k-1)(1)>0 && vecDensityIntervals.at(k-1)(1) != nUintMaxValue)
						{
							nSearchBeginIndex = vecDensityIntervals.at(k-1)(1); 
						}
						//get search area end index
						uint nSearchEndIndex = vecDensityIntervals.at(k)(2);
						if(vecDensityIntervals.at(k+1)(1)>0 && vecDensityIntervals.at(k+1)(1) != nUintMaxValue)
						{
							nSearchEndIndex = vecDensityIntervals.at(k+1)(1); 
						}
						// search minimum in the defined interval
						double fMinValue,fMaxValue;
						int nMinIndex,nMaxIndex;
						if(findMinMax(refBlockedHistogram,nSearchBeginIndex,nSearchEndIndex,nMinIndex,fMinValue,nMaxIndex,fMaxValue))
						{
							vecDensityIntervals.at(k-1)(2) = nMinIndex;
							vecDensityIntervals.at(k+1)(0) = nMinIndex;
						}
						else return false;

					}
					//delete current interval
					vecDensityIntervals.erase(vecDensityIntervals.begin()+k);
				}
				else
				{
					k++;
				}
			}
			//clean up
			for (uint i=0;i<vecDensityIntervals.size();++i)
			{
				if(vecDensityIntervals.at(i)(1)==nUintMaxValue)
				{
					vecDensityIntervals.at(i)(1) = 0;               
				}
			}
			return true;
		}
		catch (...)
		{
			return false;
		}
		return false;
	}

	bool HistogramAnalyser::HistogramAnalyserImpl::hasIntervalEnoughVoxels(std::vector<double>& refBlockedHistogram, Math::Vector3ui& vecRegion, double dThreshold)
	{
		// - check if interval has more than fThreshold voxels
		if(vecRegion(0)<0 || vecRegion(2)>(refBlockedHistogram.size()-1)) return false;
		double dNumVoxel = 0;
		for (uint i=vecRegion(0);i<=vecRegion(2);++i)
		{
			dNumVoxel += refBlockedHistogram[i];
		}
		if (dNumVoxel>dThreshold)
		{
			return true;
		}
		else
		{   
			return false;
		}
	}

	bool HistogramAnalyser::HistogramAnalyserImpl::findMinMax(const std::vector<double>& refBlockedHistogram,
		const uint nStartIndex,
		const uint nEndIndex,
		int& nMinIndex,
		double& fMinValue,
		int& nMaxIndex,
		double& fMaxValue) const
	{

		// - find both minimum and maximum in the interval from starIndex and EndIndex

		fMinValue = std::numeric_limits<double>::max();
		nMinIndex = -1;
		fMaxValue = std::numeric_limits<double>::min();
		nMaxIndex = -1;

		if (nStartIndex<0 && nEndIndex>refBlockedHistogram.size()-1)
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

	void HistogramAnalyser::HistogramAnalyserImpl::setParameter(uint nBlockSize,uint nBlockedHistogramFilterSize,uint nFirstDerivationFilterSize,uint nSecondDerivationFilterSize,float fHistThreshold,float fSecondDerivationThreshold,float fAbsoluteSectionDifferenceThreshold,float fRelativePeakSectionDifferenceThreshold, float fMinNumOfVoxelInSection, float fCarrierMaterialDistance, float fRelativePeakDistanceThreshold, float fHighDensityCarrierMaterialDistance)
	{
		m_nBlockSize = nBlockSize;
		m_nBlockedHistogramFilterSize = nBlockedHistogramFilterSize;
		m_nFirstDerivationFilterSize = nFirstDerivationFilterSize;
		m_nSecondDerivationFilterSize = nSecondDerivationFilterSize;

		m_fHistThreshold = fHistThreshold;
		m_fSecondDerivationThreshold = fSecondDerivationThreshold;
		m_fAbsoluteSectionDifferenceThreshold = fAbsoluteSectionDifferenceThreshold;
		m_fRelativePeakSectionDifferenceThreshold = fRelativePeakSectionDifferenceThreshold;
		m_fMinNumOfVoxelInSection = fMinNumOfVoxelInSection;
		m_fCarrierMaterialDistance = fCarrierMaterialDistance;
		m_fRelativePeakDistanceThreshold = fRelativePeakDistanceThreshold;
		m_fHighDensityCarrierMaterialDistance = fHighDensityCarrierMaterialDistance;
	}


	long long HistogramAnalyser::HistogramAnalyserImpl::getNumOfVoxel(const boost::shared_array<long long>& refHistogram,const uint nHistogramDataSize) const
	{
		long long nNumOfVoxels = 0;

		for (uint i=0; i<nHistogramDataSize; ++i)
		{
			nNumOfVoxels+=refHistogram[i];
		}

		return nNumOfVoxels;
	}

	bool HistogramAnalyser::HistogramAnalyserImpl::analyzeDetectedIntervals(std::vector<Math::Vector3ui>& vecDensityIntervals,std::vector<Math::Vector3ui>& vecDensityIntervalsWithoutBackground,std::vector<Math::Vector3ui>& vecDensityIntervalsWithoutBackgroundAndCarrierMaterial,uint nBackgroundPeakGreyValue,float fCarrierMaterialDistance, float fHighDensityCarrierMaterialDistance, uint nBlockSize,const std::vector<double>& refBlockedHistogram, uint nTotalGreyScaleInterval)
	{
		// - analyse the materials in vecDensityInterval
		// - find the index of background and carrier material
		// - set the object threshold: below are background and carrier material, above are the real materials

		if(vecDensityIntervals.size()==0) return false;

		double fHighDensitySupportThreshold = 2.0; // empirical threshold ! todo: move to set parameter

		int nIndexOfBackgroundInterval = -1;
		int nIndexOfCarrierMaterialInterval = -1;

		vecDensityIntervalsWithoutBackground.clear();
		vecDensityIntervalsWithoutBackgroundAndCarrierMaterial.clear();

		//compute background interval
		Math::Vector3ui vecInterval = vecDensityIntervals.at(0);
		if(nBackgroundPeakGreyValue >= vecInterval(0) && nBackgroundPeakGreyValue <= vecInterval(2))
		{
			nIndexOfBackgroundInterval=0;
		}

		// search for carrier material interval
		// are there enough materials left? there should be background index + carrier material + at least one object material interval  
		// (carrier material: material that holds the actual object that should be x-rayed) 
		if(nIndexOfBackgroundInterval != -1
			&& vecDensityIntervals.size() > std::abs(nIndexOfBackgroundInterval + 2))
		{
			// get background value index
			uint nBackgroundIntervalPeakIndex = vecDensityIntervals.at(nIndexOfBackgroundInterval)(1);

			// compute absolute distance to background peak
			uint nMaxPeakOffset = static_cast<float>(nTotalGreyScaleInterval) * fCarrierMaterialDistance / 100.0;
			uint nMaxHighDensityPeakOffset = static_cast<float>(nTotalGreyScaleInterval) * fHighDensityCarrierMaterialDistance / 100.0;

			// get peak gray value of next two intervals (if the volume is mask the first interval is the masking value, the second is the "real" background and the third the carrier material)
			uint nPotentialCarrierMaterialPeakIndex_one = vecDensityIntervals.at(nIndexOfBackgroundInterval+1)(1);

			// first check low density support material      
			if (nPotentialCarrierMaterialPeakIndex_one < nMaxPeakOffset + nBackgroundIntervalPeakIndex && nPotentialCarrierMaterialPeakIndex_one!=0) {
				nIndexOfCarrierMaterialInterval = nIndexOfBackgroundInterval+1;
			}
			// now check high density support material (large amount needed!)
			if (nPotentialCarrierMaterialPeakIndex_one < nMaxHighDensityPeakOffset + nBackgroundIntervalPeakIndex && nPotentialCarrierMaterialPeakIndex_one!=0){
				// to prevent material misinterpretation the support material peak must have at least 0.5 time the amount of background peak value
				double fPotentialSupportMaterialPeakAmount = refBlockedHistogram.at(nPotentialCarrierMaterialPeakIndex_one/nBlockSize);
				double fPotentialBackgroundPeakAmount = refBlockedHistogram.at(nBackgroundIntervalPeakIndex/nBlockSize);
				if (fPotentialBackgroundPeakAmount < fHighDensitySupportThreshold * fPotentialSupportMaterialPeakAmount){
					nIndexOfCarrierMaterialInterval = nIndexOfBackgroundInterval+1;         
				}         
			}

			//check the next potential material      
			if(vecDensityIntervals.size() > std::abs(nIndexOfBackgroundInterval + 2)) {

				// first check low density support material
				uint nPotentialCarrierMaterialPeakIndex_two = vecDensityIntervals.at(nIndexOfBackgroundInterval+2)(1);
				if (nPotentialCarrierMaterialPeakIndex_two < nMaxPeakOffset + nBackgroundIntervalPeakIndex && nPotentialCarrierMaterialPeakIndex_two!=0)
				{
					nIndexOfCarrierMaterialInterval = nIndexOfBackgroundInterval+2;
					nIndexOfBackgroundInterval = nIndexOfBackgroundInterval+1;
				}
				// now check high density support material (large amount needed!)
				if (nPotentialCarrierMaterialPeakIndex_two < nMaxHighDensityPeakOffset + nBackgroundIntervalPeakIndex && nPotentialCarrierMaterialPeakIndex_two!=0){
					// to prevent material misinterpretation the support material peak must have at least 0.5 times the amount of background peak value
					double fPotentialSupportMaterialPeakAmount = refBlockedHistogram.at(nPotentialCarrierMaterialPeakIndex_two/nBlockSize);
					double fPotentialBackgroundPeakAmount = refBlockedHistogram.at(nBackgroundIntervalPeakIndex/nBlockSize);
					if (fPotentialBackgroundPeakAmount < fHighDensitySupportThreshold * fPotentialSupportMaterialPeakAmount){
						nIndexOfCarrierMaterialInterval = nIndexOfBackgroundInterval+1;         
					}         
				}
			}
		}

		// copy intervals
		for (uint i = 0; i < vecDensityIntervals.size(); i++) {
			if(i > std::abs(nIndexOfBackgroundInterval)) {
				vecDensityIntervalsWithoutBackground.push_back(vecDensityIntervals.at(i));
				if(i != std::abs(nIndexOfCarrierMaterialInterval)){
					vecDensityIntervalsWithoutBackgroundAndCarrierMaterial.push_back(vecDensityIntervals.at(i));
				}
			}
		}

		// set object threshold (all above background and carrier material)
		if(nIndexOfBackgroundInterval != -1)
		{
			if(nIndexOfCarrierMaterialInterval != -1)
			{
				m_nObjectThreshold = vecDensityIntervals.at(nIndexOfCarrierMaterialInterval)(2);
			}
			else
			{
				m_nObjectThreshold = vecDensityIntervals.at(nIndexOfBackgroundInterval)(2);
			}
		}
		else
		{
			m_nObjectThreshold = 0;
		}

		return true;
		return true;
	}

	bool HistogramAnalyser::HistogramAnalyserImpl::modifyHistogram_MaskingGreyValue(bool bRemove,double fThreshold,double fMaxDiffThreshold)
	{
		// - depending on bRemove: search for a masking peak that is much higher than maxima left and right of it
		// - if desired delte the found maximum (it belongs to a high density material)
		// - else search for a hidden background peak
		if(bRemove)
		{
			double fMinValue,fAbsoluteMaxValue,fLeftMax,fRightMax;
			int nMinIndex,nMaxIndex,nAbsoluteMaxIndex;
			//find the abolute maximum
			findMinMax(m_vecBlockedHistogram, 0, m_vecBlockedHistogram.size() - 1, nMinIndex, fMinValue, nAbsoluteMaxIndex, fAbsoluteMaxValue);
			if(nAbsoluteMaxIndex!=0) {
				//find the maximum in the interval below the absolute maximum index
				findMinMax(m_vecBlockedHistogram, 0, nAbsoluteMaxIndex - 1, nMinIndex, fMinValue, nMaxIndex, fLeftMax);
			} else {
				fLeftMax=0;
			}
			if(std::abs(nAbsoluteMaxIndex) < m_vecBlockedHistogram.size() - 1) {
				//find the maximum in the interval above the absolute maximum index 
				findMinMax(m_vecBlockedHistogram,nAbsoluteMaxIndex + 1, m_vecBlockedHistogram.size() - 1, nMinIndex, fMinValue, nMaxIndex, fRightMax);
			} else {
				fRightMax=m_vecBlockedHistogram.size() - 1;
			}
			//if the absolute maximum is higher than left and right maximum (times a factor) we have found a masking peak
			if (fAbsoluteMaxValue>fThreshold*fLeftMax && fAbsoluteMaxValue>fThreshold*fRightMax) {
				m_vecMaskingPeak(0)=nAbsoluteMaxIndex;
				m_vecMaskingPeak(1)=fAbsoluteMaxValue;

				//delete the masking peak
				m_vecBlockedHistogram.at(nAbsoluteMaxIndex)
					= m_vecBlockedHistogram.at(std::max<int>(nAbsoluteMaxIndex - 1, 0)) * 0.5
					+ m_vecBlockedHistogram.at(std::min<int>(nAbsoluteMaxIndex + 1, m_vecBlockedHistogram.size()-1)) * 0.5;
			}
			else 
			{
				// search for another hidden background peak
				for (uint i=1; i<(m_vecBlockedHistogram.size()-1);++i)
				{
					if(m_vecBlockedHistogram.at(i)>fMaxDiffThreshold*m_vecBlockedHistogram.at(i-1) && m_vecBlockedHistogram.at(i)>fMaxDiffThreshold*m_vecBlockedHistogram.at(i+1) && (m_vecBlockedHistogram.at(i-1)>0 || m_vecBlockedHistogram.at(i+1)>0))
					{
						if(m_vecMaskingPeak(1)<m_vecBlockedHistogram.at(i))
						{
							m_vecMaskingPeak(0)=i;
							m_vecMaskingPeak(1)=m_vecBlockedHistogram.at(i);
						}   
						//m_vecBlockedHistogram.at(i)=m_vecBlockedHistogram.at(std::max<int>((i-1),0))*0.5+m_vecBlockedHistogram.at(std::min<int>((i+1),m_vecBlockedHistogram.size()-1))*0.5;               
						//break;
					}               
				}  
				if (m_vecMaskingPeak(0) != - 1)
				{
					m_vecBlockedHistogram.at(m_vecMaskingPeak(0))=m_vecBlockedHistogram.at(std::max<int>((m_vecMaskingPeak(0)-1),0))*0.5   +    m_vecBlockedHistogram.at(std::min<int>((m_vecMaskingPeak(0)+1),m_vecBlockedHistogram.size()-1))*0.5;               
				}
			}
		}
		else
		{
			if(m_vecMaskingPeak(0)>=0 && m_vecMaskingPeak(1)>=0)
				m_vecBlockedHistogram.at(m_vecMaskingPeak(0)) = m_vecMaskingPeak(1);
		}
		return true;
	}
	}
}
