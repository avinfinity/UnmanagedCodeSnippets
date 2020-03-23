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
#include <vtkRenderer.h>
#include <vtkPolyDataMapper.h>
#include "vtkProperty.h"
#include "vtkPolyData.h"
#include "vtkIdList.h"
#include "vtkSmartPointer.h"
#include "vtkDoubleArray.h"
#include "vtkDataArray.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkOBJReader.h"
#include "vtkOBJExporter.h"
#include "vtkLandmarkTransform.h"
#include "vtkMatrix4x4.h"
#include "vtkPLYWriter.h"
#include "vtkCellData.h"
#include "vtkArrowSource.h"
namespace tr
{

Display3DRoutines::Display3DRoutines()
{

}

void Display3DRoutines::displayMultiPolyData(std::vector<vtkSmartPointer < vtkPolyData >> meshes)
{
	//As of now only implemented for the case where only 2 meshes are provided
	vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	vtkSmartPointer<vtkInteractorStyleTrackballCamera> style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
	renderWindowInteractor->SetRenderWindow(renderWindow);
	renderWindowInteractor->SetInteractorStyle(style);
	double xmins[2] = { 0, 0.5 };
	double xmaxs[2] = { 0.5, 1 };
	double ymins[2] = { 0, 0 };
	double ymaxs[2] = { 1, 1 };
	for (unsigned int i = 0; i < 2; i++)
	{
		vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
		renderWindow->AddRenderer(renderer);
		renderer->SetViewport(xmins[i], ymins[i], xmaxs[i], ymaxs[i]);
		vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
		mapper->SetInputData(meshes[i]);
		vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
		actor->SetMapper(mapper);
		renderer->AddActor(actor);
		renderer->ResetCamera();

		renderWindow->Render();
	}
	renderWindowInteractor->Start();
}

void Display3DRoutines::displaySegmentation(std::vector<vtkSmartPointer<vtkPolyData>> meshes)
{
	std::vector<vtkSmartPointer<vtkPolyDataMapper>> mappers(meshes.size());
	std::vector<vtkSmartPointer<vtkActor>> actors(meshes.size());
	vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
	std::vector<unsigned int> colors({ 1,4,2,7 });
	for (int i = 0; i < meshes.size(); i++)
	{
		mappers[i] = vtkSmartPointer<vtkPolyDataMapper>::New();
		mappers[i]->SetInputData(meshes[i]);
		mappers[i]->ScalarVisibilityOff();
		
		actors[i] = vtkSmartPointer<vtkActor>::New();
		actors[i]->GetProperty()->SetColor((double)((colors[i] >> 2) & 1), (double)((colors[i] >> 1) & 1), (double)((colors[i] >> 0) & 1));
		actors[i]->GetProperty()->SetOpacity(0.5);
		actors[i]->SetMapper(mappers[i]);
		renderer->AddActor(actors[i]);
	}

	vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->AddRenderer(renderer);
	
	vtkSmartPointer<vtkRenderWindowInteractor> interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	interactor->SetRenderWindow(renderWindow);

	vtkSmartPointer<vtkInteractorStyleTrackballCamera> style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
	interactor->SetInteractorStyle(style);
	renderer->SetBackground(0, 0, 0);
	renderWindow->Render();
	interactor->Start();
}

void Display3DRoutines::displayPolyData( vtkSmartPointer< vtkPolyData > mesh )
{
  vtkSmartPointer< vtkRenderer > renderer = vtkSmartPointer< vtkRenderer >::New();

  vtkSmartPointer< vtkPolyDataMapper > meshMapper = vtkSmartPointer< vtkPolyDataMapper >::New();

  meshMapper->SetInputData( mesh );

  vtkSmartPointer< vtkActor > meshActor = vtkSmartPointer< vtkActor >::New();

  meshActor->GetProperty()->BackfaceCullingOn();

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

  window->GlobalWarningDisplayOff();
  window->Render();

  interactor->Start();
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


vtkSmartPointer<vtkPolyData> Display3DRoutines::displayMesh(std::vector< Eigen::Vector3f >& vertices, std::vector< unsigned int >& indices)
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
  vColors->GlobalWarningDisplayOff();
  // Add the three colors we have created to the array
  vtkSmartPointer< vtkIdList > triangle = vtkSmartPointer< vtkIdList >::New();

  int id = 0;

  for( int vv = 0; vv < numVerts ; vv++ )
  {
    unsigned char col[] = { 255 , 255 , 255 };

    vColors->InsertNextTupleValue(col);

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
	return dataSet;
       //tr::Display3DRoutines::displayPolyData( dataSet );
}


vtkSmartPointer<vtkPolyData> Display3DRoutines::displayMesh(std::vector< Eigen::Vector3f >& vertices, std::vector< int >& indices)
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
	vColors->GlobalWarningDisplayOff();

	// Add the three colors we have created to the array
	vtkSmartPointer< vtkIdList > triangle = vtkSmartPointer< vtkIdList >::New();

	int id = 0;

	for (int vv = 0; vv < numVerts; vv++)
	{
		unsigned char col[] = { 255, 255, 255 };

		vColors->InsertNextTupleValue(col);

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
	return dataSet;
	//tr::Display3DRoutines::displayPolyData(dataSet);
}



vtkSmartPointer<vtkPolyData> Display3DRoutines::displayMeshWithFaceColors(std::vector< Eigen::Vector3f >& vertices, std::vector< unsigned int >& indices , std::vector< Eigen::Vector3f >& colors )
{
	vtkSmartPointer< vtkPolyData > dataSet = vtkSmartPointer< vtkPolyData >::New();
	vtkSmartPointer< vtkPoints > points = vtkSmartPointer< vtkPoints >::New();
	vtkSmartPointer< vtkUnsignedCharArray > vColors = vtkSmartPointer< vtkUnsignedCharArray >::New();

	int numVerts = vertices.size();
	int numIndices = indices.size() / 3;

	points->Allocate(numVerts);
	dataSet->Allocate(numIndices);

	vColors->SetNumberOfComponents(3);
	vColors->GlobalWarningDisplayOff();
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

		vColors->InsertNextTupleValue(col);

	}

	std::cout << " num colors tuples : " << numIndices << " " << colors.size() << std::endl;

	dataSet->SetPoints(points);
	dataSet->GetCellData()->SetScalars(vColors);


	return dataSet;
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
  vColors->GlobalWarningDisplayOff();

  // Add the three colors we have created to the array
  vtkSmartPointer< vtkIdList > triangle = vtkSmartPointer< vtkIdList >::New();

  int id = 0;

  for( int vv = 0; vv < numVerts ; vv++ )
  {
    unsigned char col[] = { colors[ vv ]( 0 ) * 255 , colors[ vv ]( 1 ) * 255 , colors[ vv ]( 2 ) * 255 };

    vColors->InsertNextTupleValue(col);

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
  vColors->GlobalWarningDisplayOff();

  // Add the three colors we have created to the array
  vtkSmartPointer< vtkIdList > vert = vtkSmartPointer< vtkIdList >::New();
  int id = 0;

  for( int vv = 0; vv < numVerts ; vv++ )
  {
    unsigned char col[] = { colors[vv]( 0 ) * 255 , colors[ vv ]( 1 ) * 255 , colors[vv]( 2 ) * 255 };
    vColors->InsertNextTupleValue(col);

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

            vColors->InsertNextTupleValue(col);

            points->InsertNextPoint( vertices[ vv ]( 0 ) , vertices[ vv ]( 1 ) , vertices[ vv ]( 2 ) );

            vert->Reset();

            vert->InsertNextId( vv );

            dataSet->InsertNextCell( VTK_VERTEX , vert );

        }

        dataSet->SetPoints( points );
        dataSet->GetPointData()->SetScalars( vColors );

        displayPolyData( dataSet );
    }


	void Display3DRoutines::displayNormals( const std::vector< Eigen::Vector3f >& vertices, const std::vector< Eigen::Vector3f >& normals, float scale )
	{

		vtkSmartPointer< vtkPolyData > dataSet = vtkSmartPointer< vtkPolyData >::New();
		vtkSmartPointer< vtkPoints > points = vtkSmartPointer< vtkPoints >::New();
		vtkSmartPointer< vtkUnsignedCharArray > lColors = vtkSmartPointer< vtkUnsignedCharArray >::New();

		int numVerts = vertices.size();

		points->Allocate(2 * numVerts);
		dataSet->Allocate(numVerts);

		lColors->SetNumberOfComponents(3);
		lColors->SetName("Colors");
		lColors->GlobalWarningDisplayOff();

		// Add the three colors we have created to the array
		vtkSmartPointer< vtkIdList > line = vtkSmartPointer< vtkIdList >::New();

		int id = 0;

		for (int vv = 0; vv < numVerts; vv++)
		{
			unsigned char col[] = { 255, 255, 255 };

			lColors->InsertNextTupleValue(col);
			lColors->InsertNextTupleValue(col);

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

void Display3DRoutines::displayPointsLines(const std::vector< Eigen::Vector3f>& vertices, const std::vector<std::vector<Eigen::Vector3f>>& lines, const std::vector<Eigen::Vector3f>& vertexColors, const std::vector<Eigen::Vector3f>& lineColors)
{
	//Vertices
	vtkSmartPointer< vtkPolyData > vDataSet = vtkSmartPointer< vtkPolyData >::New();
	vtkSmartPointer< vtkPoints > vPoints = vtkSmartPointer< vtkPoints>::New();
	vtkSmartPointer< vtkUnsignedCharArray > vColors = vtkSmartPointer< vtkUnsignedCharArray >::New();

	int numVerts = vertices.size();
	vPoints->Allocate(numVerts);
	vDataSet->Allocate(numVerts);
	vColors->SetNumberOfComponents(3);
	vColors->SetName("Vertices Colors");
	vColors->GlobalWarningDisplayOff();

	vtkSmartPointer< vtkIdList > vert = vtkSmartPointer< vtkIdList >::New();
	for (unsigned int i = 0; i < numVerts; i++)
	{
		unsigned char col[] = { vertexColors[i](0) * 255, vertexColors[i](1) * 255, vertexColors[i](2) * 255 };
		vColors->InsertNextTupleValue(col);
		
		vPoints->InsertNextPoint(vertices[i](0), vertices[i](1), vertices[i](2));
		vert->Reset();
		vert->InsertNextId(i);
		vDataSet->InsertNextCell(VTK_VERTEX, vert);
	}
	vDataSet->SetPoints(vPoints);
	vDataSet->GetPointData()->SetScalars(vColors);

	//Lines
	vtkSmartPointer< vtkPolyData > lDataSet = vtkSmartPointer< vtkPolyData >::New();
	vtkSmartPointer< vtkPoints > lPoints = vtkSmartPointer< vtkPoints >::New();
	vtkSmartPointer< vtkUnsignedCharArray > lColors = vtkSmartPointer< vtkUnsignedCharArray >::New();

	int numLines = lines.size();
	lPoints->Allocate(2 * numLines);
	lDataSet->Allocate(numLines);
	
	lColors->SetNumberOfComponents(3);
	lColors->SetName("Line colors");
	lColors->GlobalWarningDisplayOff();

	vtkSmartPointer< vtkIdList > line = vtkSmartPointer< vtkIdList >::New();
	for (unsigned int i = 0; i < numLines; i++)
	{
		unsigned char col[] = { lineColors[i](0)*255, lineColors[i](1)*255, lineColors[i](2)*255 };
		
		lColors->InsertNextTupleValue(col);
		lColors->InsertNextTupleValue(col);

		lPoints->InsertNextPoint(lines[i][0](0), lines[i][0](1), lines[i][0](2));
		lPoints->InsertNextPoint(lines[i][1](0), lines[i][1](1), lines[i][1](2));

		line->Reset();
		line->InsertNextId(2 * i);
		line->InsertNextId(2 * i + 1);
		lDataSet->InsertNextCell(VTK_LINE, line);
	}
	lDataSet->SetPoints(lPoints);
	lDataSet->GetPointData()->SetScalars(lColors);

	std::vector<vtkSmartPointer<vtkPolyData>> polyDatas(2);
	polyDatas[0] = vDataSet;
	polyDatas[1] = lDataSet;
	tr::Display3DRoutines::displayPolyData(polyDatas);
}

Display3DRoutines::~Display3DRoutines()
{

}


}
