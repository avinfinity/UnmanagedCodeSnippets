/*
 * Copyright 2015 <copyright holder> <email>
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 */

#ifndef POISSONMESHGENERATOR_H
#define POISSONMESHGENERATOR_H

#include "eigenincludes.h"
#include "vector"

namespace imt{

  namespace volume{

class PoissonMeshGenerator 
{
  


  
public:
  

  
   struct Settings
   {
     float mScaleFactor;
     unsigned char mSkipSamples;
     int mRefineOctree;
     float mConfidenceThreshold;
     int mComponentSize;
     bool mCleanDegenerated;
     
     int mDepth , mMinDepth , mFullDepth , mKernelDepth , mCGDepth , mAdaptiveExponent , mIters;
     int mMaxSolveDepth; 
     int mBoundaryType , mNumThreads;
     float mSamplesPerNode , mScale  , mCSSolverAccuracy , mPointWeight;
     
     
     bool mUseConfidence , mUseNormalWeights , mComplete , mShowResidual , mNonManifold , mPolygonMesh;
       
     
   } ;
  

  PoissonMeshGenerator();
  
  
  void PoissonMeshGenerator::compute( std::vector< Eigen::Vector3f >& points, std::vector< Eigen::Vector3f >& normals, int& depth,
	                                  std::vector< Eigen::Vector3f >& surfacePoints, std::vector< unsigned int >& surfaceIndices );

  
  ~PoissonMeshGenerator();
  
protected:

void convertToMesh( const  std::vector< Eigen::Vector3f >& vertices , const std::vector< unsigned int >& indices );  

void filterMesh();
  
  
protected:
  
  
  Settings mSettings;

  
  
};

  }
}

#endif // POISSONMESHGENERATOR_H
