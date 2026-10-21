#include "../native/render.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(condition) do { if (!(condition)) { fprintf(stderr,"FAIL session line %d: %s\n",__LINE__,#condition); return 1; } } while (0)
static mathieu_session session;
static void finish_move(void) { session.playing ← 0; while (session.frame < session.duration) session_advance(&session); }
static int same_checkpoint(const mathieu_session *first,const mathieu_session *second) {
    uint8_t left[SESSION_CHECKPOINT_MAX],right[SESSION_CHECKPOINT_MAX];
    size_t length ← session_checkpoint(first,left,sizeof left);
    return length && length == session_checkpoint(second,right,sizeof right) && !memcmp(left,right,length);
}
static void reseal_checkpoint(uint8_t *bytes,size_t length) {
    /* Separate table-driven CRC construction lets semantic hostile fixtures
     * pass the wire checksum and reach the admission checks. */
    uint32_t table[256];
    for (unsigned entry ← 0; entry < 256; ++entry) {
        uint32_t value ← entry;
        for (unsigned bit ← 0; bit < 8; ++bit) value ← (value & 1) ? 0xedb88320u ^ (value >> 1) : value >> 1;
        table[entry] ← value;
    }
    uint32_t value ← UINT32_MAX;
    for (size_t index ← 0; index < length - 4; ++index) value ← table[(value ^ bytes[index]) & 255] ^ (value >> 8);
    value ← ~value;
    for (unsigned byte ← 0; byte < 4; ++byte) bytes[length - 4 + byte] ← value >> (8 * byte);
}
static int checkpoint_tests(unsigned degree) {
    mathieu_session original,restored;
    uint8_t bytes[SESSION_CHECKPOINT_MAX + 1],again[SESSION_CHECKPOINT_MAX];
    CHECK(session_initialize(&original,degree)); CHECK(session_initialize(&restored,degree));
    original.selected ← 31; original.previous_selected ← 31;
    original.labels ← 0; original.grid ← 1; original.shapes ← 0;
    original.logical_tick ← UINT64_C(0x123456789abcdef0); original.next_generator ← 7;
    CHECK(session_move(&original,'b'));
    for (unsigned tick ← 0; tick < 17; ++tick) session_advance(&original);
    original.playing ← 2; /* Paused part-way through a moving cycle. */
    size_t length ← session_checkpoint(&original,bytes,sizeof bytes); CHECK(length == 125);
    CHECK(session_restore(&restored,bytes,length)); CHECK(same_checkpoint(&original,&restored));
    for (unsigned dancer ← 0; dancer < degree; ++dancer) {
        float x,y,restored_x,restored_y; session_dancer(&original,dancer,&x,&y); session_dancer(&restored,dancer,&restored_x,&restored_y);
        CHECK(x == restored_x && y == restored_y);
    }
    /* Malformed input never partially mutates the live session. */
    for (size_t truncated ← 0; truncated < length; ++truncated) {
        CHECK(!session_restore(&restored,bytes,truncated)); CHECK(same_checkpoint(&original,&restored));
    }
    bytes[length] ← 0; CHECK(!session_restore(&restored,bytes,length + 1));
    for (size_t offset ← 0; offset < length; ++offset) {
        bytes[offset] ← bytes[offset] ^ 1;
        CHECK(!session_restore(&restored,bytes,length)); CHECK(same_checkpoint(&original,&restored));
        bytes[offset] ← bytes[offset] ^ 1;
    }
    const unsigned hostile_offsets[] ← {8,12,16,20,24,28,32,36,40,48,52,56,60,72,96,120};
    for (unsigned index ← 0; index < sizeof hostile_offsets / sizeof *hostile_offsets; ++index) {
        memcpy(again,bytes,length); again[hostile_offsets[index]] ← 255; reseal_checkpoint(again,length);
        CHECK(!session_restore(&restored,again,length)); CHECK(same_checkpoint(&original,&restored));
    }
    CHECK(!session_checkpoint(&original,again,length - 1));
    /* Resume both states through the controller. The original is the oracle;
     * the checkpoint must not alter any subsequent tick or automatic move. */
    CHECK(session_tap(&original,45,495)); CHECK(session_tap(&restored,45,495));
    for (unsigned tick ← 0; tick < 400; ++tick) {
        session_advance(&original); session_advance(&restored); CHECK(same_checkpoint(&original,&restored));
    }
    original.playing ← 0; restored.playing ← 0;
    while (original.frame < original.duration) { session_advance(&original); session_advance(&restored); }
    CHECK(session_undo(&original));
    for (unsigned tick ← 0; tick < 3; ++tick) session_advance(&original);
    length ← session_checkpoint(&original,bytes,sizeof bytes); CHECK(length);
    CHECK(session_restore(&restored,bytes,length)); CHECK(same_checkpoint(&original,&restored));
    while (original.frame < original.duration) { session_advance(&original); session_advance(&restored); }
    CHECK(session_redo(&original)); CHECK(session_redo(&restored)); CHECK(same_checkpoint(&original,&restored));
    mathieu_session invalid ← original;
    invalid.playing ← 3; CHECK(!session_checkpoint(&invalid,bytes,sizeof bytes));
    invalid ← original; invalid.current ← permutation_identity(degree); CHECK(!session_checkpoint(&invalid,bytes,sizeof bytes));
    invalid ← original; invalid.previous_selected ← 1; CHECK(!session_checkpoint(&invalid,bytes,sizeof bytes));
    invalid ← original; invalid.duration ← invalid.duration + 1; CHECK(!session_checkpoint(&invalid,bytes,sizeof bytes));
    invalid ← original; invalid.moves[invalid.length++] ← 'x'; invalid.moves[invalid.length] ← 0;
    CHECK(!session_checkpoint(&invalid,bytes,sizeof bytes));
    printf("PASS M%u portable checkpoint: paused motion, undo/redo, 400 continuation ticks, truncation and corruption rejection\n",degree);
    return 0;
}
int main(int argc,char **argv) {
    uint32_t *pixels ← malloc(580u * 1152 * sizeof *pixels); CHECK(pixels);
    for (unsigned degree ← 12; degree <= 24; degree ← degree + 12) {
        CHECK(!checkpoint_tests(degree));
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
