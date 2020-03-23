//#include "Stdafx.h"
#include "vtkOptimizedCutter.h"
#include "vtkContourHelperCustom.h"
#include <vtkInformationVector.h>
#include <vtkInformation.h>
#include <vtkImplicitFunction.h>
#include <vtkIdList.h>

#include <vtkRectilinearGrid.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkIncrementalPointLocator.h>
#include <vtkCell.h>
#include <vtkCellArray.h>

//#include <vtkContourHelper.h>

CellLocatorVisitor* CellLocatorVisitor::New()
{
	return new CellLocatorVisitor;
}

int CellLocatorVisitor::getBoundsByIndex(int idx, double bounds[6])
{
	int nSubdiv = 1;
	int level = 0;
	int nNodes = 1;

	while (idx >= nNodes) {
		nSubdiv *= 2;
		nNodes += nSubdiv * nSubdiv * nSubdiv;
		level++;
	}

	int firstInLevel = nNodes - nSubdiv*nSubdiv*nSubdiv;

	double sizeWithinLevel[3];
	for (int i = 0; i < 3; ++i) {
		sizeWithinLevel[i] = (this->Bounds[2 * i + 1] - this->Bounds[2 * i]) / nSubdiv;
	}

	idx -= firstInLevel;

	for (int i = 0; i < 3; ++i) {
		bounds[2 * i] = this->Bounds[2 * i] + sizeWithinLevel[i] * (idx % nSubdiv);
		bounds[2 * i + 1] = bounds[2 * i] + sizeWithinLevel[i];
		idx /= nSubdiv;
	}
	return level;
}

void CellLocatorVisitor::getChildren(int idx, int children[8])
{
	int nSubdiv = 1;
	int level = 0;
	int nNodes = 1;

	while (idx >= nNodes) {
		nSubdiv *= 2;
		nNodes += nSubdiv * nSubdiv * nSubdiv;
		level++;
	}

	int firstInParentLevel = nNodes - nSubdiv*nSubdiv*nSubdiv;

	idx -= firstInParentLevel;

	int firstChildIndex = nNodes;
	int nNodesInDirection = 2;
	for (int i = 0; i < 3; ++i) {
		firstChildIndex += (idx % (nSubdiv)) * nNodesInDirection;
		nNodesInDirection *= 2 * nSubdiv;
		idx /= (nSubdiv);
	}

	int outit = 0;
	for (int k = 0; k < 2; k++) {
		for (int j = 0; j < 2; j++) {
			for (int i = 0; i < 2; i++) {
				children[outit++] = firstChildIndex + i + 2 * nSubdiv * j + 4 * nSubdiv * nSubdiv * k;
			}
		}
	}
}

void CellLocatorVisitor::visitCells(std::function<bool(double[6])> cellChecker, std::function<void(vtkCell*, int)> visitor)
{
	//this->BuildLocatorIfNeeded();
	this->ClearCellHasBeenVisited();

	std::queue<int> octantsToVisit;

	octantsToVisit.push(0);

	while (!octantsToVisit.empty()) {
		int currentOctant = octantsToVisit.front();
		octantsToVisit.pop();

		// Get the octant bounds 
		double bounds[6];
		int level = getBoundsByIndex(currentOctant, bounds);

		if (cellChecker(bounds)) {
			vtkIdList* cellsIds = this->Tree[currentOctant];
			if (level == this->Level) {
				// Leaf
				if (!cellsIds) {
					// Empty leaf
					continue;
				}

				for (int i = 0; i < cellsIds->GetNumberOfIds(); ++i)
				{
					int cellId = cellsIds->GetId(i);
					if (!this->CellHasBeenVisited[cellId]) {
						visitor(this->DataSet->GetCell(cellId), cellId);
						this->CellHasBeenVisited[cellId] = 1;
					}
				}
			}
			else {
				vtkIdList* cellsIds = this->Tree[currentOctant];
				if (cellsIds != (void*)1) {
					continue;
				}

				int children[8];

				// Get children
				getChildren(currentOctant, children);
				for (int i = 0; i < 8; ++i) {
					octantsToVisit.push(children[i]);
				}
			}
		}
	}
}

vtkOptimizedCutter* vtkOptimizedCutter::New()
{
	return new vtkOptimizedCutter;
}

void vtkOptimizedCutter::setLocatorVisitor(CellLocatorVisitor* locator)
{
	std::cout << " setting locator " << std::endl;

	_locator = locator;
}

void vtkOptimizedCutter::setPlaneSlicer( PlaneSlicerCPU *slicer )
{
	mPlaneSlicer = slicer;
}

