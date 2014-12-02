/***************************************************
 File Name:	distance.cpp
 Abstract:
 Author:	zhangzhelucky@126.com
 Update History:
     14-2-Dec	Starting this project
     14-3-Dec	First version Done.

****************************************************/

#include "string.h"
#include "math.h"
#include <stdlib.h>
#include <iostream>

using namespace std;


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
unsigned long nOutlier;		// the # of outliers


// Data Structure -----------------------------------
struct sItem
{
    unsigned long ItemId;
    double* AttrData;
    unsigned long Neighbour;
};

// Functions ----------------------------------------
double GetSqDistance( sItem* , sItem* );
bool isNeighbour( sItem* , sItem* );
bool AnalysePara(int , char *);
bool InitData();
void SeleteOutlier();


int main(int argc, char *argv[])
{
    if( ! AnalysePara( argc, *argv[]) )
	return 0;

    if( ! InitData() )
	return 0;
    
    NormalizeData();

    for (int i = 0; i < nTotalItem; i++) 
      for (int j = i+1; j < nTotalItem; j++) 
	if( isNeighbour( (pItem+i),(pItem+j) ) )
	{
	    ++ ((pItem+i)->Neighbour);
	    ++ ((pItem+j)->Neighbour);
	}

    SeleteOutlier();    

    return 0;
}


// Function ================================================
//   Analyse the input parameters, which follow the command
//   The command line to run this program:
// dbod [-r Input_file_name]  [-n Number_of_item_in_the_file]
//      [-a Number_of_attribute_of_each_item]
//      [-c A_fraction_of_total_item] [-d Neighbour_radius]
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
	cout << "Parameter Error. Please Check." << endl;
	return false;
    }

    for (int i = 0; i < argc; i++) 
    {
	if( 0 == strcmp("-r", argv +i ) )
	{
	    if( strlen( argv + i+1) > 255 )
	    {
		cout << "File name too long." << endl;
		return false;
	    }

	    strcpy( sFile, argv + i+1 );
	    ++ i;
	}
	else if( 0 == strcmp("-n", argv +i ) )
	{
	    nTotalItem = atof( argv +i+1);
	    if( nTotalItem <= 0 )
	    {
		cout << "Negative item number." << endl;
		return false;
	    }
	    ++ i;
	}
	else if( 0 == strcmp("-a", argv +i ) )
	{
	    nAttr = atof( argv +i+1);
	    if( nAttr <= 0 )
	    {
		cout << "Negative item attribute number." << endl;
		return false;
	    }
	    ++ i;
	}
	else if( 0 == strcmp("-c", argv +i ) )
	{
	    fFracRatio = atof( argv +i+1);
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

	    ++ i;
	}
	else if( 0 == strcmp("-d", argv +i ) )
	{
	    fNeibRadius = atof( argv +i+1);
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
    ifstream dataFile( sFile, ios::in |ios::nocreate, 1 );
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
	AttrMin[i] = temp;
	AttrMax[j] = temp;	    
    }

    (pItem +i)->Neighbour = 0;

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
	    AttrMax[j] =( (AttMax[j]<temp) ? temp : AttrMax[j] );
	    AttrMin[j] =( (AttMin[j]>temp) ? temp : AttrMin[j] );
	}

	(pItem +i)->Neighbour = 0;
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
	  pow(*((item_1 +i)->AttrData) - *((item_1 +i)->AttrData), 2);
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
    
    if( SqDis > SqNeiRadius )
      return false;
    else
      return true;
}


// Function ================================================
//   Select outliers 
// 
void SeleteOutlier()
{
    nOutlier = 0;
    unsigned long NeibThres = fFracRatio * nTotalItem;

    for (int i = 0; i < nTotalItem; i++) 
    {
	if( (pItem + i)->Neighbours < NeibThres )
	{
	    ++ nOutlier;
	    // The item 'i' is a outlier
	    // Here to do with this outlier
	    /////////////////////////////////////
	    // TODO:
	    //
	    /////////////////////////////////////
	}
    }
}


