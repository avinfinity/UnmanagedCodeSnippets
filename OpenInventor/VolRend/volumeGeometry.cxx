/*----------------------------------------------------------------------------------------
Example program.
Purpose : Demonstrate how to create and use a simple volumeData and volume geometry.
          A standard manipulator allows the geometry to be moved through the volume.
author : Jerome Hummel
August 2002

Note:
  Correct rendering of volume geometry requires OpenGL 3D texturing.

  All boards that report OpenGL version 1.2 or higher are required to
  support 3D texturing and are capable of rendering the correct image.

  However on some boards, e.g. GeForce2, this support is not hardware
  accelerated and may be very slow.  Especially for actual volume
  rendering using SoVolumeRender.  By default VolumeViz tries to
  detect this situation and automatically *disable* use of 3D textures.
  In this case you may see volume geometry rendered with no texture
  (i.e. just gray triangles).

  To enable use of 3D textures in VolumeViz in all cases, set the
  environment variable IVVR_USE_TEX3D to 1 (see SoPreferences).
----------------------------------------------------------------------------------------*/

//header files
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoShapeHints.h>
#include <Inventor/manips/SoTransformerManip.h>

#include <VolumeViz/nodes/SoVolumeIndexedTriangleStripSet.h>
#include <VolumeViz/nodes/SoVolumeRenderingQuality.h>

// Positions of all of the vertices:
static const float 
vertexPositions[16][3] =
{
  { -0.9f,-0.9f,-0.3f }, { -0.3f,-0.9f, 0.0f }, {  0.3f,-0.9f, 0.0f }, {  0.6f,-0.9f,-0.3f },
  { -0.9f,-0.3f,-0.1f }, { -0.3f,-0.3f, 0.0f }, {  0.3f,-0.3f, 0.0f }, {  0.6f,-0.3f,-0.1f },
  { -0.9f, 0.3f, 0.1f }, { -0.3f, 0.3f, 0.0f }, {  0.3f, 0.3f, 0.0f }, {  0.6f, 0.3f, 0.1f },
  { -0.9f, 0.9f, 0.3f }, { -0.3f, 0.9f, 0.0f }, {  0.3f, 0.9f, 0.0f }, {  0.6f, 0.9f, 0.3f }
};

// Number of vertices in each strip.
static int32_t
coordIndex[27] =
{
  0, 4, 1, 5, 2, 6, 3, 7, -1,
  4, 8, 5, 9, 6,10, 7,11, -1,
  8,12, 9,13,10,14,11,15, -1
};

static SoVolumeIndexedTriangleStripSet *pVolGeom;
static SoVolumeRenderingQuality *pVolRenderingQuality;

///////////////////////////////////////////////////////////////////////
//
SoSeparator*
createVolumeGeometryGraph()
{
  // Create an SoVolumeRenderingQuality
  pVolRenderingQuality = new SoVolumeRenderingQuality;
  pVolRenderingQuality->interpolateOnMove = TRUE;
  pVolRenderingQuality->lightingModel = SoVolumeRenderingQuality::OPENGL;
  pVolRenderingQuality->edgeDetect2DMethod = SoVolumeRenderingQuality::LUMINANCE|SoVolumeRenderingQuality::DEPTH;
  pVolRenderingQuality->surfaceScalarExponent = 8;

  // Define coordinates and base color for volume geometry
  SoVertexProperty *myVertexProperty = new SoVertexProperty;
  myVertexProperty->orderedRGBA.set1Value( 0, 0xFFFFFFFF );
  myVertexProperty->vertex.setValues( 0, 16, vertexPositions );

  // Volume geometry node
  pVolGeom = new SoVolumeIndexedTriangleStripSet;
  pVolGeom->coordIndex.setValues( 0, 27, coordIndex );
  pVolGeom->vertexProperty.setValue( myVertexProperty );
  pVolGeom->offset = 0.0f;
  pVolGeom->clipGeometry = FALSE;

  // This ShapeHints will enable two-sided lighting for the geometry
  SoShapeHints *pHints = new SoShapeHints;
  pHints->vertexOrdering = SoShapeHints::COUNTERCLOCKWISE;
  pHints->creaseAngle = 3.14f;

  // Assemble the scene graph
  // Note: Volume geometry must appear after the SoVolumeData node.
  SoSeparator *root = new SoSeparator;
  root->addChild( pVolRenderingQuality );
  root->addChild( pHints );                 // Shape hints
  root->addChild( new SoTransformerManip ); // Manip to move geometry
  root->addChild( pVolGeom );               // Volume geometry

  //root->renderCaching = SoSeparator::OFF;

  return root;
}

///////////////////////////////////////////////////////////////////////
//
void
setVolumeGeometryClipping( SbBool clipGeometry )
{
  pVolGeom->clipGeometry = clipGeometry;
}

///////////////////////////////////////////////////////////////////////
//
void
setVolumeGeometryOffset( float offset )
{
  pVolGeom->offset = offset;
}

///////////////////////////////////////////////////////////////////////
//
void
setVolumeGeometryInterpolation( SbBool interpolation )
{
  pVolGeom->interpolation = interpolation ? SoVolumeShape::LINEAR : SoVolumeShape::NEAREST ;
}

///////////////////////////////////////////////////////////////////////
//
void
setVolumeGeometryOutline( SbBool outlined )
{
  pVolRenderingQuality->voxelOutline = outlined ;
}
