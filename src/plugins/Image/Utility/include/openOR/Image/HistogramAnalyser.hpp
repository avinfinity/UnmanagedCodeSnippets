//****************************************************************************
// (c) 2012 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
#ifndef openOR_HistogramAnalyser_hpp
#define openOR_HistogramAnalyser_hpp

#include <openOR/Callable.hpp> //basic
#include <openOR/DataSettable.hpp> //basic
#include <openOR/Utility/Types.hpp> //openOR_core
#include <openOR/Math/vector.hpp> //openOr__core

#include <string>
#include <memory>

#include <openOR/Image/Image1DData.hpp> //Image_ImageData

#include <boost/shared_array.hpp>

#include <openOR/Defs/Image_Utility.hpp> 

namespace openOR {
   namespace Image {

      //! Ported from FBPARTViewer (VolumeHistogramAnalyser)
      class OPENOR_IMAGE_UTILITY_API HistogramAnalyser : public Callable, public DataSettable {

      public:
         HistogramAnalyser();
         virtual ~HistogramAnalyser();

         void operator()() const;
         void setData(const AnyPtr& data, const std::string& name = "");
         uint32 objectThreshold() const;
         uint32 backgroundPeak() const;

         bool carrierMaterial() const;
         void setCarrierMaterial(bool newCarrierMaterial);

         bool background() const;
         void setBackground(bool newBackground);

         bool searchforhighdensitycarriermaterial() const;
         void setSearchforhighdensitycarriermaterial(bool newValue);

      private:
         // openOR-secific stuff
         std::tr1::shared_ptr<Image1DData<Triple<size_t> > > m_pRegions;
         std::tr1::shared_ptr<Image1DDataUI64> m_pHistogram;
         std::tr1::shared_ptr<uint32> m_pObjectThreshold, m_pBackground;
         bool m_background, m_carrierMaterial, m_searchforhighdensitycarriermaterial;

         // FBPARTViewer copied stuff
         // needed to create a new class because of const constraint on operator()()
         struct HistogramAnalyserImpl {
            HistogramAnalyserImpl();
            ~HistogramAnalyserImpl();

            std::vector<Math::Vector3ui> getDensityIntervals(bool bBackground = true,bool bCarrierMaterial=true);
            uint getBackgroundPeakGreyValue();
            uint getObjectThreshold();

            void setParameter(uint nBlockSize,uint nBlockedHistogramFilterSize,uint nFirstDerivationFilterSize,uint nSecondDerivationFilterSize,float fHistThreshold,float fSecondDerivationThreshold,float fAbsoluteSectionDifferenceThreshold,float fRelativePeakSectionDifferenceThreshold, float fMinNumOfVoxelInSection,float fCarrierMaterialDistance,float fRelativePeakDistanceThreshold, float fHighDensityCarrierMaterialDistance);
            bool computeDensityIntervals(boost::shared_array<long long>& m_Histogram, uint m_nHistogramDataSize, bool bSearchforhighdensitycarriermaterial = true);

         private:

            // blocking size
            uint m_nBlockSize;

            // filter length for the blocked histogram
            uint m_nBlockedHistogramFilterSize;

            // filter length for first derivation
            uint m_nFirstDerivationFilterSize;

            // filter length for second derivation
            uint m_nSecondDerivationFilterSize;

            // mimimum voxel number for maxima detection in percent
            float m_fHistThreshold;

            // minimum second derivation height for maxima detection relative to number of voxels in percent
            float m_fSecondDerivationThreshold;

            // minimum difference between two sections in percent relative to voxel number
            float m_fAbsoluteSectionDifferenceThreshold;

            // minimum difference between a peak and section borders in percent
            float m_fRelativePeakSectionDifferenceThreshold;

            //min number of voxel in section
            float m_fMinNumOfVoxelInSection;

            //minimum distance between two peaks in percent of total histogram size
            float m_fRelativePeakDistanceThreshold;

            // relative density based on background gray value
            float m_fCarrierMaterialDistance;

            // relative density based on background gray value
            float m_fHighDensityCarrierMaterialDistance;

