// Matrix.h
#ifndef __IMT_MATRIX_H__
#define __IMT_MATRIX_H__


#include <Vector.h>


namespace Izm
{
	namespace Numerics
	{


		class Matrix
		{
		private:
			double **hadr;

			int xdim;
			int ydim;
			int rank;

			void allocateMem();
			void freeMem();

		public:
			explicit Matrix(int _xdim = 1, int _ydim = 1);
			Matrix(const Matrix& A);
			~Matrix();

			int xDim() const;
			void setxDim(int _xdim);

			int yDim() const;
			void setyDim(int _ydim);

			void setDim(int _xdim, int _ydim);
			void clear();

			int Rank() { return rank; }
			void setRank(int _rank) { rank = _rank; }

			double& operator ()(int i, int j);
			double operator ()(int i, int j) const;

			double *const operator [](int i);
			const double *const operator [](int i) const;

			Vector Column(int i) const;
			Vector Row(int i) const;

			const Matrix& operator =(const Matrix& A);

			const Matrix& operator +=(const Matrix &B);
			const Matrix& operator *=(const double a);

			friend Matrix operator *(const Matrix& A, const Matrix& B);
			friend Matrix operator +(const Matrix& A, const Matrix& B);
			friend Matrix operator -(const Matrix& A, const Matrix& B);
			friend Vector operator *(const Matrix& A, const Vector& x);

			friend Matrix operator*(const double a, const Matrix& B);
			friend Matrix transpose(const Matrix& A);
			friend Matrix dyadic(const Vector& x, const Vector& y);

			friend void Projection(const Vector& n, const Matrix& A, Matrix& B);

			friend std::ostream& operator <<(std::ostream& os, const Matrix& A);
		};
	}
}

// Matrix.h


#endif