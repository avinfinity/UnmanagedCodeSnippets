//****************************************************************************
// (c) 2012 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
#ifndef openOR_HistogramProbabilityAnalyser_hpp
#define openOR_HistogramProbabilityAnalyser_hpp

#include <openOR/Callable.hpp> //basic
#include <openOR/DataSettable.hpp> // basic
#include <openOR/Utility/Types.hpp> //openOR_core
#include <openOR/Math/vector.hpp> //openOR_core

#include <string>
#include <memory>

#include <openOR/Image/Image1DData.hpp>

#include <boost/shared_array.hpp>

#include <openOR/Defs/Image_Utility.hpp>

namespace openOR {
	namespace Image {

		
		class OPENOR_IMAGE_UTILITY_API HistogramProbabilityAnalyser : public Callable, public DataSettable {

		public:
			HistogramProbabilityAnalyser();
			virtual ~HistogramProbabilityAnalyser();

			void operator()() const;
			void setData(const AnyPtr& data, const std::string& name = "");
			void setHighDensityMaterialSearch(bool b){ m_bProbabilityBasedHighDensityMaterialSearch = b; };

		private:
			
			std::tr1::shared_ptr<Image1DData<Triple<size_t> > > m_pRegions;
			std::tr1::shared_ptr<Image1DData<Quad<double> > > m_pResult;
			std::tr1::shared_ptr<Image1DDataUI64> m_pHistogram;
			bool m_bProbabilityBasedHighDensityMaterialSearch;

			
			struct HistogramProbabilityAnalyserImpl {
				HistogramProbabilityAnalyserImpl() {};
				~HistogramProbabilityAnalyserImpl() {};

				std::vector <Math::Vector4d> getSplittedNormalDistributions(boost::shared_array<long long> Histogram, uint nHistogramDataSize,std::vector<Math::Vector3ui>& vecDensityIntervals,bool bProbabilityBasedHighDensityMaterialSearch = true,bool bLog10Normalization=false,uint nNumOfRefinements = 0);

			private:

				Math::Vector4d getSplittedNormalDistributionOfAHistogram(boost::shared_array<long long> vecData, uint m_nHistogramDataSize, uint nBeginIndex, uint nEndIndex, uint nPeak,bool bLog10Normalization);
				Math::Vector4d refineSplittedNormalDistributionOfAHistogram(boost::shared_array<long long> vecData,boost::shared_array<long long> vecDummyData,uint nHistogramDataSize, Math::Vector4d vecPre,Math::Vector4d vecCurrent,Math::Vector4d vecPost,bool bLog10Normalization);
				Math::Vector4d probabilityBasedHighDensityMaterialSearch(boost::shared_array<long long> pHistogram, uint nHistogramDataSize, uint nLeftBorder, uint nRightBorder, uint nPeak,bool bLog10Normalization,std::vector<Math::Vector3ui>& vecDensityIntervals,Math::Vector4d& vecCurrentLastMaterial);
				double computeNormalDistributedProbability(double mean,double stddev,double x);
				Math::Vector2d getNormalDistributionOfAHistogram(std::vector<long long>& vecData,bool bLog10Normalization);
			};
		};
	}
}
#endif //openOR_HistogramAnalyser_hpp