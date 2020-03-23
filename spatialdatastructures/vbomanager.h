/*
 * Copyright 2014 <copyright holder> <email>
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

#ifndef VBOMANAGER_H
#define VBOMANAGER_H

#include "openglincludes.h"

#ifdef VC_QOPENGL_FUNCTIONS
#include "QOpenGLFunctions"
#include "QOpenGLFunctions_4_1_Core"
#include <QOpenGLShader>
#endif

#include "spatialdatastructures/defintions.h"
#include "Eigen/Dense"


#ifdef VC_QOPENGL_FUNCTIONS
class VBOManager : public QOpenGLFunctions_4_1_Core
#else
class VBOManager
#endif
{
  
  GLuint mVBOs[ 4 ];
  
  int mNumTriangles , mNumVertices , mNumPoints ;
  
  std::vector< EditableHbrFace* > mFaceMap;
  std::vector< EditableHbrVertex* > mVertexMap;
  
  std::vector< GLuint > mTriangleCollector ;
  std::vector< GLfloat > mVertexCollector ;
  std::vector< VertexData > mVertexCollector2;
  std::vector< EditableHbrFace* > mCollectedFaces;
  std::vector< EditableHbrVertex* > mCollectedVertices;
  
  int mNumCollectedTriangles , mNumCollectedVertices;
  int mNumAllocatedVertices , mNumAllocatedTriangles;
  
protected:

  
  
public:
  
    VBOManager();
    
    void init( EditableHbrMesh *mesh , std::vector< Eigen::Vector3f > &points );
    void init( EditableHbrMesh *mesh , std::vector< VertexData > &points );
    
    void collectTriangleToAdd( EditableHbrFace* face );
    void collectVertexToAdd( EditableHbrVertex* vertex , VertexData &data );
    void addTriangles();    
    void addVertices();
    void removeTriangle( int triangleId );
    void updateVertex( int vId , float *vertex );
    void updateVertexData( int vId , VertexData &data );
    void updateTriangle( int tId , GLuint *indices );
    void addPoints( float *vertices , int numVertices );
    void removeVertex( int vertexId );  

    void bindVBOs();
    void unbindVBOs();
    
    void draw( QOpenGLShaderProgram *program );
    void draw2( QOpenGLShaderProgram *program );
};

#endif // VBOMANAGER_H
