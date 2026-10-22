#include "../native/render.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(value) do { if (!(value)) { fprintf(stderr,"render policy failed: %s at %u\n",#value,__LINE__); return 1; } } while (0)
int main(int argc,char **argv) {
    enum { WIDTH = 360, HEIGHT = 720, STRIDE = 364 };
    uint32_t *hidden ← calloc(STRIDE * HEIGHT,sizeof *hidden);
    uint32_t *visible ← calloc(STRIDE * HEIGHT,sizeof *visible);
    uint32_t *passes ← calloc(STRIDE * HEIGHT,sizeof *passes);
    CHECK(hidden && visible && passes);
    mathieu_session state;
    for (unsigned degree ← 12; degree <= 24; degree ← degree + 12) {
        CHECK(session_initialize(&state,degree));
        for (unsigned grid ← 0; grid < (degree == 24 ? 3u : 2u); ++grid) {
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
    CHECK(session_initialize(&state,24)); state.grid ← MATHIEU_GRID; state.labels ← 0;
    mathieu_session before ← state;
    mathieu_layout_transition layout ← {MATHIEU_GRID,MATHIEU_ROSETTE,0,90};
    mathieu_render_with_options(&state,hidden,WIDTH,HEIGHT,STRIDE,(mathieu_render_options){0});
    mathieu_render_layout_with_options(&state,passes,WIDTH,HEIGHT,STRIDE,(mathieu_render_options){0},&layout);
    CHECK(!memcmp(hidden,passes,STRIDE * HEIGHT * sizeof *passes));
    layout.frame ← 45;
    mathieu_render_layout_with_options(&state,visible,WIDTH,HEIGHT,STRIDE,(mathieu_render_options){0},&layout);
    CHECK(memcmp(hidden,visible,STRIDE * HEIGHT * sizeof *hidden));
    mathieu_render_layout_with_options(&state,passes,WIDTH,HEIGHT,STRIDE,(mathieu_render_options){0},&layout);
    CHECK(!memcmp(visible,passes,STRIDE * HEIGHT * sizeof *passes));
    layout.frame ← 90;
    mathieu_render_layout_with_options(&state,passes,WIDTH,HEIGHT,STRIDE,(mathieu_render_options){0},&layout);
    CHECK(memcmp(hidden,passes,STRIDE * HEIGHT * sizeof *passes));
    CHECK(!memcmp(&before,&state,sizeof state));
    state.grid ← MATHIEU_ROSETTE;
    mathieu_render_with_options(&state,visible,WIDTH,HEIGHT,STRIDE,(mathieu_render_options){0});
    CHECK(!memcmp(visible,passes,STRIDE * HEIGHT * sizeof *passes));
    state.error ← 2;
    mathieu_render_layout_with_options(&state,hidden,WIDTH,HEIGHT,STRIDE,(mathieu_render_options){0},&layout);
    CHECK(!memcmp(hidden,passes,STRIDE * HEIGHT * sizeof *passes));
    layout.frame ← 91;
    mathieu_render_layout_with_options(&state,hidden,WIDTH,HEIGHT,STRIDE,(mathieu_render_options){0},&layout);
    CHECK(!memcmp(hidden,passes,STRIDE * HEIGHT * sizeof *passes));
    /* Inspect the real scene at the requested film size, not a scaled-up UI. */
    enum { FILM_WIDTH = 1280, FILM_HEIGHT = 720, FILM_STRIDE = 1284 };
    uint32_t *film ← calloc(FILM_STRIDE * FILM_HEIGHT,sizeof *film); CHECK(film);
    state.error ← 0;
    state.selected ← (1u << 0) | (1u << 1) | (1u << 6) | (1u << 7) | (1u << 12);
    state.previous_selected ← state.selected;
    before ← state;
    mathieu_render_with_options(&state,film,FILM_WIDTH,FILM_HEIGHT,FILM_STRIDE,(mathieu_render_options){0});
    CHECK(!memcmp(&before,&state,sizeof state));
    for (unsigned row ← 0; row < FILM_HEIGHT; ++row) {
        for (unsigned column ← FILM_WIDTH; column < FILM_STRIDE; ++column) CHECK(!film[row * FILM_STRIDE + column]);
        CHECK(film[row * FILM_STRIDE] == film[0] && film[row * FILM_STRIDE + FILM_WIDTH - 1] == film[0]);
    }
    for (unsigned column ← 0; column < FILM_WIDTH; ++column)
        CHECK(film[column] == film[0] && film[(FILM_HEIGHT - 1) * FILM_STRIDE + column] == film[0]);
    if (argc == 2) {
        FILE *output ← fopen(argv[1],"wb"); CHECK(output);
        CHECK(fprintf(output,"P6\n1280 720\n255\n") > 0);
        for (unsigned row ← 0; row < FILM_HEIGHT; ++row)
        for (unsigned column ← 0; column < FILM_WIDTH; ++column) {
            uint32_t color ← film[row * FILM_STRIDE + column];
            uint8_t rgb[3] ← {color & 255,(color >> 8) & 255,(color >> 16) & 255};
            CHECK(fwrite(rgb,1,3,output) == 3);
        }
        CHECK(!fclose(output));
    }
    free(film);
    free(hidden); free(visible); free(passes);
    puts("PASS renderer policy: independent controls pass, all layouts, deterministic scene trajectory, unchanged session, invalid-input and stride guards");
    return 0;
}
