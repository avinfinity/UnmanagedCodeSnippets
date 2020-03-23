//****************************************************************************
// (c) 2012 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
#ifndef openOR_ZeissBackend_hpp
#define openOR_ZeissBackend_hpp

#include <openOR/Defs/Zeiss_ZeissBackend.hpp>
#include <openOR/Image/Image1DData.hpp> //Image_ImageData
#include <openOR/Image/Image2DData.hpp> //Image-ImageData
#include <openOR/Image/Image3DData.hpp> //Image_ImageData
#include <openOR/Image/Image3DSizeDescriptor.hpp>  //Image_ImageData
#include <openOR/Image/DataCollector.hpp> //Image_Utility

#include <openOR/Math/vector.hpp> //OpenOR_core

#include <openOR/Progressable.hpp> //basic
#include <openOR/Cancelable.hpp> //basic

#include <boost/tr1/memory.hpp>

#include <vector>
#include <string>
#include <memory>

#define DEBUGINFOS 1

namespace openOR {

	struct ROIContainer;

	struct OPENOR_ZEISS_ZEISSBACKEND_API ZeissBackend : Progressable, Cancelable {

		// For Helper functions
		typedef  std::tr1::shared_ptr<Image::Image3DDataUI16> VolumeType;
		typedef  std::pair<Math::Vector3ui, Math::Vector3ui>  ROIType;

		ZeissBackend();
		virtual ~ZeissBackend();

		// Interface for external Data
		void setInputFilename(const std::string& filename);  // Defaults to "input.vgi"
		void setInputDataDesciptor(const std::string& filename, const std::tr1::shared_ptr<Image::Image3DSizeDescriptor>& pSize, size_t skipHeader); // for the case of reading a naked raw file
		void setOutputFilename(const std::string& basename);
		void setData(const AnyPtr& data);
		void setVolume(VolumeType pVolume);
		void setOutputVolume(VolumeType pVolume);

		// Parameters
		void setMaxMemorySize(const size_t& maxMemorySize);
		void setDownSampler(const int& downsampler);
		void setBorderSize(size_t borderSize);
		void setMinRegionSize(size_t minEdgeLength, size_t minRegionVolume);
		void setUseMultiScaleHistogram(int newUseMultiScaleHistogram);
		void setCorrectRegionShift(bool newCorrectRegionShift);
		void setOutputMaterialIndex(bool materialIndex);
		void setExpandMaskTimes(int expandMask);
		void setAutoScaling(bool autosclaing);
		void setNumberSeparationThreads(const unsigned int& numberSeparationThreads);
		void setSearchForHighDensityCarrierMaterial(bool bSearch);

		// Individual Workflow Steps 
		// \warning Call all Steps in the listed order!
		void load();                               // Step 1
		void createHistogramRegions();             // Step 2
		void calclulateRegionsOfInterest();        // Step 4
		void separateAndSave();                    // Step 5

		virtual void operator()();                 // Alternative: Do the entire workflow in one go

		// Benchmark
		double doBenchmark(char* path, size_t size, bool reset);
		void setHistTime(size_t time);

		double saveBenchmark(bool reset);
		void setBenchmarkFile(char* path, size_t size);
		double getSegmentationTimeEstimation(int nNumMats);
		double getMaterialRegionsTimeEstimation(size_t nCurrentWidth, size_t nCurrentHeight, size_t nCurrentDepth);

		// Segmentation
		void segmentMaterials();
		unsigned int firstMaterialIndex();
		void setFirstMaterialIndex(unsigned int nFirstMaterialIndex);
		unsigned int segmentationRadius();
		void setSegmentationRadius(unsigned int nSegmentationRadius);


		size_t objectThreshold() const;            // Todo: this should be done the usual way (i.e. through setData)
		void overrideObjectThreshold(size_t t);    // works only if you do that after createHistogramRegions and before calcROI
		size_t background();
		Math::Vector3ui getScalingFactor();        // valid only after load()

		std::tr1::shared_ptr<Image::Image3DDataUI16> objectMap() const;
		std::tr1::shared_ptr<Image::Image3DDataUI16> materialMap() const;
		std::tr1::shared_ptr<Image::Image3DDataUI16> volume() const;

		// cancelable interface
		void cancel();
		bool isCanceled() const;

		// progressable interface
		double progress() const;
		std::string description() const;

		// special helper to make it possible to use external Data as input
		std::tr1::shared_ptr<Image::DataCollector<uint16> > createVolumeCollector(Math::Vector3ui&);

