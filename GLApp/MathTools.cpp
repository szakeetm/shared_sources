#include "MathTools.h"
#include "GLTypes.h" //bool
#include <math.h>
#include <cstdio>
#include <algorithm> //std::Lower_bound
#include <sstream>
#include <iterator>

bool IsEqual(const double &a, const double &b, double toleranceRatio) {
	return fabs(a - b) < (a*toleranceRatio);
}

size_t  IDX(int i, size_t nb) {
	return (i < 0) ? (nb + i) : (i%nb);
}

size_t IDX(size_t i, size_t nb) {
	return i%nb;
}

size_t GetPower2(size_t n) {
// Return a power of 2 which is greater or equal than n
  if((n & (n-1))==0) {
    // already a power of 2
    return n;
  } else {
    // Get the power of 2 above
    int p = 0;
    while(n!=0) { n = n >> 1; p++; }
    return 1i64 << p;
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

double my_erf(double x)
{
	// constants
	double a1 = 0.254829592;
	double a2 = -0.284496736;
	double a3 = 1.421413741;
	double a4 = -1.453152027;
	double a5 = 1.061405429;
	double p = 0.3275911;

	// Save the sign of x
	int sign = 1;
	if (x < 0)
		sign = -1;
	x = fabs(x);

	// A&S formula 7.1.26
	double t = 1.0 / (1.0 + p*x);
	double y = 1.0 - (((((a5*t + a4)*t) + a3)*t + a2)*t + a1)*t*exp(-x*x);

	return sign*y;
}

bool compare_second(const std::pair<double, double>& lhs, const std::pair<double, double>& rhs) {
	return (lhs.second<rhs.second);
}

bool compare_first(const std::pair<double, std::vector<double>>& lhs,const double& rhs) {
	return (lhs.first<rhs);
}

double InterpolateY(const double& x, const std::vector<std::pair<double, double>>& table, const bool& limitToBounds, const bool& logarithmic) {
	//Function inspired by http://stackoverflow.com/questions/11396860/better-way-than-if-else-if-else-for-linear-interpolation
	_ASSERTE(table.size());
	if (table.size() == 1) return table[0].second; //constant value
												   // Assumes that "table" is sorted by .first
												   // Check if x is out of bound
	std::vector<std::pair<double, double> >::const_iterator lower, upper;
	bool outOfLimits = false;

	if (x >= table.back().first) {
		if (limitToBounds) return table.back().second;
		else {
			outOfLimits = true;
			lower = upper = table.end() - 1;
			lower--;
		}
	}
	else if (x < table[0].first) {
		if (limitToBounds) return table[0].second;
		else {
			outOfLimits = true;
			lower = upper = table.begin();
			upper++;
		}
	}

	// INFINITY is defined in math.h in the glibc implementation
	if (!outOfLimits) {
		lower = upper = std::lower_bound(table.begin(), table.end(), std::make_pair(x, -MY_INFINITY));
		// Corner case
		if (upper == table.begin()) return upper->second;
		lower--;
	}
	if (logarithmic) return exp(log(lower->second) + (log(upper->second) - log(lower->second))
		*(log(x) - log(lower->first)) / (log(upper->first) - log(lower->first)));
	else return lower->second + (upper->second - lower->second)*(x - lower->first) / (upper->first - lower->first);

}

std::vector<double> InterpolateVector(const double& x, const std::vector<std::pair<double, std::vector<double>>>& table, const bool& limitToBounds, const bool& logarithmic) {
	//Function inspired by http://stackoverflow.com/questions/11396860/better-way-than-if-else-if-else-for-linear-interpolation
	_ASSERTE(table.size());
	if (table.size() == 1) return table[0].second; //constant value
												   // Assumes that "table" is sorted by .first
												   // Check if x is out of bound
	std::vector<std::pair<double, std::vector<double>> >::const_iterator lower, upper;
	bool outOfLimits = false;

	if (x >= table.back().first) {
		if (limitToBounds) return table.back().second;
		else {
			outOfLimits = true;
			lower = upper = table.end() - 1;
			lower--;
		}
	}
	else if (x < table[0].first) {
		if (limitToBounds) return table[0].second;
		else {
			outOfLimits = true;
			lower = upper = table.begin();
			upper++;
		}
	}

	// INFINITY is defined in math.h in the glibc implementation
	if (!outOfLimits) {
		lower = upper = std::lower_bound(table.begin(), table.end(), x , compare_first);
		// Corner case
		if (upper == table.begin()) return upper->second;
		lower--;
	}

	if (logarithmic) {
		double overShoot = (log(x) - log(lower->first)) / (log(upper->first) - log(lower->first));
		std::vector<double> returnValues;
		for (size_t i = 0; i < lower->second.size(); i++) {
			returnValues.push_back(exp(WEIGH(log(lower->second[i]), log(upper->second[i]), overShoot)));
		}
		return returnValues;
	}
	else {
		double overShoot = (x - lower->first) / (upper->first - lower->first);
		std::vector<double> returnValues;
		for (size_t i = 0; i < lower->second.size(); i++) {
			returnValues.push_back(WEIGH(lower->second[i], upper->second[i], overShoot));
		}
		return returnValues;
	}
}

double InterpolateX(const double& y, const std::vector<std::pair<double, double>>& table, const bool& limitToBounds) {
	//Function inspired by http://stackoverflow.com/questions/11396860/better-way-than-if-else-if-else-for-linear-interpolation
	_ASSERTE(table.size());
	if (table.size() == 1) return table[0].second; //constant value

												   // Assumes that "table" is sorted by .second
												   // Check if y is out of bound
	std::vector<std::pair<double, double> >::const_iterator lower, upper;
	bool outOfLimits = false;

	if (y >= table.back().second) {
		if (limitToBounds) return table.back().first;
		else {
			outOfLimits = true;
			lower = upper = table.end() - 1;
			lower--;
		}
	}
	else if (y < table[0].second) {
		if (limitToBounds) return table[0].first;
		else {
			outOfLimits = true;
			lower = upper = table.begin();
			upper++;
		}
	}

	// INFINITY is defined in math.h in the glibc implementation
	if (!outOfLimits) {
		lower = upper = std::lower_bound(table.begin(), table.end(), std::make_pair(MY_INFINITY, y), compare_second);
		// Corner case
		if (upper == table.begin()) return upper->first;
		lower--;
	}
	return lower->first + (upper->first - lower->first)*(y - lower->second) / (upper->second - lower->second);
}

double FastLookupY(const double& x, const std::vector<std::pair<double, double>>& table, const bool& limitToBounds) {
	//Function inspired by http://stackoverflow.com/questions/11396860/better-way-than-if-else-if-else-for-linear-interpolation
	_ASSERTE(table.size());
	if (table.size() == 1) return table[0].second; //constant value

												   // Assumes that table .first is SORTED AND EQUIDISTANT
												   // Check if x is out of bound
	std::vector<std::pair<double, double> >::const_iterator lower, upper;
	bool outOfLimits = false;

	if (x >= table.back().first) {
		if (limitToBounds) return table.back().second;
		else {
			outOfLimits = true;
			lower = upper = table.end() - 1;
			lower--;
		}
	}
	else if (x < table[0].first) {
		if (limitToBounds) return table[0].second;
		else {
			outOfLimits = true;
			lower = upper = table.begin();
			upper++;
		}
	}

	if (!outOfLimits) {
		double distanceX = table[1].first - table[0].first;
		size_t lowerIndex = (int)((x - table[0].first) / distanceX);
		lower = upper = table.begin() + (lowerIndex + 1);
		// Corner case
		if (upper == table.begin()) return upper->second;
		lower--;
	}
	double result = lower->second + (upper->second - lower->second)*(x - lower->first) / (upper->first - lower->first);
	return result;
}

std::vector<std::string> SplitString(std::string const &input) {
	//Split string by whitespaces
	std::istringstream buffer(input);
	std::vector<std::string> ret;

	std::copy(std::istream_iterator<std::string>(buffer),
		std::istream_iterator<std::string>(),
		std::back_inserter(ret));
	return ret;
}

std::vector<std::string> SplitString(std::string const & input, const char & delimiter)
{
		std::vector<std::string> result;
		const char* str = _strdup(input.c_str());
		do
		{
			const char *begin = str;
			while (*str != delimiter && *str)
				str++;

			result.push_back(std::string(begin, str));
		} while (0 != *str++);
		return result;
}

bool endsWith(std::string const &fullString, std::string const &ending) {
	if (fullString.length() >= ending.length()) {
		return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
	}
	else {
		return false;
	}
}

bool beginsWith(std::string const &fullString, std::string const &ending) {
	return (fullString.compare(0, ending.length(), ending) == 0);
}