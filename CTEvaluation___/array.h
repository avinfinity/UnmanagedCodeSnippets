//  array.h

#ifndef __ARRAY_H
#define __ARRAY_H

#include "assert.h"
#include "memory.h"

namespace Izm
{
namespace Numerics
{

template <class A> class array_iterator;
template <class A> class cyclic_array_iterator;

template <class A>
bool operator ==(const cyclic_array_iterator<A>& i1, const cyclic_array_iterator<A>& i2)
{
	return ((i1.index == i2.index) && (i1.array == i2.array));
}

template <class A>
bool operator !=(const cyclic_array_iterator<A>& i1, const cyclic_array_iterator<A>& i2)
{
	return ((i1.index != i2.index) || (i1.array != i2.array));
}

template <class A>
bool operator ==(const array_iterator<A>& i1, const array_iterator<A>& i2)
{
	return ((i1.index == i2.index) && (i1.array == i2.array));
}

template <class A>
bool operator !=(const array_iterator<A>& i1, const array_iterator<A>& i2)
{
	return ((i1.index != i2.index) || (i1.array != i2.array));
}



template <class A>
class cyclic_array_iterator
{
private:
	const A* array;
	int index;	

public:
	typedef typename A::value_type value_type;

public:
	cyclic_array_iterator(const A* _array = 0, int _index = -1):
		array(_array),
		index(_index)
	{
	}

	cyclic_array_iterator(const cyclic_array_iterator& i):
		array(i.array),
		index(i.index)
	{
	}

	cyclic_array_iterator(const array_iterator<A>& i):
		array(i.Array()),
		index(i.Index())
	{
	}

	~cyclic_array_iterator()
	{
	}

	int maxIndex() { if (array != 0) return array->Size()-1; else return 0; }
	int minIndex() { return 0; }

	cyclic_array_iterator& operator =(const cyclic_array_iterator& i)
	{
		if (&i != this) {
			array  = i.array;
			index  = i.index;
		}

		return (*this);
	}

	cyclic_array_iterator& operator =(const array_iterator<A>& i)
	{
		array  = i.Array();
		index  = i.Index();

		return (*this);
	}

	friend bool operator ==<>(const cyclic_array_iterator& i1, const cyclic_array_iterator& i2);
	friend bool operator !=<>(const cyclic_array_iterator& i1, const cyclic_array_iterator& i2);

	bool Valid() const
	{
		return ((index >= 0) && (array != 0) && (index < array->Size()));
	}		

	value_type operator *() const
	{
		return (*array)[index];
	}

	cyclic_array_iterator& operator ++()
	{
		index++;
		if (index >= array->Size()) index = 0;
		return (*this);
	}

	cyclic_array_iterator& operator --()
	{
		index--;
		if (index < 0) index = array->Size()-1;
		return (*this);
	}

	int Index() const 
	{
		return index;
	}

	const A* Array() const
	{
		return array;
	}
};

template <class A>
class array_iterator
{
private:
	const A* array;
	int index;

public:
	typedef typename A::value_type value_type;

public:
	array_iterator(const A* _array = 0, int _index = -1):
		array(_array),
		index(_index)
	{
	}

	array_iterator(const array_iterator& i):
		array(i.array),
		index(i.index)
	{
	}

	array_iterator(const cyclic_array_iterator<A>& i):
		array(i.Array()),
		index(i.Index())
	{
	}

	~array_iterator()
	{
	}

	array_iterator& operator =(const array_iterator& i)
	{
		if (&i != this) {
			array  = i.array;
			index  = i.index;
		}
		return (*this);
	}

	friend bool operator ==<>(const array_iterator& i1, const array_iterator& i2);
	friend bool operator !=<>(const array_iterator& i1, const array_iterator& i2);

	bool Valid() const
	{
		return ((index >= 0) && (array != 0) && (index < array->Size()));
	}		

	value_type operator *() const
	{
		return (*array)[index];
	}

	array_iterator& operator ++()
	{
		index++;
		return (*this);
	}

	array_iterator& operator --()
	{
		index--;
		return (*this);
	}

	int Index() const 
	{
		return index;
	}

	const A* Array() const
	{
		return array;
	}
};

template <class T>
class Array
{
private:
   T   *data;
   int  size;

public:
	typedef T value_type;

	typedef array_iterator<Array<T> > const_iterator;
	typedef array_iterator<Array<T> > iterator;

	typedef cyclic_array_iterator<Array<T> > cyclic_iterator;
	typedef cyclic_array_iterator<Array<T> > const_cyclic_iterator;

public:
	Array(int _size = 1);
	Array(const Array& a);
	~Array();

