
/***************************************************
 File Name:	grid.cpp
 Abstract:
 Author:	zhangzhelucky@126.com
 Update History:
     14-5-Dec	Starting this project

****************************************************/

#include "string.h"
#include "math.h"
#include <stdlib.h>
#include <iostream>
#include <fstream>

using namespace std;

#define _SHOW_(ITEM)	cout << ITEM->ItemId <<"is an outlier" <<endl



// Data Structure -----------------------------------
struct sItem
{
    char* ItemId;
    double* AttrData;
    sItem* pNext;
};

struct sGridNode
{
    unsigned int itemNum;
    sItem* pFirst;
    sItem* pLast;
    sGridNode** pNeibour;	// A collection of its level-2 neighbours
};


// Defination of Variates ---------------------------
// Inputs
char sFile[256];	 // -r	Indicates the input data file
unsigned int nTotalItem; // -n	The total # of items in DB
unsigned int nAttr;	 // -a	The # of Attributes in each item
double fFracRatio;	 // -c	A fraction of total items
double fNeibRadius;	 // -d	The Radius of Item to identify its Neighbours

// Global Variates
sItem* pItem;			// Point to the points of items
double SqNeibRadius;		// Square of "NeibRadius"
double GridLength;		// The side length of grid
unsigned long nOutlier = 0;	// the # of outliers
unsigned long NormalThres;	// A lower threshold of normal items

