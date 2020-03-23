//****************************************************************************
// (c) 2012 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
#ifndef openOR_ProbabilitySegmentor_hpp
#define openOR_ProbabilitySegmentor_hpp

#include <openOR/Progressable.hpp> //basic
#include <openOR/Cancelable.hpp> //basic

#include <openOR/Image/Image3DData.hpp> //image_ImageData
#include <openOR/Image/Image1DData.hpp> //Image_ImageData

#include <vector>
#include <boost/shared_array.hpp>

namespace openOR {
   
   struct ProbabilitySegmentation : Progressable, Cancelable {

      ProbabilitySegmentation();
      ~ProbabilitySegmentation();

      void setData(const AnyPtr& data, const std::string& tag);
      
      void setRadius(unsigned int nRadius);
      void setOutputMaterialIndex(bool materialIndex);
      void setFirstMaterialIndex(unsigned int nFirstMaterialIndex);
      void setGPU(bool bGPU);
      void setCPUPerformanceValue(float fCPUPerformanceValue);

      void operator ()();

      // cancelable interface
      void cancel();
      bool isCanceled() const;

      // progressable interface
      double progress() const;
      std::string description() const;

      // get timings
      size_t getTimeNormalDistribution(){return m_timeNormalDistribution;}
      //size_t getTimePrimaryWeightingvector(){return m_timePrimaryWeightingvector;}
      //size_t getTimeSecondaryWeightingvector(){return m_timeSecondaryWeightingvector;}
      size_t getTimeFilterRadius(){return m_timeFilterRadius;}
      size_t getTimePostprocessing(){return m_timePostprocessing;}
      size_t getFilterRadius(){return m_nRadius;}
      float getProbabilityThreshold(){return m_fProbabilityThreshold;}
      float getCPUPerformanceValue(){return m_fCPUPerformanceValue;}
      size_t getOpenMPMaxThreads(){return m_nOpenMPMaxThreads;}
      size_t getOpenMPCurrentThreads(){return m_nOpenMPCurrentThreads;}

   private:
      typedef Image::Image3DDataUI16::ValueType VoxelType;

      bool doProbabilitySegmentation();
      bool doProbabilitySegmentationPostProcessing();

      bool filterVariableRadius(unsigned int nRadius, std::vector<Math::Vector4d>& vecPrimaryWeightingVector,
                                std::vector<Math::Vector4d>& vecSecondaryWeightingVector);
      bool computePobabilitySlices(std::vector<double*>& vecSlices, std::vector<VoxelType*>& vecOriginalSlices,
                                   const unsigned int nRadius, const unsigned int nSlice, const bool bShift = false);
      void getProbabilitySlice(double* pSlice, int nSlice);
      bool computeWeightingVector(int nRadius, std::vector<Math::Vector4d>& vecWeightingVector, double fCentralVoxelNominator,
                                  double fCentralVoxelDenominator);
      bool computeNormalDistributedProbabilityLookUpTable(unsigned int minvalue, unsigned int maxvalue, size_t nSize);
      double computeNormalDistributedProbability(double mean, double stddev, double x);
      void getVoxelProbabilityVector(double voxelValue, double fWeighting, double* vecResultVector, unsigned int nResultVectorSize);
      int getMaterialIndex(std::vector<double*>& vecSlices, std::vector<Math::Vector4d>& vecWeightingVector,
                           double* pResultVector, unsigned int x, unsigned int y, unsigned int nWidth, unsigned int nHeight,
                           unsigned int nCentralSlice, unsigned int nNumOfRegions, unsigned int nSliceArea,
                           bool bUseThreshold = true);
      //int getMaterialIndex(std::vector<VoxelType*>& vecOriginalSlices, double x, double y, double z, uint nWidth, uint nHeight);
      void setBorderPixelToZero(unsigned int nRadius);

      //int getGradientBasedMaterialIndex(std::vector<VoxelType*>& vecOriginalSlices, uint nProbabilityRadius, uint x, uint y,
      //                                  uint nWidth, uint nHeight);
      //Math::Vector3d getComputeTotalGradientDirection(std::vector<VoxelType*>& vecOriginalSlices, uint x, uint y,
      //                                                uint nWidth, uint nHeight, int nCentralSlice, int nGradientCalcRadius);
      //Math::Vector3d getInterpolatedGreyValueAndProbabilityValue(std::vector<VoxelType*>& vecOriginalSlices,
      //                                                           float x,float y, float z, uint nWidth, uint nHeight );

      //Data
      std::tr1::shared_ptr<Image::Image3DDataUI16> m_pVolumeData;                      // Input
      std::tr1::shared_ptr<Image::Image1DData<Quad<double> > > m_pVecProbabilities;    // Input

      std::tr1::shared_ptr<Image::Image3DDataUI16> m_pVolumeMaterialMap;               // Output 

      //intermediate results
      std::vector<double> m_probabilityLookUpTable;

      //parameters
      bool         m_outputMaterialIndex;
      unsigned int m_nFirstMaterialIndex;
      unsigned int m_nRadius;
      float       m_fProbabilityThreshold;
      unsigned int m_fAdditionalOriginalSlicesFactor;
      unsigned int m_nMaskSize;
      int          m_nIter;
      size_t       m_nOpenMPMaxThreads;
      size_t       m_nOpenMPCurrentThreads;
      bool         m_bGPU;
      float        m_fCPUPerformanceValue;

      //for display
      //unsigned int m_nUnsurableIndex;

      size_t m_progress;
      size_t m_progressMax;
      bool   m_canceled;

      // timings
      size_t m_timeNormalDistribution;
      //size_t m_timeHistogram;
      //size_t m_timeSecondaryWeightingvector;
      size_t m_timeFilterRadius;
      size_t m_timePostprocessing;
   };

}

#endif
