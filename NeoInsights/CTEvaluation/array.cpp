//  array.cpp
//	Gehört zur VMClasses-Bibliothek für Vector/Matrix-Rechnung
//	Ursprünglicher Autor D.Görsch - goersch.at.zeiss-izm.de

#include "array.h"

namespace Izm
{
namespace Numerics
{

// Array<double>

Array<double>::Array(int _size):
	size(_size)
{
	data = new double[size];
	memset(data,0,size*sizeof(double));
}

Array<double>::Array(const Array<double>& a):
	size(a.size)
{
	data = new double[size];
	memcpy(data,a.data,size*sizeof(double));
}

Array<double>::~Array()
{
	 delete [] data;
}

Array<double>& Array<double>::operator =(const Array<double>& a)
{
	if (&a != this) 
	{
		if (size != a.size) 
		{
			delete [] data;
			size  = a.size;
			data = new double[size];
		}
		memcpy(data,a.data,size*sizeof(double));
	}
	return *this;
}

double& Array<double>::operator [](int i)
{
   assert((i >= 0) && (i < size));
   return data[i];
}

const double& Array<double>::operator [](int i) const
{
	assert((i >= 0) && (i < size));
	return data[i];
}

double& Array<double>::operator ()(int i)
{
   assert((i >= 1) && (i <= size));
   return data[i-1];
}

const double& Array<double>::operator ()(int i) const
{
	assert((i >= 1) && (i <= size));
	return data[i-1];
}

void Array<double>::setSize(int _size)
{
	if (_size != size) 
	{
		delete [] data;
		size  = _size;
		data = new double[size];
		memset(data,0,size*sizeof(double));
	}
}

void Array<double>::clear()
{
	memset(data,0,size*sizeof(double));
}


// Array<int>

Array<int>::Array(int _size):
	size(_size)
{
	data = new int[size];
	memset(data,0,size*sizeof(int));
}

Array<int>::Array(const Array<int>& a):
	size(a.size)
{
	data = new int[size];
	memcpy(data,a.data,size*sizeof(int));
}

Array<int>::~Array()
{
	 delete [] data;
}

Array<int>& Array<int>::operator =(const Array<int>& a)
{
	if (&a != this) 
	{
		if (size != a.size) 
		{
			delete [] data;
			size  = a.size;
			data = new int[size];
		}
		memcpy(data,a.data,size*sizeof(int));
	}
	return *this;
}

int& Array<int>::operator [](int i)
{
   assert((i >= 0) && (i < size));
   return data[i];
}

const int& Array<int>::operator [](int i) const
{
	assert((i >= 0) && (i < size));
	return data[i];
}

int& Array<int>::operator ()(int i)
{
   assert((i >= 1) && (i <= size));
   return data[i-1];
}

const int& Array<int>::operator ()(int i) const
{
	assert((i >= 1) && (i <= size));
	return data[i-1];
}

void Array<int>::setSize(int _size)
{
	if (_size != size) 
	{
		delete [] data;
		size  = _size;
		data = new int[size];
		memset(data,0,size*sizeof(int));
	}
}

void Array<int>::clear()
{
	memset(data,0,size*sizeof(int));
}


}
}

// array.cpp
