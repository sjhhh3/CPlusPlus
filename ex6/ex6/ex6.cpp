// ex6.cpp : 定义控制台应用程序的入口点。
//


#include "stdafx.h"
#include <iostream>

int m_add_1(int a, int b)
{
	a = 2 * a;
	return a + b;
}

int m_add_3(int &a, int &b)
{
	a = 2 * a;
	return a + b;
}

int main()
{

	std::cout << m_add_1(100, 200) << std::endl;
	std::cout << m_add_1(100, 200) << std::endl;
	std::cout << m_add_1(100, 200) << std::endl << std::endl;

	int i = 100, j = 200;
	std::cout << m_add_3(i, j) << std::endl;
	std::cout << m_add_3(i, i) << std::endl;
	std::cout << m_add_3(i, j) << std::endl;
	std::cout << i << std::endl;

	return 0;
}

