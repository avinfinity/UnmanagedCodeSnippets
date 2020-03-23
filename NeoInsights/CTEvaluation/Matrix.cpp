// Matrix.cpp
//	Gehört zur VMClasses-Bibliothek für Vector/Matrix-Rechnung
//	Ursprünglicher Autor D.Görsch - goersch.at.zeiss-izm.de
//  wird über vmclasses.cpp eingebunden!!
#include "Matrix.h"
#include <assert.h>


namespace Izm
{
	namespace Numerics
	{

		void Matrix::allocateMem()
		{
			hadr = new double*[xdim];
			for (int i = 0; i < xdim; ++i) hadr[i] = new double[ydim];
			clear();
		}

		void Matrix::freeMem()
		{
			for (int i = 0; i < xdim; ++i) delete[] hadr[i];
			delete[] hadr;
		}

		Matrix::Matrix(int _xdim, int _ydim) :
			xdim(_xdim), ydim(_ydim), rank(_ydim)
		{
			allocateMem();
		}

		Matrix::Matrix(const Matrix& A) :
			xdim(A.xdim), ydim(A.ydim), rank(A.rank)
		{
			allocateMem();

			for (int i = 0; i < xdim; ++i) {
				memcpy(hadr[i], A.hadr[i], ydim * sizeof(double));
			}
		}

		Matrix::~Matrix()
		{
			freeMem();
		}

		double& Matrix::operator ()(int i, int j)
		{
			assert((i >= 1) && (i <= xdim) && (j >= 1) && (j <= ydim));
			return hadr[i - 1][j - 1];
		}

		double Matrix::operator ()(int i, int j) const
		{
			assert((i >= 1) && (i <= xdim) && (j >= 1) && (j <= ydim));
			return hadr[i - 1][j - 1];
		}

		double *const Matrix::operator [](int i)
		{
			assert((i >= 0) && (i < xdim));
			return hadr[i];
		}

		const double *const Matrix::operator [](int i) const
		{
			assert((i >= 0) && (i < xdim));
			return hadr[i];
		}

		int Matrix::xDim() const
		{
			return xdim;
		}

		void Matrix::setxDim(int d)
		{
			setDim(d, ydim);
		}

		int Matrix::yDim() const
		{
			return ydim;
		}

		void Matrix::setyDim(int d)
		{
			setDim(xdim, d);
		}

		void Matrix::setDim(int _xdim, int _ydim)
		{
			if ((_xdim != xdim) || (_ydim != ydim)) {
				freeMem();

				xdim = _xdim;
				ydim = _ydim;
				rank = _ydim;

				allocateMem();
			}
		}

		const Matrix& Matrix::operator =(const Matrix& A)
		{
			if (&A != this) {
				setDim(A.xdim, A.ydim);

				rank = A.rank;

				for (int i = 0; i < A.xdim; ++i) {
					memcpy(hadr[i], A.hadr[i], ydim * sizeof(double));
				}
			}
			return (*this);
		}

		void Matrix::clear()
		{
			for (int i = 0; i < xdim; ++i) {
				memset(hadr[i], 0, ydim * sizeof(double));
			}
		}

		Matrix operator *(const Matrix& A, const Matrix& B)
		{
			Matrix C(A.xdim, B.ydim);

			assert(A.ydim == B.xdim);

			for (int i = 1; i <= A.xdim; ++i) {
				for (int j = 1; j <= B.ydim; ++j) {
					double s = 0.0;
					for (int k = 1; k <= A.ydim; ++k) {
						s += A(i, k)*B(k, j);
					}
					C(i, j) = s;
				}
			}

			return C;
		}

		Vector operator *(const Matrix& A, const Vector& x)
		{
			Vector p(A.xdim);

			assert(A.ydim == x.dim);

			for (int i = 1; i <= A.xdim; ++i) {
				double s = 0.0;
				for (int j = 1; j <= A.ydim; ++j) {
					s += A(i, j)*x(j);
				}
				p(i) = s;
			}

			return p;
		}

		Matrix operator +(const Matrix& A, const Matrix& B)
		{
			Matrix C(A.xdim, B.ydim);

			assert((A.xdim == B.xdim) && (A.ydim == B.ydim));

			for (int i = 1; i <= A.xdim; ++i) {
				for (int j = 1; j <= A.ydim; ++j) {
					C(i, j) = A(i, j) + B(i, j);
				}
			}

			return C;
		}

		Matrix operator -(const Matrix& A, const Matrix& B)
		{
			Matrix C(A.xdim, B.ydim);

			assert((A.xdim == B.xdim) && (A.ydim == B.ydim));

			for (int i = 1; i <= A.xdim; ++i) {
				for (int j = 1; j <= A.ydim; ++j) {
					C(i, j) = A(i, j) - B(i, j);
				}
			}

			return C;
		}

		const Matrix& Matrix::operator +=(const Matrix &B)
		{

			assert((xdim == B.xdim) && (ydim == B.ydim));

			for (int i = 1; i <= xdim; ++i) {
				for (int j = 1; j <= ydim; ++j) {
					hadr[i - 1][j - 1] += B(i, j);
				}
			}

			return *this;
		}

		const Matrix& Matrix::operator *=(const double a)
		{
			for (int i = 1; i <= xdim; ++i) {
				for (int j = 1; j <= ydim; ++j) {
					hadr[i - 1][j - 1] *= a;
				}
			}

			return *this;
		}

		Matrix operator*(const double a, const Matrix& B)
		{
			Matrix A(B);

			A *= a;

			return A;
		}

		Vector Matrix::Column(int i) const
		{
			Vector v(xdim);

			for (int is = 0; is < xdim; is++) v[is] = hadr[is][i];

			return v;
		}

		Vector Matrix::Row(int i) const
		{
			Vector v(ydim);

			for (int is = 0; is < ydim; is++) v[is] = hadr[i][is];

			return v;
		}

		Matrix transpose(const Matrix& A)
		{
			Matrix B(A.ydim, A.xdim);

			for (int i = 0; i < A.ydim; ++i)
			{
				for (int j = 0; j < A.xdim; ++j)
				{
					B.hadr[i][j] = A.hadr[j][i];
				}
			}

			return B;
		}

		void Projection(const Vector& n, const Matrix& A, Matrix& B)
		{
			assert((n.Dim() == A.xDim()));

			const double b = -1.0 / dot(n, n);

			Vector w(A.yDim());

			// w = b*(n^T)*A
			for (int j = 1; j <= A.yDim(); ++j)
			{
				double s = 0.0;
				for (int i = 1; i <= A.xDim(); ++i)
				{
					s += A(i, j)*n(i);
				}
				w(j) = b * s;
			}

			// B = A + n*w
			for (int i = 1; i <= A.xDim(); ++i)
			{
				for (int j = 1; j <= A.yDim(); ++j)
				{
					B(i, j) = A(i, j) + n(i)*w(j);
				}
			}
		}

		std::ostream& operator <<(std::ostream& os, const Matrix& A)
		{
			os << "[ ";

			for (int i = 1; i <= A.xDim(); ++i)
			{
				for (int j = 1; j <= A.yDim(); ++j)
				{
					os << A(i, j) << " ";
				}
				os << std::endl;
			}

			os << "] ";

			return os;
		}
	}
}


// Matrix.cpp