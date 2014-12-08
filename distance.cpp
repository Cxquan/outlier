
/***************************************************
 File Name:	distance.cpp
 Abstract:
 Author:	zhangzhelucky@126.com
 Update History:
     14-2-Dec	Starting this project
     14-3-Dec	First version Done.

     14-4-Dec	Simplified the SelectOutlier function
		Simplified the Data Struction
		Changed the selection algorithm

     4-Dec	Modified some longuage mistakes
		First time Complied
     4-Dec	Modified some mistakes
		First time run correctly
     5-Dec	Changed the type of "ItemId" from "unsigned
	     long" to "char[32]"
****************************************************/

#include "string.h"
#include "math.h"
#include <stdlib.h>
#include <iostream>
#include <fstream>

using namespace std;


// Data Structure -----------------------------------
struct sItem
{
    char ItemId[32];
    //    unsigned long ItemId;
    double* AttrData;
};

// Defination of Variates ---------------------------
// Inputs
char sFile[256];	 // -r	Indicates the input data file
int nTotalItem;		 // -n	The total # of items in DB
int nAttr;		 // -a	The # of Attributes in each item
double fFracRatio;	 // -c	A fraction of total items
double fNeibRadius;	 // -d	The Radius of Item to identify its Neighbours

// Global Vatiates
sItem* pItem;			// Point to the points of items
double SqNeibRadius;		// Square of "NeibRadius"
double* AttrMax;
double* AttrMin;
unsigned long nOutlier = 0;	// the # of outliers
unsigned long NormalThres;	// A lower threshold of normal items
unsigned long* nNeighbour; // record # of neighbours for each point

// Functions ----------------------------------------
double GetSqDistance( sItem* , sItem* );
bool isNeighbour( sItem* , sItem* );
bool AnalysePara();
bool AnalysePara( int , char **);
bool InitData();
void NormalizeData();
void SelectOutlier();


int main(int argc, char *argv[])
{
    // DEBUGING ========================
    // if( ! AnalysePara( argc, argv) )
    // 	return 0;
    AnalysePara();
    // DEBUGING ========================

    if( ! InitData() )
	return 0;

    unsigned long id;
    double* temp;

    cout << "Reading data done." << endl;

    // DEBUGING ========================
    // for ( int i = 0; i < nTotalItem; ++i )
    // {
    // 	id = (pItem + i)-> ItemId;
    // 	temp = (pItem + i)-> AttrData;
    // 	cout << id <<'\t'
    // 	     << *(temp) << '\t'
    // 	     << *(temp +1) << '\t'
    // 	     << *(temp +2) << endl;
    // }
    // return 0;
    // DEBUGING ========================


    
    //    NormalizeData();

    SelectOutlier();    

    return 0;
}


// Function ================================================
//   The simply version of this function.
//
bool AnalysePara()
{
    strcpy( sFile, "regular_1000_3attr.dat" );

    nTotalItem = 1000;

    nAttr = 3;

    fFracRatio = 0.1;
    NormalThres = ( 1-fFracRatio ) * nTotalItem - 1;

    fNeibRadius = 0.99;
    SqNeibRadius = fNeibRadius * fNeibRadius;

    return true;
}


// Function ================================================
//   Analyse the input parameters, which follow the command
//   The command line to run this program:
// outlier [-r Input_file_name]  [-n Number_of_item_in_the_file]
//         [-a Number_of_attribute_of_each_item]
//	   [-c A_fraction_of_total_item] [-d Neighbour_radius]
// 
//   Get the running parameters from the command line
//
// Return:
//   If the command goes with the usage above, return TRUE
//   Else, return FALSE and the program will be killed
//
bool AnalysePara(int argc, char *argv[])
{
    if( 10 != argc )
    {
	cout << "Wrong Parameter. Please Check." << endl;
	return false;
    }

    for (int i = 0; i < argc; i++) 
    {
	if( 0 == strcmp("-r", *argv +i ) )
	{
	    if( strlen( *argv + i+1) > 255 )
	    {
		cout << "File name too long." << endl;
		return false;
	    }

	    strcpy( sFile, *argv + i+1 );
	    ++ i;
	}
	else if( 0 == strcmp("-n", *argv +i ) )
	{
	    nTotalItem = atof( *argv +i+1);
	    if( nTotalItem <= 0 )
	    {
		cout << "Negative item number." << endl;
		return false;
	    }
	    ++ i;
	}
	else if( 0 == strcmp("-a", *argv +i ) )
	{
	    nAttr = atof( *argv +i+1);
	    if( nAttr <= 0 )
	    {
		cout << "Negative item attribute number." << endl;
		return false;
	    }
	    ++ i;
	}
	else if( 0 == strcmp("-c", *argv +i ) )
	{
	    fFracRatio = atof( *argv +i+1);
	    if( fFracRatio < 0 )
	    {
		cout << "Negative fraction ratio." << endl;
		return false;
	    }
	    else if( fFracRatio > 1 )
	    {
		cout << "Fraction ratio higher than 1." << endl;
		return false;
	    }

	    NormalThres = ( 1-fFracRatio ) * nTotalItem - 1;
	    ++ i;
	}
	else if( 0 == strcmp("-d", *argv +i ) )
	{
	    fNeibRadius = atof( *argv +i+1);
	    SqNeibRadius = fNeibRadius * fNeibRadius;
	    if( fNeibRadius <= 0 )
	    {
		cout << "Negative Radius." << endl;
		return false;
	    }
	    ++ i;
	}
    }

    return true;
}


