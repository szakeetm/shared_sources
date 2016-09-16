/*
  File:        DrawingArea.cpp
  Description: Simple drawing area
  Program:     MolFlow
  Author:      R. KERSEVAN / J-L PONS / M ADY
  Copyright:   E.S.R.F / CERN

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
*/

#include "GLApp/GLApp.h"
#include "DrawingArea.h"
#include "GLApp/GLToolkit.h"
#include <math.h>
#include <malloc.h>

// ----------------------------------------------

DrawingArea::DrawingArea():GLComponent(0) {

  p1.nbPts = 4;
  p1.pts   = (VERTEX2D *)malloc( p1.nbPts* sizeof(VERTEX2D) );
  p1.pts[0].u = 50.0;
  p1.pts[0].v = 50.0;
  p1.pts[1].u = 150.0;
  p1.pts[1].v = 50.0;
  p1.pts[2].u = 150.0;
  p1.pts[2].v = 150.0;
  p1.pts[3].u = 50.0;
  p1.pts[3].v = 150.0;
  p1.sign = 1.0;

  // Test 1
  p21.nbPts = 7;
  p21.pts   = (VERTEX2D *)malloc( p21.nbPts* sizeof(VERTEX2D) );
  p21.pts[0].u = 80.0;
  p21.pts[0].v = 30.0;
  p21.pts[1].u = 100.0;
  p21.pts[1].v = 120.0;
  p21.pts[2].u = 80.0;
  p21.pts[2].v = 130.0;
  p21.pts[3].u = 120.0;
  p21.pts[3].v = 160.0;
  p21.pts[4].u = 170.0;
  p21.pts[4].v =  80.0;
  p21.pts[5].u = 130.0;
  p21.pts[5].v = 175.0;
  p21.pts[6].u = 10.0;
  p21.pts[6].v = 100.0;
  p21.sign = 1.0;

  nbPoly1 = IntersectPoly(&p1,&p21,NULL,&result1);

  // Test 2
  p22.nbPts = 4;
  p22.pts   = (VERTEX2D *)malloc( p22.nbPts* sizeof(VERTEX2D) );
  p22.pts[0].u = 70.0;
  p22.pts[0].v = 50.0;
  p22.pts[1].u = 150.0;
  p22.pts[1].v = 50.0;
  p22.pts[2].u = 150.0;
  p22.pts[2].v = 160.0;
  p22.pts[3].u = 70.0;
  p22.pts[3].v = 150.0;
  p22.sign = 1.0;

  nbPoly2 = IntersectPoly(&p1,&p22,NULL,&result2);

  // Test 3
  p23.nbPts = 4;
  p23.pts   = (VERTEX2D *)malloc( p23.nbPts* sizeof(VERTEX2D) );
  p23.pts[0].u = 50.0;
  p23.pts[0].v = 50.0;
  p23.pts[1].u = 150.0;
  p23.pts[1].v = 50.0;
  p23.pts[2].u = 150.0;
  p23.pts[2].v = 150.0;
  p23.pts[3].u = 50.0;
  p23.pts[3].v = 150.0;
  p23.sign = 1.0;

  nbPoly3 = IntersectPoly(&p1,&p23,NULL,&result3);

  // Test 4
  p24.nbPts = 4;
  p24.pts   = (VERTEX2D *)malloc( p24.nbPts* sizeof(VERTEX2D) );
  p24.pts[0].u = 70.0;
  p24.pts[0].v = 50.0;
  p24.pts[1].u = 140.0;
  p24.pts[1].v = 50.0;
  p24.pts[2].u = 140.0;
  p24.pts[2].v = 140.0;
  p24.pts[3].u = 70.0;
  p24.pts[3].v = 140.0;
  p24.sign = 1.0;

  nbPoly4 = IntersectPoly(&p1,&p24,NULL,&result4);

  // Test 5
  p25.nbPts = 4;
  p25.pts   = (VERTEX2D *)malloc( p25.nbPts* sizeof(VERTEX2D) );
  p25.pts[0].u = 50.0;
  p25.pts[0].v = 50.0;
  p25.pts[1].u = 140.0;
  p25.pts[1].v = 50.0;
  p25.pts[2].u = 140.0;
  p25.pts[2].v = 140.0;
  p25.pts[3].u = 50.0;
  p25.pts[3].v = 140.0;
  p25.sign = 1.0;

  nbPoly5 = IntersectPoly(&p1,&p25,NULL,&result5);

  // Test 6
  p26.nbPts = 8;
  p26.pts   = (VERTEX2D *)malloc( p26.nbPts* sizeof(VERTEX2D) );
  p26.pts[0].u = 70.0;
  p26.pts[0].v = 150.0;
  p26.pts[1].u = 90.0;
  p26.pts[1].v = 150.0;
  p26.pts[2].u = 90.0;
  p26.pts[2].v = 170.0;
  p26.pts[3].u = 110.0;
  p26.pts[3].v = 170.0;
  p26.pts[4].u = 110.0;
  p26.pts[4].v = 130.0;
  p26.pts[5].u = 130.0;
  p26.pts[5].v = 130.0;
  p26.pts[6].u = 130.0;
  p26.pts[6].v = 190.0;
  p26.pts[7].u = 70.0;
  p26.pts[7].v = 190.0;
  p26.sign = 1.0;

  nbPoly6 = IntersectPoly(&p1,&p26,NULL,&result6);

  // Test 7
  p27.nbPts = 4;
  p27.pts   = (VERTEX2D *)malloc( p27.nbPts* sizeof(VERTEX2D) );
  p27.pts[0].u = 160.0;
  p27.pts[0].v = 40.0;
  p27.pts[1].u = 160.0;
  p27.pts[1].v = 190.0;
  p27.pts[2].u = 50.0;
  p27.pts[2].v = 190.0;
  p27.pts[3].u = 50.0;
  p27.pts[3].v = 150.0;
  p27.sign = 1.0;

  nbPoly7 = IntersectPoly(&p1,&p27,NULL,&result7);

  // Test 8
  p28.nbPts = 4;
  p28.pts   = (VERTEX2D *)malloc( p28.nbPts* sizeof(VERTEX2D) );
  p28.pts[0].u = 150.0;
  p28.pts[0].v = 70.0;
  p28.pts[1].u = 190.0;
  p28.pts[1].v = 70.0;
  p28.pts[2].u = 190.0;
  p28.pts[2].v = 120.0;
  p28.pts[3].u = 150.0;
  p28.pts[3].v = 120.0;
  p28.sign = 1.0;

  nbPoly8 = IntersectPoly(&p1,&p28,NULL,&result8);

  // Test 9
  p29.nbPts = 3;
  p29.pts   = (VERTEX2D *)malloc( p29.nbPts* sizeof(VERTEX2D) );
  p29.pts[0].u = 170.0;
  p29.pts[0].v = 70.0;
  p29.pts[1].u = 190.0;
  p29.pts[1].v = 70.0;
  p29.pts[2].u = 180.0;
  p29.pts[2].v = 120.0;
  p29.sign = 1.0;

  nbPoly9 = IntersectPoly(&p1,&p29,NULL,&result9);

  // Test A
  p2A.nbPts = 3;
  p2A.pts   = (VERTEX2D *)malloc( p2A.nbPts* sizeof(VERTEX2D) );
  p2A.pts[0].u = 150.0;
  p2A.pts[0].v = 50.0;
  p2A.pts[1].u = 180.0;
  p2A.pts[1].v = 70.0;
  p2A.pts[2].u = 170.0;
  p2A.pts[2].v = 120.0;
  p2A.sign = 1.0;

  nbPolyA = IntersectPoly(&p1,&p2A,NULL,&resultA);

  // Test B
  p2B.nbPts = 4;
  p2B.pts   = (VERTEX2D *)malloc( p2B.nbPts* sizeof(VERTEX2D) );
  p2B.pts[0].u = 160.0;
  p2B.pts[0].v = 50.0;
  p2B.pts[1].u = 160.0;
  p2B.pts[1].v = 190.0;
  p2B.pts[2].u = 50.0;
  p2B.pts[2].v = 190.0;
  p2B.pts[3].u = 60.0;
  p2B.pts[3].v = 150.0;
  p2B.sign = 1.0;

  nbPolyB = IntersectPoly(&p1,&p2B,NULL,&resultB);

  // Test C
  p2C.nbPts = 4;
  p2C.pts   = (VERTEX2D *)malloc( p2C.nbPts* sizeof(VERTEX2D) );
  p2C.pts[0].u = 50.0;
  p2C.pts[0].v = 50.0;
  p2C.pts[1].u = 150.0;
  p2C.pts[1].v = 50.0;
  p2C.pts[2].u = 75.0;
  p2C.pts[2].v = 100.0;
  p2C.pts[3].u = 50.0;
  p2C.pts[3].v = 150.0;
  p2C.sign = 1.0;

  nbPolyC = IntersectPoly(&p1,&p2C,NULL,&resultC);

  // Test D
  p2D.nbPts = 6;
  p2D.pts   = (VERTEX2D *)malloc( p2D.nbPts* sizeof(VERTEX2D) );
  p2D.pts[0].u = 150.0;
  p2D.pts[0].v = 50.0;
  p2D.pts[1].u = 170.0;
  p2D.pts[1].v = 50.0;
  p2D.pts[2].u = 190.0;
  p2D.pts[2].v = 100.0;
  p2D.pts[3].u = 120.0;
  p2D.pts[3].v = 170.0;
  p2D.pts[4].u = 110.0;
  p2D.pts[4].v = 170.0;
  p2D.pts[5].u = 160.0;
  p2D.pts[5].v = 60.0;
  p2D.sign = 1.0;

  nbPolyD = IntersectPoly(&p1,&p2D,NULL,&resultD);

  // Test E
  p2E.nbPts = 3;
  p2E.pts   = (VERTEX2D *)malloc( p2E.nbPts* sizeof(VERTEX2D) );
  p2E.pts[0].u = 120.0;
  p2E.pts[0].v = 70.0;
  p2E.pts[1].u = 140.0;
  p2E.pts[1].v = 70.0;
  p2E.pts[2].u = 130.0;
  p2E.pts[2].v = 120.0;
  p2E.sign = 1.0;

  nbPolyE = IntersectPoly(&p1,&p2E,NULL,&resultE);

  // Test F
  p2F.nbPts = 3;
  p2F.pts   = (VERTEX2D *)malloc( p2F.nbPts* sizeof(VERTEX2D) );
  p2F.pts[0].u = 50.0;
  p2F.pts[0].v = 50.0;
  p2F.pts[1].u = 140.0;
  p2F.pts[1].v = 70.0;
  p2F.pts[2].u = 130.0;
  p2F.pts[2].v = 120.0;
  p2F.sign = 1.0;

  nbPolyF = IntersectPoly(&p1,&p2F,NULL,&resultF);

  // Test G
  p2G.nbPts = 10;
  p2G.pts   = (VERTEX2D *)malloc( p2G.nbPts* sizeof(VERTEX2D) );
  p2G.pts[0].u = 40.0;
  p2G.pts[0].v = 60.0;
  p2G.pts[1].u = 160.0;
  p2G.pts[1].v = 60.0;
  p2G.pts[2].u = 160.0;
  p2G.pts[2].v = 100.0;
  p2G.pts[3].u = 40.0;
  p2G.pts[3].v = 100.0;
  p2G.pts[4].u = 45.0;
  p2G.pts[4].v = 90.0;
  p2G.pts[5].u = 155.0;
  p2G.pts[5].v = 90.0;
  p2G.pts[6].u = 155.0;
  p2G.pts[6].v = 80.0;
  p2G.pts[7].u = 45.0;
  p2G.pts[7].v = 80.0;
  p2G.pts[8].u = 45.0;
  p2G.pts[8].v = 90.0;
  p2G.pts[9].u = 40.0;
  p2G.pts[9].v = 100.0;
  p2G.sign = 1.0;

  nbPolyG = IntersectPoly(&p2G,&p1,NULL,&resultG);

  // Test H
  p2H.nbPts = 10;
  p2H.pts   = (VERTEX2D *)malloc( p2H.nbPts* sizeof(VERTEX2D) );
  int vH[] = {1,1,1,0,1,1,1,1,0,1};
  p2H.pts[0].u = 40.0;
  p2H.pts[0].v = 60.0;
  p2H.pts[1].u = 160.0;
  p2H.pts[1].v = 60.0;
  p2H.pts[2].u = 160.0;
  p2H.pts[2].v = 100.0;
  p2H.pts[3].u = 40.0;
  p2H.pts[3].v = 100.0;
  p2H.pts[4].u = 75.0;
  p2H.pts[4].v = 90.0;
  p2H.pts[5].u = 155.0;
  p2H.pts[5].v = 90.0;
  p2H.pts[6].u = 155.0;
  p2H.pts[6].v = 80.0;
  p2H.pts[7].u = 75.0;
  p2H.pts[7].v = 80.0;
  p2H.pts[8].u = 75.0;
  p2H.pts[8].v = 90.0;
  p2H.pts[9].u = 40.0;
  p2H.pts[9].v = 100.0;
  p2H.sign = 1.0;

  nbPolyH = IntersectPoly(&p1,&p2H,vH,&resultH);

  // Test I
  p2I.nbPts = 14;
  p2I.pts   = (VERTEX2D *)malloc( p2I.nbPts* sizeof(VERTEX2D) );
  int vI[] = {1,1,1,0,1,1,1,1,1,1,1,1,0,1};
  p2I.pts[0].u = 40.0;
  p2I.pts[0].v = 40.0;
  p2I.pts[1].u = 160.0;
  p2I.pts[1].v = 40.0;
  p2I.pts[2].u = 160.0;
  p2I.pts[2].v = 160.0;
  p2I.pts[3].u = 40.0;
  p2I.pts[3].v = 160.0;
  p2I.pts[4].u = 75.0;
  p2I.pts[4].v = 140.0;
  p2I.pts[5].u = 155.0;
  p2I.pts[5].v = 140.0;
  p2I.pts[6].u = 155.0;
  p2I.pts[6].v = 130.0;
  p2I.pts[7].u = 140.0;
  p2I.pts[7].v = 130.0;
  p2I.pts[8].u = 140.0;
  p2I.pts[8].v = 120.0;
  p2I.pts[9].u  = 155.0;
  p2I.pts[9].v  = 120.0;
  p2I.pts[10].u = 155.0;
  p2I.pts[10].v = 80.0;
  p2I.pts[11].u = 75.0;
  p2I.pts[11].v = 80.0;
  p2I.pts[12].u = 75.0;
  p2I.pts[12].v = 140.0;
  p2I.pts[13].u = 40.0;
  p2I.pts[13].v = 160.0;
  p2I.sign = 1.0;

  nbPolyI = IntersectPoly(&p1,&p2I,vI,&resultI);

  // Test J
  p2J.nbPts = 10;
  p2J.pts   = (VERTEX2D *)malloc( p2J.nbPts* sizeof(VERTEX2D) );
  p2J.pts[0].u = 40.0;
  p2J.pts[0].v = 40.0;
  p2J.pts[1].u = 160.0;
  p2J.pts[1].v = 40.0;
  p2J.pts[2].u = 160.0;
  p2J.pts[2].v = 160.0;
  p2J.pts[3].u = 40.0;
  p2J.pts[3].v = 160.0;
  p2J.pts[4].u = 45.0;
  p2J.pts[4].v = 155.0;
  p2J.pts[5].u = 155.0;
  p2J.pts[5].v = 155.0;
  p2J.pts[6].u = 155.0;
  p2J.pts[6].v = 45.0;
  p2J.pts[7].u = 45.0;
  p2J.pts[7].v = 45.0;
  p2J.pts[8].u = 45.0;
  p2J.pts[8].v = 155.0;
  p2J.pts[9].u = 40.0;
  p2J.pts[9].v = 160.0;
  p2J.sign = 1.0;

  nbPolyJ = IntersectPoly(&p2J,&p1,NULL,&resultJ);

}

