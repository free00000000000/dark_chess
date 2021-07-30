// C/C++ 
// pcg32_random_r: randam uint32_t type number
// bounded_rand: randam range uint32_t type number, [0, range)

#include <stdint.h>

typedef struct { uint64_t state;  uint64_t inc; } pcg32_random_t;

pcg32_random_t rng;

uint32_t pcg32_random_r()
{
    uint64_t oldstate = rng.state;
    // Advance internal state
    rng.state = oldstate * 6364136223846793005ULL + (rng.inc|1);
    // Calculate output function (XSH RR), uses old state for max ILP
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

uint32_t bounded_rand(uint32_t range) {
    uint32_t x = pcg32_random_r();
    uint64_t m = uint64_t(x) * uint64_t(range);
    return m >> 32;
}