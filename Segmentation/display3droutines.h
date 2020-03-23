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


namespace tr {

class Display3DRoutines
{

public:

  Display3DRoutines();

  static void displayPolyData( vtkSmartPointer< vtkPolyData > mesh );

  static void displayPolyData( std::vector< vtkSmartPointer< vtkPolyData > > meshes );

  static void displayMultiPolyData(std::vector < vtkSmartPointer<vtkPolyData >> meshes); //displays meshes in multiple viewports (but it is implemented now for only 2 meshes)

  static void displaySegmentation(std::vector< vtkSmartPointer<vtkPolyData>> meshes); //Displays the meshes of materials with some transperancy
  
  static void displayMesh( std::vector< Eigen::Vector3f >& vertices , std::vector< Eigen::Vector3f >& colors , std::vector< unsigned int >& indices  );

  static vtkSmartPointer<vtkPolyData> displayMeshWithFaceColors(std::vector< Eigen::Vector3f >& vertices, std::vector< unsigned int >& indices, std::vector< Eigen::Vector3f >& colors);

  static vtkSmartPointer<vtkPolyData> displayMesh(std::vector< Eigen::Vector3f >& vertices, std::vector< unsigned int >& indices);

  static vtkSmartPointer<vtkPolyData> displayMesh(std::vector< Eigen::Vector3f >& vertices, std::vector< int >& indices );

  static void displayMesh( Eigen::MatrixXd& vertices , Eigen::MatrixXi& indices );

  static void displayPointSet( std::vector< Eigen::Vector3f >& vertices , std::vector< Eigen::Vector3f >& colors );


  static void displayPointSetWithScale(std::vector< Eigen::Vector4f >& vertices, std::vector< Eigen::Vector3f >& colors);

  static void displayNormals(const std::vector< Eigen::Vector3f >& vertices, const std::vector< Eigen::Vector3f >& normals, float scale = 1.0 );

  static void displayPointsLines(const std::vector< Eigen::Vector3f>& vertices, const std::vector<std::vector<Eigen::Vector3f>>& lines, const std::vector<Eigen::Vector3f>& vertexColors, const std::vector<Eigen::Vector3f>& lineColors);

  virtual ~Display3DRoutines();

};

}

#endif // TR_DISPLAY3DROUTINES_H
