// Random.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
using namespace std;


int main()
{
	cout << "Element	------	Value" << endl;
	srand(time(0));
	int array1[11];
	cout << array1[0] << endl;
	cout << array1[1] << endl;

	for (int x = 1; x <=10; x++) {       // 因为x用作数组元素代号，如果从1开始就将array1[0]未录入数据。栈中的初值0xcccccccc化成整数就是-858993460.
		array1[x] = 1 + (rand() % 100);
		cout << x << "	------	  " << array1[x] << endl;
	}
	cout << array1[1] << endl;
	return 0;
}

