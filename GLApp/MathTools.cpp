#include "MathTools.h"
#include <math.h>
#include <cstdio>

int IsEqual(const double &a, const double &b, double tolerance) {
	return fabs(a - b) < tolerance;
}

int GetPower2(int n) {
// Return a power of 2 which is greater or equal than n
  if((n & (n-1))==0) {
    // already a power of 2
    return n;
  } else {
    // Get the power of 2 above
    int p = 0;
    while(n!=0) { n = n >> 1; p++; }
    return 1 << p;
  }

}

double RoundAngle(double a) {
// Return a in [-PI,PI]
  double r=a;
  while(r<-PI) r+=2.0*PI;
  while(r> PI) r-=2.0*PI;
  return r;

}

char* FormatMemory(size_t size) {
	return FormatMemoryLL((long long)size);
}

char* FormatMemoryLL(long long size) {

	static char ret[256];
	const char *suffixStr[] = { "KB", "MB", "GB", "TB", "PB" };
	double dSize = (double)size;
	int suffix = 0;

	while (dSize >= 1024.0 && suffix < 4) {
		dSize /= 1024.0;
		suffix++;
	}

	if (suffix == 0) {
		sprintf(ret, "%u bytes", (unsigned int)size);
	}
	else {
		if (fabs(dSize - floor(dSize)) < 1e-3)
			sprintf(ret, "%.0f%s", dSize, suffixStr[suffix - 1]);
		else
			sprintf(ret, "%.2f%s", dSize, suffixStr[suffix - 1]);
	}
	return ret;

}