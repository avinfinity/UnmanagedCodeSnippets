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

#include "vbomanager.h"


VBOManager::VBOManager()
{

}


void VBOManager::init( EditableHbrMesh *mesh , std::vector< Eigen::Vector3f > &points )
{  
  
#ifdef VC_OPENGL_FUNCTIONS  
  
  initializeOpenGLFunctions();

#endif

  mNumTriangles = mesh->GetNumFaces();
  mNumPoints = points.size();

  int size = points.size() *  sizeof( Eigen::Vector3f );
  
  int numCells = mesh->GetNumFaces();
  
  std::vector< GLuint > indices( 3 * numCells );
  
  for( int ff = 0 ; ff < numCells ; ff++ )
  {
    EditableHbrFace* face  = mesh->GetFace( ff );
    
    indices[ 3 * ff ] = face->GetVertex( 0 )->GetID();
    indices[ 3 * ff + 1 ] = face->GetVertex( 1 )->GetID();
    indices[ 3 * ff + 2 ] = face->GetVertex( 2 )->GetID();
  }
  
  
  glGenBuffers( 2 , mVBOs ); 
    // Transfer vertex data to VBO 0
  glBindBuffer( GL_ARRAY_BUFFER , mVBOs[ 0 ] );
  glBufferData( GL_ARRAY_BUFFER , size , points.data() , GL_STATIC_DRAW );
  
  // Transfer index data to VBO 1
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER , mVBOs[ 1 ] );
  glBufferData( GL_ELEMENT_ARRAY_BUFFER , indices.size() * sizeof(GLuint) , indices.data() , GL_STATIC_DRAW );
  
  glBindBuffer( GL_ARRAY_BUFFER , 0 );
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER , 0 );

}

void VBOManager::init( EditableHbrMesh *mesh , std::vector< VertexData > &points )
{
  
#ifdef VC_OPENGL_FUNCTIONS   
  initializeOpenGLFunctions(); 
#endif

  mNumTriangles = mesh->GetNumFaces();
  mNumPoints = points.size();

  int size = points.size() * sizeof( VertexData );

  mNumVertices = points.size();

  mNumCollectedVertices = 0;
  mNumCollectedTriangles = 0;

  //std::cout<<" vertex data size : "<<sizeof( VertexData )<<std::endl;
  
  int numCells = mesh->GetNumFaces();
  
  std::vector< GLuint > indices( 3 * numCells );
  
  for( int ff = 0 ; ff < numCells ; ff++ )
  {
    EditableHbrFace* face  = mesh->GetFace( ff );
    
    indices[ 3 * ff ] = face->GetVertex( 0 )->GetID();
    indices[ 3 * ff + 1 ] = face->GetVertex( 1 )->GetID();
    indices[ 3 * ff + 2 ] = face->GetVertex( 2 )->GetID();
  }
  
  glGenBuffers( 2 , mVBOs ); 
    // Transfer vertex data to VBO 0
  glBindBuffer( GL_ARRAY_BUFFER , mVBOs[ 0 ] );
  glBufferData( GL_ARRAY_BUFFER , size * 2 , NULL , GL_DYNAMIC_DRAW );
  glBufferSubData( GL_ARRAY_BUFFER , 0 , size , points.data() );

  // Transfer index data to VBO 1
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER , mVBOs[ 1 ] );
  glBufferData( GL_ELEMENT_ARRAY_BUFFER , indices.size() * sizeof(GLuint) * 2 , NULL , GL_DYNAMIC_DRAW );
  glBufferSubData( GL_ELEMENT_ARRAY_BUFFER , 0 , indices.size() * sizeof(GLuint) , indices.data() );
  
  glBindBuffer( GL_ARRAY_BUFFER , 0 );
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER , 0 );

}


void VBOManager::draw( QOpenGLShaderProgram* program )
{
    // Tell OpenGL which VBOs to use
    glBindBuffer( GL_ARRAY_BUFFER , mVBOs[0] );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mVBOs[1] );
    
    // Offset for position
    int offset = 0;

    // Tell OpenGL programmable pipeline how to locate vertex position data
    int vertexLocation = program->attributeLocation("position");
    
    program->enableAttributeArray(vertexLocation);
    
    glVertexAttribPointer( vertexLocation , 3 , GL_FLOAT , GL_FALSE , sizeof( Eigen::Vector3f ) , (const void *)offset );
    
    glDrawElements( GL_TRIANGLES , 3 * mNumTriangles , GL_UNSIGNED_INT , 0 );
}


