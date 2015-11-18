// ex4.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>

using namespace std;

class Property {
     public:
		  float setandgetNumber(float num) {
			 x = num;
			 return x;
		 }
	   /* float TypeConversion(){
			  float y = x;
			  return (int)y;
	      }*/
	 private:
		 float x;
};

int main()
{
	Property anwser;
    cout << anwser.setandgetNumber(5.234234) << endl;
	system("Pause");
    return 0;
}

