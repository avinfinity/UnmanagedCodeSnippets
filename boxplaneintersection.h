#ifndef __BOXPLANEINTERSECTION_H__
#define __BOXPLANEINTERSECTION_H__


//box plane intersection can be handled by using marching cubes

class BoxPlaneIntersection
{


public:


	struct Box{

		double _Corners[24];
	};


	struct Plane{

		double _Position[3], _Normal[3];
	};

	BoxPlaneIntersection();

	void compute(const Box& box, const Plane& plane);


protected:

	struct Edge{

		int _Ends[2];
	};

	struct Face{

		int _Edges[4];
	};


	bool computeIntersectionWithEdge(int edgeId , double* intersection );



	Face _BoxFaces[6];
	Edge _BoxEdges[12];
	Box _Box;
	Plane _Plane;

};



#endif