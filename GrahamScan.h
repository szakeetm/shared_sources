
#pragma once

//#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include <time.h>
using namespace std;

#include "Vector.h"

//--------------------GRpoint DATA STRUCTURE---------------------------
class GRpoint;

//--------------------GLOBAL VARIABLES---------------------------
const int NumPoints = 15; // n<1000
//SDL_Surface *screen; //GLOBAL SDL GRAPHICS SURFACE POINTER

//--------------------GRAHAM'S SCAN FUNCTIONS---------------------------
void grahamInit(); //INITIALIZE VARIABLES, RANDOMLY GENERATE POINTS,
                   //LOCATE MIN GRpoint, AND SORT POINTS BY RELATIVE ANGLES
void grahamMain(Vector2d *projectedPoints,size_t nbPoints,int *returnList); //SETUP, RUN GRAHAM'S SCAN, AND DISPLAY RESULTS
void grahamScan(GRpoint *P); //ACTUAL GRAHAM'S SCAN PROCEDURE
void constructReturnList();
bool isConvexPoint(GRpoint *P); //TEST GRpoint FOR CONVEXITY
void addPoint(GRpoint GRpoint); //ADDS GRpoint TO DOUBLELY LINKED LIST (USED DURING SORTING)
double findAngle(double x1, double y1, double x2, double y2); //FIND ANGLE GIVEN TWO POINTS

/*
//--------------------AUXILARY GRAPHICS FUNCTIONS---------------------------
void initScreen(); //SETUP THE GRAPHICS SURFACE AND WINDOW
void drawPoints(); //DRAW POINTS FROM GLOBAL DOUBLELY LINKED LIST
void drawLine(GRpoint *A, GRpoint *B, int color); //DRAWS A LINE WITH 3 COLOR POSSIBILITIES
void drawPermeter(int color); //DRAWS PERIMETER WITH 3 COLOR POSSIBILITIES
void graphicsLoop(); //MAIN GRAPHICS LOOP
void printPoints(); //PRINTS ALL POINTS IN DOUBLELY LINKED LIST
*/
