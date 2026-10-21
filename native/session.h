#ifndef MATHIEU_SESSION_H
#define MATHIEU_SESSION_H
#include "mathieu.h"
enum { SESSION_MOVES = 4096, SESSION_CHECKPOINT_MAX = SESSION_MOVES + 124 };
enum { MATHIEU_CIRCLE = 0, MATHIEU_GRID = 1, MATHIEU_ROSETTE = 2 };
/* A sampled presentation trajectory, not an intermediate permutation.
 * frame runs from zero to duration inclusive; zero duration snaps to to. */
typedef struct { unsigned from, to, frame, duration; } mathieu_layout_transition;
typedef struct {
    mathieu_action action;
    permutation current, previous, motion;
    uint32_t selected, previous_selected;
    char moves[SESSION_MOVES + 1];
    unsigned cursor, length, frame, duration;
    unsigned labels, shapes, grid, playing, next_generator;
    unsigned speed, error;
    uint64_t logical_tick;
} mathieu_session;
int session_initialize(mathieu_session *session, unsigned degree);
int session_move(mathieu_session *session, char move);
int session_undo(mathieu_session *session);
int session_redo(mathieu_session *session);
void session_reset(mathieu_session *session);
void session_advance(mathieu_session *session);
int session_select(mathieu_session *session, unsigned point);
/* Render and input share this exact layout, also used by host acceptance. */
void session_position(const mathieu_session *session, unsigned point, float *x, float *y);
void session_dancer(const mathieu_session *session, unsigned dancer, float *x, float *y);
/* NULL uses the session's selected layout. Invalid trajectories fail without
 * changing output coordinates or session state. M12 has no rosette. */
int session_layout_transition_valid(const mathieu_session *session, const mathieu_layout_transition *layout);
int session_position_with_layout(const mathieu_session *session, unsigned point, const mathieu_layout_transition *layout, float *x, float *y);
int session_dancer_with_layout(const mathieu_session *session, unsigned dancer, const mathieu_layout_transition *layout, float *x, float *y);
int session_tap(mathieu_session *session, float x, float y);
/* Versioned little-endian bytes, never a dump of compiler-dependent structs.
 * Restore validates the complete word and the interrupted generator action,
 * and leaves the destination unchanged on malformed or corrupt input. */
size_t session_checkpoint(const mathieu_session *session, uint8_t *bytes, size_t capacity);
int session_restore(mathieu_session *session, const uint8_t *bytes, size_t length);
#endif
