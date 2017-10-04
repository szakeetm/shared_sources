#pragma once
#include <vector>

bool    IsEqual(const double &a, const double &b, double toleranceRatio=1E-6);
double RoundAngle(double a);
size_t    GetPower2(size_t n);
double Pow10(const double& a);

template <typename TYPE> TYPE Min(const TYPE& x, const TYPE& y) { return (x < y) ? x : y; }
template <typename TYPE> TYPE Max(const TYPE& x, const TYPE& y) { return (x < y) ? y : x; }
template <typename T1, typename T2, typename T3> void Saturate(T1& x, const T2& min, const T3& max) { if (x<min) x = min; if (x>max) x = max; }
size_t  IDX(int i, size_t nb);
size_t IDX(size_t i, size_t nb);
size_t Next(int i, size_t nb);
size_t Next(size_t i, size_t nb);
size_t Previous(int i, size_t nb);
size_t Previous(size_t i, size_t nb);
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
double InterpolateY(const double& x, const std::vector<std::pair<double, double>>& table, const bool& logarithmic = false, const bool& allowExtrapolate = false );
double InterpolateX(const double& y, const std::vector<std::pair<double, double>>& table, const bool& logarithmic = false, const bool& allowExtrapolate = false);
double InterpolateXY(const double& lookupValue, const std::vector<std::pair<double, double>>& table, const bool& first, const bool& logarithmic, const bool& allowExtrapolate);
std::vector<double> InterpolateVectorY(const double& x, const std::vector<std::pair<double, std::vector<double>>>& table, const bool& logarithmic = false, const bool& allowExtrapolate = false );
double InterpolateVectorX(const double& y, const std::vector<std::pair<double, std::vector<double>>>& table, const size_t& elementIndex, const bool& logarithmic = false, const bool& allowExtrapolate = false);
double QuadraticInterpolateX(const double & y, const double & a, const double & b, const double & c, const double & FA, const double & FB, const double & FC);
//double FastLookupY(const double& x, const std::vector<std::pair<double, double>>& table, const bool& allowExtrapolate = false);

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

int my_lower_bound(const double& key, double* A,const size_t& size);
int my_lower_bound(const double& key, const std::vector<double>& A);
int my_lower_bound(const double& key, const std::vector<std::pair<double, double>>& A, const bool& first);
int my_lower_bound(const double& key, const std::vector<std::pair<double, std::vector<double>>>& A, const bool& first, const size_t& elementIndex);



int weighed_lower_bound_X(const double& key, const double& weigh, double* A, double* B, const size_t& size);

double GetElement(const std::pair<double, double>& pair, const bool& first);
double GetElement(const std::pair<double, std::vector<double>> & pair, const bool& first, const size_t& elementIndex);