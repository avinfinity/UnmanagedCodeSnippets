
#include "boxplaneintersection.h"
#include "vector"





BoxPlaneIntersection::BoxPlaneIntersection()
{
	_BoxFaces[0]._Edges[0] = 0;
	_BoxFaces[0]._Edges[1] = 1;
	_BoxFaces[0]._Edges[2] = 2;
	_BoxFaces[0]._Edges[3] = 3;

	_BoxFaces[1]._Edges[0] = 0;
	_BoxFaces[1]._Edges[1] = 5;
	_BoxFaces[1]._Edges[2] = 8;
	_BoxFaces[1]._Edges[3] = 4;

	_BoxFaces[2]._Edges[0] = 2;
	_BoxFaces[2]._Edges[1] = 6;
	_BoxFaces[2]._Edges[2] = 9;
	_BoxFaces[2]._Edges[3] = 5;

	_BoxFaces[3]._Edges[0] = 10;
	_BoxFaces[3]._Edges[1] = 6;
	_BoxFaces[3]._Edges[2] = 1;
	_BoxFaces[3]._Edges[3] = 7;

	_BoxFaces[4]._Edges[0] = 0;
	_BoxFaces[4]._Edges[1] = 7;
	_BoxFaces[4]._Edges[2] = 11;
	_BoxFaces[4]._Edges[3] = 4;

	_BoxFaces[5]._Edges[0] = 8;
	_BoxFaces[5]._Edges[1] = 9;
	_BoxFaces[5]._Edges[2] = 10;
	_BoxFaces[5]._Edges[3] = 11;


	_BoxEdges[0]._Ends[0] = 0;
	_BoxEdges[0]._Ends[1] = 3;

	_BoxEdges[1]._Ends[0] = 3;
	_BoxEdges[1]._Ends[1] = 2;

	_BoxEdges[2]._Ends[0] = 2;
	_BoxEdges[2]._Ends[1] = 1;

	_BoxEdges[3]._Ends[0] = 1;
	_BoxEdges[3]._Ends[1] = 0;

	_BoxEdges[4]._Ends[0] = 0;
	_BoxEdges[4]._Ends[1] = 4;

	_BoxEdges[5]._Ends[0] = 1;
	_BoxEdges[5]._Ends[1] = 5;

	_BoxEdges[6]._Ends[0] = 2;
	_BoxEdges[6]._Ends[1] = 6;

	_BoxEdges[7]._Ends[0] = 3;
	_BoxEdges[7]._Ends[1] = 7;

	_BoxEdges[8]._Ends[0] = 4;
	_BoxEdges[8]._Ends[1] = 5;

	_BoxEdges[9]._Ends[0] = 5;
	_BoxEdges[9]._Ends[1] = 6;

	_BoxEdges[10]._Ends[0] = 6;
	_BoxEdges[10]._Ends[1] = 7;

	_BoxEdges[11]._Ends[0] = 7;
	_BoxEdges[11]._Ends[1] = 4;

}

void BoxPlaneIntersection::compute(const BoxPlaneIntersection::Box& box, const BoxPlaneIntersection::Plane& plane)
{
	std::vector< bool > isEdgeIntersected(12, false);

	std::vector<double> intersections(12 * 3, 0);

	//first compute intersection with all the edges
	for (int ee = 0; ee < 12; ee++)
	{
		double intersection[3];

		if (computeIntersectionWithEdge(ee, intersection))
		{
			isEdgeIntersected[ee] = true;

			intersections[3 * ee] = intersection[0];
			intersections[3 * ee + 1] = intersection[1];
			intersections[3 * ee + 2] = intersection[2];
		}

	}


	std::vector<std::pair<int, int>> intersectionPairs;

	//now connect edges to form the face
	//for creating connections we need to process the faces

	for (int ff = 0; ff < 6; ff++)
	{
		int eId1 = _BoxFaces[ff]._Edges[0];
		int eId2 = _BoxFaces[ff]._Edges[1];
		int eId3 = _BoxFaces[ff]._Edges[2];
		int eId4 = _BoxFaces[ff]._Edges[3];

		int intersectionCount = 0;

		int intersectionEdge[2];

		//two edges must be intersected
		if (isEdgeIntersected[eId1])
		{
			intersectionEdge[intersectionCount] = eId1;
			intersectionCount++;
		}

		if (isEdgeIntersected[eId2])
		{
			intersectionEdge[intersectionCount] = eId2;
			intersectionCount++;
		}

		if (isEdgeIntersected[eId3])
		{
			intersectionEdge[intersectionCount] = eId3;
			intersectionCount++;
		}

		if (isEdgeIntersected[eId4])
		{
			intersectionEdge[intersectionCount] = eId4;
			intersectionCount++;
		}


		if (intersectionCount == 2)
		{
			std::pair< int, int > intersectionPair;

			intersectionPair.first = intersectionEdge[0];
			intersectionPair.second = intersectionEdge[1];

			intersectionPairs.push_back(intersectionPair);
		}
	}

	std::vector<int> polygon;

	int numIntersectionEdges = intersectionPairs.size();



	//for ( int ee = 0; ee < numIntersectionEdges; ee++ )
	//{
	//	polygon.push_back(  )

	//	for (int ee2 = 0; ee2 < numIntersectionEdges; ee2++)
	//	{
	//		if (ee == ee2)
	//			continue;


	//	}
	//}




}


bool BoxPlaneIntersection::computeIntersectionWithEdge(int edgeId, double* intersection)
{

	int vId1 = _BoxEdges[edgeId]._Ends[0];
	int vId2 = _BoxEdges[edgeId]._Ends[1];

	float adotn = _Box._Corners[3 * vId1] * _Plane._Normal[0] + _Box._Corners[3 * vId1 + 1] * _Plane._Normal[1] + _Box._Corners[3 * vId1 + 2] * _Plane._Normal[2];
	float bdotn = _Box._Corners[3 * vId2] * _Plane._Normal[0] + _Box._Corners[3 * vId2 + 1] * _Plane._Normal[1] + _Box._Corners[3 * vId2 + 2] * _Plane._Normal[2];

	float pdotn = _Plane._Normal[0] * _Plane._Position[0] + _Plane._Normal[1] * _Plane._Position[1] + _Plane._Normal[2] * _Plane._Position[2];

	float t = (pdotn - bdotn) / (adotn - bdotn);

	if (t < 0 || t > 1)
		return false;

	intersection[0] = _Box._Corners[3 * vId1] * t + _Box._Corners[3 * vId2] * (1 - t);
	intersection[1] = _Box._Corners[3 * vId1 + 1] * t + _Box._Corners[3 * vId2 + 1] * (1 - t);
	intersection[2] = _Box._Corners[3 * vId1 + 2] * t + _Box._Corners[3 * vId2 + 2] * (1 - t);

	return true;

}