// ex3.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <iostream>

using namespace std;

float Datatypeconversion(float x, float y);

int main()
{
	cout << (int)Datatypeconversion(15.6372, 6.0324);

	system("Pause");
    return 0;
}

float Datatypeconversion(float x, float y) {
	float a = x + y;
	return a;
}