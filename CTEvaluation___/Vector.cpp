// Vector.cpp
//	Gehört zur VMClasses-Bibliothek für Vector/Matrix-Rechnung
//	Ursprünglicher Autor D.Görsch - goersch.at.zeiss-izm.de
//  Klasse für allg. Vektor
//  wird über vmclasses.cpp eingebunden!!

void Vector::allocateMem()
{
	hadr = new double[dim];
	clear();
}

void Vector::freeMem()
{
	delete [] hadr;
}


Vector::Vector(int _dim):
	dim(_dim)
{
	allocateMem();
}


Vector::Vector(const Vector& x):
	dim(x.dim)
{
	allocateMem();
	memcpy(hadr,x.hadr,dim*sizeof(double));
}


Vector::~Vector()
{
	freeMem();
}


int Vector::Dim() const
{
	return dim;
}


void Vector::setDim(int _dim)
{
	if (_dim != dim) 
	{
		dim = _dim;
		freeMem();
		allocateMem();
	}
}

void Vector::clear()
{
	memset(hadr,0,dim*sizeof(double));
}

double& Vector::operator ()(int t)
{
	assert((t >= 1) && (t <= dim));
	return hadr[t-1];
}


double Vector::operator ()(int t) const
{
	assert((t >= 1) && (t <= dim));
	return hadr[t-1];
}

double& Vector::operator [](int t)
{
	assert((t >= 0) && (t < dim));
	return hadr[t];
}

double Vector::operator [](int t) const
{
	assert((t >= 0) && (t < dim));
	return hadr[t];
}

const Vector& Vector::operator =(double a)
{
	for (int i = 0; i < dim; ++i)
	{
		hadr[i] = a;
	}

	return (*this);
}

const Vector& Vector::operator =(const Vector& x)
{
	if (&x != this) 
	{
		setDim(x.dim);
		memcpy(hadr,x.hadr,dim*sizeof(double));
	}

	return (*this);
}

const Vector& Vector::operator +=(const Vector& x)
{
	assert(dim == x.dim);

	for (int i = 0; i < dim; ++i) 
	{
		hadr[i] += x.hadr[i];
	}

	return (*this);
}


const Vector& Vector::operator -=(const Vector& x)
{
	assert(dim == x.dim);

	for (int i = 0; i < dim; ++i) 
	{
		hadr[i] -= x.hadr[i];
	}

	return (*this);
}


const Vector& Vector::operator *=(double a)
{
	for (int i = 0; i < dim; ++i) 
	{
		hadr[i] *= a;
	}

	return (*this);
}

const Vector& Vector::operator /=(double a)
{
	for (int i = 0; i < dim; ++i) 
	{
		hadr[i] /= a;
	}

	return (*this);
}

Vector operator +(const Vector& x, const Vector& y)
{
	Vector s(x.dim);

	assert(x.dim == y.dim);

	for (int i = 0; i < x.dim; ++i)
	{
		s.hadr[i] = x.hadr[i] + y.hadr[i];
	}

	return s;
}

Vector operator -(const Vector& x, const Vector& y)
{
	Vector s(x.dim);

	assert(x.dim == y.dim);

	for (int i = 0; i < x.dim; ++i) 
	{
		s.hadr[i] = x.hadr[i] - y.hadr[i];
	}

	return s;
}

Vector operator *(double a, const Vector& x)
{
	Vector s(x.dim);

	for (int i = 0; i < x.dim; ++i) 
	{
		s.hadr[i] = a*x.hadr[i];
	}

	return s;
}

Vector operator *(const Vector& x, double a)
{
   return a*x;
}

Vector operator /(const Vector& x, double a)
{
	Vector s(x.dim);

	for (int i = 0; i < x.dim; ++i) 
	{
		s.hadr[i] = x.hadr[i]/a;
	}

	return s;
}

