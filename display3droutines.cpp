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


#include "display3droutines.h"
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkInteractorStyleRubberBandZoom.h>
#include <vtkRenderer.h>
#include <vtkPolyDataMapper.h>
#include "vtkProperty.h"
#include "vtkincludes.h"
#include "vtkCellData.h"
#include <vtkActor.h>
#include <vtkImageAccumulate.h>
#include <vtkImageData.h>
#include <vtkImageExtractComponents.h>
#include <vtkJPEGReader.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkXYPlotActor.h>
#include <vtkTable.h>
#include <vtkFloatArray.h>
#include <vtkContextView.h>
#include <vtkContextScene.h>
#include <vtkChartXY.h>
#include <vtkPlot.h>
#include "iostream"
#include "vtkDoubleArray.h"
#include "vtkFieldData.h"
#include "QSurfaceFormat"
#include "QOpenGLContext"
#include "QDebug"

namespace tr
{

Display3DRoutines::Display3DRoutines()
{

}


void Display3DRoutines::displayPolyData( vtkSmartPointer< vtkPolyData > mesh )
{
  vtkSmartPointer< vtkRenderer > renderer = vtkSmartPointer< vtkRenderer >::New();
  
  vtkSmartPointer< vtkPolyDataMapper > meshMapper = vtkSmartPointer< vtkPolyDataMapper >::New();
  
  meshMapper->SetInputData( mesh );
  
  vtkSmartPointer< vtkActor > meshActor = vtkSmartPointer< vtkActor >::New();

  meshActor->GetProperty()->SetColor(1.0, 0 , 0);

  //meshActor->GetProperty()->BackfaceCullingOn();
  
  meshActor->SetMapper( meshMapper );
  
  renderer->AddActor( meshActor );

//   meshActor->GetProperty()->SetAmbient(1.0);
//   meshActor->GetProperty()->SetDiffuse(0.0);
//   meshActor->GetProperty()->SetSpecular(0.0);
  
  vtkSmartPointer< vtkRenderWindow > window = vtkSmartPointer< vtkRenderWindow >::New();
  vtkSmartPointer< vtkRenderWindowInteractor > interactor = vtkSmartPointer< vtkRenderWindowInteractor >::New();
  vtkSmartPointer< vtkInteractorStyleTrackballCamera > style = vtkSmartPointer< vtkInteractorStyleTrackballCamera >::New();  
  
  window->AddRenderer( renderer );
  window->SetInteractor( interactor );
  interactor->SetInteractorStyle( style );
  
  window->Render();
  
  interactor->Start();
}


void Display3DRoutines::plotProfiles( std::vector<std::vector<Eigen::Vector2f>>& profiles , std::vector<Eigen::Vector3f>& profileColors )
{
	// Create a table with some points in it
	vtkSmartPointer<vtkTable> table =
		vtkSmartPointer<vtkTable>::New();

	vtkSmartPointer<vtkFloatArray> arrX = vtkSmartPointer<vtkFloatArray>::New();
	arrX->SetName("X Axis");

	table->AddColumn(arrX);

	//vtkSmartPointer<vtkFloatArray> arrC = vtkSmartPointer<vtkFloatArray>::New();
	//arrC->SetName("Cosine");
	//table->AddColumn(arrC);

	//vtkSmartPointer<vtkFloatArray> arrS = vtkSmartPointer<vtkFloatArray>::New();
	//arrS->SetName("Sine");

	//table->AddColumn(arrS);

	int nProfiles = profiles.size();

	int profileSize = profiles[0].size();

	for ( int pp = 0; pp < nProfiles; pp++ )
	{
		vtkSmartPointer<vtkFloatArray> arr = vtkSmartPointer<vtkFloatArray>::New();
	
		QString profileName = "Profile" + QString::number(pp);

		arr->SetName(profileName.toStdString().c_str());

		table->AddColumn(arr);
	}

	// Fill in the table with some example values
	//int numPoints = 69;
	//float inc = 7.5 / (numPoints - 1);
	table->SetNumberOfRows(nProfiles);
	for (int i = 0; i < profileSize; ++i)
	{
		//table->SetValue(i, 0, i * inc);
		//table->SetValue(i, 1, cos(i * inc));
		//table->SetValue(i, 2, sin(i * inc));

		for (int pp = 0; pp < profileSize; pp++)
		{
			table->SetValue(i, pp, profiles[i][pp](1));
		}
	}

	// Set up the view
	vtkSmartPointer<vtkContextView> view = vtkSmartPointer<vtkContextView>::New();
	view->GetRenderer()->SetBackground(1.0, 1.0, 1.0);

	// Add multiple line plots, setting the colors etc
	vtkSmartPointer<vtkChartXY> chart = vtkSmartPointer<vtkChartXY>::New();
	view->GetScene()->AddItem(chart);

	
	for (int pp = 0; pp < nProfiles; pp++)
	{
		//table->SetValue(i, pp, profiles[i][pp](1));
		vtkPlot *line = chart->AddPlot(vtkChart::LINE);

		line->SetWidth(1.0);

#if VTK_MAJOR_VERSION <= 5
		line->SetInput(table, 0, 1);
#else
		line->SetInputData(table, 0, pp);
#endif
		line->SetColor(pp, 255 * profileColors[pp](0), 255 * profileColors[pp](1), 255 * profileColors[pp](2));
	}


//	line->SetWidth(1.0);
//	line = chart->AddPlot(vtkChart::LINE);
//#if VTK_MAJOR_VERSION <= 5
//	line->SetInput(table, 0, 2);
//#else
//	line->SetInputData(table, 0, 2);
//#endif
//	line->SetColor(255, 0, 0, 255);
//	line->SetWidth(5.0);

	// For dotted line, the line type can be from 2 to 5 for different dash/dot
	// patterns (see enum in vtkPen containing DASH_LINE, value 2):
#ifndef WIN32
	line->GetPen()->SetLineType(vtkPen::DASH_LINE);
#endif
	// (ifdef-ed out on Windows because DASH_LINE does not work on Windows
	//  machines with built-in Intel HD graphics card...)

	//view->GetRenderWindow()->SetMultiSamples(0);

	// Start interactor
	view->GetInteractor()->Initialize();
	view->GetInteractor()->Start();
}



void Display3DRoutines::displayPolyData( std::vector< vtkSmartPointer< vtkPolyData > > meshes)
{
  
  int numMeshes = meshes.size();
  
 std::vector< vtkSmartPointer< vtkPolyDataMapper > > meshMappers ( numMeshes );
 
 std::vector< vtkSmartPointer< vtkActor > > meshActors( numMeshes );
 
   vtkSmartPointer< vtkRenderer > renderer = vtkSmartPointer< vtkRenderer >::New();
  
  for( int mm = 0; mm < numMeshes ; mm++ )
  {
    meshMappers[ mm ] = vtkSmartPointer< vtkPolyDataMapper >::New();
    meshActors[ mm ] = vtkSmartPointer< vtkActor >::New();
    
    meshMappers[ mm ]->SetInputData( meshes[ mm ] );
    
    meshActors[ mm ]->SetMapper( meshMappers[ mm ] );
    
    renderer->AddActor( meshActors[ mm ] );
  }

  

//   meshActor->GetProperty()->SetAmbient(1.0);
//   meshActor->GetProperty()->SetDiffuse(0.0);
//   meshActor->GetProperty()->SetSpecular(0.0);
  
  vtkSmartPointer< vtkRenderWindow > window = vtkSmartPointer< vtkRenderWindow >::New();
  vtkSmartPointer< vtkRenderWindowInteractor > interactor = vtkSmartPointer< vtkRenderWindowInteractor >::New();
  vtkSmartPointer< vtkInteractorStyleTrackballCamera > style = vtkSmartPointer< vtkInteractorStyleTrackballCamera >::New();  
  window->AddRenderer( renderer );
  window->SetInteractor( interactor );
  interactor->SetInteractorStyle( style );
  
  window->Render();
  
  interactor->Start();
}


void Display3DRoutines::displayMesh(std::vector< Eigen::Vector3f >& vertices, std::vector< unsigned int >& indices)
{
  vtkSmartPointer< vtkPolyData > dataSet = vtkSmartPointer< vtkPolyData >::New();
  vtkSmartPointer< vtkPoints > points = vtkSmartPointer< vtkPoints >::New();
  vtkSmartPointer< vtkUnsignedCharArray > vColors = vtkSmartPointer< vtkUnsignedCharArray >::New();
  
  int numVerts = vertices.size();
  int numIndices = indices.size() / 3;
  
  points->Allocate( numVerts );
  dataSet->Allocate( numIndices );
  
  vColors->SetNumberOfComponents(3);
  vColors->SetName("Colors");
 
  // Add the three colors we have created to the array
  vtkSmartPointer< vtkIdList > triangle = vtkSmartPointer< vtkIdList >::New();
  
  int id = 0;
  
  for( int vv = 0; vv < numVerts ; vv++ )
  {
    unsigned char col[] = { 255 , 255 , 255 };
	
    vColors->InsertNextTypedTuple(col);
	
    points->InsertNextPoint( vertices[ vv ]( 0 ) , vertices[ vv ]( 1 ) , vertices[ vv ]( 2 ) );
    
  }
  
  for( int ii = 0; ii < numIndices ; ii++ )
  {
    triangle->Reset();
	
    triangle->InsertNextId( indices[ 3 * ii ] );
    triangle->InsertNextId( indices[ 3 * ii + 1 ] );
    triangle->InsertNextId( indices[ 3 * ii + 2 ] );
    
    dataSet->InsertNextCell( VTK_TRIANGLE , triangle );
  }
  
    dataSet->SetPoints( points );
    dataSet->GetPointData()->SetScalars( vColors );
    
       tr::Display3DRoutines::displayPolyData( dataSet );
}


void Display3DRoutines::displayMesh(std::vector< Eigen::Vector3f >& vertices, std::vector< int >& indices)
{
	vtkSmartPointer< vtkPolyData > dataSet = vtkSmartPointer< vtkPolyData >::New();
	vtkSmartPointer< vtkPoints > points = vtkSmartPointer< vtkPoints >::New();
	vtkSmartPointer< vtkUnsignedCharArray > vColors = vtkSmartPointer< vtkUnsignedCharArray >::New();

	int numVerts = vertices.size();
	int numIndices = indices.size() / 3;

	points->Allocate(numVerts);
	dataSet->Allocate(numIndices);

	vColors->SetNumberOfComponents(3);
	vColors->SetName("Colors");

	// Add the three colors we have created to the array
	vtkSmartPointer< vtkIdList > triangle = vtkSmartPointer< vtkIdList >::New();

	int id = 0;

	for (int vv = 0; vv < numVerts; vv++)
	{
		unsigned char col[] = { 255, 255, 255 };

		vColors->InsertNextTypedTuple(col);

		points->InsertNextPoint(vertices[vv](0), vertices[vv](1), vertices[vv](2));

	}

	for (int ii = 0; ii < numIndices; ii++)
	{
		triangle->Reset();

		triangle->InsertNextId(indices[3 * ii]);
		triangle->InsertNextId(indices[3 * ii + 1]);
		triangle->InsertNextId(indices[3 * ii + 2]);

		dataSet->InsertNextCell(VTK_TRIANGLE, triangle);
	}

	dataSet->SetPoints(points);
	dataSet->GetPointData()->SetScalars(vColors);

	tr::Display3DRoutines::displayPolyData(dataSet);
}


void Display3DRoutines::displayMesh(std::vector< Eigen::Vector3f >& vertices, Eigen::Vector3f color )
{
	std::vector<Eigen::Vector3f> colors(vertices.size(), color);

	std::vector<unsigned int> surfaceIndices(vertices.size(), 0);

	unsigned int numVertices = surfaceIndices.size();

	for (unsigned int vv = 0; vv < numVertices; vv++)
	{
		surfaceIndices[vv] = vv;
	}

	displayMesh(vertices,  surfaceIndices);//colors,
}


void Display3DRoutines::displayMeshWithFaceColors(std::vector< Eigen::Vector3f >& vertices, std::vector< unsigned int >& indices , std::vector< Eigen::Vector3f >& colors )
{
	vtkSmartPointer< vtkPolyData > dataSet = vtkSmartPointer< vtkPolyData >::New();
	vtkSmartPointer< vtkPoints > points = vtkSmartPointer< vtkPoints >::New();
	vtkSmartPointer< vtkUnsignedCharArray > vColors = vtkSmartPointer< vtkUnsignedCharArray >::New();

	int numVerts = vertices.size();
	int numIndices = indices.size() / 3;

	points->Allocate(numVerts);
	dataSet->Allocate(numIndices);

	vColors->SetNumberOfComponents(3);
//	vColors->SetName("Colors");

	// Add the three colors we have created to the array
	vtkSmartPointer< vtkIdList > triangle = vtkSmartPointer< vtkIdList >::New();

	int id = 0;

	for (int vv = 0; vv < numVerts; vv++)
	{

		points->InsertNextPoint(vertices[vv](0), vertices[vv](1), vertices[vv](2));

	}

	for (int ii = 0; ii < numIndices; ii++)
	{
		triangle->Reset();

		triangle->InsertNextId(indices[3 * ii]);
		triangle->InsertNextId(indices[3 * ii + 1]);
		triangle->InsertNextId(indices[3 * ii + 2]);

		dataSet->InsertNextCell(VTK_TRIANGLE, triangle);

		unsigned char col[] = { colors[ii](0) * 255, colors[ii](1) * 255, colors[ii](2) * 255 };

		vColors->InsertNextTypedTuple(col);

	}

	std::cout << " num colors tuples : " << numIndices << " " << colors.size() << std::endl;

	dataSet->SetPoints(points);
	dataSet->GetCellData()->SetScalars(vColors);



	tr::Display3DRoutines::displayPolyData(dataSet);
}



void Display3DRoutines::displayMesh(std::vector< Eigen::Vector3f >& vertices, std::vector< Eigen::Vector3f >& colors, std::vector< unsigned int >& indices)
{
  vtkSmartPointer< vtkPolyData > dataSet = vtkSmartPointer< vtkPolyData >::New();
  vtkSmartPointer< vtkPoints > points = vtkSmartPointer< vtkPoints >::New();
  vtkSmartPointer< vtkUnsignedCharArray > vColors = vtkSmartPointer< vtkUnsignedCharArray >::New();
  
  int numVerts = vertices.size();
  int numIndices = indices.size() / 3;
  
  points->Allocate( numVerts );
  dataSet->Allocate( numIndices );
  
  vColors->SetNumberOfComponents(3);
  vColors->SetName("Colors");
 
  // Add the three colors we have created to the array
  vtkSmartPointer< vtkIdList > triangle = vtkSmartPointer< vtkIdList >::New();
  
  int id = 0;
  
  for( int vv = 0; vv < numVerts ; vv++ )
  {
    unsigned char col[] = { colors[ vv ]( 0 ) * 255 , colors[ vv ]( 1 ) * 255 , colors[ vv ]( 2 ) * 255 };
	
    vColors->InsertNextTypedTuple(col);
	
    points->InsertNextPoint( vertices[ vv ]( 0 ) , vertices[ vv ]( 1 ) , vertices[ vv ]( 2 ) );
    
  }
  
  for( int ii = 0; ii < numIndices ; ii++ )
  {
    triangle->Reset();
	
    triangle->InsertNextId( indices[ 3 * ii ] );
    triangle->InsertNextId( indices[ 3 * ii + 1 ] );
    triangle->InsertNextId( indices[ 3 * ii + 2 ] );
    
    dataSet->InsertNextCell( VTK_TRIANGLE , triangle );
  }
  
    dataSet->SetPoints( points );
    dataSet->GetPointData()->SetScalars( vColors );
    

    
    tr::Display3DRoutines::displayPolyData( dataSet );
}


void Display3DRoutines::displayPointSet(std::vector< Eigen::Vector3f >& vertices, std::vector< Eigen::Vector3f >& colors)
{
  vtkSmartPointer< vtkPolyData > dataSet = vtkSmartPointer< vtkPolyData >::New();
  vtkSmartPointer< vtkPoints > points = vtkSmartPointer< vtkPoints >::New();
  vtkSmartPointer< vtkUnsignedCharArray > vColors = vtkSmartPointer< vtkUnsignedCharArray >::New();
  
  int numVerts = vertices.size();
  
  points->Allocate( numVerts );
  dataSet->Allocate( numVerts );
  
  vColors->SetNumberOfComponents(3);
  vColors->SetName("Colors");
 
  // Add the three colors we have created to the array
  vtkSmartPointer< vtkIdList > vert = vtkSmartPointer< vtkIdList >::New();
  
  int id = 0;
  
  for( int vv = 0; vv < numVerts ; vv++ )
  {
    unsigned char col[] = { colors[vv]( 0 ) * 255 , colors[ vv ]( 1 ) * 255 , colors[vv]( 2 ) * 255 };
	
    vColors->InsertNextTypedTuple(col);
	
    points->InsertNextPoint( vertices[ vv ]( 0 ) , vertices[ vv ]( 1 ) , vertices[ vv ]( 2 ) );
    
    vert->Reset();
	
    vert->InsertNextId( vv );
    
    dataSet->InsertNextCell( VTK_VERTEX , vert );
    
  }
  
    dataSet->SetPoints( points );
    dataSet->GetPointData()->SetScalars( vColors );
    
    displayPolyData( dataSet );
}
    
