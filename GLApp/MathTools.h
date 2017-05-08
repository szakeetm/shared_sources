#pragma once
#include <vector>
#include "GLTypes.h" //BOOL, TRUE, FALSE

BOOL    IsEqual(const double &a, const double &b, double tolerance=1E-8);
double RoundAngle(double a);
int    GetPower2(int n);
#define MAX(x,y) (((x)<(y))?(y):(x))
#define MIN(x,y) (((x)<(y))?(x):(y))
#define SATURATE(x,min,max) {if((x)<(min)) x=(min); if(x>(max)) x=(max);}
#define IDX(i,nb) (((i)<0)?(nb)+(i):(i)%(nb))
#define NEXT_OF(list,elementIterator) (std::next(element)==list.end())?list.begin():std::next(element);
#define WEIGH(a,b,weigh) a + (b-a)*weigh
#define IS_ZERO(x) (fabs((x))<1e-10)
#define Sqr(a) (a)*(a)
#define PI 3.14159265358979323846
#define DET22(_11,_12,_21,_22) ( (_11)*(_22) - (_21)*(_12) )
#define DET33(_11,_12,_13,_21,_22,_23,_31,_32,_33)  \
  ((_11)*( (_22)*(_33) - (_32)*(_23) ) +            \
   (_12)*( (_23)*(_31) - (_33)*(_21) ) +            \
   (_13)*( (_21)*(_32) - (_31)*(_22) ))
#define VERY_SMALL 1.0E-30
#define MY_INFINITY 1.e100

char  *FormatMemory(size_t size);
char  *FormatMemoryLL(long long size);


double my_erf(double x);
double InterpolateY(const double& x, const std::vector<std::pair<double, double>>& table, const BOOL& limitToBounds = FALSE, const BOOL& logarithmic = FALSE);
std::vector<double> InterpolateVector(const double& x, const std::vector<std::pair<double, std::vector<double>>>& table, const BOOL& limitToBounds = FALSE, const BOOL& logarithmic = FALSE);
double InterpolateX(const double& y, const std::vector<std::pair<double, double>>& table, const BOOL& limitToBounds = FALSE);
double FastLookupY(const double& x, const std::vector<std::pair<double, double>>& table, const BOOL& limitToBounds = FALSE);

template <typename TYPE> BOOL Contains(std::vector<TYPE> vec, const TYPE& value) {
	return (std::find(vec.begin(), vec.end(), value) != vec.end());
}

std::vector<std::string> SplitString(std::string const &input);

template <class K, class T> int my_lower_bound(const K& key, T* A, const size_t& size)
//"iterative" version of algorithm, modified from https://en.wikipedia.org/wiki/Binary_search_algorithm
//key: searched value
//A: ordered arrray of lookup values
//size: array length
//returns index of last lower value, or -1 if key not found

{
	int L = 0;
	int R = size - 1;
	// continue searching while [imin,imax] is not empty
	while (true)
	{
		if (L > R) return -1; //key not found
		int M = (L + R) / 2;
		if (A[M] < key) {
			L = M + 1;
		}
		else if (A[M] > key) {
			R = M - 1;
		}
		if (((M>=0) && (A[M] <= key)) && (((M+1)<size) && (A[M + 1] > key))) return M;
	}
}

template <class K, class T> int my_binary_search(const K& key, T* A, const size_t& size)
//"iterative" version of algorithm, modified from https://en.wikipedia.org/wiki/Binary_search_algorithm
//key: searched value
//A: ordered arrray of lookup values
//size: array length
//returns index of last lower value, or -1 if key not found

{
	int imin = 0;
	int imax = size - 1;
	// continue searching while [imin,imax] is not empty
	while (imin <= imax)
	{
		// calculate the midpoint for roughly equal partition
		int imid = (imin + imax) / 2;
		if (imid == size - 1 || imid == 0 || (A[imid] <= key && key < A[imid + 1])) {
			// key found at index imid
			return imid;
		}
		// determine which subarray to search
		else if (A[imid] < key) {
			// change min index to search upper subarray
			imin = imid + 1;
		}
		else
		{
			// change max index to search lower subarray
			imax = imid - 1;
		}
	}
	// key was not found
	return -1;
}

template <class T> int my_binary_search(const double& key, std::vector<T> A, const size_t& size) {
	return my_binary_search(key, &(A[0]), size);
}