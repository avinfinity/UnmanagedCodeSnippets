#include "vtkSinglePrecisionCutter.h"
#include <vtkInformationVector.h>
#include <vtkInformation.h>
#include <vtkSmartPointer.h>
#include <vtkIdList.h>
#include "planeslicercpu.h"

vtkSinglePrecisionCutter* vtkSinglePrecisionCutter::New()
{
	return new vtkSinglePrecisionCutter;
}


void vtkSinglePrecisionCutter::setPlaneSlicer(PlaneSlicerCPU *slicer)
{
	mPlaneSlicer = slicer;
}

void vtkSinglePrecisionCutter::setPlaneInfo(float *position, float *normal)
{
	mPosition[0] = position[0];
	mPosition[1] = position[1];
	mPosition[2] = position[2];
	
	mNormal[0] = normal[0];
	mNormal[1] = normal[1];
	mNormal[2] = normal[2];
}


int vtkSinglePrecisionCutter::RequestData(
	vtkInformation *request,
	vtkInformationVector **inputVector,
	vtkInformationVector *outputVector)
{
	std::cout << " inside single precision cutter " << std::endl;

	vtkInformation *outInfo = outputVector->GetInformationObject(0);

	vtkPolyData *output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

	std::vector< float > edges;

	mPlaneSlicer->computeSlice(mPosition, mNormal, edges);

	std::cout << " computed num edges  " << edges.size() / 6 << std::endl;

	output->Allocate(edges.size() / 6);

	vtkSmartPointer< vtkPoints > edgePoints = vtkSmartPointer< vtkPoints >::New();

	edgePoints->Allocate(edges.size());

	int nEdges = edges.size() / 6;

	vtkSmartPointer< vtkIdList > edge = vtkSmartPointer< vtkIdList >::New();

	for (int ee = 0; ee < nEdges; ee++)
	{
		edge->Reset();

		edgePoints->InsertNextPoint(edges[6 * ee], edges[6 * ee + 1], edges[6 * ee + 2]);
		edgePoints->InsertNextPoint(edges[6 * ee + 3], edges[6 * ee + 4], edges[6 * ee + 5]);

		edge->InsertNextId(2 * ee);
		edge->InsertNextId(2 * ee + 1);

		output->InsertNextCell(VTK_LINE, edge);
	}

	output->SetPoints(edgePoints);


	return 1;
}