    void Display3DRoutines::displayPointSetWithScale(std::vector< Eigen::Vector4f >& vertices, std::vector< Eigen::Vector3f >& colors)
    {
        vtkSmartPointer< vtkPolyData > dataSet = vtkSmartPointer< vtkPolyData >::New();
        vtkSmartPointer< vtkPoints > points = vtkSmartPointer< vtkPoints >::New();
        vtkSmartPointer< vtkUnsignedCharArray > vColors = vtkSmartPointer< vtkUnsignedCharArray >::New();
        
        int numVerts = vertices.size();
        
        points->Allocate( numVerts );
        dataSet->Allocate( numVerts );
        
        vColors->SetNumberOfComponents(3);
        vColors->SetName("Colors");
        
        // Add the three colors we have created to the array
        vtkSmartPointer< vtkIdList > vert = vtkSmartPointer< vtkIdList >::New();
        
        int id = 0;
        
        for( int vv = 0; vv < numVerts ; vv++ )
        {
            unsigned char col[] = { colors[vv]( 0 ) * 255 , colors[ vv ]( 1 ) * 255 , colors[vv]( 2 ) * 255 };
            
            vColors->InsertNextTypedTuple(col);
            
            points->InsertNextPoint( vertices[ vv ]( 0 ) , vertices[ vv ]( 1 ) , vertices[ vv ]( 2 ) );
            
            vert->Reset();
            
            vert->InsertNextId( vv );
            
            dataSet->InsertNextCell( VTK_VERTEX , vert );
            
        }
        
        dataSet->SetPoints( points );
        dataSet->GetPointData()->SetScalars( vColors );
        
        displayPolyData( dataSet );
    }


