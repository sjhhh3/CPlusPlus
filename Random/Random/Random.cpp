// Random.cpp : �������̨Ӧ�ó������ڵ㡣
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

	for (int x = 1; x <=10; x++) {       // ��Ϊx��������Ԫ�ش��ţ������1��ʼ�ͽ�array1[0]δ¼�����ݡ�ջ�еĳ�ֵ0xcccccccc������������-858993460.
		array1[x] = 1 + (rand() % 100);
		cout << x << "	------	  " << array1[x] << endl;
	}
	cout << array1[1] << endl;
	return 0;
}

