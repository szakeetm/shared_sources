/*
  File:        SearchInfo.h
  Description: 2D 'scientific oriented' chart component
               Handle search within the chart data on mouse click
               C++ port of fr.esrf.tangoatk.widget.util.chart
  Author:      J-L PONS (2007)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
*/

#ifndef _GLCHARTSEARCHINFO_
#define _GLCHARTSEARCHINFO_

class SearchInfo {

public:

   /** True when a point has been found */
   BOOL     found;
   /** X pixel coordinates of the point found */
   int         x;
   /** Y pixel coordinates of the point found */
   int         y;
   /** Axis on which the view containing the point is displayed */
   GLAxis      *axis;

   /** Y DataView which contains the point */
   GLDataView  *dataView;
   
   /** Handle to the y value */
   DataList    *value;

   /** X DataView which countaing the point (XY monitoring) */
   GLDataView  *xdataView;

   /** Handle to the X value (XY monitoring)*/
   DataList    *xvalue;
   
   /** Square distance from click to point (pixel) */
   double      dist;
   
   /** placement of the tooltip panel */
   int         placement;   
   
   /** index in the dataView that contains the clicked point */
   int         clickIdx;   
   
   SearchInfo(int x,int y,GLDataView *dataView,GLAxis *axis,DataList *value,double dist,int placement,int idx) 
   {
     this->found=true;
     this->x=x;
     this->y=y;
     this->dataView=dataView;
     this->value=value;
     this->dist=dist;
     this->placement=placement;
     this->axis=axis;
     this->xvalue=NULL;
     this->xdataView=NULL;
     this->clickIdx=idx;
   }

   void setXValue(DataList *d,GLDataView  *x) {
	   this->xvalue    = d;
	   this->xdataView = x;
   }

   SearchInfo() 
   {
     this->found=FALSE;
     this->dist=1e100;
     this->clickIdx=-1;
   }
   
};

#endif /* _GLCHARTSEARCHINFO_ */