	void Display3DRoutines::displayNormals( std::vector< Eigen::Vector3f >& vertices, std::vector< Eigen::Vector3f >& normals, float scale )
	{

		vtkSmartPointer< vtkPolyData > dataSet = vtkSmartPointer< vtkPolyData >::New();
		vtkSmartPointer< vtkPoints > points = vtkSmartPointer< vtkPoints >::New();
		vtkSmartPointer< vtkUnsignedCharArray > lColors = vtkSmartPointer< vtkUnsignedCharArray >::New();

		int numVerts = vertices.size();

		points->Allocate(2 * numVerts);
		dataSet->Allocate(numVerts);

		lColors->SetNumberOfComponents(3);
		lColors->SetName("Colors");

		// Add the three colors we have created to the array
		vtkSmartPointer< vtkIdList > line = vtkSmartPointer< vtkIdList >::New();

		int id = 0;

		for (int vv = 0; vv < numVerts; vv++)
		{
			unsigned char col[] = { 255, 255, 255 };

			lColors->InsertNextTypedTuple(col);

			points->InsertNextPoint( vertices[vv](0), vertices[vv](1), vertices[vv](2) );

			Eigen::Vector3f np = vertices[vv] + scale * normals[vv];

			points->InsertNextPoint(np(0), np(1), np(2));

			line->Reset();

			line->InsertNextId( 2 * vv );
			line->InsertNextId(2 * vv + 1);

			dataSet->InsertNextCell( VTK_LINE , line );

		}

		std::cout << points->GetNumberOfPoints() << " " << dataSet->GetNumberOfCells() << std::endl;

		dataSet->SetPoints(points);
		dataSet->GetPointData()->SetScalars(lColors);

		displayPolyData(dataSet);

	}