// Grid variates
double* AttrMax;		// Regist the max and min value of each attribute
double* AttrMin;
unsigned int* AttrCell;		// The # of cell of each attribute
unsigned int* AttrOffset;	// The memory offset of each attribute
sGridNode* Grid;		// Point to the grid
int CellNum;			// The # of node in grid map
int* NeibPoint;			// 
int NeibNum;			// Neighbour cell of one node
// Functions ----------------------------------------
// double GetSqDistance( sItem* , sItem* );
// bool isNeighbour( sItem* , sItem* );
bool AnalysePara();
bool AnalysePara( int , char **);
bool InitData();
void Partition();
//void NormalizeData();
void SelectOutlier();
int getLevel_1_Sum();
int getLevel_2_Sum();


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
    GridLength = ( fNeibRadius /2 ) / pow(nAttr, 0.5);

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
	    GridLength = ( fNeibRadius /2 ) / pow(nAttr, 0.5);
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

    // Initialize memory ===================
    pItem = new sItem[ nTotalItem ];
    AttrMax = new double[ nAttr ];
    AttrMin = new double[ nAttr ];
    AttrCell = new unsigned int[ nAttr ];

    // Get data =============================
    // Get first Item ----------
    dataFile >> ( pItem->ItemId );
    pItem->pNext = NULL;

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

    // Get rest Items ----------
    for (int i = 1; i < nTotalItem; i++)
    {
	dataFile >> ( (pItem +i)->ItemId );
	pItem->pNext = NULL;

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
//
// Return:
//
void Partition()
{
    // Make cells ================
    CellNum = 1;
    NeibNum = pow( 7, nAttr) - pow( 3, nAttr) + 1;

    for ( int i = 0; i < nAttr; ++i )
    {
	double tempRang = *( AttrMax +i ) - *( AttrMin +i );
	*(AttrCell +i) = (int)( tempRang / GridLength ) + 1;

	// Initialize the memory offset of current attribute
	*(AttrOffset + i) = CellNum;

	// Calcutate the total # of cells
	CellNum *= *(AttrCell +i);
    }

    Grid = new sGridNode[ CellNum ];
    for ( int i = 0; i < CellNum; ++i )
    {
	( Grid + i) -> itemNum = 0;
	( Grid + i) -> pFirst = NULL;
	( Grid + i) -> pLast = NULL;
	for ( int j = 0; j < NeibNum; ++j )
	  ((Grid + i) -> pNeibour)[j] = NULL;
    }

    // Partition items ===========
    int tempPoint = 0;
    for ( int i = 0; i < nTotalItem; ++i )
    {
	for ( int j = 0; j < nAttr; ++j )
	{
	    int temp = ( *((pItem +i)->AttrData +j) - *(AttrMin+j) )
		       / GridLength ;
	    tempPoint += temp * (*(AttrOffset +j));
	}

	sGridNode* pNode = Grid + tempPoint;
	if ( 0 == pNode->itemNum)
	  pNode->pFirst = pItem +i;
	else 
	  pNode->pLast->pNext = pItem + i;

	pNode->pLast = pItem + i;
	++ ( pNode -> itemNum);
    }

}


// Function ================================================
//
//
void getLevel_1_Sum( int node, int* sum, int attr = 0 )
{
    if( nAttr == attr )
    {
	int _pos;
	for ( int j = 0; j < nAttr; ++j )
	  _pos += *(NeibPoint+j) * (*AttrOffset+j);
	if ( (node + _pos) >= CellNum || (node + _pos) < 0 )
	    return;
	else
	  *sum += (Grid + _pos)->itemNum;
    }
    else
    {
	for ( int i = -1; i <= 1; ++i )
	{
	    *(NeibPoint + attr) = i;
	    getLevel_1_Sum( node , sum , attr+1 );
	}
    }
}


// Function ================================================
//
//
void getLevel_2_Sum( int node, int* sum, int attr = 0 )
{
    if( nAttr == attr )
    {
	int _pos;
	for ( int j = 0; j < nAttr; ++j )
	  _pos += *(NeibPoint+j) * (*AttrOffset+j);

	if ( (node + _pos) >= CellNum || (node + _pos) < 0 )
	    return;
	else
	{
	    *sum += (Grid + _pos)->itemNum;
	    int i = 0;
	    while ( NULL != *((Grid + node)->pNeibour) + i )
	      ++ i;
	    ((Grid + node)->pNeibour)[i] = (Grid + _pos);
	}
    }
    else
    {
	for ( int i = -3; i <= 3; ++i )
	{
	    if ( i > 1 || i < -1)
	      continue;
	    *(NeibPoint + attr) = i;
	    getLevel_2_Sum( node , sum , attr+1 );
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
    for ( int i = 0; i < CellNum; ++i )
    {
	// If current node is empty--------------------
	if ( 0 == (Grid +i)->itemNum)
	  break;

	// Checking level-1 cell pruning rule ---------
	int level_1_sum = 0;
	getLevel_1_Sum( i, &level_1_sum );
	
	// Checking level-2 cell pruning rule ---------
	int level_2_sum = 0;
	getLevel_2_Sum( i, &level_2_sum);

	// Pruning ------------------------------------
	if( level_1_sum >= NormalThres )
	{
	    // All items in current node are NORMAL
	}
	else if ( (level_2_sum + level_1_sum) < NormalThres )
	{
	    // ALL items in current node are OUTLIERS
	    sItem* tempItem = (Grid + i)->pFirst;
	    while ( NULL != tempItem );
	    {
		_SHOW_( tempItem );
		tempItem = tempItem->pNext;
	    } 
	}
	else
	{
	    // To check if items in this cell is outlier one by one
	    sItem* unknownItem = (Grid + i)->pFirst;
	    while ( NULL != unknownItem );
	    {
		sGridNode* NeibGrid =  *((Grid + i)->pNeibour) ;
		int tempSum = 0;
		while ( NULL != NeibGrid ) 
		{
		    sItem* NeibItem = NeibGrid->pFirst;
		    for ( int i = 0; i < NeibGrid->itemNum; ++i )
		    {
			if ( isNeighbour( unknownItem, NeibItem ) )
			{
			    ++ tempSum;
			    if ( tempSum >= NormalThres -level_1_sum )
			    {
				tempSum = -1;
				break;
			    }
			}
			NeibItem = NeibItem->pNext;
		    }
		    if ( -1 ==  tempSum )
		      break;
		    unknownItem = unknownItem->pNext;
		    ++ NeibGrid;
		}
		if ( tempSum < NormalThres -level_1_sum )
		  _SHOW_( unknownItem );
		unknownItem = unknownItem->pNext;
	    }
	}
    }
}