		size_t usedHistogramResolutions();

	private:
		// workflow helper 
		void loadFile(const std::string& fileName);
		void separateAndSave(const std::string& basename);

		// Segmentation / multi-res Histogram helper
		//std::vector<unsigned int> computeMultiResolutionFactors(std::tr1::shared_ptr<Image::Image3DDataUI16> pVolume, Math::Vector3ui minSize, bool bDimensionsOrVoxels = false, bool bUseMultipleOfTwo = true, uint nResolutions = 3,bool bLargeVolumeAdjustment = true);
		void correctRegionShifts(std::vector<std::tr1::shared_ptr<Image::Image1DData<openOR::Triple<size_t> > > >& vecMultiResolutionDensityIntervals, const std::vector<int>& vecShifts) const;

		// Export helper
		void expandRegionMasks();
		void purgeMask();

		// Input
		std::string                                          m_inputFN;
		std::string                                          m_outputBN;
		// Input descriptor
		std::tr1::shared_ptr<Image::Image3DSizeDescriptor>   m_pInputSizeDescriptor;
		size_t                                               m_inputSkipHeader;
		bool                                                 m_isRawInput;

		// Data
		VolumeType                                           m_pVolumeData;
		std::tr1::shared_ptr<Image::Image1DDataUI64>         m_pHistogramFirst;
		std::tr1::shared_ptr<Image::Image1DDataUI64>         m_pHistogramSecond;
		std::tr1::shared_ptr<Image::Image1DDataUI64>         m_pHistogramThird;
		std::tr1::shared_ptr<Image::Image1DDataUI64>         m_pHistogramFourth;
		std::tr1::shared_ptr<Image::Image1DData<Triple<size_t> > > m_pRegionsOrg, m_pRegionsHalf, m_pRegionsThird, m_pRegionsFourth, m_pRegions; 
		std::tr1::shared_ptr<Image::Image1DData<Quad<double> > > m_pProbabilities;
		size_t                                               m_objectThreshold;
		size_t                                               m_background;
		std::tr1::shared_ptr<ROIContainer>                   m_pVROIs;
		VolumeType                                           m_pOutputMap;
		std::tr1::shared_ptr<ROIContainer>                   m_pMaterialBoundingBoxes;
		size_t                                               m_usedHistogramResolutions;

		// for progress tracking
		void setCurrentStep(const AnyPtr& step);
		std::tr1::shared_ptr<Progressable>                   m_pCurrentStep;
		size_t                                               m_currentStepNum;
		const size_t                                         m_numSteps;
		bool                                                 m_inOneGoFlag;

		// for canceling
		std::tr1::shared_ptr<Cancelable>                     m_pCanCancelCurrentStep;
		bool                                                 m_canceled;

		// config
		size_t                                               m_maxMemorySize;
		int                                                  m_downSampler;
		size_t                                               m_ROIBorderSize;
		size_t                                               m_ROIMinEdgeLength;
		size_t                                               m_ROIMinVolume;
		bool                                                 m_outputMaterialIndex;
		unsigned int                                         m_nFirstMaterialIndex;
		unsigned int                                         m_nSegmentationRadius;
		int                                                  m_expandMaskTimes;
		bool                                                 m_bGPU;
		float                                                m_fProbabilityThreshold;
		size_t                                               m_nFilterRadius;
		size_t                                               m_nOpenMPCurrentThreads;
		size_t                                               m_nOpenMPMaxThreads;
		float                                                m_fCPUPerformanceValue;
		bool                                                 m_bAutoScaling;
		bool                                                 m_bMTSurfSegment;
		unsigned int                                         m_numberSeparationThreads;
		bool                                                 m_bSearchForHighDensityCarrierMaterial;

		// for benchmark
		std::string                                          m_strPathBenchmark;
		size_t                                               m_timeNormalDistribution;
		size_t                                               m_timeFilterRadius;
		size_t                                               m_timePostprocessing;
		size_t                                               m_timeHist;

		// state tracking
		bool                                                 m_isScaling;
		// 1: use only one histogram
		// 2: use two histograms sampled -1 and -2
		// 3: use three histograms sampled -1, -2 and -3
		// 4: use four histograms sampled -1, -2 and -4
		int                                                  m_useMultiScaleHistogram;
		bool                                                 m_correctRegionShift;
		Math::Vector3ui                                      m_scalingFactor;
	};
}

#endif //openOR_ZeissBackend_hpp