	void Display3DRoutines::displayVolume( imt::volume::VolumeInfo& volInfo )
	{
		
	}



void Display3DRoutines::displayMesh( Eigen::MatrixXd& vertices , Eigen::MatrixXi& indices )
{
  std::vector< Eigen::Vector3f > verts( vertices.rows() );
  std::vector< unsigned int > ind( indices.rows() * 3 );
  
  int numVerts = vertices.rows();
  int numTriangles = indices.rows();
  
  for( int vv = 0; vv < numVerts ; vv++ )
  {
    verts[ vv ]( 0 ) = vertices( vv , 0 );
    verts[ vv ]( 1 ) = vertices( vv , 1 );
    verts[ vv ]( 2 ) = vertices( vv , 2 );
  }
  
  
  for( int tt = 0; tt < numTriangles ; tt++ )
  {
    ind[ 3 * tt ] = indices( tt , 0 );
    ind[ 3 * tt + 1 ] = indices( tt , 1 );
    ind[ 3 * tt + 2 ] = indices( tt , 2 );
  }
  
  displayMesh( verts , ind );
}


void Display3DRoutines::displayMesh(std::vector<double>& vertices, std::vector<unsigned int >& indices)
{

	int nVertices = vertices.size() / 3;
	int nTriangles = indices.size() / 3;

	std::vector<Eigen::Vector3f> displayVertices(nVertices);
	
	for (int vv = 0; vv < nVertices; vv++)
	{
		displayVertices[vv](0) = vertices[3 * vv];
		displayVertices[vv](1) = vertices[3 * vv + 1];
		displayVertices[vv](2) = vertices[3 * vv + 2];
	}

	displayMesh(displayVertices, indices);

}


void Display3DRoutines::displayEdges(std::vector<double>& edgeVertices)
{
	vtkSmartPointer< vtkPolyData > dataSet = vtkSmartPointer< vtkPolyData >::New();
	vtkSmartPointer< vtkPoints > points = vtkSmartPointer< vtkPoints >::New();
	vtkSmartPointer< vtkUnsignedCharArray > vColors = vtkSmartPointer< vtkUnsignedCharArray >::New();

	int numVerts = edgeVertices.size() / 3;
	int numIndices = numVerts; //indices.size() / 3;

	points->Allocate(numVerts);
	dataSet->Allocate(numIndices);

	vColors->SetNumberOfComponents(3);
	vColors->SetName("Colors");

	// Add the three colors we have created to the array
	vtkSmartPointer< vtkIdList > triangle = vtkSmartPointer< vtkIdList >::New();

	int id = 0;

	for (int vv = 0; vv < numVerts; vv++)
	{
		unsigned char col[] = { 255 , 255 , 255 };

		vColors->InsertNextTypedTuple(col);

		points->InsertNextPoint(edgeVertices[ 3 * vv], edgeVertices[3 * vv + 1], edgeVertices[3 * vv + 2]);

	}

	for (int ii = 0; ii < numIndices / 2; ii++)
	{
		triangle->Reset();

		triangle->InsertNextId(2 * ii);
		triangle->InsertNextId(2 * ii + 1);
		
		dataSet->InsertNextCell(VTK_LINE, triangle);
	}

	dataSet->SetPoints(points);
	dataSet->GetPointData()->SetScalars(vColors);

	tr::Display3DRoutines::displayPolyData(dataSet);
}




vtkSmartPointer<vtkPolyData> Display3DRoutines::generateCage( double startX , double startY, double startZ, double width, double height, double depth)
{
	vtkSmartPointer<vtkPolyData> cagePolyData = vtkSmartPointer<vtkPolyData>::New();

	vtkSmartPointer<vtkPoints> cagePoints = vtkSmartPointer<vtkPoints>::New();

	vtkSmartPointer<vtkIdList> edge = vtkSmartPointer<vtkIdList>::New();

	cagePolyData->Allocate(24);


	for(int zz = 0; zz < 2; zz++)
		for(int yy = 0; yy < 2; yy++)
			for (int xx = 0; xx < 2; xx++)
			{
				double p[3] = { startX + xx * width , startY + yy * height , startZ + zz * depth };

				cagePoints->InsertNextPoint(p);

				int pId = zz * 4 + yy * 2 + xx;

				if (xx == 0)
				{
					int pId2 = pId + 1;

					edge->Reset();

					edge->InsertNextId(pId);
					edge->InsertNextId(pId2);

					cagePolyData->InsertNextCell(VTK_LINE, edge);

				}

				if (yy == 0)
				{
					int pId2 = pId + 2;

					edge->Reset();

					edge->InsertNextId(pId);
					edge->InsertNextId(pId2);

					cagePolyData->InsertNextCell(VTK_LINE, edge);

				}

				if (zz == 0)
				{
					int pId2 = pId + 4;

					edge->Reset();

					edge->InsertNextId(pId);
					edge->InsertNextId(pId2);

					cagePolyData->InsertNextCell(VTK_LINE, edge);

				}

				
			}

	cagePolyData->SetPoints(cagePoints);


	return cagePolyData;

}



vtkSmartPointer<vtkPolyData> Display3DRoutines::generatePolyData(std::vector<double>& vertices, std::vector<unsigned int >& indices, Eigen::Vector3f color)
{
	vtkSmartPointer< vtkPolyData > dataSet = vtkSmartPointer< vtkPolyData >::New();
	vtkSmartPointer< vtkPoints > points = vtkSmartPointer< vtkPoints >::New();
	vtkSmartPointer< vtkUnsignedCharArray > vColors = vtkSmartPointer< vtkUnsignedCharArray >::New();

	int numVerts = vertices.size() / 3;
	int numIndices = indices.size() / 3;

	points->Allocate(numVerts);
	dataSet->Allocate(numIndices);

	vColors->SetNumberOfComponents(3);
	vColors->SetName("Colors");

	// Add the three colors we have created to the array
	vtkSmartPointer< vtkIdList > triangle = vtkSmartPointer< vtkIdList >::New();

	int id = 0;

	for (int vv = 0; vv < numVerts; vv++)
	{
		unsigned char col[] = { 255 * color(0) , 255 * color(1), 255 * color(2) };

		vColors->InsertNextTypedTuple(col);

		points->InsertNextPoint(vertices[3 * vv], vertices[3 * vv + 1], vertices[ 3 * vv + 2]);

	}

	for (int ii = 0; ii < numIndices; ii++)
	{
		triangle->Reset();

		triangle->InsertNextId(indices[3 * ii]);
		triangle->InsertNextId(indices[3 * ii + 1]);
		triangle->InsertNextId(indices[3 * ii + 2]);

		dataSet->InsertNextCell(VTK_TRIANGLE, triangle);
	}

	dataSet->SetPoints(points);
	dataSet->GetPointData()->SetScalars(vColors);

	return dataSet;
}



Display3DRoutines::~Display3DRoutines()
{

}

void Display3DRoutines::checkMaxSupportedOpenGLVersion(std::pair< int, int >& version){
 	QSurfaceFormat surfaceFormat;
 	surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
         surfaceFormat.setVersion(4, 5);
 
 	QOpenGLContext *context = new QOpenGLContext();
 
 	context->setFormat(surfaceFormat);
 	context->create();

	QPair< int, int > format = context->format().version();

	version.first = format.first;
	version.second = format.second;

	qDebug() << " opengl format : " << format.first << " " << format.second << endl;

}

void Display3DRoutines::displayHistogram( std::vector< long >& histogram, short minVal, short maxVal )
{
	vtkSmartPointer<vtkXYPlotActor> plot = vtkSmartPointer<vtkXYPlotActor>::New();

	plot->SetLegendPosition(0, 0);
	plot->ExchangeAxesOff();
	plot->SetLabelFormat("%g");
	plot->SetTitleFontSize(1);
	plot->SetXTitle("Gray Level");
	plot->SetYTitle("Frequency");
	plot->SetXValuesToValue();

	plot->LegendOn();

	plot->SetXRange(0, VTK_UNSIGNED_SHORT_MAX);
	plot->SetYRange(minVal, maxVal);

	vtkSmartPointer< vtkImageData > plotData = vtkSmartPointer< vtkImageData >::New();

	int size = histogram.size();

	plotData->SetDimensions(size, 1, 1);

	plotData->AllocateScalars( VTK_LONG , 1);

	long *plotArray = (long *)plotData->GetScalarPointer();

	memcpy(plotArray, histogram.data(), histogram.size() * sizeof(long));

	plot->AddDataSetInput(plotData);

	// Visualize the histogram(s)
	vtkSmartPointer<vtkRenderer> renderer =
		vtkSmartPointer<vtkRenderer>::New();
	renderer->AddActor(plot);

	vtkSmartPointer<vtkRenderWindow> renderWindow =
		vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->AddRenderer(renderer);
	renderWindow->SetSize(640, 480);

	vtkSmartPointer<vtkRenderWindowInteractor> interactor =
		vtkSmartPointer<vtkRenderWindowInteractor>::New();
	interactor->SetRenderWindow(renderWindow);

	vtkSmartPointer< vtkInteractorStyleRubberBandZoom > style = vtkSmartPointer< vtkInteractorStyleRubberBandZoom >::New();

	interactor->SetInteractorStyle(style);

	// Initialize the event loop and then start it
	interactor->Initialize();
	interactor->Start();


}

void Display3DRoutines::displayHistogram(std::vector< int64_t >& histogram, short minVal, short maxVal)
{
	vtkSmartPointer<vtkXYPlotActor> plot = vtkSmartPointer<vtkXYPlotActor>::New();

	plot->SetLegendPosition(0, 0);
	plot->ExchangeAxesOff();
	plot->SetLabelFormat("%g");
	plot->SetTitleFontSize(1);
	plot->SetXTitle("Gray Level");
	plot->SetYTitle("Frequency");
	plot->SetXValuesToValue();

	plot->LegendOn();

	plot->SetXRange(0, VTK_UNSIGNED_SHORT_MAX);
	plot->SetYRange(minVal, maxVal);

	vtkSmartPointer< vtkImageData > plotData = vtkSmartPointer< vtkImageData >::New();

	int size = histogram.size();

	plotData->SetDimensions(size, 1, 1);

	plotData->AllocateScalars(VTK_LONG, 1);

	long *plotArray = (long *)plotData->GetScalarPointer();

	memcpy(plotArray, histogram.data(), histogram.size() * sizeof(long));

	plot->AddDataSetInput(plotData);

	// Visualize the histogram(s)
	vtkSmartPointer<vtkRenderer> renderer =
		vtkSmartPointer<vtkRenderer>::New();
	renderer->AddActor(plot);

	vtkSmartPointer<vtkRenderWindow> renderWindow =
		vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->AddRenderer(renderer);
	renderWindow->SetSize(640, 480);

	vtkSmartPointer<vtkRenderWindowInteractor> interactor =
		vtkSmartPointer<vtkRenderWindowInteractor>::New();
	interactor->SetRenderWindow(renderWindow);

	vtkSmartPointer< vtkInteractorStyleRubberBandZoom > style = vtkSmartPointer< vtkInteractorStyleRubberBandZoom >::New();

	interactor->SetInteractorStyle(style);

	// Initialize the event loop and then start it
	interactor->Initialize();
	interactor->Start();


}


void arrayToPolyData(std::vector< Eigen::Vector3f >& vertices, std::vector< unsigned int >& indices, vtkSmartPointer< vtkPolyData > dataSet)
{
	vtkSmartPointer< vtkPoints > points = vtkSmartPointer< vtkPoints >::New();
	vtkSmartPointer< vtkUnsignedCharArray > vColors = vtkSmartPointer< vtkUnsignedCharArray >::New();

	int numVerts = vertices.size();
	int numIndices = indices.size() / 3;

	points->Allocate(numVerts);
	dataSet->Allocate(numIndices);

	vColors->SetNumberOfComponents(3);
	vColors->SetName("Colors");

	// Add the three colors we have created to the array
	vtkSmartPointer< vtkIdList > triangle = vtkSmartPointer< vtkIdList >::New();

	int id = 0;

	for (int vv = 0; vv < numVerts; vv++)
	{
		unsigned char col[] = { 255, 255, 255 };

		vColors->InsertNextTypedTuple(col);

		points->InsertNextPoint(vertices[vv](0), vertices[vv](1), vertices[vv](2));

	}

	for (int ii = 0; ii < numIndices; ii++)
	{
		triangle->Reset();

		triangle->InsertNextId(indices[3 * ii]);
		triangle->InsertNextId(indices[3 * ii + 1]);
		triangle->InsertNextId(indices[3 * ii + 2]);

		dataSet->InsertNextCell(VTK_TRIANGLE, triangle);
	}

	dataSet->SetPoints(points);
	dataSet->GetPointData()->SetScalars(vColors);
}




void Display3DRoutines::displayMeshes2ViewPort(std::vector< std::vector< Eigen::Vector3f > >& verticesSet, std::vector< std::vector< unsigned int > >& indicesSet)
{

	if (verticesSet.size() < 2 || indicesSet.size() < 2)
		return;

	double xmins[2] = { 0, 0.5 };
	double xmaxs[2] = { 0.5, 1 };
	double ymins[2] = { 0, 0 };
	double ymaxs[2] = { 1, 1  };

	vtkSmartPointer< vtkRenderWindow > window = vtkSmartPointer< vtkRenderWindow >::New();

	vtkSmartPointer< vtkRenderWindowInteractor > interactor = vtkSmartPointer< vtkRenderWindowInteractor >::New();

	vtkSmartPointer< vtkInteractorStyleTrackballCamera > style = vtkSmartPointer< vtkInteractorStyleTrackballCamera >::New();

	interactor->SetInteractorStyle(style);

	window->SetInteractor(interactor);

	for (int ii = 0; ii < 2; ii++)
	{
		vtkSmartPointer< vtkPolyData > meshData = vtkSmartPointer< vtkPolyData >::New();

		arrayToPolyData(verticesSet[ii], indicesSet[ii], meshData);

		vtkSmartPointer< vtkRenderer > renderer = vtkSmartPointer< vtkRenderer >::New();

		renderer->SetViewport(xmins[ii], ymins[ii], xmaxs[ii], ymaxs[ii]);


		// Create a mapper and actor
		vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();

		mapper->SetInputData(meshData);

		vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
		actor->SetMapper(mapper);
		renderer->AddActor(actor);

		window->AddRenderer(renderer);

		window->Render();

		window->SetWindowName("Mesh Comparison");
	}


	interactor->Start();
}



}