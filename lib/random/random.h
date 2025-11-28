/**
 * @file randomlib.h
 * @brief A comprehensive random number generation library in C
 * 
 * This library provides various random number generation methods
 * with options for seeding, different distributions, and ranges.
 */

 #ifndef RANDOMLIB_H
 #define RANDOMLIB_H
 
 #include <stdint.h>
 #include <stddef.h>  /* For size_t */
 #include <stdbool.h> /* For bool type */
 
 /**
  * @brief Random number generator state structure
  */
 typedef struct random_state {
     uint64_t state[4];  /* State for xoshiro256** algorithm */
     bool is_seeded;     /* Flag to check if RNG has been seeded */
 } random_state_t;
 
 /**
  * @brief Distribution types for random number generation
  */
 typedef enum {
     RANDOM_UNIFORM,     // Uniform distribution
     RANDOM_NORMAL,      // Normal (Gaussian) distribution
     RANDOM_EXPONENTIAL, // Exponential distribution
     RANDOM_POISSON      // Poisson distribution
 } random_distribution_t;
 
 /**
  * @brief Initialize a random state with system time as seed
  * 
  * @param state Pointer to random state structure
  */
 void random_init(random_state_t *state);
 
 /**
  * @brief Initialize a random state with a user-provided seed
  * 
  * @param state Pointer to random state structure
  * @param seed The seed value to use
  */
 void random_init_with_seed(random_state_t *state, uint64_t seed);
 
 /**
  * @brief Initialize a random state with multiple seed values
  * 
  * @param state Pointer to random state structure
  * @param seed1 First seed value
  * @param seed2 Second seed value
  * @param seed3 Third seed value
  * @param seed4 Fourth seed value
  */
 void random_init_with_seeds(random_state_t *state, uint64_t seed1, uint64_t seed2, 
                           uint64_t seed3, uint64_t seed4);
 
 /**
  * @brief Get a random 64-bit unsigned integer
  * 
  * @param state Pointer to random state structure
  * @return A random uint64_t value
  */
 uint64_t random_uint64(random_state_t *state);
 
 /**
  * @brief Get a random 32-bit unsigned integer
  * 
  * @param state Pointer to random state structure
  * @return A random uint32_t value
  */
 uint32_t random_uint32(random_state_t *state);
 
 /**
  * @brief Get a random integer within a specified range [min, max]
  * 
  * @param state Pointer to random state structure
  * @param min Minimum value (inclusive)
  * @param max Maximum value (inclusive)
  * @return A random integer in the specified range
  */
 int random_int_range(random_state_t *state, int min, int max);
 
 /**
  * @brief Get a random float between 0.0 and 1.0
  * 
  * @param state Pointer to random state structure
  * @return A random float in [0.0, 1.0)
  */
 float random_float(random_state_t *state);
 
 /**
  * @brief Get a random double between 0.0 and 1.0
  * 
  * @param state Pointer to random state structure
  * @return A random double in [0.0, 1.0)
  */
 double random_double(random_state_t *state);
 
 /**
  * @brief Get a random float within a specified range
  * 
  * @param state Pointer to random state structure
  * @param min Minimum value (inclusive)
  * @param max Maximum value (inclusive)
  * @return A random float in the specified range
  */
 float random_float_range(random_state_t *state, float min, float max);
 
 /**
  * @brief Get a random double within a specified range
  * 
  * @param state Pointer to random state structure
  * @param min Minimum value (inclusive)
  * @param max Maximum value (inclusive)
  * @return A random double in the specified range
  */
 double random_double_range(random_state_t *state, double min, double max);
 
 /**
  * @brief Get a random boolean value
  * 
  * @param state Pointer to random state structure
  * @return A random boolean value (true or false)
  */
 bool random_bool(random_state_t *state);
 
 /**
  * @brief Get a random boolean value with specified probability
  * 
  * @param state Pointer to random state structure
  * @param probability Probability of returning true (0.0 to 1.0)
  * @return A random boolean value based on the given probability
  */
 bool random_bool_prob(random_state_t *state, double probability);
 
 /**
  * @brief Get a random number from a specified distribution
  * 
  * @param state Pointer to random state structure
  * @param dist Distribution type
  * @param param1 First parameter for the distribution
  * @param param2 Second parameter for the distribution (if needed)
  * @return A random number from the specified distribution
  */
 double random_distribution(random_state_t *state, random_distribution_t dist, 
                         double param1, double param2);
 
 /**
  * @brief Generate random bytes
  * 
  * @param state Pointer to random state structure
  * @param buffer Pointer to the buffer to fill with random bytes
  * @param size Number of bytes to generate
  */
 void random_bytes(random_state_t *state, void *buffer, size_t size);
 
 /**
  * @brief Shuffle an array of elements
  * 
  * @param state Pointer to random state structure
  * @param array Pointer to the array to shuffle
  * @param element_size Size of each element in bytes
  * @param array_length Number of elements in the array
  */
 void random_shuffle(random_state_t *state, void *array, size_t element_size, size_t array_length);
 
 /**
  * @brief Get a random index based on weights
  * 
  * @param state Pointer to random state structure
  * @param weights Array of weights
  * @param num_weights Number of weights in the array
  * @return A random index based on the provided weights
  */
 size_t random_weighted_index(random_state_t *state, const double *weights, size_t num_weights);
 
 /**
  * @brief Jump function for the random state
  * This is equivalent to 2^128 calls to random_uint64
  * 
  * @param state Pointer to random state structure
  */
 void random_jump(random_state_t *state);
 
 /**
  * @brief Long jump function for the random state
  * This is equivalent to 2^192 calls to random_uint64
  * 
  * @param state Pointer to random state structure
  */
 void random_long_jump(random_state_t *state);
 
 /**
  * @brief Global random state instance
  */
 extern random_state_t global_random_state;
 
 /**
  * @brief Global initialization function
  * This initializes the global random state with system time
  */
 void random_global_init(void);
 
 /**
  * @brief Global initialization function with seed
  * 
  * @param seed The seed value to use
  */
 void random_global_init_with_seed(uint64_t seed);
 
 /**
  * @brief Global uint64 random function
  * 
  * @return A random uint64_t value
  */
 uint64_t random_global_uint64(void);
 
 /**
  * @brief Global uint32 random function
  * 
  * @return A random uint32_t value
  */
 uint32_t random_global_uint32(void);
 
 /**
  * @brief Global int range random function
  * 
  * @param min Minimum value (inclusive)
  * @param max Maximum value (inclusive)
  * @return A random integer in the specified range
  */
 int random_global_int_range(int min, int max);
 
 /**
  * @brief Global float random function
  * 
  * @return A random float in [0.0, 1.0)
  */
 float random_global_float(void);
 
 /**
  * @brief Global double random function
  * 
  * @return A random double in [0.0, 1.0)
  */
 double random_global_double(void);
 
 /**
  * @brief Global float range random function
  * 
  * @param min Minimum value (inclusive)
  * @param max Maximum value (inclusive)
  * @return A random float in the specified range
  */
 float random_global_float_range(float min, float max);
 
 /**
  * @brief Global double range random function
  * 
  * @param min Minimum value (inclusive)
  * @param max Maximum value (inclusive)
  * @return A random double in the specified range
  */
 double random_global_double_range(double min, double max);
 
 #endif /* RANDOMLIB_H */