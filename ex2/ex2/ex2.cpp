// ex2.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <iostream>

using namespace std;

int main()
{
	float a = 1.43;
	float b = (int) a;
	if (typeid(b) == typeid(int)) cout << "int" <<b;
	if (typeid(b) == typeid(float)) cout << "float" <<b;


	system("pause");
    return 0;
}