// ----------------------------------------------

void DrawingArea::DrawPoly(POLYGON *p,double tx,double ty) {

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
  glBegin(GL_LINES);
  for(int i=0;i<p->nbPts;i++) {
    int i1 = IDX(i  ,p->nbPts);
    int i2 = IDX(i+1,p->nbPts);
    glVertex2d(p->pts[i1].u+tx,p->pts[i1].v+ty);
    glVertex2d(p->pts[i2].u+tx,p->pts[i2].v+ty);
  }
  glEnd();

  /*
  for(int i=0;i<p->nbPts;i++) {
    char tmp[32];
    sprintf(tmp,"%d",i);
    GLToolkit::GetDialogFont()->DrawText(p->pts[i].u+tx+5,p->pts[i].v+ty+5,tmp,FALSE);
  }
  */

}

// ----------------------------------------------

void DrawingArea::DrawTest(POLYGON *p,int nb,POLYGON *r,int tx,int ty) {

  glColor3f(1.0f,0.0f,0.0f);
  DrawPoly(&p1,tx,ty);
  glColor3f(0.0f,1.0f,0.0f);
  DrawPoly(p,tx,ty);

  glColor3f(0.0f,0.0f,1.0f);
  for(int i=0;i<nb;i++) 
    DrawPoly(r+i,tx,ty);

  char tmp[256];
  char tmp2[256];
  sprintf(tmp,"Nb = %d  (",nb);
  for(int i=0;i<nb;i++) {
    sprintf(tmp2,"%d ",r[i].nbPts);
    strcat(tmp,tmp2);
  }
  strcat(tmp,")");
  GLToolkit::GetDialogFont()->SetTextColor(1.0f,1.0f,0.0f);
  GLToolkit::GetDialogFont()->DrawText(tx+50,ty+190,tmp,FALSE);

}

