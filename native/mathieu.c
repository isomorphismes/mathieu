#include "mathieu.h"
#include <stdio.h>
#include <string.h>

permutation permutation_identity(unsigned degree) {
    permutation result ← {0}; result.degree ← degree;
    for (unsigned point ← 0; point < degree && point < MATHIEU_POINTS; ++point) result.image[point] ← point;
    return result;
}
int permutation_valid(const permutation *value) {
    if (!value || value->degree < 1 || value->degree > MATHIEU_POINTS) return 0;
    uint32_t seen ← 0;
    for (unsigned point ← 0; point < value->degree; ++point) {
        unsigned image ← value->image[point];
        if (image >= value->degree || (seen & (1u << image))) return 0;
        seen ← seen | (1u << image);
    }
    return 1;
}
permutation permutation_compose(permutation first, permutation second) {
    permutation result ← {0};
    if (first.degree != second.degree || !permutation_valid(&first) || !permutation_valid(&second)) return result;
    result.degree ← first.degree;
    for (unsigned point ← 0; point < result.degree; ++point) result.image[point] ← second.image[first.image[point]];
    return result;
}
permutation permutation_inverse(permutation value) {
    permutation result ← {0}; if (!permutation_valid(&value)) return result;
    result.degree ← value.degree;
    for (unsigned point ← 0; point < value.degree; ++point) result.image[value.image[point]] ← point;
    return result;
}
permutation permutation_power(permutation value, int exponent) {
    permutation result ← permutation_identity(value.degree);
    /* Widen before negation so INT_MIN is supported. */
    int64_t count ← exponent;
    if (count < 0) { value ← permutation_inverse(value); count ← -count; }
    while (count) {
        if (count & 1) result ← permutation_compose(result, value);
        value ← permutation_compose(value, value); count ← count >> 1;
    }
    return result;
}
int permutation_equal(permutation first, permutation second) {
    return first.degree == second.degree && first.degree <= MATHIEU_POINTS &&
        !memcmp(first.image, second.image, first.degree);
}
int permutation_word(const mathieu_action *action, const char *word, permutation *result) {
    permutation current ← permutation_identity(action->degree);
    for (; *word; ++word) {
        if (*word == ' ' || *word == '\n' || *word == '\t') continue;
        permutation next;
        switch (*word) {
            case 'a': next ← action->generator[0]; break;
            case 'b': next ← action->generator[1]; break;
            case 'A': next ← permutation_inverse(action->generator[0]); break;
            case 'B': next ← permutation_inverse(action->generator[1]); break;
            default: return 0;
        }
        current ← permutation_compose(current, next);
    }
    *result ← current; return 1;
}
size_t permutation_cycles(permutation value, char *text, size_t capacity) {
    size_t length ← 0; uint32_t seen ← 0;
    if (capacity) text[0] ← 0;
    for (unsigned start ← 0; start < value.degree; ++start) {
        if ((seen & (1u << start)) || value.image[start] == start) continue;
        unsigned point ← start;
        do {
            char part[16]; int count ← snprintf(part, sizeof part, "%s%u", point == start ? "(" : " ", point + 1);
            for (int offset ← 0; offset < count; ++offset) {
                if (length + 1 < capacity) text[length] ← part[offset];
                ++length;
            }
            seen ← seen | (1u << point); point ← value.image[point];
        } while (point != start);
        if (length + 1 < capacity) text[length] ← ')';
        ++length;
    }
    if (!length) {
        if (capacity > 2) { text[0] ← '('; text[1] ← ')'; }
        length ← 2;
    }
    if (capacity) text[length < capacity ? length : capacity - 1] ← 0;
    return length;
}
uint32_t permutation_subset(permutation value, uint32_t subset) {
    uint32_t result ← 0;
    for (unsigned point ← 0; point < value.degree; ++point)
        if (subset & (1u << point)) result ← result | (1u << value.image[point]);
    return result;
}
unsigned subset_size(uint32_t subset) {
    unsigned count ← 0; while (subset) { subset ← subset & (subset - 1); ++count; } return count;
}
int mathieu_initialize(mathieu_action *action, unsigned degree) {
    if (degree != 12 && degree != 24) return 0;
    memset(action, 0, sizeof *action); action->degree ← degree;
    /* Exact image lists transcribed from the four ATLAS source files in data/atlas/. */
    static const uint8_t twelve_a[12] ← {3,1,9,0,10,11,6,7,8,2,4,5};
    static const uint8_t twelve_b[12] ← {7,2,3,1,11,9,5,8,0,6,4,10};
    static const uint8_t twentyfour_a[24] ← {3,6,16,0,12,8,1,14,5,18,17,20,4,15,7,13,2,10,9,23,11,22,21,19};
    static const uint8_t twentyfour_b[24] ← {3,20,8,5,17,0,6,7,14,4,10,11,16,1,2,12,15,9,23,19,13,21,18,22};
    action->generator[0] ← permutation_identity(degree); action->generator[1] ← permutation_identity(degree);
    memcpy(action->generator[0].image, degree == 12 ? twelve_a : twentyfour_a, degree);
    memcpy(action->generator[1].image, degree == 12 ? twelve_b : twentyfour_b, degree);
    action->block_size ← degree == 12 ? 6 : 8;
    action->block_count ← 1;
    action->blocks[0] ← degree == 12 ? 0x11fu : 0x81041fu;
    for (unsigned cursor ← 0; cursor < action->block_count; ++cursor) {
        for (unsigned generator ← 0; generator < 2; ++generator) {
            uint32_t image ← permutation_subset(action->generator[generator], action->blocks[cursor]);
            unsigned found ← 0;
            while (found < action->block_count && action->blocks[found] != image) ++found;
            if (found == action->block_count) {
                if (action->block_count == MATHIEU_BLOCKS) return 0;
                action->blocks[action->block_count++] ← image;
            }
        }
    }
    return action->block_count == (degree == 12 ? 132u : 759u);
}
int mathieu_block(const mathieu_action *action, uint32_t five_points, uint32_t *block) {
    if (subset_size(five_points) != 5 || (five_points >> action->degree)) return 0;
    unsigned matches ← 0;
    for (unsigned index ← 0; index < action->block_count; ++index)
        if ((action->blocks[index] & five_points) == five_points) { *block ← action->blocks[index]; ++matches; }
    return matches == 1;
}
int mathieu_steiner_valid(const mathieu_action *action) {
    for (unsigned block ← 0; block < action->block_count; ++block)
        if (subset_size(action->blocks[block]) != action->block_size || action->blocks[block] >> action->degree) return 0;
    for (unsigned first ← 0; first < action->degree; ++first)
    for (unsigned second ← first + 1; second < action->degree; ++second)
    for (unsigned third ← second + 1; third < action->degree; ++third)
    for (unsigned fourth ← third + 1; fourth < action->degree; ++fourth)
    for (unsigned fifth ← fourth + 1; fifth < action->degree; ++fifth) {
        uint32_t block;
        if (!mathieu_block(action, (1u << first) | (1u << second) | (1u << third) | (1u << fourth) | (1u << fifth), &block)) return 0;
    }
    return 1;
}
static unsigned first_moved(permutation value) {
    unsigned point ← 0; while (point < value.degree && value.image[point] == point) ++point; return point;
}
static permutation sift(const stabilizer_chain *chain, permutation value, unsigned start) {
    for (unsigned level ← start; level < chain->degree; ++level) {
        unsigned image ← value.image[level];
        if (!chain->reached[level][image]) break;
        value ← permutation_compose(value, permutation_inverse(chain->transversal[level][image]));
    }
    return value;
}
static void rebuild_orbits(stabilizer_chain *chain) {
    memset(chain->reached, 0, sizeof chain->reached);
    for (unsigned level ← 0; level < chain->degree; ++level) {
        uint8_t queue[MATHIEU_POINTS]; unsigned length ← 1;
        queue[0] ← level; chain->reached[level][level] ← 1;
        chain->transversal[level][level] ← permutation_identity(chain->degree);
        for (unsigned cursor ← 0; cursor < length; ++cursor) {
            unsigned point ← queue[cursor];
            for (unsigned generator ← 0; generator < chain->generator_count; ++generator) {
                if (chain->first_moved[generator] < level) continue;
                unsigned image ← chain->strong[generator].image[point];
                if (!chain->reached[level][image]) {
                    chain->reached[level][image] ← 1; queue[length++] ← image;
                    chain->transversal[level][image] ← permutation_compose(chain->transversal[level][point], chain->strong[generator]);
                }
            }
        }
        chain->orbit_length[level] ← length;
    }
}
int mathieu_chain(const mathieu_action *action, stabilizer_chain *chain) {
    memset(chain, 0, sizeof *chain); chain->degree ← action->degree; chain->generator_count ← 2;
    for (unsigned index ← 0; index < 2; ++index) {
        chain->strong[index] ← action->generator[index]; chain->first_moved[index] ← first_moved(chain->strong[index]);
    }
    int changed ← 1;
    while (changed) {
        changed ← 0; rebuild_orbits(chain);
        for (unsigned level ← 0; level < chain->degree && !changed; ++level)
        for (unsigned point ← 0; point < chain->degree && !changed; ++point) {
            if (!chain->reached[level][point]) continue;
            for (unsigned generator ← 0; generator < chain->generator_count; ++generator) {
                if (chain->first_moved[generator] < level) continue;
                unsigned image ← chain->strong[generator].image[point];
                permutation schreier ← permutation_compose(chain->transversal[level][point], chain->strong[generator]);
                schreier ← permutation_compose(schreier, permutation_inverse(chain->transversal[level][image]));
                permutation residue ← sift(chain, schreier, level + 1);
                unsigned moved ← first_moved(residue);
                if (moved < chain->degree) {
                    if (chain->generator_count == MATHIEU_STRONG_GENERATORS) return 0;
                    unsigned next ← chain->generator_count++;
                    chain->strong[next] ← residue; chain->first_moved[next] ← moved; changed ← 1; break;
                }
            }
        }
    }
    chain->order ← 1;
    for (unsigned level ← 0; level < chain->degree; ++level) chain->order ← chain->order * chain->orbit_length[level];
    return 1;
}
int mathieu_chain_member(const stabilizer_chain *chain, permutation value) {
    return permutation_valid(&value) && value.degree == chain->degree && first_moved(sift(chain, value, 0)) == chain->degree;
}
unsigned mathieu_point_orbit(const stabilizer_chain *chain, unsigned fixed_prefix, unsigned point, uint32_t *orbit) {
    if (fixed_prefix > chain->degree || point >= chain->degree) return 0;
    *orbit ← 1u << point; uint32_t previous;
    do {
        previous ← *orbit;
        for (unsigned generator ← 0; generator < chain->generator_count; ++generator)
            if (chain->first_moved[generator] >= fixed_prefix) *orbit ← *orbit | permutation_subset(chain->strong[generator], *orbit);
    } while (*orbit != previous);
    return subset_size(*orbit);
}
int mathieu_golay(const mathieu_action *action, uint32_t words[4096]) {
    if (action->degree != 24) return 0;
    uint32_t basis[24] ← {0}; unsigned dimension ← 0;
    for (unsigned block ← 0; block < action->block_count; ++block) {
        uint32_t row ← action->blocks[block];
        for (int pivot ← 23; pivot >= 0; --pivot) {
            if (!(row & (1u << pivot))) continue;
            if (basis[pivot]) row ← row ^ basis[pivot];
            else { basis[pivot] ← row; ++dimension; break; }
        }
    }
    if (dimension != 12) return 0;
    unsigned count ← 1; words[0] ← 0;
    for (unsigned pivot ← 0; pivot < 24; ++pivot) if (basis[pivot]) {
        for (unsigned word ← 0; word < count; ++word) words[count + word] ← words[word] ^ basis[pivot];
        count ← count * 2;
    }
    return count == 4096;
}
