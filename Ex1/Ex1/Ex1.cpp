// Ex1.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
using namespace std;

int main()
{
	int i, j, a;
	cout << "Please enter how big the Christmas tree is: ";
	cin >> a;
	for (i = 0; i < a; i++)
	{
		for (j = i; j < a+1; j++)
		{
			cout <<" ";
		}
		for (j = a-i*2; j < a+1; j++)
		{
			cout << "*";
		}
		cout << "\n";

	}
	for (int m = 0; m < 3; m++)
	{
		for (int n = 0; n < a+1; n++)
		{
			cout << " ";
		}
		cout<< "*" << "\n";
	}


	system("PAUSE");
    return 0;
}

