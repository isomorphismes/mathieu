#include "../native/monomial.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REQUIRE(condition) do { if (!(condition)) { fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#condition); exit(1); } } while (0)
typedef int dense_operator[MATHIEU_POINTS][MATHIEU_POINTS];

static void dense(signed_monomial value, dense_operator matrix) {
    memset(matrix,0,sizeof(dense_operator));
    for (unsigned source ← 0; source < value.positions.degree; ++source)
        matrix[value.positions.image[source]][source] ← (value.negative_sources & (1u << source)) ? -1 : 1;
}
/* Ordinary column-vector matrix multiplication, deliberately independent of
 * the producer's source-mask formula. Applying first then second is B A. */
static void require_dense_product(signed_monomial first, signed_monomial second) {
    dense_operator first_matrix,second_matrix,actual;
    dense(first,first_matrix); dense(second,second_matrix);
    signed_monomial product ← monomial_compose(first,second);
    REQUIRE(monomial_valid(&product)); dense(product,actual);
    unsigned degree ← first.positions.degree;
    for (unsigned row ← 0; row < degree; ++row)
    for (unsigned column ← 0; column < degree; ++column) {
        int expected ← 0;
        for (unsigned inner ← 0; inner < degree; ++inner)
            expected ← expected + second_matrix[row][inner] * first_matrix[inner][column];
        REQUIRE(actual[row][column] == expected);
    }
}
static void require_inverse(signed_monomial value) {
    dense_operator forward,backward;
    signed_monomial reversed ← monomial_inverse(value);
    dense(value,forward); dense(reversed,backward);
    for (unsigned row ← 0; row < value.positions.degree; ++row)
    for (unsigned column ← 0; column < value.positions.degree; ++column)
        REQUIRE(backward[row][column] == forward[column][row]);
    REQUIRE(monomial_equal(monomial_compose(value,reversed),monomial_identity(value.positions.degree)));
    REQUIRE(monomial_equal(monomial_compose(reversed,value),monomial_identity(value.positions.degree)));
}
static void require_relabel(signed_monomial value, permutation coordinates) {
    dense_operator before,after;
    dense(value,before); dense(monomial_relabel(value,coordinates),after);
    for (unsigned row ← 0; row < value.positions.degree; ++row)
    for (unsigned column ← 0; column < value.positions.degree; ++column)
        REQUIRE(before[row][column] == after[coordinates.image[row]][coordinates.image[column]]);
    REQUIRE(monomial_equal(monomial_relabel(monomial_relabel(value,coordinates),permutation_inverse(coordinates)),value));
}
static void exhaustive_three_points(void) {
    static const uint8_t images[6][3] ← {{0,1,2},{0,2,1},{1,0,2},{1,2,0},{2,0,1},{2,1,0}};
    signed_monomial operators[48]; unsigned count ← 0;
    for (unsigned index ← 0; index < 6; ++index)
    for (unsigned signs ← 0; signs < 8; ++signs) {
        signed_monomial value ← monomial_identity(3);
        memcpy(value.positions.image,images[index],3); value.negative_sources ← signs;
        operators[count++] ← value; require_inverse(value); require_relabel(value,operators[0].positions);
        unsigned actual_order ← 0; signed_monomial repeated ← monomial_identity(3);
        do { repeated ← monomial_compose(repeated,value); ++actual_order; REQUIRE(actual_order <= 6); }
        while (!monomial_equal(repeated,monomial_identity(3)));
        REQUIRE(monomial_order(value) == actual_order);
        REQUIRE(monomial_equal(monomial_power(value,-3),monomial_power(monomial_inverse(value),3)));
    }
    for (unsigned first ← 0; first < count; ++first)
    for (unsigned second ← 0; second < count; ++second) {
        require_dense_product(operators[first],operators[second]);
        require_relabel(operators[first],operators[second].positions);
    }
    puts("PASS all 2304 signed three-point products against dense matrix multiplication");
}
static void signs_and_refusals(void) {
    signed_monomial turn ← monomial_identity(2); turn.positions.image[0] ← 1; turn.positions.image[1] ← 0;
    turn.negative_sources ← 2;
    REQUIRE(monomial_order(turn) == 4);
    REQUIRE(monomial_scalar_sign(monomial_power(turn,2)) == -1);
    REQUIRE(monomial_scalar_sign(monomial_power(turn,4)) == 1);
    REQUIRE(monomial_scalar_sign(turn) == 0);
    REQUIRE(monomial_equal(monomial_power(turn,INT_MIN),monomial_identity(2)));
    signed_monomial fixed ← monomial_identity(12); fixed.negative_sources ← 1;
    REQUIRE(monomial_order(fixed) == 2 && monomial_scalar_sign(fixed) == 0);
    REQUIRE(monomial_equal(monomial_inverse(fixed),fixed));
    signed_monomial invalid ← monomial_identity(0), too_large ← monomial_identity(25);
    REQUIRE(!monomial_valid(NULL) && !monomial_valid(&invalid) && !monomial_valid(&too_large));
    invalid ← fixed; invalid.negative_sources ← 1u << 12; REQUIRE(!monomial_valid(&invalid));
    REQUIRE(!monomial_equal(monomial_power(invalid,0),monomial_identity(12)));
    REQUIRE(!monomial_equal(invalid,invalid) && !monomial_order(invalid));
    REQUIRE(!monomial_scalar_sign(invalid));
    invalid ← fixed; invalid.positions.image[1] ← 0; REQUIRE(!monomial_valid(&invalid));
    invalid ← monomial_compose(fixed,turn); REQUIRE(!monomial_valid(&invalid));
    invalid ← monomial_inverse(invalid); REQUIRE(!monomial_valid(&invalid));
    invalid ← monomial_relabel(fixed,turn.positions); REQUIRE(!monomial_valid(&invalid));
    for (unsigned degree ← 12; degree <= 24; degree ← degree + 12) {
        mathieu_action action; REQUIRE(mathieu_initialize(&action,degree));
        signed_monomial first ← monomial_identity(degree), second ← monomial_identity(degree);
        first.positions ← action.generator[0]; first.negative_sources ← 0x95a95au & ((1u << degree) - 1);
        second.positions ← action.generator[1]; second.negative_sources ← 0x6a56a5u & ((1u << degree) - 1);
        require_dense_product(first,second); require_inverse(first); require_inverse(second);
        require_relabel(first,second.positions);
        signed_monomial minus_identity ← monomial_identity(degree); minus_identity.negative_sources ← (1u << degree) - 1;
        REQUIRE(monomial_scalar_sign(minus_identity) == -1);
        REQUIRE(monomial_equal(monomial_compose(first,minus_identity),monomial_compose(minus_identity,first)));
        REQUIRE(monomial_equal(monomial_power(first,(int)monomial_order(first)),monomial_identity(degree)));
    }
    puts("PASS negative fixed points, central scalar signs, inverse, power and malformed-operator refusal");
}
static void golay_coordinate_boundary(void) {
    mathieu_action atlas; uint32_t words[4096],basis[12];
    REQUIRE(mathieu_initialize(&atlas,24) && mathieu_golay(&atlas,words));
    permutation coordinates ← permutation_identity(24);
    for (unsigned point ← 0; point < 24; ++point) coordinates.image[point] ← (point + 1) % 24;
    permutation reversed ← permutation_inverse(coordinates);
    for (unsigned row ← 0; row < 12; ++row) basis[row] ← permutation_subset(reversed,words[1u << row]);
    REQUIRE(mathieu_golay_coordinates_match(&atlas,basis,coordinates));
    /* This arbitrary relabeling is test data, never the book map. Its odd
     * 24-cycle cannot preserve the ATLAS code (whose group is M24 <= A24). */
    REQUIRE(!mathieu_golay_coordinates_match(&atlas,basis,permutation_identity(24)));
    uint32_t saved ← basis[0]; basis[0] ← basis[1];
    REQUIRE(!mathieu_golay_coordinates_match(&atlas,basis,coordinates)); basis[0] ← saved;
    basis[0] ← basis[0] | (1u << 24); REQUIRE(!mathieu_golay_coordinates_match(&atlas,basis,coordinates)); basis[0] ← saved;
    coordinates.image[1] ← coordinates.image[0]; REQUIRE(!mathieu_golay_coordinates_match(&atlas,basis,coordinates));
    REQUIRE(!mathieu_golay_coordinates_match(NULL,basis,permutation_identity(24)));
    REQUIRE(!mathieu_golay_coordinates_match(&atlas,NULL,permutation_identity(24)));
    REQUIRE(!mathieu_golay_coordinates_match(&atlas,basis,permutation_identity(12)));
    puts("PASS all-word Golay coordinate admission, wrong-map and deficient-basis refusal (synthetic coordinates only)");
}
int main(void) {
    exhaustive_three_points(); signs_and_refusals(); golay_coordinate_boundary(); return 0;
}