// ----------------------------------------------

void DrawingArea::Paint() {

  GLComponent::Paint();
  DrawTest(&p21,nbPoly1,result1,0,-20);
  DrawTest(&p22,nbPoly2,result2,220,-20);
  DrawTest(&p23,nbPoly3,result3,440,-20);
  DrawTest(&p24,nbPoly4,result4,660,-20);
  DrawTest(&p25,nbPoly5,result5,860,-20);
  DrawTest(&p26,nbPoly6,result6,0,180);
  DrawTest(&p27,nbPoly7,result7,220,180);
  DrawTest(&p28,nbPoly8,result8,440,180);
  DrawTest(&p29,nbPoly9,result9,660,180);
  DrawTest(&p2A,nbPolyA,resultA,850,180);
  DrawTest(&p2B,nbPolyB,resultB,0,380);
  DrawTest(&p2C,nbPolyC,resultC,220,380);
  DrawTest(&p2D,nbPolyD,resultD,440,380);
  DrawTest(&p2E,nbPolyE,resultE,660,380);
  DrawTest(&p2F,nbPolyF,resultF,860,380);
  DrawTest(&p2G,nbPolyG,resultG,0,560);
  DrawTest(&p2H,nbPolyH,resultH,220,560);
  DrawTest(&p2I,nbPolyI,resultI,440,560);
  DrawTest(&p2J,nbPolyJ,resultJ,660,560);
  
  /*
  int pointX[] = {10,300,250,20};
  int pointY[] = {10, 20,200,280};
  GLToolkit::DrawPoly(2,DASHSTYLE_DASH_DOT,255,255,0,4,pointX,pointY);
  */

}
