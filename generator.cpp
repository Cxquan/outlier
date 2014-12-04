
/***************************************************
 File Name:	generator.cpp
 Abstract:
 Author:	zhangzhelucky@126.com
 Update History:
     14-4-Dec	Starting

****************************************************/

#include "math.h"
#include <stdlib.h>
#include <iostream>
#include <fstream>

using namespace std;

int regular();
void read();


int main(int argc, char *argv[])
{
    int total = regular();
    cout << total << endl;

    read();

    return 0;
}


// Function ----------------------------------------
//   Generate data items, which were posited on the
// vertex of cube.
// 
//   File name		"regular_1000_3attr.dat"
//   # of items		1000
//   # of attributes	3
//   Interval		2.3
// 
// Retrun: The # of items generated.
// -------------------------------------------------
int regular()
{
    int total = 0;
    ofstream datafile("regular_1000_3attr.dat", ios::trunc);

    for ( int i = 0; i < 10 ; ++i )
      for ( int j = 0; j < 10 ; ++j )
	for ( int k = 0; k < 10 ; ++k )
	{
	    // Generate ItemID
	    datafile << total << '\t';

	    // Genarate data
	    // datafile << (double)i + 0.1 << '\t' 
	    // 	     << (double)j + 0.2 << '\t' 
	    // 	     << (double)k + 0.3 << endl;

	    datafile << i << '\t' 
	    	     << j << '\t' 
	    	     << k << endl;

	    ++total;
	}

    datafile.close();
    return total;
}


void read()
{
    ifstream datafile("regular_1000_3attr.dat");

    double temp;

    for ( int i = 0; i < 1000 ; ++i )
    {
	for ( int j = 0; j < 3; ++j )
	{
	    datafile >> temp;
	    cout << temp << "\t|\t";
	}

	datafile >> temp;
	cout << temp << endl;

    }

    datafile.close();
    
}


