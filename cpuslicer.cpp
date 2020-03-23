#include "cpuslicer.h"
#include "atomic"
#include <iostream>
#include "omp.h"

#define VALUES_PER_POINT 7
#define NUM_VALUES_PER_LINE 14
#define MAX_THREAD_PER_BLOCK 64
#define DEFAULT_VALUE -10000000
#define FLOAT_COMPARISON_DIFFERENCE 0.000000001

struct float3
{

	float x, y, z;

	float3()
	{
		x = 0;
		y = 0;
		z = 0;
	}

	float3(const float& xp, const float& yp, const float& zp) : x(xp), y(yp), z(zp)
	{

	}
	
};

struct int3
{

	int x, y, z;

	int3()
	{
		x = 0;
		y = 0;
		z = 0;
	}

	int3(const float& xp, const float& yp, const float& zp) : x(xp), y(yp), z(zp)
	{

	}
};



float computeEdgePlaneIntersection(float3 point1, float3 point2, float a, float b, float c, float d)
{
	float numerator = a*point1.x + b*point1.y + c*point1.z + d;
	float denominator = (a*point1.x + b*point1.y + c*point1.z) - (a*point2.x + b*point2.y + c*point2.z);
	float t = numerator / denominator;

	return t;
}

float3 interpolate(float3 point1, float3 point2, float t)
{
	float3 result(((1 - t)*point1.x + t*point2.x), ((1 - t)*point1.y + t*point2.y), ((1 - t)*point1.z + t*point2.z));

	return result;
}

float interpolateSingleValue(float point1, float point2, float t)
{
	float result = (1 - t)*point1 + t*point2;
	return result;
}


