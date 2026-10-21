#include "render.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
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
            shape == 3 ? dy >= -radius && dy <= radius && fabsf(dx) <= (dy + radius) * 0.6f : dx * dx + dy * dy <= radius * radius;
        if (inside) pixel(surface,center_x + column,center_y + row,color);
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
static uint32_t dancer_color(unsigned dancer,unsigned degree) {
    float hue ← 6.0f * dancer / degree; unsigned section ← (unsigned)hue;
    unsigned rising ← (hue - section) * 190, falling ← 190 - rising;
    unsigned red ← 0,green ← 0,blue ← 0;
    switch (section) {
        case 0: red ← 190; green ← rising; break; case 1: red ← falling; green ← 190; break;
        case 2: green ← 190; blue ← rising; break; case 3: green ← falling; blue ← 190; break;
        case 4: red ← rising; blue ← 190; break; default: red ← 190; blue ← falling; break;
    }
    return 0xff000000u | (red + 55) | ((green + 55) << 8) | ((blue + 55) << 16);
}
static void button(canvas *surface,unsigned column,float y,const char *label,int active) {
    rectangle(surface,column * 90 + 3,y,84,44,active ? 0xff625026 : 0xff302a25);
    text(surface,column * 90 + 10,y + 16,1.5f,label,9,0xffeee8df);
}
void mathieu_render(const mathieu_session *session,uint32_t *pixels,unsigned width,unsigned height,unsigned stride) {
    canvas surface ← {pixels,width,height,stride, fminf(width / 360.0f,height / 720.0f),0,0};
    surface.offset_x ← (width - surface.scale * 360) * 0.5f; surface.offset_y ← (height - surface.scale * 720) * 0.5f;
    for (unsigned row ← 0; row < height; ++row) for (unsigned column ← 0; column < width; ++column) pixels[row * stride + column] ← 0xff181410;
    char caption[512]; snprintf(caption,sizeof caption,"M%u  EXACT PERMUTATION DANCE",session->action.degree);
    text(&surface,10,17,1.6f,caption,40,0xffeee8df);
    button(&surface,0,48,"M12",session->action.degree == 12); button(&surface,1,48,"M24",session->action.degree == 24);
    button(&surface,2,48,session->grid ? "CIRCLE" : "GRID",session->grid); button(&surface,3,48,"LABELS",session->labels);
    button(&surface,0,100,"A",0); button(&surface,1,100,"A INV",0); button(&surface,2,100,"B",0); button(&surface,3,100,"B INV",0);
    uint32_t selected ← session->frame < session->duration ? session->previous_selected : session->selected;
    uint32_t block ← 0; if (subset_size(selected) == 5) mathieu_block(&session->action,selected,&block);
    for (unsigned point ← 0; point < session->action.degree; ++point) {
        float x,y; session_position(session,point,&x,&y); dot(&surface,x,y,13,0,0xff453b32);
        if (session->labels) { snprintf(caption,sizeof caption,"%u",point + 1); text(&surface,x - 5,y - 23,1,caption,3,0xffbfb4a4); }
    }
    /* Arrows depict only the currently moving cycle, with their exact endpoints. */
    if (session->frame < session->duration) {
        for (unsigned dancer ← 0; dancer < session->action.degree; ++dancer) {
            float x,y,from_x,from_y,to_x,to_y;
            session_dancer(session,dancer,&x,&y);
            session_position(session,session->previous.image[dancer],&from_x,&from_y);
            session_position(session,session->current.image[dancer],&to_x,&to_y);
            if (hypotf(x - from_x,y - from_y) < 0.1f || hypotf(x - to_x,y - to_y) < 0.1f) continue;
            line(&surface,from_x,from_y,to_x,to_y,0xff887559);
            float angle ← atan2f(to_y - from_y,to_x - from_x);
            line(&surface,to_x,to_y,to_x - 8 * cosf(angle - .5f),to_y - 8 * sinf(angle - .5f),0xffddc6a0);
            line(&surface,to_x,to_y,to_x - 8 * cosf(angle + .5f),to_y - 8 * sinf(angle + .5f),0xffddc6a0);
        }
    }
    for (unsigned dancer ← 0; dancer < session->action.degree; ++dancer) {
        unsigned position ← session->frame < session->duration ? session->previous.image[dancer] : session->current.image[dancer];
        float x,y; session_dancer(session,dancer,&x,&y);
        if (block & (1u << position)) dot(&surface,x,y,17,0,0xff90822f);
        if (selected & (1u << position)) dot(&surface,x,y,14,0,0xfff4e6c2);
        dot(&surface,x,y,11,session->shapes ? dancer % 4 : 0,dancer_color(dancer,session->action.degree));
        if (session->labels) { snprintf(caption,sizeof caption,"%u",dancer + 1); text(&surface,x - (dancer < 9 ? 3 : 6),y - 3,1,caption,3,0xff181410); }
    }
    button(&surface,0,474,session->playing == 1 ? "PAUSE" : "PLAY",session->playing == 1);
    button(&surface,1,474,"STEP",0); button(&surface,2,474,"UNDO",0); button(&surface,3,474,"REDO",0);
    button(&surface,0,526,"RESET",0); button(&surface,1,526,"CLEAR 5",0); button(&surface,2,526,"SHAPES",session->shapes);
    button(&surface,3,526,session->speed == 12 ? "SPEED 1" : session->speed == 24 ? "SPEED 1/2" : "STATIC",session->speed == 1);
    snprintf(caption,sizeof caption,"WORD %u/%u: ",session->cursor,session->length);
    text(&surface,8,583,1.2f,caption,40,0xffeee8df);
    unsigned start ← session->cursor > 40 ? session->cursor - 40 : 0;
    for (unsigned index ← start; index < session->cursor; ++index) {
        snprintf(caption,sizeof caption,"%c%s",session->moves[index],session->moves[index] >= 'A' && session->moves[index] <= 'Z' ? "-" : "+");
        text(&surface,8 + (index - start) * 8.5f,600,0.7f,caption,2,0xffd4ba7f);
    }
    permutation_cycles(session->current,caption,sizeof caption); text(&surface,8,619,1.1f,caption,150,0xffddd4c3);
    snprintf(caption,sizeof caption,"SELECT 5: %u/5  %s",subset_size(session->selected),subset_size(session->selected) == 5 ? (session->action.degree == 12 ? "HEXAD" : "OCTAD") : "TAP POSITIONS");
    text(&surface,8,681,1.1f,caption,49,0xffddc6a0);
    if (session->error) text(&surface,8,701,1,session->error == 1 ? "HISTORY FULL - RESET OR UNDO" : session->error == 2 ? "SAVE FAILED - CHECK FREE STORAGE" : "FRAME FAILED - REOPEN APP",40,0xff8989ff);
}
