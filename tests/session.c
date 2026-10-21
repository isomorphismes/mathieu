#include "../native/render.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(condition) do { if (!(condition)) { fprintf(stderr,"FAIL session line %d: %s\n",__LINE__,#condition); return 1; } } while (0)
static mathieu_session session;
static void finish_move(void) { session.playing ← 0; while (session.frame < session.duration) session_advance(&session); }
int main(int argc,char **argv) {
    uint32_t *pixels ← malloc(580u * 1152 * sizeof *pixels); CHECK(pixels);
    for (unsigned degree ← 12; degree <= 24; degree ← degree + 12) {
        CHECK(session_initialize(&session,degree)); permutation identity ← session.current;
        CHECK(!session_tap(&session,-1,0)); CHECK(!session_tap(&session,361,0));
        for (unsigned layout ← 0; layout < 2; ++layout) {
            session.grid ← layout; session_reset(&session);
            for (unsigned point ← 0; point < 5; ++point) {
                float x,y; session_position(&session,point,&x,&y); CHECK(session_tap(&session,x,y));
            }
            CHECK(session.selected == 31); CHECK(!session_select(&session,5));
            CHECK(session_tap(&session,225,120)); /* actual B button hit test */
            CHECK(permutation_equal(session.current,session.action.generator[1]));
            CHECK(session.selected == permutation_subset(session.action.generator[1],31));
            uint32_t original_block, moved_block;
            CHECK(mathieu_block(&session.action,31,&original_block)); CHECK(mathieu_block(&session.action,session.selected,&moved_block));
            CHECK(moved_block == permutation_subset(session.action.generator[1],original_block));
            session.playing ← 2; unsigned frame ← session.frame; session_advance(&session); CHECK(session.frame == frame);
            session.playing ← 0; finish_move();
            for (unsigned dancer ← 0; dancer < degree; ++dancer) {
                float x,y,expected_x,expected_y; session_dancer(&session,dancer,&x,&y);
                session_position(&session,session.action.generator[1].image[dancer],&expected_x,&expected_y);
                CHECK(fabsf(x - expected_x) < .001f && fabsf(y - expected_y) < .001f);
            }
            CHECK(session_tap(&session,225,495)); finish_move(); CHECK(permutation_equal(session.current,identity)); CHECK(session.selected == 31);
            CHECK(session_tap(&session,315,495)); finish_move(); CHECK(permutation_equal(session.current,session.action.generator[1]));
            CHECK(session_undo(&session)); finish_move(); CHECK(session_move(&session,'a')); finish_move(); CHECK(!session_redo(&session));
            for (unsigned move ← 0; move < 40; ++move) { CHECK(session_move(&session,(move & 1) ? 'b' : 'A')); finish_move(); }
            while (session.cursor) { CHECK(session_undo(&session)); finish_move(); }
            CHECK(permutation_equal(session.current,identity)); CHECK(session.selected == 31);
            session.labels ← 0; session.shapes ← 0;
            for (unsigned pixel ← 0; pixel < 580u * 1152; ++pixel) pixels[pixel] ← 0x12345678;
            mathieu_render(&session,pixels,576,1152,580);
            for (unsigned row ← 0; row < 1152; ++row) CHECK(pixels[row * 580 + 576] == 0x12345678);
            for (unsigned dancer ← 0; dancer < degree; ++dancer) {
                float x,y; session_dancer(&session,dancer,&x,&y);
                unsigned center_x ← x * 1.6f,center_y ← y * 1.6f;
                CHECK(pixels[center_y * 580 + center_x] != 0xff181410);
            }
            uint32_t original_colors[24];
            for (unsigned dancer ← 0; dancer < degree; ++dancer) {
                float x,y; session_dancer(&session,dancer,&x,&y);
                original_colors[dancer] ← pixels[(unsigned)(y * 1.6f) * 580 + (unsigned)(x * 1.6f)];
                for (unsigned earlier ← 0; earlier < dancer; ++earlier) CHECK(original_colors[earlier] != original_colors[dancer]);
            }
            CHECK(session_move(&session,'b')); finish_move(); mathieu_render(&session,pixels,576,1152,580);
            for (unsigned dancer ← 0; dancer < degree; ++dancer) {
                float x,y; session_position(&session,session.action.generator[1].image[dancer],&x,&y);
                CHECK(pixels[(unsigned)(y * 1.6f) * 580 + (unsigned)(x * 1.6f)] == original_colors[dancer]);
            }
            CHECK(session_tap(&session,45,548)); CHECK(session.cursor == 0 && session.selected == 0);
        }
        printf("PASS M%u touch → exact state → cycle-staged dancer endpoints; selection equivariance, pause, undo/redo/reset and both layouts; 576x1152 raster stride guards\n",degree);
    }
    if (argc == 2) {
        CHECK(session_initialize(&session,24)); session.selected ← 31;
        CHECK(session_move(&session,'b')); finish_move();
        mathieu_render(&session,pixels,576,1152,580);
        FILE *frame ← fopen(argv[1],"wb"); CHECK(frame);
        CHECK(fprintf(frame,"P6\n576 1152\n255\n") > 0);
        for (unsigned row ← 0; row < 1152; ++row) for (unsigned column ← 0; column < 576; ++column) {
            uint32_t color ← pixels[row * 580 + column];
            uint8_t rgb[3] ← {color & 255,(color >> 8) & 255,(color >> 16) & 255}; CHECK(fwrite(rgb,1,3,frame) == 3);
        }
        CHECK(!fclose(frame));
    }
    free(pixels); return 0;
}