	const T* const DataPointer() const { return data; }
	T* const DataPointer() { return data; }

	int Size()  const { return size; }
	void setSize(int _size);

	void clear();

	Array& operator =(const Array& a);

	T& operator [](int i);
	const T& operator [](int i) const;

	T& operator ()(int i);
	const T& operator ()(int i) const;

	iterator begin() const { return iterator(this,0); }
	iterator end() const { return iterator(this,size); }
};

// Array<T>

template <class T>
Array<T>::Array(int _size):
	size(_size)
{
   data = new T[size];
   for (int i = 0; i < size; ++i) data[i] = T();
}

template <class T>
Array<T>::Array(const Array& a):
	size(a.size)
{
   data = new T[size];
   for (int i = 0; i < size; ++i) data[i] = a.data[i];
}

template <class T>
Array<T>::~Array()
{
	 delete [] data;
}

template <class T>
Array<T>& Array<T>::operator =(const Array& a)
{
	if (&a != this) {
		if (size != a.size) {
			delete [] data;
			size  = a.size;
			data = new T[size];
		}
		for (int i = 0; i < size; ++i) data[i] = a.data[i];
	}
	return *this;
}

template <class T>
T& Array<T>::operator [](int i)
{
   assert((i >= 0) && (i < size));
   return data[i];
}

template <class T>
const T& Array<T>::operator [](int i) const
{
	assert((i >= 0) && (i < size));
	return data[i];
}

template <class T>
T& Array<T>::operator ()(int i)
{
   assert((i >= 1) && (i <= size));
   return data[i-1];
}

template <class T>
const T& Array<T>::operator ()(int i) const
{
	assert((i >= 1) && (i <= size));
	return data[i-1];
}

template <class T>
void Array<T>::setSize(int _size)
{
   if (_size != size) {
      delete [] data;
      size  = _size;
      data = new T[size];
      for (int i = 0; i < size; ++i) data[i] = T();
   }
}

template <class T>
void Array<T>::clear()
{
	for (int i = 0; i < size; ++i) data[i] = T();
}

template <>
class Array<double>
{
private:
   double *data;
   int size;

public:
	typedef double value_type;
	typedef double& referance_type;

	typedef array_iterator<Array<double> > const_iterator;
	typedef array_iterator<Array<double> > iterator;

	typedef cyclic_array_iterator<Array<double> > cyclic_iterator;
	typedef cyclic_array_iterator<Array<double> > const_cyclic_iterator;

public:
	Array(int _size = 1);
	Array(const Array& a);
	~Array();

	const double* const DataPointer() const { return data; }
	double* const DataPointer() { return data; }

	int Size()  const { return size; }
	void setSize(int _size);

	void clear();

	Array& operator =(const Array& a);

	double& operator [](int i);
	const double& operator [](int i) const;

	double& operator ()(int i);
	const double& operator ()(int i) const;

	iterator begin() const { return iterator(this,0); }
	iterator end() const { return iterator(this,size); }
};

//TODO: Remove specialized template definition after MS bug-fixing (expected for VS2004)
// Workaround for template generation bug in VS 2003 -> we force the compiler to generate
// the following template instances here.
template array_iterator<Array<double> >;
template cyclic_array_iterator<Array<double> >;

typedef Array<double> DoubleArray;

template <>
class Array<int>
{
private:
	int *data;
	int size;

public:
	typedef int value_type;
	typedef int& referance_type;

	typedef array_iterator<Array<int> > const_iterator;
	typedef array_iterator<Array<int> > iterator;

	typedef cyclic_array_iterator<Array<int> > cyclic_iterator;
	typedef cyclic_array_iterator<Array<int> > const_cyclic_iterator;

public:
	Array(int _size = 1);
	Array(const Array& a);
	~Array();

	const int* const DataPointer() const { return data; }
	int* const DataPointer() { return data; }

	int Size()  const { return size; }
	void setSize(int _size);

	void clear();

	Array& operator =(const Array& a);

	int& operator [](int i);
	const int& operator [](int i) const;

	int& operator ()(int i);
	const int& operator ()(int i) const;

	iterator begin() const { return iterator(this,0); }
	iterator end() const { return iterator(this,size); }
};

//TODO: Remove specialized template definition after MS bug-fixing (expected for VS2004)
// Workaround for template generation bug in VS 2003 -> we force the compiler to generate
// the following template instances here.
template array_iterator<Array<int> >;
template cyclic_array_iterator<Array<int> >;

typedef Array<int> IntArray;

}
}

#endif // __ARRAY_H
