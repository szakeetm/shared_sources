#pragma once
#include <vector>

bool    IsEqual(const double &a, const double &b, double toleranceRatio=1E-6);
double RoundAngle(double a);
size_t    GetPower2(size_t n);

template <typename TYPE> TYPE Min(const TYPE& x, const TYPE& y) { return (x < y) ? x : y; }
template <typename TYPE> TYPE Max(const TYPE& x, const TYPE& y) { return (x < y) ? y : x; }
template <typename T1, typename T2, typename T3> void Saturate(T1& x, const T2& min, const T3& max) { if (x<min) x = min; if (x>max) x = max; }
size_t  IDX(int i, size_t nb);
size_t IDX(size_t i, size_t nb);
#define NEXT_OF(list,element) (std::next(element)==(list).end())?(list).begin():std::next(element);

template <typename TYPE> bool IsZero(const TYPE& x) { return fabs(x)<1E-10; }
template <typename TYPE> TYPE Sqr(const TYPE& a) { return a*a; }
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
double Weigh(const double& a, const double& b, const double& weigh);
double InterpolateY(const double& x, const std::vector<std::pair<double, double>>& table, const bool& limitToBounds = false, const bool& logarithmic = false);
std::vector<double> InterpolateVector(const double& x, const std::vector<std::pair<double, std::vector<double>>>& table, const bool& limitToBounds = false, const bool& logarithmic = false);
double InterpolateX(const double& y, const std::vector<std::pair<double, double>>& table, const bool& limitToBounds = false);
double FastLookupY(const double& x, const std::vector<std::pair<double, double>>& table, const bool& limitToBounds = false);

template <typename TYPE> bool Contains(std::vector<TYPE> vec, const TYPE& value) {
	return (std::find(vec.begin(), vec.end(), value) != vec.end());
}

template <typename TYPE> size_t FirstIndex(std::vector<TYPE> vec, const TYPE& value) {
	return (std::find(vec.begin(), vec.end(), value) - vec.begin());
}

std::vector<std::string> SplitString(std::string const &input);
std::vector<std::string> SplitString(std::string const &input,const char &delimiter);

bool endsWith(std::string const & fullString, std::string const & ending);

bool beginsWith(std::string const & fullString, std::string const & ending);

double InverseQuadraticInterpolation(const double & y, const double & a, const double & b, const double & c, const double & FA, const double & FB, const double & FC);

template <class K, class T> int my_lower_bound(const K& key, T* A, const size_t& size)
//"iterative" version of algorithm, modified from https://en.wikipedia.org/wiki/Binary_search_algorithm
//key: searched value
//A: ordered arrray of lookup values
//size: array length
//returns index of last lower value, or -1 if key not found

{
	if (size == 0) return -1;
	if (size == 1) {
		if (key < A[0]) return -1;
		else return 0;
	}
	int L = 0;
	int R = (int)(size - 1);
	int M;
	// continue searching while [imin,imax] is not empty
	while (L <= R)
	{
		M = (L + R) / 2;
		if (A[M] < key && key < A[M+1]) { 
			// key found at index M
			return M;
		}
		else if (A[M] < key) {
			L = M + 1;
		}
		else {
			R = M - 1;
		}
	}
	//Not found
	if (M == 0)
		return -1; //key lower than first element
	else
		return (int) size - 1; //key larger than last element
}

int weighed_lower_bound_X(const double& key, const double& weigh, double* A, double* B, const size_t& size);

template <class K, class T> int my_binary_search(const K& key, T* A, const size_t& size)
//"iterative" version of algorithm, modified from https://en.wikipedia.org/wiki/Binary_search_algorithm
//key: searched value
//A: ordered arrray of lookup values
//size: array length
//returns index of last lower value, or -1 if key not found

{
	int imin = 0;
	int imax = (int)size - 1;
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