#ifndef __IMT_CPUSLICER_H__
#define __IMT_CPUSLICER_H__

class CPUSlicer
{
	
	
	public:
	
	CPUSlicer( const float *vertexArray, const int vertexArrayLength, const int *indexArray, const int& indexArrayLength);
              
	~CPUSlicer();

     bool computeSlice(float a, float b, float c, float d, float* resultArray, int& resultArrayLength);
     
	 //System::String^ getDebugString();

    //bool IsSlicerValid();

     
	protected:

		const float *vertexArray_;
		int vertexArrayLength_;
		const int *indexArray_;
		int indexArrayLength_;
	
};




#endif