void vtkOptimizedCutter::setPlaneInfo(float *position, float *normal)
{
	mPosition = position;
	mNormal = normal;
}


int vtkOptimizedCutter::RequestData(
	vtkInformation *request,
	vtkInformationVector **inputVector,
	vtkInformationVector *outputVector)
{

	//std::cout << " inside vtk optimized cutter " << std::endl;

	// get the info objects
	vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
	vtkInformation *outInfo = outputVector->GetInformationObject(0);

	// get the input and output
	//vtkDataSet *input = vtkDataSet::SafeDownCast( inInfo->Get(vtkDataObject::DATA_OBJECT()));
	vtkPolyData *output = vtkPolyData::SafeDownCast( outInfo->Get(vtkDataObject::DATA_OBJECT()) );

	std::vector< float > edges;

	mPlaneSlicer->computeSlice(mPosition, mNormal, edges);

	output->Allocate( edges.size() / 6 );

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



	//vtkDebugMacro(<< "Executing cutter");
	//if (!this->CutFunction)
	//{
	//	vtkErrorMacro("No cut function specified");
	//	return 0;
	//}

	//if (!input)
	//{
	//	// this could be a table in a multiblock structure, i.e. no cut!
	//	return 0;
	//}

	//if (input->GetNumberOfPoints() < 1 || this->GetNumberOfContours() < 1)
	//{
	//	return 1;
	//}

	//if ((input->GetDataObjectType() == VTK_STRUCTURED_POINTS ||
	//	input->GetDataObjectType() == VTK_IMAGE_DATA) &&
	//	input->GetCell(0) && input->GetCell(0)->GetCellDimension() >= 3)
	//{
	//	this->StructuredPointsCutter(input, output, request, inputVector, outputVector);
	//}
	//else if (input->GetDataObjectType() == VTK_STRUCTURED_GRID &&
	//	input->GetCell(0) &&
	//	input->GetCell(0)->GetCellDimension() >= 3)
	//{
	//	this->StructuredGridCutter(input, output);
	//}
	//else if (input->GetDataObjectType() == VTK_RECTILINEAR_GRID &&
	//	static_cast<vtkRectilinearGrid *>(input)->GetDataDimension() == 3)
	//{
	//	this->RectilinearGridCutter(input, output);
	//}
	//else if (input->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
	//{
	//	vtkDebugMacro(<< "Executing Unstructured Grid Cutter");
	//	this->UnstructuredGridCutter(input, output);
	//}
	//else
	//{
	//	vtkDebugMacro(<< "Executing DataSet Cutter");
	//this->DataSetVisitorCutter(input, output);
	//}

	return 1;
}

