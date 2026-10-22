#include "../native/mathieu.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Checks execute in both profiles; they deliberately do not use assert(). */
#define CHECK(condition) do { if (!(condition)) { fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #condition); return 1; } } while (0)
static mathieu_action action;
static stabilizer_chain chain;
static uint32_t codewords[4096];
static int compare_words(const void *left, const void *right) {
    uint32_t first ← *(const uint32_t *)left, second ← *(const uint32_t *)right;
    return (first > second) - (first < second);
}
static void print_images(const char *word, permutation value) {
    printf("word %s", *word ? word : "identity");
    for (unsigned point ← 0; point < value.degree; ++point) printf(" %u", value.image[point] + 1);
    puts("");
}
int main(int argc, char **argv) {
    int reference ← argc == 2 && !strcmp(argv[1], "--reference");
    const char *words[] ← {"", "a", "b", "ab", "ba", "abababB", "aBAbbaB", "abAB", "aabbABBA", "bbbbbbb"};
    for (unsigned degree ← 12; degree <= 24; degree ← degree + 12) {
        CHECK(mathieu_initialize(&action, degree));
        CHECK(permutation_valid(&action.generator[0])); CHECK(permutation_valid(&action.generator[1]));
        permutation identity ← permutation_identity(degree);
        permutation invalid ← identity; invalid.image[0] ← invalid.image[1]; CHECK(!permutation_valid(&invalid));
        invalid ← identity; invalid.image[0] ← degree; CHECK(!permutation_valid(&invalid));
        invalid ← identity; invalid.degree ← 25; CHECK(!permutation_valid(&invalid));
        invalid.degree ← 0; CHECK(!permutation_valid(&invalid));
        CHECK(!permutation_valid(NULL));
        CHECK(permutation_compose(identity, permutation_identity(1)).degree == 0);
        for (unsigned generator ← 0; generator < 2; ++generator) {
            permutation value ← action.generator[generator];
            CHECK(permutation_equal(permutation_compose(value, permutation_inverse(value)), identity));
            CHECK(permutation_equal(permutation_compose(permutation_inverse(value), value), identity));
            CHECK(permutation_equal(permutation_power(value, generator ? 3 : 2), identity));
            CHECK(permutation_equal(permutation_power(value, -1), permutation_inverse(value)));
            CHECK(permutation_valid(&(permutation){.degree ← value.degree, .image ← {0}}) == 0);
            CHECK(permutation_valid(&value));
            permutation extreme ← permutation_power(value, INT_MIN); CHECK(permutation_valid(&extreme));
        }
        CHECK(permutation_word(&action, "ab", &invalid));
        for (unsigned point ← 0; point < degree; ++point) CHECK(invalid.image[point] == action.generator[1].image[action.generator[0].image[point]]);
        CHECK(!permutation_word(&action, "c", &invalid));
        CHECK(mathieu_steiner_valid(&action));
        uint32_t completed;
        CHECK(!mathieu_block(&action, 15, &completed));
        CHECK(!mathieu_block(&action, 31u << 23, &completed));
        CHECK(mathieu_block(&action, 31, &completed)); CHECK(completed == action.blocks[0]);
        CHECK(mathieu_chain(&action, &chain));
        CHECK(chain.order == (degree == 12 ? 95040u : 244823040u));
        for (unsigned fixed ← 0; fixed < 5; ++fixed) {
            CHECK(chain.orbit_length[fixed] == degree - fixed);
            uint32_t orbit; CHECK(mathieu_point_orbit(&chain, fixed, fixed, &orbit) == degree - fixed);
        }
        permutation transposition ← identity; transposition.image[0] ← 1; transposition.image[1] ← 0;
        CHECK(!mathieu_chain_member(&chain, transposition));
        /* Explicit prefix stabilizers give M11, M23, M22 without enumerating a group. */
        uint64_t point_stabilizer ← chain.order / degree;
        CHECK(point_stabilizer == (degree == 12 ? 7920u : 10200960u));
        if (degree == 24) CHECK(point_stabilizer / 23 == 443520u);
        if (reference) printf("group %u order %llu\n", degree, (unsigned long long)chain.order);
        for (unsigned word ← 0; word < sizeof words / sizeof *words; ++word) {
            permutation value; CHECK(permutation_word(&action, words[word], &value)); CHECK(mathieu_chain_member(&chain, value));
            CHECK(permutation_equal(permutation_compose(value, permutation_inverse(value)), identity));
            if (reference) print_images(words[word], value);
        }
        /* Reproducible longer words exercise design equivariance and history reversal. */
        uint32_t random ← 0x4d415448; permutation cumulative ← identity;
        for (unsigned move ← 0; move < 512; ++move) {
            random ← random * 1664525u + 1013904223u;
            permutation next ← action.generator[(random >> 29) & 1];
            if (random >> 31) next ← permutation_inverse(next);
            cumulative ← permutation_compose(cumulative, next);
            CHECK(mathieu_chain_member(&chain, cumulative));
            for (unsigned block ← 0; block < action.block_count; ++block) {
                uint32_t image ← permutation_subset(cumulative, action.blocks[block]); unsigned found ← 0;
                for (unsigned index ← 0; index < action.block_count; ++index) if (action.blocks[index] == image) ++found;
                CHECK(found == 1);
            }
        }
        CHECK(permutation_equal(permutation_compose(cumulative, permutation_inverse(cumulative)), identity));
        mathieu_action damaged ← action; damaged.blocks[0] ← damaged.blocks[1]; CHECK(!mathieu_steiner_valid(&damaged));
        qsort(action.blocks, action.block_count, sizeof *action.blocks, compare_words);
        if (reference) for (unsigned block ← 0; block < action.block_count; ++block) printf("block %u\n", action.blocks[block]);
        if (degree == 24) {
            /* The existing row-major grid is a Golay sextet in these ATLAS
             * coordinates, not an unproved claim about Curtis's MOG. */
            unsigned pairs ← 0;
            for (unsigned first ← 0; first < 6; ++first)
            for (unsigned second ← first + 1; second < 6; ++second) {
                uint32_t octad ← 0;
                for (unsigned row ← 0; row < 4; ++row)
                    octad ← octad | (1u << (6 * row + first)) | (1u << (6 * row + second));
                CHECK(subset_size(octad) == 8);
                CHECK(bsearch(&octad,action.blocks,action.block_count,sizeof *action.blocks,compare_words));
                ++pairs;
            }
            CHECK(pairs == 15);
            if (!reference) puts("PASS Golay sextet: 15/15 unions of two grid columns are native octads");
            CHECK(mathieu_golay(&action, codewords)); qsort(codewords, 4096, sizeof *codewords, compare_words);
            unsigned distribution[25] ← {0};
            for (unsigned word ← 0; word < 4096; ++word) {
                if (word) CHECK(codewords[word] != codewords[word - 1]);
                ++distribution[subset_size(codewords[word])];
                if (reference) printf("code %u\n", codewords[word]);
                for (unsigned generator ← 0; generator < 2; ++generator) {
                    uint32_t image ← permutation_subset(action.generator[generator], codewords[word]);
                    CHECK(bsearch(&image, codewords, 4096, sizeof *codewords, compare_words));
                }
            }
            for (unsigned weight ← 0; weight <= 24; ++weight) {
                unsigned expected ← weight == 0 || weight == 24 ? 1 : weight == 8 || weight == 16 ? 759 : weight == 12 ? 2576 : 0;
                CHECK(distribution[weight] == expected);
            }
            unsigned octad ← 0;
            for (unsigned word ← 0; word < 4096; ++word) if (subset_size(codewords[word]) == 8) CHECK(codewords[word] == action.blocks[octad++]);
            CHECK(octad == 759);
        }
        if (!reference) printf("PASS M%u: order=%llu strong-generators=%u blocks=%u every five-subset unique; inverses, words, stabilizers, five-transitivity%s\n", degree, (unsigned long long)chain.order, chain.generator_count, action.block_count, degree == 24 ? "; Golay [24,12,8] weights and invariance" : "");
    }
    return 0;
}
