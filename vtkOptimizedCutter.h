#ifndef __VTK_OPTIMIZEDCUTTER_H__
#define __VTK_OPTIMIZEDCUTTER_H__

#include "vtkCutter.h"
#include "vtkRectilinearGrid.h"
#include "vtkDoubleArray.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkIncrementalPointLocator.h"
#include "vtkContourHelper.h"
#include "vtkCellLocator.h"
#include "queue"
#include "functional"
#include "planeslicercpu.h"

class CellLocatorVisitor : public vtkCellLocator {
public:
	vtkTypeMacro(CellLocatorVisitor, vtkCellLocator);

	int getBoundsByIndex(int idx, double bounds[6]);

	void getChildren(int idx, int children[8]);

	void visitCells( std::function<bool(double[6])> cellChecker, std::function<void(vtkCell*, int)> visitor);

	static CellLocatorVisitor *New();
};




class vtkOptimizedCutter : public vtkCutter {

	vtkTypeMacro( vtkOptimizedCutter, vtkCutter );

	static vtkOptimizedCutter *New();


	int RequestData( vtkInformation *request,
		             vtkInformationVector **inputVector,
		             vtkInformationVector *outputVector);

	void setLocatorVisitor(CellLocatorVisitor* locator);

	void setPlaneSlicer(PlaneSlicerCPU *slicer);

	void setPlaneInfo(float *position, float *normal);


protected:

	void DataSetVisitorCutter(vtkDataSet *input, vtkPolyData *output);


	CellLocatorVisitor* _locator;

	PlaneSlicerCPU *mPlaneSlicer;

	float *mPosition, *mNormal;

};



#endif