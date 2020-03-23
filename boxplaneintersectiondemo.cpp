
#include "boxplaneintersection.h"



int main( int argc , char **argv )
{
	
	BoxPlaneIntersection intersector;

	BoxPlaneIntersection::Box box;
	BoxPlaneIntersection::Plane plane;

	plane._Position[0] = 0.5;
	plane._Position[1] = 0.5;
	plane._Position[2] = 0.5;

	plane._Normal[0] = 0.5;
	plane._Normal[1] = 0.5;
	plane._Normal[2] = 0.5;

	double norm = plane._Normal[0] * plane._Normal[0] + plane._Normal[1] * plane._Normal[1] + plane._Normal[2] * plane._Normal[2];

	plane._Normal[0] /= norm;
	plane._Normal[1] /= norm;
	plane._Normal[2] /= norm;

	BoxPlaneIntersection::Box box;

	box._Corners[0] = 0;
	box._Corners[1] = 0;
	box._Corners[2] = 0;

	box._Corners[3] = 1;
	box._Corners[4] = 0;
	box._Corners[5] = 0;

	box._Corners[6] = 1;
	box._Corners[7] = 1;
	box._Corners[8] = 0;

	box._Corners[9] = 0;
	box._Corners[10] = 1;
	box._Corners[11] = 0;

	box._Corners[12] = 0;
	box._Corners[13] = 0;
	box._Corners[14] = 1;

	box._Corners[15] = 1;
	box._Corners[16] = 0;
	box._Corners[17] = 1;

	box._Corners[18] = 1;
	box._Corners[19] = 1;
	box._Corners[20] = 1;

	box._Corners[21] = 0;
	box._Corners[22] = 1;
	box._Corners[23] = 1;

	intersector.compute(box, plane);
	
	
	return 0;
}