void VBOManager::draw2( QOpenGLShaderProgram* program )
{
    // Tell OpenGL which VBOs to use
    glBindBuffer( GL_ARRAY_BUFFER , mVBOs[0] );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mVBOs[1] );
    
    // Offset for position
    int offset = 0;

    // Tell OpenGL programmable pipeline how to locate vertex position data
    int vertexLocation = program->attributeLocation( "position" );
    
    program->enableAttributeArray(vertexLocation);
    
    glVertexAttribPointer( vertexLocation , 3 , GL_FLOAT , GL_FALSE , sizeof( VertexData ) , (const void *)offset );

    int underRegion = program->attributeLocation( "underRegion" );
    
    program->enableAttributeArray(underRegion);

    offset += sizeof( Eigen::Vector3f );
    
    glVertexAttribPointer( underRegion , 1 , GL_FLOAT , GL_FALSE , sizeof( VertexData ) , (const void *)offset );

    int isSplittable = program->attributeLocation( "isSplittable" );
    
    program->enableAttributeArray(isSplittable);

    offset += sizeof( float );
    
    glVertexAttribPointer( isSplittable , 1 , GL_FLOAT , GL_FALSE , sizeof( VertexData ) , (const void *)offset );
    
    glDrawElements( GL_TRIANGLES , 3 * mNumTriangles , GL_UNSIGNED_INT , 0 );
}


void VBOManager::bindVBOs()
{
	// Tell OpenGL which VBOs to use
    glBindBuffer( GL_ARRAY_BUFFER , mVBOs[0] );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mVBOs[1] );
}

void VBOManager::unbindVBOs()
{
	// Tell OpenGL which VBOs to use
    glBindBuffer( GL_ARRAY_BUFFER , 0 );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER , 0 );
}


void VBOManager::collectTriangleToAdd(EditableHbrFace* face)
{
  if( mNumCollectedTriangles ==  mCollectedFaces.size() )
  {
	  std::cout<<" num collected triangles : "<<mNumCollectedTriangles<<std::endl;

    mTriangleCollector.resize( ( 2 * mNumCollectedTriangles + 1 ) * 3 );
    mCollectedFaces.resize(  2 * mNumCollectedTriangles + 1   );
  }
  
  mTriangleCollector[ 3 * mNumCollectedTriangles ] = face->GetVertex( 0 )->GetID();
  mTriangleCollector[ 3 * mNumCollectedTriangles + 1 ] = face->GetVertex( 1 )->GetID();
  mTriangleCollector[ 3 * mNumCollectedTriangles + 2 ] = face->GetVertex( 2 )->GetID();
  
  mCollectedFaces[ mNumCollectedTriangles ] = face;
  
  mNumCollectedTriangles++;
}

void VBOManager::collectVertexToAdd( EditableHbrVertex* vertex , VertexData &data )
{
   if( mNumCollectedVertices == mVertexCollector2.size() )
   {
     mCollectedVertices.resize( 2 * mNumCollectedVertices + 1 );
     mVertexCollector2.resize( 2 * mNumCollectedVertices + 1 );
   }

   assert( vertex->GetID() == ( mNumVertices + mNumCollectedVertices ) );

   mVertexCollector2[ mNumCollectedVertices++ ] = data;
   
}



void VBOManager::addTriangles()
{  
  if( mNumCollectedTriangles == 0 )
    return;
  
  int offSet = mNumTriangles * 3 * sizeof( GLuint );
  
  glBufferSubData( GL_ELEMENT_ARRAY_BUFFER  , offSet , mNumCollectedTriangles * 3 * sizeof( GLuint ) , mTriangleCollector.data() );  
  
  mNumTriangles += mNumCollectedTriangles; 

  mNumCollectedTriangles = 0;
}

void VBOManager::addVertices()
{
	if( mNumCollectedVertices == 0 )
		return;

	std::cout<<" adding vertices : "<<mNumCollectedVertices<<std::endl;

	int offSet = mNumVertices * sizeof( VertexData );

	glBufferSubData( GL_ARRAY_BUFFER  , offSet , mNumCollectedVertices * sizeof( VertexData ) , mVertexCollector2.data() );

	mNumVertices += mNumCollectedVertices;

	mNumCollectedVertices = 0;
}


void VBOManager::removeTriangle( int triangleId )
{
  if( triangleId >= mNumTriangles )
  {
    return;    
  }
  else if( triangleId == mNumTriangles - 1 )
  {
    mNumTriangles--;
    
    return;
  }
  
  int readOffSet = 3 * ( mNumTriangles - 1 ) * sizeof( GLuint );
  
  int writeOffset = 3 * triangleId;
  
  int size = 3 * sizeof( GLuint );
  
  glCopyBufferSubData( mVBOs[ 1 ] , mVBOs[ 1 ] , readOffSet , writeOffset , size );  
  
  mFaceMap[ triangleId ] = mFaceMap[ mNumTriangles - 1 ];
  
  mFaceMap[ triangleId ]->glID() = triangleId;
  
  mNumTriangles--;  
}

void VBOManager::updateVertex(int vId, float* vertex)
{
  glBufferSubData(  GL_ARRAY_BUFFER , 3 * vId * sizeof( float ) ,  3 , vertex );
}

void VBOManager::updateVertexData( int vId , VertexData &data )
{
	glBufferSubData(  GL_ARRAY_BUFFER , vId * sizeof( VertexData ) ,  sizeof( VertexData ) , &data );
}

void VBOManager::updateTriangle( int tId , GLuint *indices )
{
	glBufferSubData(  GL_ELEMENT_ARRAY_BUFFER , tId * sizeof( GLuint ) * 3 ,  sizeof( GLuint ) * 3 , indices );
}



