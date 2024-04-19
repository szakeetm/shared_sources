
#pragma once

//#include "TruncatedGaussian/rtnorm.hpp"

/* Maximum generated random value */
#define RK_STATE_LEN 624
#define RK_MAX 0xFFFFFFFFUL
/* Magic Mersenne Twister constants */
#define MERSENNE_N 624
#define MERSENNE_M 397
#define MATRIX_A 0x9908b0dfUL
#define UPPER_MASK 0x80000000UL
#define LOWER_MASK 0x7fffffffUL

#ifdef WIN
// Disable "unary minus operator applied to unsigned type, result still unsigned" warning.
#pragma warning(disable : 4146)
#endif

unsigned long GenerateSeed(size_t offsetIndex);

class MersenneTwister {

/* State of the RNG */
    struct rk_state
    {
        unsigned long key[RK_STATE_LEN];
        int pos;
    };
    unsigned long seed;

public:
    MersenneTwister();
    void   SetSeed(unsigned long seed); // Initialise the random generator with the specified seed
    double rnd(); // Returns a uniform distributed double value in the interval ]0,1[

    double Gaussian(const double sigma);

    unsigned long GetSeed();

private:
    rk_state localState;

    /* Slightly optimised reference implementation of the Mersenne Twister */
    unsigned long rk_random();

    double rk_double();
};