//----------------------------------------------------------------------------
void vtkOptimizedCutter::DataSetVisitorCutter(vtkDataSet *input, vtkPolyData *output)
{

	std::cout << " inside dataset visitor cutter " << std::endl;

	// Make a timer object - need to get some frame rates/render times
	vtkTimeStamp timer;

	timer.Modified();

	vtkIdType i;
	int iter;
	vtkPoints *cellPts;
	vtkDoubleArray *cellScalars;
	vtkCellArray *newVerts, *newLines, *newPolys;
	vtkPoints *newPoints;
	vtkDoubleArray *cutScalars;
	double value, s;
	vtkIdType estimatedSize, numCells = input->GetNumberOfCells();
	vtkIdType numPts = input->GetNumberOfPoints();
	int numCellPts;
	vtkPointData *inPD, *outPD;
	vtkCellData *inCD = input->GetCellData(), *outCD = output->GetCellData();
	vtkIdList *cellIds;
	int numContours = this->ContourValues->GetNumberOfContours();
	int abortExecute = 0;

	cellScalars = vtkDoubleArray::New();

	// Create objects to hold output of contour operation
	//
	estimatedSize = static_cast<vtkIdType>(
		pow(static_cast<double>(numCells), .75)) * numContours;
	estimatedSize = estimatedSize / 1024 * 1024; //multiple of 1024
	if (estimatedSize < 1024)
	{
		estimatedSize = 1024;
	}

	newPoints = vtkPoints::New();
	// set precision for the points in the output
	if (this->OutputPointsPrecision == vtkAlgorithm::DEFAULT_PRECISION)
	{
		vtkPointSet *inputPointSet = vtkPointSet::SafeDownCast(input);
		if (inputPointSet)
		{
			newPoints->SetDataType(inputPointSet->GetPoints()->GetDataType());
		}
		else
		{
			newPoints->SetDataType(VTK_FLOAT);
		}
	}
	else if (this->OutputPointsPrecision == vtkAlgorithm::SINGLE_PRECISION)
	{
		newPoints->SetDataType(VTK_FLOAT);
	}
	else if (this->OutputPointsPrecision == vtkAlgorithm::DOUBLE_PRECISION)
	{
		newPoints->SetDataType(VTK_DOUBLE);
	}
	newPoints->Allocate(estimatedSize, estimatedSize / 2);
	newVerts = vtkCellArray::New();
	newVerts->Allocate(estimatedSize, estimatedSize / 2);
	newLines = vtkCellArray::New();
	newLines->Allocate(estimatedSize, estimatedSize / 2);
	newPolys = vtkCellArray::New();
	newPolys->Allocate(estimatedSize, estimatedSize / 2);
	cutScalars = vtkDoubleArray::New();
	cutScalars->SetNumberOfTuples(numPts);

	// Interpolate data along edge. If generating cut scalars, do necessary setup
	if (this->GenerateCutScalars)
	{
		inPD = vtkPointData::New();
		inPD->ShallowCopy(input->GetPointData());//copies original attributes
		inPD->SetScalars(cutScalars);
	}
	else
	{
		inPD = input->GetPointData();
	}
	outPD = output->GetPointData();
	outPD->InterpolateAllocate(inPD, estimatedSize, estimatedSize / 2);
	outCD->CopyAllocate(inCD, estimatedSize, estimatedSize / 2);

	// locator used to merge potentially duplicate points
	if (this->Locator == NULL)
	{
		this->CreateDefaultLocator();
	}
	this->Locator->InitPointInsertion(newPoints, input->GetBounds());

	// Loop over all points evaluating scalar function at each point
	//
	for (i = 0; i < numPts; i++)
	{
		s = this->CutFunction->FunctionValue(input->GetPoint(i));
		cutScalars->SetComponent(i, 0, s);
	}

	// Compute some information for progress methods
	//
	vtkIdType numCuts = numContours*numCells;
	vtkIdType progressInterval = numCuts / 20 + 1;
	int cut = 0;


	vtkContourHelperCustom helper(this->Locator, newVerts, newLines, newPolys, inPD, inCD, outPD, outCD, estimatedSize, this->GenerateTriangles != 0);
	if (this->SortBy == VTK_SORT_BY_VALUE || this->SortBy == VTK_SORT_BY_CELL)
	{
		for (iter = 0; iter < numContours && !abortExecute; iter++)
		{
			_locator->visitCells(
				[&](double bounds[6]) {
				double sign = this->CutFunction->EvaluateFunction(bounds[0], bounds[2], bounds[4]);

				bool oneSide = true;
				for (int i = 0; i < 2 && oneSide; ++i) {
					for (int j = 2; j < 4 && oneSide; ++j) {
						for (int k = 4; k < 6; ++k) {
							double sign2 = this->CutFunction->EvaluateFunction(bounds[i], bounds[j], bounds[k]);
							if (sign * sign2 <= 0) {
								oneSide = false;
								break;
							}
						}
					}
				}

				return !oneSide;
			},




				[&](vtkCell* cell, int cellId) {
				if (!(++cut % progressInterval))
				{
					vtkDebugMacro(<< "Cutting #" << cut);
					this->UpdateProgress(static_cast<double>(cut) / numCuts);
					abortExecute = this->GetAbortExecute();
				}

				cellPts = cell->GetPoints();
				cellIds = cell->GetPointIds();

				numCellPts = cellPts->GetNumberOfPoints();
				cellScalars->SetNumberOfTuples(numCellPts);
				for (i = 0; i < numCellPts; i++)
				{
					s = cutScalars->GetComponent(cellIds->GetId(i), 0);
					cellScalars->SetTuple(i, &s);
				}

				value = this->ContourValues->GetValue(iter);

				helper.Contour(cell, value, cellScalars, cellId);
			});
		} // for all contour values
	}
	//*/
	// Update ourselves.  Because we don't know upfront how many verts, lines,
	// polys we've created, take care to reclaim memory.
	//
	cellScalars->Delete();
	cutScalars->Delete();

	if (this->GenerateCutScalars)
	{
		inPD->Delete();
	}

	output->SetPoints(newPoints);
	newPoints->Delete();

	if (newVerts->GetNumberOfCells())
	{
		output->SetVerts(newVerts);
	}
	newVerts->Delete();

	if (newLines->GetNumberOfCells())
	{
		output->SetLines(newLines);
	}
	newLines->Delete();

	if (newPolys->GetNumberOfCells())
	{
		output->SetPolys(newPolys);
	}
	newPolys->Delete();

	this->Locator->Initialize();//release any extra memory
	output->Squeeze();

	std::cout << " time spent : " << timer.GetMTime() << std::endl;
}
