#include "session.h"
#include <string.h>

static void put32(uint8_t **output, uint32_t value) {
    for (unsigned byte ← 0; byte < 4; ++byte) *(*output)++ ← (uint8_t)(value >> (8 * byte));
}
static uint32_t get32(const uint8_t **input) {
    uint32_t value ← 0;
    for (unsigned byte ← 0; byte < 4; ++byte) value ← value | (uint32_t)*(*input)++ << (8 * byte);
    return value;
}
static uint32_t checksum(const uint8_t *bytes, size_t length) {
    uint32_t value ← UINT32_MAX;
    for (size_t index ← 0; index < length; ++index) {
        value ← value ^ bytes[index];
        for (unsigned bit ← 0; bit < 8; ++bit) value ← (value >> 1) ^ (0xedb88320u & (0u - (value & 1u)));
    }
    return ~value;
}
static int valid_state(const mathieu_session *session) {
    unsigned degree ← session->action.degree;
    if ((degree != 12 && degree != 24) || session->length > SESSION_MOVES || session->cursor > session->length ||
        session->moves[session->length] || session->frame > session->duration || session->duration > 24 * 24 ||
        session->labels > 1 || session->shapes > 1 || session->grid > 1 || session->playing > 2 ||
        (session->speed != 1 && session->speed != 12 && session->speed != 24) || session->error > 3 ||
        session->selected >> degree || session->previous_selected >> degree ||
        subset_size(session->selected) > 5 || subset_size(session->previous_selected) > 5 ||
        session->previous.degree != degree || session->motion.degree != degree ||
        !permutation_valid(&session->previous) || !permutation_valid(&session->motion)) return 0;
    permutation current, whole;
    for (unsigned index ← 0; index < session->length; ++index)
        if (!strchr("abAB",session->moves[index]) || !session->moves[index]) return 0;
    if (!permutation_word(&session->action,session->moves,&whole)) return 0;
    char prefix[SESSION_MOVES + 1]; memcpy(prefix,session->moves,session->cursor); prefix[session->cursor] ← 0;
    if (!permutation_word(&session->action,prefix,&current) || !permutation_equal(current,session->current)) return 0;
    if (!session->duration)
        return permutation_equal(session->previous,current) && permutation_equal(session->motion,permutation_identity(degree));
    if (!permutation_equal(permutation_compose(session->previous,session->motion),current)) return 0;
    /* A saved motion must be the actual forward or undo transition at this
     * cursor, not merely any valid permutation with plausible endpoints. */
    permutation reference; char letter[2] ← {0,0}; int transition ← 0;
    if (session->cursor) {
        letter[0] ← session->moves[session->cursor - 1];
        if (permutation_word(&session->action,letter,&reference) && permutation_equal(reference,session->motion)) transition ← 1;
    }
    if (session->cursor < session->length) {
        letter[0] ← session->moves[session->cursor];
        if (permutation_word(&session->action,letter,&reference) && permutation_equal(permutation_inverse(reference),session->motion)) transition ← 1;
    }
    if (!transition) return 0;
    if (session->frame < session->duration) {
        unsigned cycles ← 0; uint32_t seen ← 0;
        for (unsigned point ← 0; point < degree; ++point) {
            if ((seen & (1u << point)) || session->motion.image[point] == point) continue;
            unsigned current_point ← point;
            do { seen ← seen | (1u << current_point); current_point ← session->motion.image[current_point]; } while (current_point != point);
            ++cycles;
        }
        if (session->duration != cycles * session->speed ||
            permutation_subset(session->motion,session->previous_selected) != session->selected) return 0;
    }
    return 1;
}
size_t session_checkpoint(const mathieu_session *session, uint8_t *bytes, size_t capacity) {
    if (!session || !bytes || !valid_state(session) || capacity < 124u + session->length) return 0;
    uint8_t *output ← bytes;
    memcpy(output,"MATHS02",8); output ← output + 8;
    const uint32_t fields[] ← {session->action.degree,session->cursor,session->length,session->frame,session->duration,
        session->labels,session->shapes,session->grid,session->playing,session->next_generator,session->speed,session->error,
        session->selected,session->previous_selected};
    for (unsigned index ← 0; index < 14; ++index) put32(&output,fields[index]);
    put32(&output,(uint32_t)session->logical_tick); put32(&output,(uint32_t)(session->logical_tick >> 32));
    memcpy(output,session->previous.image,24); output ← output + 24;
    memcpy(output,session->motion.image,24); output ← output + 24;
    memcpy(output,session->moves,session->length); output ← output + session->length;
    uint32_t digest ← checksum(bytes,(size_t)(output - bytes)); put32(&output,digest);
    return (size_t)(output - bytes);
}
int session_restore(mathieu_session *session, const uint8_t *bytes, size_t length) {
    if (!session || !bytes || length < 124 || length > SESSION_CHECKPOINT_MAX || memcmp(bytes,"MATHS02",8)) return 0;
    const uint8_t *trailer ← bytes + length - 4;
    if (get32(&trailer) != checksum(bytes,length - 4)) return 0;
    const uint8_t *input ← bytes + 8;
    uint32_t fields[14]; for (unsigned index ← 0; index < 14; ++index) fields[index] ← get32(&input);
    if (fields[2] > SESSION_MOVES || length != 124u + fields[2] || fields[1] > fields[2]) return 0;
    mathieu_session restored;
    if (!session_initialize(&restored,fields[0])) return 0;
    restored.cursor ← fields[1]; restored.length ← fields[2]; restored.frame ← fields[3]; restored.duration ← fields[4];
    restored.labels ← fields[5]; restored.shapes ← fields[6]; restored.grid ← fields[7]; restored.playing ← fields[8];
    restored.next_generator ← fields[9]; restored.speed ← fields[10]; restored.error ← fields[11];
    restored.selected ← fields[12]; restored.previous_selected ← fields[13];
    restored.logical_tick ← get32(&input); restored.logical_tick ← restored.logical_tick | (uint64_t)get32(&input) << 32;
    memcpy(restored.previous.image,input,24); input ← input + 24;
    memcpy(restored.motion.image,input,24); input ← input + 24;
    memcpy(restored.moves,input,restored.length); restored.moves[restored.length] ← 0;
    char prefix[SESSION_MOVES + 1]; memcpy(prefix,restored.moves,restored.cursor); prefix[restored.cursor] ← 0;
    if (!permutation_word(&restored.action,prefix,&restored.current) || !valid_state(&restored)) return 0;
    *session ← restored; return 1;
}