// Function ================================================
// Initialize data
// 1. Read data from the data file;
// 2. Get the max and min of each attributes of all the items,
//    this is calculated during the data reading process
// 
// Return:
//   If the data file is open correctly, return TRUE
// 
bool InitData()
{
    ifstream dataFile( sFile );
    if( ! dataFile.is_open() )
    {
	cout << "Open file error." << endl;
	return false;
    }

    // Initialize memory
    pItem = new sItem[ nTotalItem ];
    AttrMax = new double[ nAttr ];
    AttrMin = new double[ nAttr ];

    // Get data =============================
    // Get first Item -------------
    dataFile >> ( pItem->ItemId );

    pItem->AttrData = new double[ nAttr ];
    for (int j=0; j < nAttr; j++)
    {
	double temp;
	dataFile >> temp;
	*((pItem->AttrData) + j) = temp;
	// Get the max and min of the Attributes
	AttrMin[j] = temp;
	AttrMax[j] = temp;	    
    }

    // Get rest Items -------------
    for (int i = 1; i < nTotalItem; i++)
    {
	dataFile >> ( (pItem +i)->ItemId );

	(pItem +i)->AttrData = new double[ nAttr ];
	for (int j=0; j < nAttr; j++)
	{
	    double temp;
	    dataFile >> temp;
	    *( ((pItem +i)->AttrData) + j) = temp;

	    // Get the max and min of the Attributes
	    AttrMax[j] =( (AttrMax[j]<temp) ? temp : AttrMax[j] );
	    AttrMin[j] =( (AttrMin[j]>temp) ? temp : AttrMin[j] );
	}
    }
    
    dataFile.close();
    return true;
}


// Function ================================================
// Normalize the data in follow formula
//   out = ( in - min ) / ( max - min )
//
//   The output data should be in the range of 0 and 1
//
// 
void NormalizeData()
{
    for (int j = 0; j < nAttr; j++)		// Attribute Loop
    {
	double Rang = AttrMax[j] - AttrMin[j];
	for (int i = 0; i < nTotalItem; i++)	// Item Loop
	{
	    double temp = *( ((pItem +i)->AttrData) + j);
	    temp = (temp - AttrMin[j] ) / Rang;
	    *( ((pItem +i)->AttrData) + j) = temp;
	}
    }
}


// Function ================================================
//   Compute and return the Squared distance of two item
// 
double GetSqDistance( sItem* item_1, sItem* item_2)
{
    double SqDis = 0;
    for (int i = 0; i < nAttr; i++) 
    {
	SqDis += 
	  pow( *(item_1->AttrData +i) - *(item_2->AttrData +i), 2);
    }
    
    return SqDis;
}


// Function ================================================
//   To identify if two items are Neighbours based on thire
// distance.
//   Note that the "distance" refered here is exactly the 
// "Squared Distance" to avoid a great deal of costly square
// root arithnetic. 
//   The comparison is made between the Squared Distance 
// and the Squared Neighbour Radius to identify the 
//
// Return:
//   If the two items are Neighbours, return TRUE
//   Else, return FALSE
// 
bool isNeighbour( sItem* item_1, sItem* item_2 )
{
    double SqDis = GetSqDistance( item_1, item_2);
    
    if( SqDis > SqNeibRadius )
      return false;
    else
      return true;
}


// Function ================================================
//   Select outliers 
// 
void SelectOutlier()
{
    cout << "Outliers:" << endl;

	nNeighbour = new long[ nTotalItem ];
	memset( nNeighbour, 0, sizeof( nNeighbour ) );

    for (int i = 0; i < nTotalItem; i++) 
    {
	for (int j = i+1; j < nTotalItem; j++) 
	{
	    if( nNeighbour[i] >= NormalThres)
	      break;
	    else if( isNeighbour( (pItem+i),(pItem+j) ) )
		{
	      ++nNeighbour[i];
		  ++nNeighbour[j];
		}
	}

	if( nNeighbour[i] < NormalThres)
	{
	    // The item 'i' is a outlier
	    // Here to do with this outlier
	    /////////////////////////////////////
	    // TODO:
	    ++ nOutlier;
	    cout << (pItem + i)-> ItemId << " with only " 
		 << nNeighbour[i] << "\tNeighbours"
		 << endl;

	    /////////////////////////////////////

	}
    }

}


