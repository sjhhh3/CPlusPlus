// Example_06.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <windows.h>
#include <ppl.h>

using namespace std;
using namespace concurrency;

template <class Function>
int time_call(Function &&f)
{
	int begin = GetTickCount();
	f();
	return GetTickCount() - begin;
}

// A is a m*n matrix, B is a n*k matrix, C=A*B ia a m*k matrix
void matrix_multiplication(double** A, double** B, double** C, int m, int n, int k)
{
	for (int i = 0; i < m; i++)
	{
		for (int j = 0; j < k; j++)
		{
			double tt = 0.0;
			for (int l = 0; l < n; l++)
			{
				tt += A[i][l] * B[l][j];
			}
			C[i][j] = tt;
		}
	}
}

void matrix_multiplication_parallel(double** A, double** B, double** C, int m, int n, int k)
{
	parallel_for(0, m, [&](int i)
	{
		for (int j = 0; j < k; j++)
		{
			double tt = 0.0;
			for (int l = 0; l < n; l++)
			{
				tt += A[i][l] * B[l][j];
			}
			C[i][j] = tt;
		}
	});
}

int _tmain(int argc, _TCHAR* argv[])
{
	int i, j, m = 800, n = 600, k = 1000;
	double** A = new double*[m];
	for (i = 0; i < m; i++)
		A[i] = new double[n];

	double **B = new double*[n];
	for (i = 0; i < n; i++)
		B[i] = new double[k];

	double **C = new double*[m];
	for (i = 0; i < m; i++)
		C[i] = new double[k];

	for (i = 0; i < m; i++)
	{
		for (j = 0; j < n; j++)
		{
			A[i][j] = i + j;
		}
	}

	for (i = 0; i < n; i++)
	{
		for (j = 0; j < k; j++)
		{
			B[i][j] = i + j;
		}
	}

	cout << "Serial: " << endl;
	cout << time_call([&]{matrix_multiplication(A, B, C, m, n, k); }) << " milliseconds.  ";
	cout << C[20][30] << endl << endl;
	cout << "Parallel: " << endl;
	cout << time_call([&]{matrix_multiplication_parallel(A, B, C, m, n, k); }) << " milliseconds.  ";
	cout << C[20][30] << endl;

	for (i = 0; i < m; i++)
		delete[]C[i];
	delete C;

	for (i = 0; i < n; i++)
		delete[]B[i];
	delete B;

	for (i = 0; i < m; i++)
		delete[]A[i];
	delete A;

	system("pause");
	return 0;
}
