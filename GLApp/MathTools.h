#pragma once

int    IsEqual(const double &a, const double &b, double tolerance=1E-8);
double RoundAngle(double a);
int    GetPower2(int n);
#define MAX(x,y) (((x)<(y))?(y):(x))
#define MIN(x,y) (((x)<(y))?(x):(y))
#define SATURATE(x,min,max) {if(x<(min)) x=(min); if(x>(max)) x=(max);}
#define IDX(i,nb) (((i)<0)?nb+(i):(i)%(nb))
#define NEXT_OF(list,elementIterator) (std::next(element)==list.end())?list.begin():std::next(element);
#define WEIGH(a,b,weigh) a+(b-a)*weigh
#define IS_ZERO(x) (fabs((x))<1e-10)
#define Sqr(a) a*a
#define PI 3.14159265358979323846
#define DET22(_11,_12,_21,_22) ( (_11)*(_22) - (_21)*(_12) )
#define DET33(_11,_12,_13,_21,_22,_23,_31,_32,_33)  \
  ((_11)*( (_22)*(_33) - (_32)*(_23) ) +            \
   (_12)*( (_23)*(_31) - (_33)*(_21) ) +            \
   (_13)*( (_21)*(_32) - (_31)*(_22) ))

char  *FormatMemory(size_t size);
char  *FormatMemoryLL(long long size);