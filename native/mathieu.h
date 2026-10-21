#ifndef MATHIEU_H
#define MATHIEU_H
#include <stddef.h>
#include <stdint.h>

enum { MATHIEU_POINTS = 24, MATHIEU_BLOCKS = 759, MATHIEU_STRONG_GENERATORS = 256 };
typedef struct { unsigned degree; uint8_t image[MATHIEU_POINTS]; } permutation;
typedef struct {
    unsigned degree, block_size, block_count;
    permutation generator[2];
    uint32_t blocks[MATHIEU_BLOCKS];
} mathieu_action;
typedef struct {
    unsigned degree, generator_count;
    permutation strong[MATHIEU_STRONG_GENERATORS];
    unsigned first_moved[MATHIEU_STRONG_GENERATORS];
    unsigned orbit_length[MATHIEU_POINTS];
    uint8_t reached[MATHIEU_POINTS][MATHIEU_POINTS];
    permutation transversal[MATHIEU_POINTS][MATHIEU_POINTS];
    uint64_t order;
} stabilizer_chain;

/* Right action: compose(first, second) sends i to second(first(i)).
 * Public coordinates are ATLAS labels minus one, in both groups and designs. */
permutation permutation_identity(unsigned degree);
int permutation_valid(const permutation *value);
permutation permutation_compose(permutation first, permutation second);
permutation permutation_inverse(permutation value);
permutation permutation_power(permutation value, int exponent);
int permutation_equal(permutation first, permutation second);
int permutation_word(const mathieu_action *action, const char *word, permutation *result);
size_t permutation_cycles(permutation value, char *text, size_t capacity);
uint32_t permutation_subset(permutation value, uint32_t subset);
unsigned subset_size(uint32_t subset);
int mathieu_initialize(mathieu_action *action, unsigned degree);
int mathieu_block(const mathieu_action *action, uint32_t five_points, uint32_t *block);
int mathieu_steiner_valid(const mathieu_action *action);
int mathieu_chain(const mathieu_action *action, stabilizer_chain *chain);
int mathieu_chain_member(const stabilizer_chain *chain, permutation value);
unsigned mathieu_point_orbit(const stabilizer_chain *chain, unsigned fixed_prefix, unsigned point, uint32_t *orbit);
int mathieu_golay(const mathieu_action *action, uint32_t words[4096]);
#endif
