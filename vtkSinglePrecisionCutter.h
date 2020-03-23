#ifndef __VTKSINGLEPRECISIONCUTTER_H__
#define __VTKSINGLEPRECISIONCUTTER_H__


#include <vtkCutter.h>

class PlaneSlicerCPU;

class vtkSinglePrecisionCutter : public vtkCutter 
{
	
	
	public:
	
		vtkTypeMacro(vtkSinglePrecisionCutter, vtkCutter);

		static vtkSinglePrecisionCutter *New();

	void setPlaneSlicer( PlaneSlicerCPU *slicer );
	

	void setPlaneInfo(float *position, float *normal);
	

	int RequestData( vtkInformation *request,
		             vtkInformationVector **inputVector,
		             vtkInformationVector *outputVector);
	

protected:

	PlaneSlicerCPU *mPlaneSlicer;

	float mPosition[3], mNormal[3];
	
};



#endif