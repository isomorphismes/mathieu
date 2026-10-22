#include "render.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
/* 0xRRGGBB to the existing little-endian RGBA8888 framebuffer. */
#define RGB(value) (0xff000000u | (((value) >> 16) & 255u) | ((value) & 0x00ff00u) | (((value) & 255u) << 16))
static const uint32_t paper_color ← RGB(0xf3eedf), ink_color ← RGB(0x29211f);
static const uint32_t quiet_ink_color ← RGB(0x857d6c), slot_color ← RGB(0xd8d1bf);
static const uint32_t button_color ← RGB(0xe7e0cc), active_color ← RGB(0xccc591);
static const uint32_t block_color ← RGB(0x798e87), selected_color ← RGB(0xc27d38);
typedef struct { uint32_t *pixels; unsigned width,height,stride; float scale,offset_x,offset_y; } canvas;
static void pixel(canvas *surface, int x, int y, uint32_t color) {
    if (x >= 0 && y >= 0 && (unsigned)x < surface->width && (unsigned)y < surface->height) surface->pixels[y * surface->stride + x] ← color;
}
static void rectangle(canvas *surface, float x, float y, float width, float height, uint32_t color) {
    int left ← surface->offset_x + x * surface->scale, top ← surface->offset_y + y * surface->scale;
    int right ← surface->offset_x + (x + width) * surface->scale, bottom ← surface->offset_y + (y + height) * surface->scale;
    for (int row ← top; row < bottom; ++row) for (int column ← left; column < right; ++column) pixel(surface,column,row,color);
}
static void dot(canvas *surface, float x, float y, float radius, unsigned shape, uint32_t color) {
    int center_x ← surface->offset_x + x * surface->scale, center_y ← surface->offset_y + y * surface->scale;
    int extent ← ceilf(radius * surface->scale);
    for (int row ← -extent; row <= extent; ++row) for (int column ← -extent; column <= extent; ++column) {
        float dx ← column / surface->scale, dy ← row / surface->scale;
        int inside ← shape == 1 ? fabsf(dx) <= radius * 0.85f && fabsf(dy) <= radius * 0.85f :
            shape == 2 ? fabsf(dx) + fabsf(dy) <= radius * 1.2f :
            shape == 3 ? dy >= -radius && dy <= radius && fabsf(dx) <= (dy + radius) * 0.6f :
            shape == 4 ? fabsf(dx) <= radius && fabsf(dy) <= radius * .87f && .5f * fabsf(dx) + .866f * fabsf(dy) <= radius * .866f :
            shape == 5 ? dy >= -radius * .85f && dy <= radius && fabsf(dx) <= radius * .88f * (dy < 0 ? 1 : 1 - dy / radius) :
            dx * dx + dy * dy <= radius * radius;
        if (inside) pixel(surface,center_x + column,center_y + row,color);
    }
}
static void ring(canvas *surface,float x,float y,float radius,float thickness,uint32_t color) {
    int center_x ← surface->offset_x + x * surface->scale, center_y ← surface->offset_y + y * surface->scale;
    int extent ← ceilf((radius + thickness) * surface->scale);
    float inner ← (radius - thickness) * (radius - thickness), outer ← (radius + thickness) * (radius + thickness);
    for (int row ← -extent; row <= extent; ++row) for (int column ← -extent; column <= extent; ++column) {
        float dx ← column / surface->scale, dy ← row / surface->scale, squared ← dx * dx + dy * dy;
        if (squared >= inner && squared <= outer) pixel(surface,center_x + column,center_y + row,color);
    }
}
static void line(canvas *surface,float x,float y,float end_x,float end_y,uint32_t color) {
    float dx ← end_x - x,dy ← end_y - y; unsigned steps ← ceilf(hypotf(dx,dy) * surface->scale);
    for (unsigned step ← 0; step <= steps; ++step) {
        float fraction ← steps ? (float)step / steps : 0;
        dot(surface,x + dx * fraction,y + dy * fraction,0.55f,0,color);
    }
}
/* Original compact 5×7 display alphabet; rows are five-bit masks. */
static const char alphabet[] ← "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ() -:/+>";
static const uint8_t glyphs[][7] ← {
 {14,17,19,21,25,17,14},{4,12,4,4,4,4,14},{14,17,1,2,4,8,31},{30,1,1,14,1,1,30},{2,6,10,18,31,2,2},
 {31,16,16,30,1,1,30},{14,16,16,30,17,17,14},{31,1,2,4,8,8,8},{14,17,17,14,17,17,14},{14,17,17,15,1,1,14},
 {14,17,17,31,17,17,17},{30,17,17,30,17,17,30},{14,17,16,16,16,17,14},{30,17,17,17,17,17,30},
 {31,16,16,30,16,16,31},{31,16,16,30,16,16,16},{14,17,16,23,17,17,15},{17,17,17,31,17,17,17},
 {14,4,4,4,4,4,14},{7,2,2,2,2,18,12},{17,18,20,24,20,18,17},{16,16,16,16,16,16,31},
 {17,27,21,21,17,17,17},{17,25,21,19,17,17,17},{14,17,17,17,17,17,14},{30,17,17,30,16,16,16},
 {14,17,17,17,21,18,13},{30,17,17,30,20,18,17},{15,16,16,14,1,1,30},{31,4,4,4,4,4,4},
 {17,17,17,17,17,17,14},{17,17,17,17,17,10,4},{17,17,17,21,21,27,17},{17,17,10,4,10,17,17},
 {17,17,10,4,4,4,4},{31,1,2,4,8,16,31},{2,4,8,8,8,4,2},{8,4,2,2,2,4,8},
 {0,0,0,0,0,0,0},{0,0,0,31,0,0,0},{0,4,4,0,4,4,0},{1,2,2,4,8,8,16},{0,4,4,31,4,4,0},{16,8,4,2,4,8,16}
};
static void text(canvas *surface,float x,float y,float size,const char *value,unsigned limit,uint32_t color) {
    float start ← x; unsigned count ← 0;
    for (; *value && count < limit; ++value,++count) {
        char character ← *value; if (character >= 'a' && character <= 'z') character ← character - 'a' + 'A';
        if (character == '\n' || x + 5 * size > 353) { x ← start; y ← y + 9 * size; if (character == '\n') continue; }
        const char *found ← strchr(alphabet,character);
        if (found) {
            unsigned index ← found - alphabet;
            for (unsigned row ← 0; row < 7; ++row) for (unsigned column ← 0; column < 5; ++column)
                if (glyphs[index][row] & (1u << (4 - column))) rectangle(surface,x + column * size,y + row * size,size,size,color);
        }
        x ← x + 6 * size;
    }
}
/* Small 3×5 digits never compete with the suit glyphs. */
static void tiny_number(canvas *surface,float x,float y,unsigned number,uint32_t color) {
    static const uint8_t digits[10][5] ← {
        {7,5,5,5,7},{2,6,2,2,7},{7,1,7,4,7},{7,1,7,1,7},{5,5,7,1,1},
        {7,4,7,1,7},{7,4,7,5,7},{7,1,1,1,1},{7,5,7,5,7},{7,5,7,1,7}
    };
    unsigned count ← number >= 10 ? 2 : 1;
    for (unsigned place ← 0; place < count; ++place) {
        unsigned digit ← count == 2 && !place ? number / 10 : number % 10;
        for (unsigned row ← 0; row < 5; ++row) for (unsigned column ← 0; column < 3; ++column)
            if (digits[digit][row] & (1u << (2 - column))) rectangle(surface,x + (place * 4 + column) * .6f,y + row * .6f,.6f,.6f,color);
    }
}
static uint32_t dancer_color(unsigned dancer,unsigned degree,int suits) {
    /* Moonrise2, karthik/wesanderson: Wands, Cups, Swords, Pentacles. */
    static const uint32_t palette[4] ← {RGB(0xc27d38),RGB(0x798e87),RGB(0x29211f),RGB(0xccc591)};
    unsigned forms ← degree / 4, suit ← dancer / forms;
    uint32_t color ← palette[suit];
    if (suits) return color;
    /* The optional dot view retains distinct colors for all labeled dancers. */
    unsigned tint ← (dancer % forms) * 11, red ← color & 255, green ← (color >> 8) & 255, blue ← (color >> 16) & 255;
    red ← (red * (100 - tint) + 243 * tint) / 100;
    green ← (green * (100 - tint) + 238 * tint) / 100;
    blue ← (blue * (100 - tint) + 223 * tint) / 100;
    return 0xff000000u | red | (green << 8) | (blue << 16);
}
static void tarot_suit(canvas *surface,unsigned suit,uint32_t color) {
    if (suit == 0) {
        line(surface,-2.2f,3.7f,2.2f,-3.7f,color);
        line(surface,-.7f,1.2f,-2.5f,.1f,color); line(surface,.6f,-1,2.5f,-.3f,color);
        dot(surface,2.2f,-3.7f,.8f,0,color);
    } else if (suit == 1) {
        line(surface,-2.7f,-2.8f,2.7f,-2.8f,color);
        line(surface,-2.7f,-2.8f,-2,0,color); line(surface,2.7f,-2.8f,2,0,color);
        line(surface,-2,0,0,1.4f,color); line(surface,2,0,0,1.4f,color);
        line(surface,0,1.4f,0,3.6f,color); line(surface,-2,3.6f,2,3.6f,color);
    } else if (suit == 2) {
        line(surface,0,-4,0,3.7f,color);
        line(surface,0,-4,-1.3f,-2.2f,color); line(surface,0,-4,1.3f,-2.2f,color);
        line(surface,-2.6f,1.3f,2.6f,1.3f,color); line(surface,-1,3.7f,1,3.7f,color);
    } else {
        ring(surface,0,0,3.9f,.35f,color);
        for (unsigned point ← 0; point < 5; ++point) {
            float start ← -1.57079632679f + point * 1.25663706144f, end ← start + 2.51327412287f;
            line(surface,2.8f * cosf(start),2.8f * sinf(start),2.8f * cosf(end),2.8f * sinf(end),color);
        }
    }
}
static void dancer_marker(canvas *surface,float x,float y,unsigned dancer,unsigned degree,int suits,int labels) {
    /* Integer pixel anchor makes a dancer's entire marker transport unchanged. */
    canvas marker ← *surface;
    marker.offset_x ← (int)(surface->offset_x + x * surface->scale);
    marker.offset_y ← (int)(surface->offset_y + y * surface->scale);
    unsigned forms ← degree / 4, suit ← dancer / forms, form ← dancer % forms;
    dot(&marker,0,0,8,suits ? form : 0,dancer_color(dancer,degree,suits));
    if (suits) tarot_suit(&marker,suit,suit == 2 ? paper_color : ink_color);
    if (labels) tiny_number(&marker,8.5f,5.5f,dancer + 1,quiet_ink_color);
}
static void button(canvas *surface,unsigned column,float y,const char *label,int active) {
    rectangle(surface,column * 90 + 3,y,84,44,active ? active_color : button_color);
    text(surface,column * 90 + 10,y + 16,1.4f,label,9,ink_color);
}
static canvas interactive_canvas(uint32_t *pixels,unsigned width,unsigned height,unsigned stride) {
    canvas surface ← {pixels,width,height,stride, fminf(width / 360.0f,height / 720.0f),0,0};
    surface.offset_x ← (width - surface.scale * 360) * 0.5f; surface.offset_y ← (height - surface.scale * 720) * 0.5f;
    return surface;
}
void mathieu_render_controls(const mathieu_session *session,uint32_t *pixels,unsigned width,unsigned height,unsigned stride) {
    if (!pixels || !width || !height || stride < width) return;
    canvas surface ← interactive_canvas(pixels,width,height,stride);
    char caption[512]; snprintf(caption,sizeof caption,"M%u  EXACT PERMUTATION DANCE",session->action.degree);
    text(&surface,10,17,1.5f,caption,40,ink_color);
    button(&surface,0,48,"M12",session->action.degree == 12); button(&surface,1,48,"M24",session->action.degree == 24);
    button(&surface,2,48,!session->grid ? "GRID" : session->grid == MATHIEU_GRID && session->action.degree == 24 ? "ROSETTE" : "CIRCLE",session->grid); button(&surface,3,48,"LABELS",session->labels);
    button(&surface,0,100,"A",0); button(&surface,1,100,"A INV",0); button(&surface,2,100,"B",0); button(&surface,3,100,"B INV",0);
    button(&surface,0,474,session->playing == 1 ? "PAUSE" : "PLAY",session->playing == 1);
    button(&surface,1,474,"STEP",0); button(&surface,2,474,"UNDO",0); button(&surface,3,474,"REDO",0);
    button(&surface,0,526,"RESET",0); button(&surface,1,526,"CLEAR 5",0); button(&surface,2,526,"SUITS",session->shapes);
    button(&surface,3,526,session->speed == 12 ? "SPEED 1" : session->speed == 24 ? "SPEED 1/2" : "STATIC",session->speed == 1);
    snprintf(caption,sizeof caption,"WORD %u/%u: ",session->cursor,session->length);
    text(&surface,8,583,1.2f,caption,40,ink_color);
    unsigned start ← session->cursor > 40 ? session->cursor - 40 : 0;
    for (unsigned index ← start; index < session->cursor; ++index) {
        snprintf(caption,sizeof caption,"%c%s",session->moves[index],session->moves[index] >= 'A' && session->moves[index] <= 'Z' ? "-" : "+");
        text(&surface,8 + (index - start) * 8.5f,600,0.7f,caption,2,block_color);
    }
    permutation_cycles(session->current,caption,sizeof caption); text(&surface,8,619,1.1f,caption,150,ink_color);
    snprintf(caption,sizeof caption,"SELECT 5: %u/5  %s",subset_size(session->selected),subset_size(session->selected) == 5 ? (session->action.degree == 12 ? "HEXAD" : "OCTAD") : "TAP POSITIONS");
    text(&surface,8,681,1.1f,caption,49,quiet_ink_color);
    if (session->error) text(&surface,8,701,1,session->error == 1 ? "HISTORY FULL - RESET OR UNDO" : session->error == 2 ? "SAVE FAILED - CHECK FREE STORAGE" : "FRAME FAILED - REOPEN APP",40,selected_color);
}
static void draw_scene(const mathieu_session *session,canvas surface,const mathieu_layout_transition *layout) {
    uint32_t selected ← session->frame < session->duration ? session->previous_selected : session->selected;
    uint32_t block ← 0; if (subset_size(selected) == 5) mathieu_block(&session->action,selected,&block);
    for (unsigned point ← 0; point < session->action.degree; ++point) {
        float x,y; session_position_with_layout(session,point,layout,&x,&y); ring(&surface,x,y,2,.4f,slot_color);
        if (session->labels) tiny_number(&surface,x - (point < 9 ? .9f : 2.1f),y - 14,point + 1,quiet_ink_color);
    }
    /* Arrows depict only the currently moving cycle, with their exact endpoints. */
    if (session->frame < session->duration) {
        for (unsigned dancer ← 0; dancer < session->action.degree; ++dancer) {
            float x,y,from_x,from_y,to_x,to_y;
            session_dancer_with_layout(session,dancer,layout,&x,&y);
            session_position_with_layout(session,session->previous.image[dancer],layout,&from_x,&from_y);
            session_position_with_layout(session,session->current.image[dancer],layout,&to_x,&to_y);
            if (hypotf(x - from_x,y - from_y) < 0.1f || hypotf(x - to_x,y - to_y) < 0.1f) continue;
            line(&surface,from_x,from_y,to_x,to_y,block_color);
            float angle ← atan2f(to_y - from_y,to_x - from_x);
            line(&surface,to_x,to_y,to_x - 8 * cosf(angle - .5f),to_y - 8 * sinf(angle - .5f),block_color);
            line(&surface,to_x,to_y,to_x - 8 * cosf(angle + .5f),to_y - 8 * sinf(angle + .5f),block_color);
        }
    }
    for (unsigned dancer ← 0; dancer < session->action.degree; ++dancer) {
        unsigned position ← session->frame < session->duration ? session->previous.image[dancer] : session->current.image[dancer];
        float x,y; session_dancer_with_layout(session,dancer,layout,&x,&y);
        if (block & (1u << position)) ring(&surface,x,y,12.5f,.6f,block_color);
        if (selected & (1u << position)) ring(&surface,x,y,10.5f,.7f,selected_color);
        dancer_marker(&surface,x,y,dancer,session->action.degree,session->shapes,session->labels);
    }
}
static void clear_scene(canvas surface) {
    for (unsigned row ← 0; row < surface.height; ++row) for (unsigned column ← 0; column < surface.width; ++column)
        surface.pixels[row * surface.stride + column] ← paper_color;
}
void mathieu_render_scene(const mathieu_session *session,uint32_t *pixels,unsigned width,unsigned height,unsigned stride) {
    if (!pixels || !width || !height || stride < width || !session_layout_transition_valid(session,NULL)) return;
    canvas surface ← interactive_canvas(pixels,width,height,stride);
    clear_scene(surface); draw_scene(session,surface,NULL);
}
void mathieu_render_with_options(const mathieu_session *session,uint32_t *pixels,unsigned width,unsigned height,unsigned stride,mathieu_render_options options) {
    mathieu_render_layout_with_options(session,pixels,width,height,stride,options,NULL);
}
void mathieu_render_layout_with_options(const mathieu_session *session,uint32_t *pixels,unsigned width,unsigned height,unsigned stride,mathieu_render_options options,const mathieu_layout_transition *layout) {
    if (!pixels || !width || !height || stride < width || !session_layout_transition_valid(session,layout)) return;
    if (options.controls_visible) {
        canvas surface ← interactive_canvas(pixels,width,height,stride);
        clear_scene(surface); draw_scene(session,surface,layout);
        mathieu_render_controls(session,pixels,width,height,stride);
    } else {
        /* Bounds include all layouts, marker extents, labels and curved swaps.
         * Fit the whole scene at source; no framebuffer crop or UI erasure. */
        canvas surface ← {pixels,width,height,stride,fminf(width / 340.0f,height / 340.0f),0,0};
        surface.offset_x ← width * .5f - 180 * surface.scale;
        surface.offset_y ← height * .5f - 308 * surface.scale;
        clear_scene(surface); draw_scene(session,surface,layout);
    }
}
void mathieu_render(const mathieu_session *session,uint32_t *pixels,unsigned width,unsigned height,unsigned stride) {
    mathieu_render_with_options(session,pixels,width,height,stride,(mathieu_render_options){1});
}
