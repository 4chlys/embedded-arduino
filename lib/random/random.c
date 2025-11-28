/**
 * @file randomlib.c
 * @brief Implementation of the random number generation library
 */

 #include "random.h"
 #include <stdlib.h>
 #include <stdbool.h>
 #include <string.h>
 #include <time.h>
 #include <math.h>
 
 /* Global random state instance */
 random_state_t global_random_state = {{0, 0, 0, 0}, false};
 
 /**
  * SplitMix64 algorithm for seed expansion
  * Used to initialize xoshiro256** state from a single seed
  */
 static uint64_t splitmix64(uint64_t *x) {
     uint64_t z = (*x += 0x9E3779B97F4A7C15);
     z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9;
     z = (z ^ (z >> 27)) * 0x94D049BB133111EB;
     return z ^ (z >> 31);
 }
 
 void random_init(random_state_t *state) {
     uint64_t seed = (uint64_t)time(NULL);
     random_init_with_seed(state, seed);
 }
 
 void random_init_with_seed(random_state_t *state, uint64_t seed) {
     uint64_t s = seed;
     state->state[0] = splitmix64(&s);
     state->state[1] = splitmix64(&s);
     state->state[2] = splitmix64(&s);
     state->state[3] = splitmix64(&s);
     state->is_seeded = true;
 }
 
 void random_init_with_seeds(random_state_t *state, uint64_t seed1, uint64_t seed2, 
                            uint64_t seed3, uint64_t seed4) {
     state->state[0] = seed1;
     state->state[1] = seed2;
     state->state[2] = seed3;
     state->state[3] = seed4;
     state->is_seeded = true;
 }
 
 /* xoshiro256** algorithm - one of the fastest modern PRNGs with good statistical properties */
 static inline uint64_t rotl(const uint64_t x, int k) {
     return (x << k) | (x >> (64 - k));
 }
 
 uint64_t random_uint64(random_state_t *state) {
     if (!state->is_seeded) {
         random_init(state);
     }
     
     const uint64_t result = rotl(state->state[1] * 5, 7) * 9;
     const uint64_t t = state->state[1] << 17;
     
     state->state[2] ^= state->state[0];
     state->state[3] ^= state->state[1];
     state->state[1] ^= state->state[2];
     state->state[0] ^= state->state[3];
     
     state->state[2] ^= t;
     state->state[3] = rotl(state->state[3], 45);
     
     return result;
 }
 
 uint32_t random_uint32(random_state_t *state) {
     return (uint32_t)(random_uint64(state) >> 32);
 }
 
 int random_int_range(random_state_t *state, int min, int max) {
     if (min > max) {
         int temp = min;
         min = max;
         max = temp;
     }
     
     uint64_t range = (uint64_t)(max - min) + 1;
     uint64_t scale = 0xFFFFFFFFFFFFFFFFULL / range;
     uint64_t limit = range * scale;
     uint64_t r;
     
     do {
         r = random_uint64(state);
     } while (r >= limit);
     
     return min + (int)(r / scale);
 }
 
 float random_float(random_state_t *state) {
     return (random_uint32(state) >> 8) / 16777216.0f;
 }
 
 double random_double(random_state_t *state) {
     return (random_uint64(state) >> 11) / 9007199254740992.0;
 }
 
 float random_float_range(random_state_t *state, float min, float max) {
     return min + random_float(state) * (max - min);
 }
 
 double random_double_range(random_state_t *state, double min, double max) {
     return min + random_double(state) * (max - min);
 }
 
 bool random_bool(random_state_t *state) {
     return random_uint64(state) & 1;
 }
 
 bool random_bool_prob(random_state_t *state, double probability) {
     if (probability <= 0.0) return false;
     if (probability >= 1.0) return true;
     return random_double(state) < probability;
 }
 
 /* Box-Muller transform for normal distribution */
 static double normal_distribution(random_state_t *state, double mean, double stddev) {
     static int has_spare = 0;
     static double spare;
     double u, v, s, norm;
     
     if (has_spare) {
         has_spare = 0;
         return mean + stddev * spare;
     }
     
     do {
         u = random_double(state) * 2.0 - 1.0;
         v = random_double(state) * 2.0 - 1.0;
         s = u * u + v * v;
     } while (s >= 1.0 || s == 0.0);
     
     s = sqrt(-2.0 * log(s) / s);
     spare = v * s;
     has_spare = 1;
     
     return mean + stddev * u * s;
 }
 
 /* Exponential distribution using inverse transform sampling */
 static double exponential_distribution(random_state_t *state, double lambda) {
     double u;
     do {
         u = random_double(state);
     } while (u == 0.0);
     
     return -log(u) / lambda;
 }
 
 /* Poisson distribution using Knuth's algorithm */
 static double poisson_distribution(random_state_t *state, double lambda) {
     double L = exp(-lambda);
     double p = 1.0;
     int k = 0;
     
     do {
         k++;
         p *= random_double(state);
     } while (p > L);
     
     return k - 1;
 }
 
 double random_distribution(random_state_t *state, random_distribution_t dist, 
                          double param1, double param2) {
     switch (dist) {
         case RANDOM_UNIFORM:
             return random_double_range(state, param1, param2);
         case RANDOM_NORMAL:
             return normal_distribution(state, param1, param2);
         case RANDOM_EXPONENTIAL:
             return exponential_distribution(state, param1);
         case RANDOM_POISSON:
             return poisson_distribution(state, param1);
         default:
             return random_double(state);
     }
 }
 
 void random_bytes(random_state_t *state, void *buffer, size_t size) {
     unsigned char *buf = (unsigned char *)buffer;
     size_t generated = 0;
     uint64_t r;
     
     while (generated + 8 <= size) {
         r = random_uint64(state);
         memcpy(buf + generated, &r, 8);
         generated += 8;
     }
     
     if (generated < size) {
         r = random_uint64(state);
         memcpy(buf + generated, &r, size - generated);
     }
 }
 
 /* Fisher-Yates shuffle algorithm */
 void random_shuffle(random_state_t *state, void *array, size_t element_size, size_t array_length) {
     unsigned char *base = (unsigned char *)array;
     unsigned char *temp = malloc(element_size);
     
     if (!temp) return;
     
     for (size_t i = array_length - 1; i > 0; i--) {
         size_t j = random_int_range(state, 0, i);
         
         if (i != j) {
             /* Swap elements i and j */
             memcpy(temp, base + i * element_size, element_size);
             memcpy(base + i * element_size, base + j * element_size, element_size);
             memcpy(base + j * element_size, temp, element_size);
         }
     }
     
     free(temp);
 }
 
 size_t random_weighted_index(random_state_t *state, const double *weights, size_t num_weights) {
     double total_weight = 0.0;
     size_t i;
     
     for (i = 0; i < num_weights; i++) {
         total_weight += weights[i];
     }
     
     double random_value = random_double_range(state, 0, total_weight);
     double cumulative_weight = 0.0;
     
     for (i = 0; i < num_weights; i++) {
         cumulative_weight += weights[i];
         if (random_value <= cumulative_weight) {
             return i;
         }
     }
     
     return num_weights - 1; /* Fallback to last index */
 }
 
 /* This is the jump function for the generator. It is equivalent
    to 2^128 calls to random_uint64(); it can be used to generate 2^128
    non-overlapping subsequences for parallel computations. */
 void random_jump(random_state_t *state) {
     static const uint64_t JUMP[] = {0x180ec6d33cfd0aba, 0xd5a61266f0c9392c, 0xa9582618e03fc9aa, 0x39abdc4529b1661c};
     
     uint64_t s0 = 0;
     uint64_t s1 = 0;
     uint64_t s2 = 0;
     uint64_t s3 = 0;
     
     for (int i = 0; i < sizeof(JUMP) / sizeof(*JUMP); i++) {
         for (int b = 0; b < 64; b++) {
             if (JUMP[i] & ((uint64_t)1 << b)) {
                 s0 ^= state->state[0];
                 s1 ^= state->state[1];
                 s2 ^= state->state[2];
                 s3 ^= state->state[3];
             }
             random_uint64(state);
         }
     }
     
     state->state[0] = s0;
     state->state[1] = s1;
     state->state[2] = s2;
     state->state[3] = s3;
 }
 
 /* This is the long-jump function for the generator. It is equivalent to
    2^192 calls to random_uint64(); it can be used to generate 2^64 starting points,
    from each of which jump() will generate 2^64 non-overlapping subsequences
    for parallel distributed computations. */
 void random_long_jump(random_state_t *state) {
     static const uint64_t LONG_JUMP[] = {0x76e15d3efefdcbbf, 0xc5004e441c522fb3, 0x77710069854ee241, 0x39109bb02acbe635};
     
     uint64_t s0 = 0;
     uint64_t s1 = 0;
     uint64_t s2 = 0;
     uint64_t s3 = 0;
     
     for (int i = 0; i < sizeof(LONG_JUMP) / sizeof(*LONG_JUMP); i++) {
         for (int b = 0; b < 64; b++) {
             if (LONG_JUMP[i] & ((uint64_t)1 << b)) {
                 s0 ^= state->state[0];
                 s1 ^= state->state[1];
                 s2 ^= state->state[2];
                 s3 ^= state->state[3];
             }
             random_uint64(state);
         }
     }
     
     state->state[0] = s0;
     state->state[1] = s1;
     state->state[2] = s2;
     state->state[3] = s3;
 }
 
 /* Global functions */
 void random_global_init(void) {
     random_init(&global_random_state);
 }
 
 void random_global_init_with_seed(uint64_t seed) {
     random_init_with_seed(&global_random_state, seed);
 }
 
 uint64_t random_global_uint64(void) {
     return random_uint64(&global_random_state);
 }
 
 uint32_t random_global_uint32(void) {
     return random_uint32(&global_random_state);
 }
 
 int random_global_int_range(int min, int max) {
     return random_int_range(&global_random_state, min, max);
 }
 
 float random_global_float(void) {
     return random_float(&global_random_state);
 }
 
 double random_global_double(void) {
     return random_double(&global_random_state);
 }
 
 float random_global_float_range(float min, float max) {
     return random_float_range(&global_random_state, min, max);
 }
 
 double random_global_double_range(double min, double max) {
     return random_double_range(&global_random_state, min, max);
 }