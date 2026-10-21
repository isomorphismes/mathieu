#include "../native/render.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(condition) do { if (!(condition)) { fprintf(stderr,"FAIL session line %d: %s\n",__LINE__,#condition); return 1; } } while (0)
static mathieu_session session;
static void finish_move(void) { session.playing ← 0; while (session.frame < session.duration) session_advance(&session); }
enum { MARKER_PATCH ← 31 };
static void copy_marker_patch(uint32_t *destination,const uint32_t *pixels,const mathieu_session *dance,unsigned point) {
    float x,y; session_position(dance,point,&x,&y);
    unsigned left ← (unsigned)(x * 1.6f) - MARKER_PATCH / 2, top ← (unsigned)(y * 1.6f) - MARKER_PATCH / 2;
    for (unsigned row ← 0; row < MARKER_PATCH; ++row)
        memcpy(destination + row * MARKER_PATCH,pixels + (top + row) * 580 + left,MARKER_PATCH * sizeof *pixels);
}
static int suit_marker_tests(unsigned degree,unsigned layout,uint32_t *pixels) {
    mathieu_session dance; CHECK(session_initialize(&dance,degree));
    dance.labels ← 0; dance.shapes ← 1; dance.grid ← layout;
    uint32_t original[24][MARKER_PATCH * MARKER_PATCH],moved[MARKER_PATCH * MARKER_PATCH];
    mathieu_render(&dance,pixels,576,1152,580);
    for (unsigned dancer ← 0; dancer < degree; ++dancer) {
        copy_marker_patch(original[dancer],pixels,&dance,dancer);
        for (unsigned earlier ← 0; earlier < dancer; ++earlier)
            CHECK(memcmp(original[earlier],original[dancer],sizeof moved));
    }
    CHECK(session_move(&dance,'b'));
    while (dance.frame < dance.duration) session_advance(&dance);
    mathieu_render(&dance,pixels,576,1152,580);
    for (unsigned dancer ← 0; dancer < degree; ++dancer) {
        copy_marker_patch(moved,pixels,&dance,dance.action.generator[1].image[dancer]);
        CHECK(!memcmp(moved,original[dancer],sizeof moved));
    }
    return 0;
}
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
static int rosette_tests(void) {
    mathieu_session dance,restored; CHECK(session_initialize(&dance,24));
    CHECK(session_tap(&dance,225,70)); CHECK(dance.grid == MATHIEU_GRID);
    CHECK(session_tap(&dance,225,70)); CHECK(dance.grid == MATHIEU_ROSETTE);
    CHECK(session_tap(&dance,225,70)); CHECK(dance.grid == MATHIEU_CIRCLE);
    dance.grid ← MATHIEU_ROSETTE;
    uint8_t saved[SESSION_CHECKPOINT_MAX];
    size_t length ← session_checkpoint(&dance,saved,sizeof saved); CHECK(length);
    CHECK(session_restore(&restored,saved,length)); CHECK(same_checkpoint(&dance,&restored));
    /* Grid checkpoints keep their original MATHS02 bytes and meaning. */
    restored.grid ← MATHIEU_GRID;
    length ← session_checkpoint(&restored,saved,sizeof saved); CHECK(length);
    CHECK(!memcmp(saved,"MATHS02",8)); CHECK(session_restore(&dance,saved,length));
    CHECK(dance.grid == MATHIEU_GRID);
    /* Five exact coordinates complete T0 union T1, then the same dancers carry
     * it through the real generator word and both site layouts. */
    const unsigned five[] ← {0,1,6,7,12};
    for (unsigned index ← 0; index < 5; ++index) CHECK(session_select(&dance,five[index]));
    uint32_t octad; CHECK(mathieu_block(&dance.action,dance.selected,&octad)); CHECK(octad == 0x0c30c3u);
    mathieu_session before ← dance;
    mathieu_layout_transition layout ← {MATHIEU_GRID,MATHIEU_ROSETTE,0,90};
    for (unsigned frame ← 0; frame <= 90; ++frame) {
        layout.frame ← frame;
        for (unsigned point ← 0; point < 24; ++point) {
            float x,y,again_x,again_y,grid_x,grid_y,rosette_x,rosette_y;
            CHECK(session_position_with_layout(&dance,point,&layout,&x,&y));
            CHECK(session_position_with_layout(&dance,point,&layout,&again_x,&again_y));
            CHECK(x == again_x && y == again_y);
            session_position(&dance,point,&grid_x,&grid_y);
            restored ← dance; restored.grid ← MATHIEU_ROSETTE;
            session_position(&restored,point,&rosette_x,&rosette_y);
            if (!frame) CHECK(x == grid_x && y == grid_y);
            if (frame == 90) CHECK(x == rosette_x && y == rosette_y);
            float fraction ← (float)frame / 90; fraction ← fraction * fraction * (3 - 2 * fraction);
            CHECK(fabsf(x - (grid_x + (rosette_x - grid_x) * fraction)) < .0001f);
            CHECK(fabsf(y - (grid_y + (rosette_y - grid_y) * fraction)) < .0001f);
            CHECK(x >= 28 && x <= 332 && y >= 156 && y <= 460);
        }
        CHECK(!memcmp(&before,&dance,sizeof dance));
    }
    dance.grid ← MATHIEU_ROSETTE;
    /* Independent expected hexagon centers catch a silent column relabeling. */
    static const float center_x[6] ← {180,270.93267f,270.93267f,180,89.06733f,89.06733f};
    static const float center_y[6] ← {203,255.5f,360.5f,413,360.5f,255.5f};
    for (unsigned column ← 0; column < 6; ++column) {
        float north_x,north_y,east_x,east_y,south_x,south_y,west_x,west_y;
        session_position(&dance,column,&north_x,&north_y); session_position(&dance,column + 6,&east_x,&east_y);
        session_position(&dance,column + 12,&south_x,&south_y); session_position(&dance,column + 18,&west_x,&west_y);
        CHECK(north_x == south_x && east_y == west_y);
        CHECK(fabsf(south_y - north_y - 48) < .0001f && fabsf(east_x - west_x - 48) < .0001f);
        CHECK(fabsf(north_x - (east_x + west_x) * .5f) < .0001f);
        CHECK(fabsf(hypotf(north_x - 180,east_y - 308) - 105) < .0001f);
        CHECK(fabsf(north_x - center_x[column]) < .0001f && fabsf(east_y - center_y[column]) < .0001f);
    }
    const char *word ← "abAB"; permutation expected;
    for (unsigned index ← 0; word[index]; ++index) {
        uint32_t old_block; CHECK(mathieu_block(&dance.action,dance.selected,&old_block));
        CHECK(session_move(&dance,word[index]));
        uint32_t moved; CHECK(mathieu_block(&dance.action,dance.selected,&moved));
        CHECK(moved == permutation_subset(dance.motion,old_block));
        while (dance.frame < dance.duration) {
            for (unsigned dancer ← 0; dancer < 24; ++dancer) {
                float x,y; CHECK(session_dancer_with_layout(&dance,dancer,&layout,&x,&y));
                CHECK(x > 23 && x < 337 && y > 151 && y < 465);
            }
            session_advance(&dance);
        }
        for (unsigned dancer ← 0; dancer < 24; ++dancer) {
            float x,y,site_x,site_y;
            session_dancer(&dance,dancer,&x,&y); session_position(&dance,dance.current.image[dancer],&site_x,&site_y);
            CHECK(x == site_x && y == site_y);
        }
    }
    CHECK(permutation_word(&dance.action,word,&expected)); CHECK(permutation_equal(dance.current,expected));
    CHECK(dance.selected == permutation_subset(expected,before.selected));
    /* Validation rejects malformed sampling rather than indexing invalid sites. */
    before ← dance; float x ← -1,y ← -2;
    layout.frame ← 91; CHECK(!session_position_with_layout(&dance,0,&layout,&x,&y)); CHECK(x == -1 && y == -2);
    layout.frame ← 0; layout.from ← 3; CHECK(!session_dancer_with_layout(&dance,0,&layout,&x,&y));
    layout.from ← MATHIEU_GRID; layout.duration ← 0;
    CHECK(session_position_with_layout(&dance,0,&layout,&x,&y));
    float site_x,site_y; session_position(&dance,0,&site_x,&site_y); CHECK(x == site_x && y == site_y);
    CHECK(!session_position_with_layout(&dance,24,&layout,&x,&y));
    CHECK(!memcmp(&before,&dance,sizeof dance));
    CHECK(session_initialize(&restored,12)); CHECK(!session_layout_transition_valid(&restored,&layout));
    puts("PASS rosette: six exact tetrad diamonds, deterministic 90-frame geometry, immutable group state, abAB octad transport, checkpoints and invalid-input rejection");
    return 0;
}
int main(int argc,char **argv) {
    uint32_t *pixels ← malloc(580u * 1152 * sizeof *pixels); CHECK(pixels);
    CHECK(!rosette_tests());
    for (unsigned degree ← 12; degree <= 24; degree ← degree + 12) {
        CHECK(!checkpoint_tests(degree));
        CHECK(session_initialize(&session,degree)); permutation identity ← session.current;
        CHECK(!session_tap(&session,-1,0)); CHECK(!session_tap(&session,361,0));
        for (unsigned layout ← 0; layout < (degree == 24 ? 3u : 2u); ++layout) {
            CHECK(!suit_marker_tests(degree,layout,pixels));
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
                CHECK(pixels[center_y * 580 + center_x] != pixels[0]);
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
        printf("PASS M%u touch → exact state → cycle-staged dancer endpoints; distinct suit/form markers transport pixel-for-pixel; selection equivariance, pause, undo/redo/reset and all layouts; 576x1152 raster stride guards\n",degree);
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
