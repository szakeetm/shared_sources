#include <stdlib.h>
#include "Random.h"
#include "Helper/MathTools.h"
#include <omp.h>

#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>

#endif // def WIN

#define  RK_STATE_LEN 624

#ifdef _WIN32

#else
#define __forceinline __attribute__((always_inline))
#endif

void MersenneTwister::SetSeed(unsigned long seed)
{
    this->seed = seed;
    seed &= 0xffffffffUL;

    /* Knuth's PRNG as used in the Mersenne Twister reference implementation */
    for (int pos=0; pos<RK_STATE_LEN; pos++)
    {
        localState.key[pos] = seed;
        seed = (1812433253UL * (seed ^ (seed >> 30)) + pos + 1) & 0xffffffffUL;
    }

    localState.pos = RK_STATE_LEN;
}

// Initialise the random generator with the specified seed

double MersenneTwister::rnd() { return rk_double(); }

double MersenneTwister::Gaussian(const double  sigma) //inline
{

    //Box-Muller transform
    //return sigma*sqrt(-2 * log(rnd()))*cos(2 * PI*rnd());

    //Generates a random number following the Gaussian distribution around 0 with 'sigma' standard deviation
    double v1, v2, r, fac;
    do {
        v1 = 2.0*rnd() - 1.0;
        v2 = 2.0*rnd() - 1.0;
        r = v1 * v1 + v2 * v2;
    } while (r >= 1.0);
    fac = sqrt(-2.0*log(r) / r);
    return v2 * fac*sigma;
}

unsigned long MersenneTwister::GetSeed() {
    return seed;
}

unsigned long GenerateSeed(size_t offsetIndex) {
    double time = omp_get_wtime();
    size_t ms = *(reinterpret_cast<size_t *>(&time)); // just use the bits for hashing
    int processId;
#ifdef _WIN32
    processId = _getpid();
#else
    processId = ::getpid();
#endif //  WIN
    return static_cast<unsigned long>(std::hash<size_t>()(ms*(std::hash<size_t>()(processId+offsetIndex))));
}

/* Slightly optimised reference implementation of the Mersenne Twister */

unsigned long MersenneTwister::rk_random() //inline
{
    unsigned long y;

    if (localState.pos == RK_STATE_LEN)
    {
        int i;

        for (i = 0; i < MERSENNE_N - MERSENNE_M; i++)
        {
            y = (localState.key[i] & UPPER_MASK) | (localState.key[i + 1] & LOWER_MASK);
            localState.key[i] = localState.key[i + MERSENNE_M] ^ (y >> 1) ^ (-(y & 1) & MATRIX_A);
        }
        for (; i < MERSENNE_N - 1; i++)
        {
            y = (localState.key[i] & UPPER_MASK) | (localState.key[i + 1] & LOWER_MASK);
            localState.key[i] = localState.key[i + (MERSENNE_M - MERSENNE_N)] ^ (y >> 1) ^ (-(y & 1) & MATRIX_A);
        }
        y = (localState.key[MERSENNE_N - 1] & UPPER_MASK) | (localState.key[0] & LOWER_MASK);
        localState.key[MERSENNE_N - 1] = localState.key[MERSENNE_M - 1] ^ (y >> 1) ^ (-(y & 1) & MATRIX_A);

        localState.pos = 0;
    }

    y = localState.key[localState.pos++];

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}

double MersenneTwister::rk_double() //inline
{
    /* shifts : 67108864 = 0x4000000, 9007199254740992 = 0x20000000000000 */
    long a = rk_random() >> 5, b = rk_random() >> 6;
    return (a * 67108864.0 + b) / 9007199254740992.0;
}

MersenneTwister::MersenneTwister() {
#if defined(DEBUG)
    SetSeed(42424242);
#else
    SetSeed(GenerateSeed(0));
#endif
}

/*
double TruncatedGaussian::GetGaussian(const double  mean, const double  sigma, const double  lowerBound, const double  upperBound) //inline
{
    std::pair<double, double> s;  // Output argument of rtnorm
    s = rtnorm(this->gen, lowerBound, upperBound, mean, sigma);
    return s.first;
}
*/