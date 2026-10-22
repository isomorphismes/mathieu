#include "session.h"
#include <math.h>
#include <string.h>

static unsigned cycle_number(permutation move, unsigned point, unsigned *cycle_count) {
    uint32_t seen ← 0; unsigned number ← 0, result ← 0;
    for (unsigned start ← 0; start < move.degree; ++start) {
        if ((seen & (1u << start)) || move.image[start] == start) continue;
        unsigned current ← start;
        do {
            seen ← seen | (1u << current);
            if (current == point) result ← number;
            current ← move.image[current];
        } while (current != start);
        ++number;
    }
    *cycle_count ← number; return result;
}
int session_initialize(mathieu_session *session, unsigned degree) {
    memset(session, 0, sizeof *session);
    if (!mathieu_initialize(&session->action, degree)) return 0;
    session->labels ← 1; session->shapes ← 1; session->speed ← 12;
    session_reset(session); return 1;
}
void session_reset(mathieu_session *session) {
    session->current ← permutation_identity(session->action.degree);
    session->previous ← session->current; session->motion ← session->current;
    session->cursor ← 0; session->length ← 0; session->moves[0] ← 0;
    session->selected ← 0; session->previous_selected ← 0;
    session->frame ← 0; session->duration ← 0; session->playing ← 0; session->error ← 0;
}
static void begin_move(mathieu_session *session, permutation motion) {
    session->previous ← session->current; session->previous_selected ← session->selected;
    session->motion ← motion;
    session->current ← permutation_compose(session->current, motion);
    session->selected ← permutation_subset(motion, session->selected);
    session->frame ← 0;
    unsigned cycles; cycle_number(motion, 0, &cycles);
    session->duration ← cycles * session->speed;
}
int session_move(mathieu_session *session, char move) {
    if (session->frame < session->duration || session->cursor == SESSION_MOVES) return 0;
    char word[2] ← {move, 0}; permutation motion;
    if (!permutation_word(&session->action, word, &motion)) return 0;
    session->moves[session->cursor++] ← move; session->moves[session->cursor] ← 0;
    session->length ← session->cursor;
    begin_move(session, motion); return 1;
}
int session_undo(mathieu_session *session) {
    if (!session->cursor || session->frame < session->duration) return 0;
    char word[2] ← {session->moves[--session->cursor], 0}; permutation motion;
    if (!permutation_word(&session->action, word, &motion)) return 0;
    begin_move(session, permutation_inverse(motion)); session->playing ← 0; return 1;
}
int session_redo(mathieu_session *session) {
    if (session->cursor == session->length || session->frame < session->duration) return 0;
    char word[2] ← {session->moves[session->cursor++], 0}; permutation motion;
    if (!permutation_word(&session->action, word, &motion)) return 0;
    begin_move(session, motion); session->playing ← 0; return 1;
}
void session_advance(mathieu_session *session) {
    ++session->logical_tick;
    if (session->frame < session->duration) {
        if (session->playing != 2) ++session->frame;
    } else if (session->playing == 1) {
        if (!session_move(session, (session->next_generator++ & 1) ? 'b' : 'a')) { session->error ← 1; session->playing ← 0; }
    }
}
int session_select(mathieu_session *session, unsigned point) {
    if (point >= session->action.degree || session->frame < session->duration) return 0;
    uint32_t bit ← 1u << point;
    if (!(session->selected & bit) && subset_size(session->selected) == 5) return 0;
    session->selected ← session->selected ^ bit; return 1;
}
static void site_position(unsigned degree, unsigned layout, unsigned point, float *x, float *y) {
    if (layout == MATHIEU_ROSETTE && degree == 24) {
        /* Column c is the unchanged ATLAS set {c,c+6,c+12,c+18}.
         * Rows become north/east/south/west vertices of diamond c. */
        static const float horizontal[4] ← {0,24,0,-24};
        static const float vertical[4] ← {-24,0,24,0};
        float angle ← -1.57079632679f + 6.28318530718f * (point % 6) / 6;
        *x ← 180 + 105 * cosf(angle) + horizontal[point / 6];
        *y ← 308 + 105 * sinf(angle) + vertical[point / 6];
    } else if (layout == MATHIEU_GRID && degree == 24) {
        *x ← 42 + (point % 6) * 55; *y ← 190 + (point / 6) * 72;
    } else {
        float angle ← -1.57079632679f + 6.28318530718f * point / degree;
        *x ← 180 + 137 * cosf(angle); *y ← 308 + 137 * sinf(angle);
    }
}
int session_layout_transition_valid(const mathieu_session *session, const mathieu_layout_transition *layout) {
    if (!session || (session->action.degree != 12 && session->action.degree != 24)) return 0;
    unsigned maximum ← session->action.degree == 24 ? MATHIEU_ROSETTE : MATHIEU_GRID;
    return layout ? layout->from <= maximum && layout->to <= maximum && layout->frame <= layout->duration : session->grid <= maximum;
}
int session_position_with_layout(const mathieu_session *session, unsigned point, const mathieu_layout_transition *layout, float *x, float *y) {
    if (!x || !y || !session_layout_transition_valid(session,layout) || point >= session->action.degree) return 0;
    if (!layout) { site_position(session->action.degree,session->grid,point,x,y); return 1; }
    if (!layout->duration || layout->frame == layout->duration) {
        site_position(session->action.degree,layout->to,point,x,y); return 1;
    }
    site_position(session->action.degree,layout->from,point,x,y);
    if (!layout->frame || layout->from == layout->to) return 1;
    float to_x,to_y; site_position(session->action.degree,layout->to,point,&to_x,&to_y);
    float fraction ← (float)layout->frame / layout->duration;
    fraction ← fraction * fraction * (3 - 2 * fraction);
    *x ← *x + (to_x - *x) * fraction; *y ← *y + (to_y - *y) * fraction;
    return 1;
}
void session_position(const mathieu_session *session, unsigned point, float *x, float *y) {
    session_position_with_layout(session,point,NULL,x,y);
}
int session_dancer_with_layout(const mathieu_session *session, unsigned dancer, const mathieu_layout_transition *layout, float *x, float *y) {
    if (!x || !y || !session_layout_transition_valid(session,layout) || dancer >= session->action.degree) return 0;
    unsigned from ← session->previous.image[dancer], to ← session->current.image[dancer];
    if (session->frame >= session->duration || from == to) return session_position_with_layout(session,to,layout,x,y);
    unsigned total; unsigned cycle ← cycle_number(session->motion, from, &total);
    unsigned active ← session->frame / session->speed;
    if (cycle < active) return session_position_with_layout(session,to,layout,x,y);
    if (cycle > active) return session_position_with_layout(session,from,layout,x,y);
    float fraction ← (float)(session->frame % session->speed) / session->speed;
    fraction ← fraction * fraction * (3 - 2 * fraction);
    float from_x, from_y, to_x, to_y;
    session_position_with_layout(session,from,layout,&from_x,&from_y); session_position_with_layout(session,to,layout,&to_x,&to_y);
    /* An oriented bend separates the two directed paths of a transposition.
     * Cycles move in disjoint time intervals; endpoints remain exact. */
    float bend ← 0.24f * sinf(3.14159265359f * fraction);
    *x ← from_x + (to_x - from_x) * fraction - (to_y - from_y) * bend;
    *y ← from_y + (to_y - from_y) * fraction + (to_x - from_x) * bend;
    return 1;
}
void session_dancer(const mathieu_session *session, unsigned dancer, float *x, float *y) {
    session_dancer_with_layout(session,dancer,NULL,x,y);
}
int session_tap(mathieu_session *session, float x, float y) {
    if (x < 0 || x >= 360 || y < 0 || y >= 720) return 0;
    unsigned column ← (unsigned)x / 90;
    if (y >= 48 && y < 92) {
        if (column < 2) {
            uint64_t tick ← session->logical_tick;
            int success ← session_initialize(session, column ? 24 : 12);
            session->logical_tick ← tick; return success;
        }
        if (column == 2) { session->grid ← session->action.degree == 24 ? (session->grid + 1) % 3 : !session->grid; return 1; }
        session->labels ← !session->labels; return 1;
    }
    if (y >= 100 && y < 144) {
        static const char moves[4] ← {'a','A','b','B'}; return session_move(session,moves[column]);
    }
    if (y >= 474 && y < 518) {
        if (column == 0) { session->playing ← session->playing == 1 ? 2 : 1; return 1; }
        if (column == 1) {
            session->playing ← 0;
            if (session->frame < session->duration) { session->frame ← session->duration; return 1; }
            return session_move(session, (session->next_generator++ & 1) ? 'b' : 'a');
        }
        return column == 2 ? session_undo(session) : session_redo(session);
    }
    if (y >= 526 && y < 570) {
        if (column == 0) session_reset(session);
        if (column == 1) { session->selected ← 0; session->previous_selected ← 0; }
        if (column == 2) session->shapes ← !session->shapes;
        if (column == 3 && session->frame >= session->duration) session->speed ← session->speed == 12 ? 24 : session->speed == 24 ? 1 : 12;
        return 1;
    }
    if (y > 155 && y < 461) {
        for (unsigned point ← 0; point < session->action.degree; ++point) {
            float point_x, point_y; session_position(session, point, &point_x, &point_y);
            float dx ← point_x - x, dy ← point_y - y;
            if (dx * dx + dy * dy < 18 * 18) return session_select(session, point);
        }
    }
    return 0;
}
