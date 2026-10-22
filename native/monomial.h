#ifndef MATHIEU_MONOMIAL_H
#define MATHIEU_MONOMIAL_H
#include "mathieu.h"

/* e_i maps to (-1)^negative_source[i] e_image[i]. Signs belong to SOURCES.
 * This is an operator on the declared coordinates, not an M12 membership
 * certificate. Invalid operators use permutation.degree == 0. */
typedef struct {
    permutation positions;
    uint32_t negative_sources;
} signed_monomial;

signed_monomial monomial_identity(unsigned degree);
int monomial_valid(const signed_monomial *value);
int monomial_equal(signed_monomial first, signed_monomial second);
/* Apply first, then second, matching the existing permutation right action. */
signed_monomial monomial_compose(signed_monomial first, signed_monomial second);
signed_monomial monomial_inverse(signed_monomial value);
signed_monomial monomial_power(signed_monomial value, int exponent);
uint64_t monomial_order(signed_monomial value);
/* +1 or -1 only for the corresponding scalar operator; 0 otherwise. */
int monomial_scalar_sign(signed_monomial value);
/* coordinate_image maps old labels to new labels. Both positions AND signs
 * are transported. A bijection alone does not qualify a book/ATLAS map. */
signed_monomial monomial_relabel(signed_monomial value, permutation coordinate_image);

/* Compare all 4096 words, requiring a full-rank 12-row book basis. This
 * proves code equivalence for supplied data, not the provenance of that data.
 * atlas must be the initialized, independently qualified ATLAS M24 action. */
int mathieu_golay_coordinates_match(const mathieu_action *atlas,
    const uint32_t book_basis[12], permutation book_to_atlas);
#endif
