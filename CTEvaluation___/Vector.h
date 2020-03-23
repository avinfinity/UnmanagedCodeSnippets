// Vector.h

class Vector
{
private:
	double *hadr;
	int dim;

	void allocateMem();
	void freeMem();

public:
	explicit Vector(int _dim = 1);
	Vector(const Vector& x);
	~Vector();

	void clear();

	int Dim() const;
	void setDim(int d);

	double& operator ()(int i);
	double operator ()(int i) const;

	double& operator [](int i);
	double operator [](int i) const;

	const Vector& operator =(double a);
	const Vector& operator =(const Vector& x);

	const Vector&  operator +=(const Vector& x);
	const Vector&  operator -=(const Vector& x);
	const Vector&  operator *=(double a);
	const Vector&  operator /=(double a);

	friend Vector operator +(const Vector& x, const Vector& y);
	friend Vector operator -(const Vector& x, const Vector& y);

	friend Vector operator *(double a, const Vector& x);
	friend Vector operator /(const Vector& x, double a);

	friend double norm(const Vector& x);
	friend double norm1(const Vector& x);
	friend double norm2(const Vector& x);
	friend double normInf(const Vector& x);
	friend double norm(const Vector& x, int n);

	friend double dot(const Vector& x, const Vector& y);
	friend Vector dot(const Vector& x, const Matrix& A);
	friend Vector dot(const Matrix& A, const Vector& x);

	friend Vector cross(const Vector& x, const Vector& y);
	friend Matrix cross(const Vector& x, const Matrix& A);
	friend Matrix cross(const Matrix& A, const Vector& x);
	
	friend Matrix dyadic(const Vector& x, const Vector& y);

	friend Vector operator *(const Matrix& A, const Vector& x);

	friend std::ostream& operator <<(std::ostream& os, const Vector& x);

	static Vector Ones(int dim);
	static Vector Zeros(int dim);
};


// Vector.h