/*
    Copyright (c) 2012, avanindra <email>
    All rights reserved.
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the <organization> nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.
    THIS SOFTWARE IS PROVIDED BY avanindra <email> ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL avanindra <email> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef TR_DISPLAY3DROUTINES_H
#define TR_DISPLAY3DROUTINES_H

//#define vtkRenderingCore_AUTOINIT 4(vtkInteractionStyle,vtkRenderingFreeType,vtkRenderingFreeTypeOpenGL,vtkRenderingOpenGL)
//#define vtkRenderingVolume_AUTOINIT 1(vtkRenderingVolumeOpenGL)

#include "vtkSmartPointer.h"
#include "vtkPolyData.h"
#include "eigenincludes.h"
#include "volumeinfo.h"
typedef signed   __int64 int64_t;
namespace tr {

class Display3DRoutines
{

public:
    
  Display3DRoutines();
  
  static void displayPolyData( vtkSmartPointer< vtkPolyData > mesh ); 
  
  static void displayPolyData( std::vector< vtkSmartPointer< vtkPolyData > > meshes ); 
  
  static void displayMesh( std::vector< Eigen::Vector3f >& vertices , std::vector< Eigen::Vector3f >& colors , std::vector< unsigned int >& indices  );

  static void displayMeshWithFaceColors(std::vector< Eigen::Vector3f >& vertices, std::vector< unsigned int >& indices, std::vector< Eigen::Vector3f >& colors);
   
  static void displayMesh( std::vector< Eigen::Vector3f >& vertices , std::vector< unsigned int >& indices  );

  static void displayMesh(std::vector< Eigen::Vector3f >& vertices, std::vector< int >& indices );

  static void displayMesh(std::vector< Eigen::Vector3f >& vertices, Eigen::Vector3f color = Eigen::Vector3f(1, 0, 0));
    
  static void displayMesh( Eigen::MatrixXd& vertices , Eigen::MatrixXi& indices );

  static void displayMesh(std::vector<double>& vertices, std::vector<unsigned int >& indices);

  static void displayEdges(std::vector<double>& edgeVertices);

  static vtkSmartPointer<vtkPolyData> generateCage(double startX, double startY, double startZ, double width, double height, double depth);
  static vtkSmartPointer<vtkPolyData> generatePolyData(std::vector<double>& vertices, std::vector<unsigned int >& indices , Eigen::Vector3f color);
 
    
  static void displayPointSet( std::vector< Eigen::Vector3f >& vertices , std::vector< Eigen::Vector3f >& colors ); 
  
  static void checkMaxSupportedOpenGLVersion( std::pair< int , int >& version );

  static void displayPointSetWithScale(std::vector< Eigen::Vector4f >& vertices, std::vector< Eigen::Vector3f >& colors);

  static void displayNormals(std::vector< Eigen::Vector3f >& vertices, std::vector< Eigen::Vector3f >& normals, float scale = 1.0 );

  //void display2Meshes( std::vector< Eigen::Vector3f >& points , std::vector< unsigned int >& indices1 , std::vector< Eigen::Vector3f >  )

  static void displayMeshes2ViewPort(std::vector< std::vector< Eigen::Vector3f > >& verticesSet, std::vector< std::vector< unsigned int > >& indicesSet);

  static void displayVolume(imt::volume::VolumeInfo& volInfo);

  static void displayHistogram(std::vector< long >& histogram, short minVal, short maxVal);

  static void displayHistogram(std::vector< int64_t >& histogram, short minVal, short maxVal);

  static void plotProfiles(std::vector<std::vector<Eigen::Vector2f>>& profiles , std::vector<Eigen::Vector3f>& profileColors);
    
  virtual ~Display3DRoutines();
  
};

}

#endif // TR_DISPLAY3DROUTINES_H