double norm(const Vector& x, int n)
{
	double result = 0.0;

	switch (n)
	{
		case 1:
		{
			for (int i = 0; i < x.dim; ++i)
			{
				result += abs(x.hadr[i]);
			}
			break;
		}
		case 2:
		{
			for (int i = 0; i < x.dim; ++i)
			{
				result += sqr(x.hadr[i]);
			}

			result = sqrt(result);
			break;
		}
		case 3:
		{
			result = abs(x.hadr[0]);

			for (int i = 1; i < x.dim; ++i)
			{
				result = max(result, abs(x.hadr[i]));
			}

			break;
		}
	}

	return result;
}

double norm(const Vector&  x)
{
	return norm(x,2);
}

double norm1(const Vector& x)
{
	return norm(x,1);
}

double norm2(const Vector& x)
{
	return norm(x,2);
}

double normInf(const Vector& x)
{
	return norm(x,3);
}

double dot(const Vector& x, const Vector& y)
{
	assert(x.dim == y.dim);

	double res = 0.0;
	
	for (int i = 0; i < x.dim; ++i)
	{
		res += x.hadr[i]*y.hadr[i];
	}

	return res;
}

Vector dot(const Vector& x, const Matrix& A)
{
	assert(x.Dim() == A.xDim());

	Vector y(A.yDim());

	for (int j = 1; j <= A.yDim(); ++j)
	{
		double s = 0.0;

		for (int i = 1; i <= A.xDim(); ++i)
		{
			s += x(i)*A(i,j);
		}

		y(j) = s;
	}

	return y;
}

Vector dot(const Matrix& A, const Vector& x)
{
	return A*x;
}

Matrix dyadic(const Vector& x, const Vector& y)
{
	Matrix m(x.Dim(),y.Dim());

	for (int i = 0; i < x.Dim(); ++i)
	{
		for(int j = 0; j < y.Dim(); ++j)
		{
			m.hadr[i][j] = x.hadr[i]*y.hadr[j];
		}
	}

	return m;
}

Vector cross(const Vector& x, const Vector& y)
{
	assert((x.Dim() == 3) && (y.Dim() == 3));

	Vector z(3);

	z(1) = x(2) * y(3) - x(3) * y(2);
	z(2) = x(3) * y(1) - x(1) * y(3);
	z(3) = x(1) * y(2) - x(2) * y(1);

	return z;
}

Matrix cross(const Vector& x, const Matrix& A)
{
	assert((x.Dim() == 3) && (A.xDim() == 3));

	Matrix B(3,A.yDim());

	for (int j = 1; j <= A.yDim(); ++j)
	{
		B(1,j) = x(2) * A(3,j) - x(3) * A(2,j);
		B(2,j) = x(3) * A(1,j) - x(1) * A(3,j);
		B(3,j) = x(1) * A(2,j) - x(2) * A(1,j);
	}

	return B;
}

Matrix cross(const Matrix& A, const Vector& x)
{
	assert((x.Dim() == 3) && (A.xDim() == 3));

	Matrix B(3,A.yDim());

	for (int j = 1; j <= A.yDim(); ++j)
	{
		B(1,j) = A(2,j) * x(3) - A(3,j) * x(2);
		B(2,j) = A(3,j) * x(1) - A(1,j) * x(3);
		B(3,j) = A(1,j) * x(2) - A(2,j) * x(1);
	}

	return B;
}

std::ostream& operator <<(std::ostream& os, const Vector& x)
{
	os << "[ ";

	for (int i = 1; i <= x.Dim(); ++i)
	{
		os << x(i) << " ";
	}

	os << "] ";

	return os;
}

Vector Vector::Ones(int dim)
{
	Vector x(dim);

	for (int i = 1; i <= x.Dim(); ++i)
	{
		x(i) = 1.0;
	}

	return x;
}

Vector Vector::Zeros(int dim)
{
	return Vector(dim);
}

// Vector.cpp