            bool blockHistogram(const boost::shared_array<long long>& refHistogram,const uint nHistogramDataSize, std::vector<double>& refBlockedHistogram,const uint nBlockSize,uint& nMaxValue) const;
            bool filterHistogram(const std::vector<double>& refOriginalHistogram, std::vector<double>& refFilteredHistogram,const uint nFilterSize) const;
            bool deriveHistogram(const std::vector<double>& refOriginalHistogram, std::vector<double>& refDerivedHistogram) const;

            bool detectExtremeValues(std::vector<int>& vecExtremeValues,long long NumOfVoxels, std::vector<double>& FilteredHistogram, uint nFilterSizeFilteredHistogram, std::vector<double>& FirstDerivation, uint nFilterSizeFirstDerivation,std::vector<double>& SecondDerivation, uint nFilterSizeSecondDerivation, float fHistThreshold, float SecondDerivationThreshold);
            bool detectSections(std::vector<int>& vecExtremeValues,std::vector<double>& BlockedHistogram,long long nNumOfVoxels,float fAbsoluteSectionDifferenceThreshold,float fRelativePeakSectionDifferenceThreshold,std::vector<uint>& vecSections,float fRelativePeakDistanceThreshold,uint nBlockedHistogramFilterSize);
            bool buildIntervals(std::vector<uint>& vecSections,std::vector<Math::Vector3ui>& vecDensityIntervals);
            bool findMissingMaxima(std::vector<double>& refBlockedHistogram,std::vector<Math::Vector3ui>&   vecDensityIntervals,long long nNumOfVoxels,float fAbsoluteSectionDifferenceThreshold,float fRelativePeakSectionDifferenceThreshold);
            bool mergeIntervals(std::vector<Math::Vector3ui>& vecDensityIntervals,std::vector<double>& refBlockedHistogram,uint nNumOfVoxels,float fRelativePeakSectionDifferenceThreshold, float fAbsoluteSectionDifferenceThreshold,uint nBlockedHistogramFilterSize, float fMinNumOfVoxelInSection);
            bool rescaleIntervals(std::vector<Math::Vector3ui>& vecDensityIntervals,uint nBlockSize);
            bool analyzeDetectedIntervals(std::vector<Math::Vector3ui>& vecDensityIntervals,std::vector<Math::Vector3ui>& vecDensityIntervalsWithoutBackground,std::vector<Math::Vector3ui>& vecDensityIntervalsWithoutBackgroundAndCarrierMaterial,uint nBackgroundPeakGreyValue,float fCarrierMaterialCoeff, float fHighDensityCarrierMaterialDistance, uint nBlockSize,const std::vector<double>& refBlockedHistogram, uint nTotalGreyScaleInterval);
            bool findMinMax(const std::vector<double>& BlockedHistogram,const uint nStartIndex,const uint nEndIndex, int& nMinIndex, double& fMinValue, int& nMaxIndex, double& fMaxValue) const;
            //has interval enough voxels?
            bool hasIntervalEnoughVoxels(std::vector<double>& refBlockedHistogram,Math::Vector3ui& vecRegion, double fThreshold);    

            //scan the histogram for masking grew value => bRemove=true removes it bRemove=false adds it
            bool modifyHistogram_MaskingGreyValue(bool bRemove,double fThreshold = 5.0,double fMaxDiffThreshold = 100.0);

            // copmute the total number of voxels of the volume
            long long getNumOfVoxel(const boost::shared_array<long long>& refHistogram,const uint nHistogramDataSize) const;

            void init();

            std::vector<double>  m_vecBlockedHistogram;
            std::vector<double>  m_vecFilteredHistogram;
            std::vector<double>  m_vecFirstDerivation;
            std::vector<double>  m_vecSecondDerivation;
            std::vector<double>  m_vecTemp;
            std::vector<int>   m_vecExtremeValues;
            std::vector<uint> m_vecSections;
            uint m_nBackgroundPeakGreyValue;
            uint m_nObjectThreshold;
            Math::Vector2ll m_vecMaskingPeak;
            std::vector<Math::Vector3ui>   m_vecDensityIntervals;
            std::vector<Math::Vector3ui>   m_vecDensityIntervalsWithoutBackground;
            std::vector<Math::Vector3ui>   m_vecDensityIntervalsWithoutBackgroundAndCarrierMaterial;

            uint m_nSectionBeginValue;
            uint m_nSectionMaxvalue;
         };
      };
   }
}
#endif //openOR_HistogramAnalyser_hpp