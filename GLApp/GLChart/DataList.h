/*
Program:     MolFlow+
Description: Monte Carlo simulator for ultra-high vacuum
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY
Copyright:   E.S.R.F / CERN
Website:     https://cern.ch/molflow

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/
#ifndef _GLCHARTDATALISTH_
#define _GLCHARTDATALISTH_

typedef struct _DataList {

	// Original coordinates
	/** x value */
	double x;	
	/** y value */
	double y;	
	//pointer to next item
	_DataList *next;
	
} DataList;

#endif /* _GLCHARTDATALISTH_ */
