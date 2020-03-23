//****************************************************************************
// (c) 2012 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
#ifndef openOR_HistogramMerger_hpp
#define openOR_HistogramMerger_hpp

#include <openOR/Callable.hpp> //basic
#include <openOR/DataSettable.hpp> //basic
#include <openOR/Utility/Types.hpp> //openOr_core
#include <openOR/Math/vector.hpp> //openOr_core

#include <string>
#include <memory>

#include <openOR/Image/Image1DData.hpp> //Image_ImageData

#include <boost/shared_array.hpp>

#include <openOR/Defs/Image_Utility.hpp>

namespace openOR {
   namespace Image {

      
      class OPENOR_IMAGE_UTILITY_API HistogramMerger : public Callable, public DataSettable {

      public:
         HistogramMerger();
         virtual ~HistogramMerger();

         void operator()() const;
         void setData(const AnyPtr& data, const std::string& name = "");

         std::vector<int> getShiftCorrections() const;

      private:

         std::vector<std::tr1::shared_ptr<openOR::Image::Image1DData<openOR::Triple<size_t> > > > m_pRegions;
         std::tr1::shared_ptr<Image1DDataUI64> m_pHistogram;
         std::tr1::shared_ptr<openOR::Image::Image1DData<openOR::Triple<size_t> > > m_pResultRegions;
         std::tr1::shared_ptr<std::vector<int> > m_shiftCorrections;

        
         // needed to create a new class because of const constraint on operator()()
         struct HistogramMergerImpl {
         public:
            HistogramMergerImpl();
            ~HistogramMergerImpl();

            std::vector<Math::Vector3ui> mergeDensityIntervals(boost::shared_array<long long>& m_Histogram, uint m_nHistogramDataSize, std::vector<std::vector<Math::Vector3ui> >& vecDensityIntervals);
            std::vector<int> computeDensityIntervalShifts(std::vector< std::vector<Math::Vector3ui> >& vecDensityIntervals); //TODO vielleicht umbennenen. Name ist der Funktion																																				//computeDensityIntervalShift sehr ähnlich

         private:

            // merging functions
            bool mergeTwoDensityIntervals(boost::shared_array<long long>& Histogram, uint nHistogramDataSize, std::vector<Math::Vector3ui> vecFirstDensityInterval,std::vector<Math::Vector3ui> vecSecondDensityInterval,std::vector<Math::Vector3ui>& resultDensityIntervals);
            bool areTwoDensityIntervalsEqual(std::vector<Math::Vector3ui> vecFirstDensityInterval, std::vector<Math::Vector3ui> vecSecondDensityInterval,uint nThreshold);
            double computeDensityIntervalShift(std::vector<Math::Vector3ui>& vecFirstDensityInterval, std::vector<Math::Vector3ui>& vecSecondDensityInterval,uint nThreshold);
            bool hasRegionEnoughVoxels(boost::shared_array<long long>& Histogram, uint nHistogramDataSize,Math::Vector3ui& vecRegion, long long nThreshold);
            bool correctIntervalShift(std::vector<Math::Vector3ui>& vecDensityInterval,float fShift);

            bool mergeTwoDensityIntervals_XYZ(boost::shared_array<long long>& Histogram, uint nHistogramDataSize, std::vector<Math::Vector3ui>& vecFirstDensityInterval,std::vector<Math::Vector3ui>& vecSecondDensityInterval,std::vector<Math::Vector3ui>& resultDensityIntervals,uint& nFirstIndex, uint&nSecondIndex,uint nShiftThreshold);
            bool mergeTwoDensityIntervals_XYnZ(boost::shared_array<long long>& Histogram, uint nHistogramDataSize, std::vector<Math::Vector3ui>& vecFirstDensityInterval,std::vector<Math::Vector3ui>& vecSecondDensityInterval,std::vector<Math::Vector3ui>& resultDensityIntervals,uint& nFirstIndex, uint&nSecondIndex,uint nShiftThreshold);
            bool mergeTwoDensityIntervals_XnYZ(boost::shared_array<long long>& Histogram, uint nHistogramDataSize, std::vector<Math::Vector3ui>& vecFirstDensityInterval,std::vector<Math::Vector3ui>& vecSecondDensityInterval,std::vector<Math::Vector3ui>& resultDensityIntervals,uint& nFirstIndex, uint&nSecondIndex,uint nShiftThreshold);
            bool mergeTwoDensityIntervals_XnYnZ(boost::shared_array<long long>& Histogram, uint nHistogramDataSize, std::vector<Math::Vector3ui>& vecFirstDensityInterval,std::vector<Math::Vector3ui>& vecSecondDensityInterval,std::vector<Math::Vector3ui>& resultDensityIntervals,uint& nFirstIndex, uint&nSecondIndex,uint nShiftThreshold);
            bool mergeTwoDensityIntervals_nXYZ(boost::shared_array<long long>& Histogram, uint nHistogramDataSize, std::vector<Math::Vector3ui>& vecFirstDensityInterval,std::vector<Math::Vector3ui>& vecSecondDensityInterval,std::vector<Math::Vector3ui>& resultDensityIntervals,uint& nFirstIndex, uint&nSecondIndex,uint nShiftThreshold);
            bool mergeTwoDensityIntervals_nXYnZ(boost::shared_array<long long>& Histogram, uint nHistogramDataSize, std::vector<Math::Vector3ui>& vecFirstDensityInterval,std::vector<Math::Vector3ui>& vecSecondDensityInterval,std::vector<Math::Vector3ui>& resultDensityIntervals,uint& nFirstIndex, uint&nSecondIndex,uint nShiftThreshold);
            bool mergeTwoDensityIntervals_nXnYZ(boost::shared_array<long long>& Histogram, uint nHistogramDataSize, std::vector<Math::Vector3ui>& vecFirstDensityInterval,std::vector<Math::Vector3ui>& vecSecondDensityInterval,std::vector<Math::Vector3ui>& resultDensityIntervals,uint& nFirstIndex, uint&nSecondIndex,uint nShiftThreshold);
            bool mergeTwoDensityIntervals_nXnYnZ(boost::shared_array<long long>& Histogram, uint nHistogramDataSize, std::vector<Math::Vector3ui>& vecFirstDensityInterval,std::vector<Math::Vector3ui>& vecSecondDensityInterval,std::vector<Math::Vector3ui>& resultDensityIntervals,uint& nFirstIndex, uint&nSecondIndex,uint nShiftThreshold);

            long long getNumOfVoxel(const boost::shared_array<long long>& refHistogram,const uint nHistogramDataSize) const;
            bool findMinMax(const std::vector<double>& BlockedHistogram,const uint nStartIndex,const uint nEndIndex, int& nMinIndex, double& fMinValue, int& nMaxIndex, double& fMaxValue) const;
            bool wasMergingSuccessful(std::vector<std::vector<Math::Vector3ui> >& vecDensityIntervals,std::vector<Math::Vector3ui>& vecResultInterval, uint nThreshold) const;

            bool shiftCorrectionForFollowingIntervals(std::vector<Math::Vector3ui>& vecIntervals,uint& nIndex, int nShift);

            double m_fMinNumOfVoxelInSection;
         };
      };
   }
}
#endif //openOR_HistogramMerger_hpp
