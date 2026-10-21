#ifndef MATHIEU_SESSION_H
#define MATHIEU_SESSION_H
#include "mathieu.h"
enum { SESSION_MOVES = 4096 };
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
int session_tap(mathieu_session *session, float x, float y);
#endif