bool computeEdges(float *resultArray, const float *vertexArray, const int *indexArray, int arraySize, int& resultArrayCount, float a, float b, float c, float d)
{

	std::atomic< int > lineCounter;

	lineCounter = 0;
	
#pragma omp parallel for
	for (int i = 0; i < arraySize; i++)
	{
		int3 indexValues(indexArray[3 * i], indexArray[3 * i + 1], indexArray[3 * i + 2]);

		bool isLineValid = false;

		float3 firstOutputVertex(DEFAULT_VALUE, DEFAULT_VALUE, DEFAULT_VALUE);
		float3 firstOutputNormal(DEFAULT_VALUE, DEFAULT_VALUE, DEFAULT_VALUE);
		float firstOutputDeviation = DEFAULT_VALUE;

		float3 secondOutputVertex(DEFAULT_VALUE, DEFAULT_VALUE, DEFAULT_VALUE);
		float3 secondOutputNormal(DEFAULT_VALUE, DEFAULT_VALUE, DEFAULT_VALUE);
		float secondOutputDeviation = DEFAULT_VALUE;

		float3 firstVertex(vertexArray[VALUES_PER_POINT * indexValues.x + 0], vertexArray[VALUES_PER_POINT * indexValues.x + 1], vertexArray[VALUES_PER_POINT * indexValues.x + 2]);
		float3 firstNormal(vertexArray[VALUES_PER_POINT * indexValues.x + 3], vertexArray[VALUES_PER_POINT * indexValues.x + 4], vertexArray[VALUES_PER_POINT * indexValues.x + 5]);
		float firstDeviation = vertexArray[VALUES_PER_POINT * indexValues.x + 6];

		float3 secondVertex(vertexArray[VALUES_PER_POINT * indexValues.y + 0], vertexArray[VALUES_PER_POINT * indexValues.y + 1], vertexArray[VALUES_PER_POINT * indexValues.y + 2]);
		float3 secondNormal(vertexArray[VALUES_PER_POINT * indexValues.y + 3], vertexArray[VALUES_PER_POINT * indexValues.y + 4], vertexArray[VALUES_PER_POINT * indexValues.y + 5]);
		float secondDeviation = vertexArray[VALUES_PER_POINT * indexValues.y + 6];

		float3 thirdVertex(vertexArray[VALUES_PER_POINT * indexValues.z + 0], vertexArray[VALUES_PER_POINT * indexValues.z + 1], vertexArray[VALUES_PER_POINT * indexValues.z + 2]);
		float3 thirdNormal(vertexArray[VALUES_PER_POINT * indexValues.z + 3], vertexArray[VALUES_PER_POINT * indexValues.z + 4], vertexArray[VALUES_PER_POINT * indexValues.z + 5]);
		float thirdDeviation = vertexArray[VALUES_PER_POINT * indexValues.z + 6];

		float fEvalFirst = a*firstVertex.x + b*firstVertex.y + c*firstVertex.z + d;
		float fEvalSecond = a*secondVertex.x + b*secondVertex.y + c*secondVertex.z + d;
		float fEvalThird = a*thirdVertex.x + b*thirdVertex.y + c*thirdVertex.z + d;


		bool isFirstZero = abs(fEvalFirst) < FLOAT_COMPARISON_DIFFERENCE;
		bool isSecondZero = abs(fEvalSecond) < FLOAT_COMPARISON_DIFFERENCE;
		bool isThirdZero = abs(fEvalThird) < FLOAT_COMPARISON_DIFFERENCE;

		//printf("Point Calculation Started %f %f %f", fEvalFirst, fEvalSecond, fEvalThird);

		// Case 1 : All Points lie on the plane
		if (isFirstZero && isSecondZero && isThirdZero)
		{
			//Special Case : Three Lines as Output
		}

		//// Case 2 : Two point lie on the Plane
		else if ((!isFirstZero && isSecondZero && isThirdZero) || (isFirstZero && !isSecondZero && isThirdZero) || (isFirstZero && isSecondZero && !isThirdZero))
		{
			if (!isFirstZero)
			{
				firstOutputVertex = secondVertex;
				secondOutputVertex = thirdVertex;
				isLineValid = true;
			}
			else if (!isSecondZero)
			{
				firstOutputVertex = firstVertex;
				secondOutputVertex = thirdVertex;
				isLineValid = true;
			}
			else if (!isThirdZero)
			{
				firstOutputVertex = firstVertex;
				secondOutputVertex = secondVertex;
				isLineValid = true;
			}
		}


		// Case 3 : One point lie on the Plane
		else if (isFirstZero || isSecondZero || isThirdZero)
		{
			if (isFirstZero)
			{
				if ((fEvalSecond > 0 && fEvalThird < 0) || (fEvalSecond < 0 && fEvalThird >0))
				{
					float intersectRatio = computeEdgePlaneIntersection(secondVertex, thirdVertex, a, b, c, d);
					float3 intersectVertex = interpolate(secondVertex, thirdVertex, intersectRatio);
					float3 intersectNormal = interpolate(secondNormal, thirdNormal, intersectRatio);
					float intersectDeviation = interpolateSingleValue(secondDeviation, thirdDeviation, intersectRatio);

					firstOutputVertex = firstVertex;
					firstOutputNormal = firstNormal;
					firstOutputDeviation = firstDeviation;

					secondOutputVertex = intersectVertex;
					secondOutputNormal = intersectNormal;
					secondOutputDeviation = intersectDeviation;
					isLineValid = true;
				}
			}
			else if (isSecondZero)
			{
				if ((fEvalFirst > 0 && fEvalThird < 0) || (fEvalFirst < 0 && fEvalThird >0))
				{
					float intersectRatio = computeEdgePlaneIntersection(firstVertex, thirdVertex, a, b, c, d);
					float3 intersectVertex = interpolate(firstVertex, thirdVertex, intersectRatio);
					float3 intersectNormal = interpolate(firstNormal, thirdNormal, intersectRatio);
					float intersectDeviation = interpolateSingleValue(firstDeviation, thirdDeviation, intersectRatio);

					firstOutputVertex = secondVertex;
					firstOutputNormal = secondNormal;
					firstOutputDeviation = secondDeviation;

					secondOutputVertex = intersectVertex;
					secondOutputNormal = intersectNormal;
					secondOutputDeviation = intersectDeviation;

					isLineValid = true;
				}
			}
			else if (isThirdZero)
			{
				if ((fEvalFirst > 0 && fEvalSecond < 0) || (fEvalFirst < 0 && fEvalSecond >0))
				{
					float intersectRatio = computeEdgePlaneIntersection(firstVertex, secondVertex, a, b, c, d);
					float3 intersectVertex = interpolate(firstVertex, secondVertex, intersectRatio);
					float3 intersectNormal = interpolate(firstNormal, secondNormal, intersectRatio);
					float intersectDeviation = interpolateSingleValue(firstDeviation, secondDeviation, intersectRatio);


					firstOutputVertex = thirdVertex;
					firstOutputNormal = thirdNormal;
					firstOutputDeviation = thirdDeviation;

					secondOutputVertex = intersectVertex;
					secondOutputNormal = intersectNormal;
					secondOutputDeviation = intersectDeviation;

					isLineValid = true;
				}
			}
		}

		//Case 4 : No point lie on the plane
		else if (!isFirstZero && !isSecondZero && !isThirdZero)
		{
			//all Point on one side of the triangle
			if ((fEvalFirst > 0 && fEvalSecond > 0 && fEvalThird > 0) || (fEvalFirst < 0 && fEvalSecond < 0 && fEvalThird < 0))
			{
			}
			//if First Point on One Side and Second/Third on other side
			else if ((fEvalFirst < 0 && fEvalSecond > 0 && fEvalThird > 0) || (fEvalFirst > 0 && fEvalSecond < 0 && fEvalThird < 0))
			{

				float intersect1Ratio = computeEdgePlaneIntersection(firstVertex, secondVertex, a, b, c, d);
				float3 intersect1Vertex = interpolate(firstVertex, secondVertex, intersect1Ratio);
				float3 intersect1Normal = interpolate(firstNormal, secondNormal, intersect1Ratio);
				float intersect1Deviation = interpolateSingleValue(firstDeviation, secondDeviation, intersect1Ratio);

				float intersect2Ratio = computeEdgePlaneIntersection(firstVertex, thirdVertex, a, b, c, d);
				float3 intersect2Vertex = interpolate(firstVertex, thirdVertex, intersect2Ratio);
				float3 intersect2Normal = interpolate(firstNormal, thirdNormal, intersect2Ratio);
				float intersect2Deviation = interpolateSingleValue(firstDeviation, thirdDeviation, intersect2Ratio);


				firstOutputVertex = intersect1Vertex;
				firstOutputNormal = intersect1Normal;
				firstOutputDeviation = intersect1Deviation;

				secondOutputVertex = intersect2Vertex;
				secondOutputNormal = intersect2Normal;
				secondOutputDeviation = intersect2Deviation;

				isLineValid = true;
			}
			//if Second Point on One Side and First/Third on other side
			else if ((fEvalFirst > 0 && fEvalSecond < 0 && fEvalThird > 0) || (fEvalFirst < 0 && fEvalSecond > 0 && fEvalThird < 0))
			{

				float intersect1Ratio = computeEdgePlaneIntersection(secondVertex, firstVertex, a, b, c, d);
				float3 intersect1Vertex = interpolate(secondVertex, firstVertex, intersect1Ratio);
				float3 intersect1Normal = interpolate(secondNormal, firstNormal, intersect1Ratio);
				float intersect1Deviation = interpolateSingleValue(secondDeviation, firstDeviation, intersect1Ratio);

				float intersect2Ratio = computeEdgePlaneIntersection(secondVertex, thirdVertex, a, b, c, d);
				float3 intersect2Vertex = interpolate(secondVertex, thirdVertex, intersect2Ratio);
				float3 intersect2Normal = interpolate(secondNormal, thirdNormal, intersect2Ratio);
				float intersect2Deviation = interpolateSingleValue(secondDeviation, thirdDeviation, intersect2Ratio);

				firstOutputVertex = intersect1Vertex;
				firstOutputNormal = intersect1Normal;
				firstOutputDeviation = intersect1Deviation;

				secondOutputVertex = intersect2Vertex;
				secondOutputNormal = intersect2Normal;
				secondOutputDeviation = intersect2Deviation;

				isLineValid = true;
			}
			//if Third Point on One Side and First/Second on other side
			else if ((fEvalFirst > 0 && fEvalSecond > 0 && fEvalThird < 0) || (fEvalFirst < 0 && fEvalSecond < 0 && fEvalThird > 0))
			{

				float intersect1Ratio = computeEdgePlaneIntersection(thirdVertex, firstVertex, a, b, c, d);
				float3 intersect1Vertex = interpolate(thirdVertex, firstVertex, intersect1Ratio);
				float3 intersect1Normal = interpolate(thirdNormal, firstNormal, intersect1Ratio);
				float intersect1Deviation = interpolateSingleValue(thirdDeviation, firstDeviation, intersect1Ratio);

				float intersect2Ratio = computeEdgePlaneIntersection(thirdVertex, secondVertex, a, b, c, d);
				float3 intersect2Vertex = interpolate(thirdVertex, secondVertex, intersect2Ratio);
				float3 intersect2Normal = interpolate(thirdNormal, secondNormal, intersect2Ratio);
				float intersect2Deviation = interpolateSingleValue(thirdDeviation, secondDeviation, intersect2Ratio);



				firstOutputVertex = intersect1Vertex;
				firstOutputNormal = intersect1Normal;
				firstOutputDeviation = intersect1Deviation;

				secondOutputVertex = intersect2Vertex;
				secondOutputNormal = intersect2Normal;
				secondOutputDeviation = intersect2Deviation;

				isLineValid = true;
			}
		}



		// Writing data to Shared Memory
		if (isLineValid)
		{
			int localLineCount = lineCounter++;

			resultArray[NUM_VALUES_PER_LINE * localLineCount + 0] = firstOutputVertex.x;
			resultArray[NUM_VALUES_PER_LINE * localLineCount + 1] = firstOutputVertex.y;
			resultArray[NUM_VALUES_PER_LINE * localLineCount + 2] = firstOutputVertex.z;

			resultArray[NUM_VALUES_PER_LINE * localLineCount + 3] = firstOutputNormal.x;
			resultArray[NUM_VALUES_PER_LINE * localLineCount + 4] = firstOutputNormal.y;
			resultArray[NUM_VALUES_PER_LINE * localLineCount + 5] = firstOutputNormal.z;

			resultArray[NUM_VALUES_PER_LINE * localLineCount + 6] = firstOutputDeviation;

			resultArray[NUM_VALUES_PER_LINE * localLineCount + 7] = secondOutputVertex.x;
			resultArray[NUM_VALUES_PER_LINE * localLineCount + 8] = secondOutputVertex.y;
			resultArray[NUM_VALUES_PER_LINE * localLineCount + 9] = secondOutputVertex.z;

			resultArray[NUM_VALUES_PER_LINE * localLineCount + 10] = secondOutputNormal.x;
			resultArray[NUM_VALUES_PER_LINE * localLineCount + 11] = secondOutputNormal.y;
			resultArray[NUM_VALUES_PER_LINE * localLineCount + 12] = secondOutputNormal.z;

			resultArray[NUM_VALUES_PER_LINE * localLineCount + 13] = secondOutputDeviation;

		}
	}


	resultArrayCount = lineCounter;
}




CPUSlicer::CPUSlicer(const float *vertexArray, const int vertexArrayLength, const int *indexArray, const int& indexArrayLength) :
vertexArray_(vertexArray), vertexArrayLength_(vertexArrayLength), indexArray_(indexArray), indexArrayLength_( indexArrayLength )
{


}


bool CPUSlicer::computeSlice(float a, float b, float c, float d, float* resultArray, int& resultArrayLength)
{
	return computeEdges(resultArray, vertexArray_, indexArray_, indexArrayLength_, resultArrayLength, a, b, c, d);
}

CPUSlicer::~CPUSlicer()
{


}

