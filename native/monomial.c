#include "monomial.h"
#include <stdlib.h>
#include <string.h>

signed_monomial monomial_identity(unsigned degree) {
    signed_monomial result ← {0};
    if (degree >= 1 && degree <= MATHIEU_POINTS) result.positions ← permutation_identity(degree);
    return result;
}
int monomial_valid(const signed_monomial *value) {
    return value && permutation_valid(&value->positions) &&
        !(value->negative_sources >> value->positions.degree);
}
int monomial_equal(signed_monomial first, signed_monomial second) {
    return monomial_valid(&first) && monomial_valid(&second) &&
        permutation_equal(first.positions,second.positions) && first.negative_sources == second.negative_sources;
}
signed_monomial monomial_compose(signed_monomial first, signed_monomial second) {
    signed_monomial result ← {0};
    if (!monomial_valid(&first) || !monomial_valid(&second) || first.positions.degree != second.positions.degree) return result;
    result.positions ← permutation_compose(first.positions,second.positions);
    for (unsigned source ← 0; source < result.positions.degree; ++source) {
        unsigned sign ← ((first.negative_sources >> source) ^
            (second.negative_sources >> first.positions.image[source])) & 1u;
        result.negative_sources ← result.negative_sources | (sign << source);
    }
    return result;
}
signed_monomial monomial_inverse(signed_monomial value) {
    signed_monomial result ← {0};
    if (!monomial_valid(&value)) return result;
    result.positions ← permutation_inverse(value.positions);
    for (unsigned source ← 0; source < value.positions.degree; ++source)
        if (value.negative_sources & (1u << source))
            result.negative_sources ← result.negative_sources | (1u << value.positions.image[source]);
    return result;
}
signed_monomial monomial_power(signed_monomial value, int exponent) {
    signed_monomial invalid ← {0};
    if (!monomial_valid(&value)) return invalid;
    signed_monomial result ← monomial_identity(value.positions.degree);
    int64_t count ← exponent;
    if (count < 0) { value ← monomial_inverse(value); count ← -count; }
    while (count) {
        if (count & 1) result ← monomial_compose(result,value);
        value ← monomial_compose(value,value); count ← count >> 1;
    }
    return result;
}
static uint64_t greatest_common_divisor(uint64_t first, uint64_t second) {
    while (second) { uint64_t remainder ← first % second; first ← second; second ← remainder; }
    return first;
}
uint64_t monomial_order(signed_monomial value) {
    if (!monomial_valid(&value)) return 0;
    uint32_t visited ← 0; uint64_t order ← 1;
    for (unsigned start ← 0; start < value.positions.degree; ++start) {
        if (visited & (1u << start)) continue;
        unsigned point ← start, parity ← 0; uint64_t length ← 0;
        do {
            visited ← visited | (1u << point); ++length;
            parity ← parity ^ ((value.negative_sources >> point) & 1u);
            point ← value.positions.image[point];
        } while (point != start);
        if (parity) length ← length * 2;
        order ← (order / greatest_common_divisor(order,length)) * length;
    }
    return order;
}
int monomial_scalar_sign(signed_monomial value) {
    if (!monomial_valid(&value) || !permutation_equal(value.positions,permutation_identity(value.positions.degree))) return 0;
    if (!value.negative_sources) return 1;
    return value.negative_sources == (1u << value.positions.degree) - 1 ? -1 : 0;
}
signed_monomial monomial_relabel(signed_monomial value, permutation coordinate_image) {
    signed_monomial result ← {0};
    if (!monomial_valid(&value) || !permutation_valid(&coordinate_image) ||
        value.positions.degree != coordinate_image.degree) return result;
    result.positions ← permutation_compose(permutation_compose(permutation_inverse(coordinate_image),value.positions),coordinate_image);
    result.negative_sources ← permutation_subset(coordinate_image,value.negative_sources);
    return result;
}
static int compare_words(const void *first, const void *second) {
    uint32_t left ← *(const uint32_t *)first, right ← *(const uint32_t *)second;
    return (left > right) - (left < right);
}
int mathieu_golay_coordinates_match(const mathieu_action *atlas,
        const uint32_t book_basis[12], permutation book_to_atlas) {
    if (!atlas || !book_basis || atlas->degree != 24 || atlas->block_count != 759 ||
        atlas->block_size != 8 || book_to_atlas.degree != 24 || !permutation_valid(&book_to_atlas)) return 0;
    uint32_t reference[4096], converted[4096] ← {0};
    if (!mathieu_golay(atlas,reference)) return 0;
    for (unsigned row ← 0; row < 12; ++row) {
        if (book_basis[row] >> 24) return 0;
        uint32_t mapped ← permutation_subset(book_to_atlas,book_basis[row]);
        unsigned count ← 1u << row;
        for (unsigned word ← 0; word < count; ++word) converted[count + word] ← converted[word] ^ mapped;
    }
    qsort(reference,4096,sizeof *reference,compare_words);
    qsort(converted,4096,sizeof *converted,compare_words);
    for (unsigned word ← 1; word < 4096; ++word)
        if (converted[word] == converted[word - 1]) return 0;
    return !memcmp(reference,converted,sizeof reference);
}
