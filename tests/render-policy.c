#include "../native/render.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(value) do { if (!(value)) { fprintf(stderr,"render policy failed: %s at %u\n",#value,__LINE__); return 1; } } while (0)
int main(void) {
    enum { WIDTH = 360, HEIGHT = 720, STRIDE = 364 };
    uint32_t *hidden ← calloc(STRIDE * HEIGHT,sizeof *hidden);
    uint32_t *visible ← calloc(STRIDE * HEIGHT,sizeof *visible);
    uint32_t *passes ← calloc(STRIDE * HEIGHT,sizeof *passes);
    CHECK(hidden && visible && passes);
    mathieu_session state;
    for (unsigned degree ← 12; degree <= 24; degree ← degree + 12) {
        CHECK(session_initialize(&state,degree));
        for (unsigned grid ← 0; grid < 2; ++grid) {
            state.grid ← grid;
            mathieu_session before ← state;
            mathieu_render_with_options(&state,hidden,WIDTH,HEIGHT,STRIDE,(mathieu_render_options){0});
            CHECK(!memcmp(&state,&before,sizeof state));
            mathieu_render(&state,visible,WIDTH,HEIGHT,STRIDE);
            mathieu_render_scene(&state,passes,WIDTH,HEIGHT,STRIDE);
            mathieu_render_controls(&state,passes,WIDTH,HEIGHT,STRIDE);
            CHECK(!memcmp(visible,passes,STRIDE * HEIGHT * sizeof *visible));
            CHECK(memcmp(hidden,visible,STRIDE * HEIGHT * sizeof *visible));
            /* Known controls pixel is paper when the pass is disabled. */
            CHECK(hidden[60 * STRIDE + 15] == hidden[0]);
            CHECK(visible[60 * STRIDE + 15] != visible[0]);
            state.error ← 2;
            mathieu_render_with_options(&state,passes,WIDTH,HEIGHT,STRIDE,(mathieu_render_options){0});
            CHECK(!memcmp(hidden,passes,STRIDE * HEIGHT * sizeof *hidden));
            mathieu_render(&state,passes,WIDTH,HEIGHT,STRIDE);
            CHECK(memcmp(visible,passes,STRIDE * HEIGHT * sizeof *visible));
            state.error ← 0;
            CHECK(session_move(&state,'b'));
            for (unsigned tick ← 0; tick < 5; ++tick) session_advance(&state);
            mathieu_render_with_options(&state,passes,WIDTH,HEIGHT,STRIDE,(mathieu_render_options){0});
            CHECK(memcmp(hidden,passes,STRIDE * HEIGHT * sizeof *hidden));
            for (unsigned row ← 0; row < HEIGHT; ++row) for (unsigned column ← WIDTH; column < STRIDE; ++column)
                CHECK(!hidden[row * STRIDE + column] && !passes[row * STRIDE + column]);
            session_reset(&state);
        }
    }
    free(hidden); free(visible); free(passes);
    puts("PASS renderer policy: independent controls pass, fitted scene, unchanged session, both degrees/layouts and stride guards");
    return 0;
}
