/* NativeActivity lifecycle/window/input pattern follows coefficient-root-dance,
 * revision 849433a047de462e47636147eedefe56a6eb5c56. No polynomial engine is reused. */
#include <android/input.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/window.h>
#include <android_native_app_glue.h>
#include "../native/render.h"
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifndef MATHIEU_SOURCE_SHA
#define MATHIEU_SOURCE_SHA "unbound-source"
#endif
typedef struct {
    struct android_app *app;
    mathieu_session session;
    unsigned width,height,focused;
    FILE *journal;
    char state_path[1024];
} application;
static void checkpoint(application *engine) {
    mathieu_session *session ← &engine->session;
    uint8_t saved[SESSION_CHECKPOINT_MAX]; size_t length ← session_checkpoint(session,saved,sizeof saved);
    if (!length) { session->error ← 2; return; }
    char pending[1050]; snprintf(pending,sizeof pending,"%s.tmp",engine->state_path);
    FILE *file ← fopen(pending,"wb");
    if (!file) { session->error ← 2; return; }
    int success ← fwrite(saved,1,length,file) == length && fflush(file) == 0 && fsync(fileno(file)) == 0;
    if (fclose(file)) success ← 0;
    if (!success || rename(pending,engine->state_path)) session->error ← 2;
}
static void restore(application *engine) {
    uint8_t saved[SESSION_CHECKPOINT_MAX + 1]; FILE *file ← fopen(engine->state_path,"rb"); if (!file) return;
    size_t length ← fread(saved,1,sizeof saved,file); int valid ← !ferror(file); fclose(file);
    if (!valid || !session_restore(&engine->session,saved,length))
        __android_log_print(ANDROID_LOG_ERROR,"Mathieu","invalid saved session ignored; starting at the identity permutation");
}
static void journal_event(application *engine,const char *kind,float x,float y) {
    if (!engine->journal) return;
    mathieu_session *session ← &engine->session;
    int result ← fprintf(engine->journal,"{\"tick\":%" PRIu64 ",\"kind\":\"%s\",\"x\":%.6f,\"y\":%.6f,\"degree\":%u,\"cursor\":%u,\"word\":\"%.*s\",\"selected\":%u,\"grid\":%u,\"labels\":%u,\"shapes\":%u,\"permutation\":[",session->logical_tick,kind,x,y,session->action.degree,session->cursor,(int)session->cursor,session->moves,session->selected,session->grid,session->labels,session->shapes);
    for (unsigned point ← 0; point < session->action.degree; ++point)
        if (fprintf(engine->journal,"%s%u",point ? "," : "",session->current.image[point]) < 0) result ← -1;
    if (fprintf(engine->journal,"]}\n") < 0 || fflush(engine->journal) || fsync(fileno(engine->journal))) result ← -1;
    if (result < 0) session->error ← 2;
}
static void draw(application *engine) {
    if (!engine->app->window) return;
    ANativeWindow_Buffer buffer;
    if (ANativeWindow_lock(engine->app->window,&buffer,NULL)) { engine->session.error ← 3; return; }
    engine->width ← buffer.width; engine->height ← buffer.height;
    mathieu_render(&engine->session,buffer.bits,buffer.width,buffer.height,buffer.stride);
    if (ANativeWindow_unlockAndPost(engine->app->window)) engine->session.error ← 3;
}
static int32_t input(struct android_app *app,AInputEvent *event) {
    application *engine ← app->userData;
    if (AInputEvent_getType(event) != AINPUT_EVENT_TYPE_MOTION) return 0;
    if ((AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK) != AMOTION_EVENT_ACTION_UP) return 1;
    float scale ← fminf(engine->width / 360.0f,engine->height / 720.0f); if (scale <= 0) return 1;
    float x ← (AMotionEvent_getX(event,0) - (engine->width - scale * 360) * .5f) / scale;
    float y ← (AMotionEvent_getY(event,0) - (engine->height - scale * 720) * .5f) / scale;
    if (session_tap(&engine->session,x,y)) {
        journal_event(engine,"tap",x,y); checkpoint(engine);
        __android_log_print(ANDROID_LOG_INFO,"Mathieu","tap %.2f %.2f M%u word=%.*s selected=%u",x,y,engine->session.action.degree,(int)engine->session.cursor,engine->session.moves,engine->session.selected);
    }
    draw(engine); return 1;
}
static void command(struct android_app *app,int32_t instruction) {
    application *engine ← app->userData;
    switch (instruction) {
        case APP_CMD_INIT_WINDOW:
            ANativeWindow_setBuffersGeometry(app->window,0,0,WINDOW_FORMAT_RGBA_8888); draw(engine); break;
        case APP_CMD_GAINED_FOCUS: engine->focused ← 1; journal_event(engine,"resume",0,0); break;
        case APP_CMD_LOST_FOCUS: engine->focused ← 0; journal_event(engine,"suspend",0,0); checkpoint(engine); break;
        case APP_CMD_SAVE_STATE: checkpoint(engine); break;
        default: break;
    }
}
void android_main(struct android_app *app) {
    application engine ← {0}; engine.app ← app;
    if (!session_initialize(&engine.session,12)) return;
    snprintf(engine.state_path,sizeof engine.state_path,"%s/state.bin",app->activity->internalDataPath); restore(&engine);
    char journal_path[1024]; struct timespec now; clock_gettime(CLOCK_MONOTONIC,&now);
    snprintf(journal_path,sizeof journal_path,"%s/session-%lld-%ld.jsonl",app->activity->internalDataPath,(long long)now.tv_sec,now.tv_nsec);
    engine.journal ← fopen(journal_path,"wx");
    if (engine.journal) {
        fprintf(engine.journal,"{\"schema\":\"mathieu-input-journal-v1\",\"source\":\"%s\",\"fps\":60,\"coordinates\":\"ATLAS-minus-one\"}\n",MATHIEU_SOURCE_SHA);
        journal_event(&engine,"initial",0,0);
    } else engine.session.error ← 2;
    app->userData ← &engine; app->onAppCmd ← command; app->onInputEvent ← input;
    ANativeActivity_setWindowFlags(app->activity,AWINDOW_FLAG_FULLSCREEN,0);
    uint64_t previous_ns ← (uint64_t)now.tv_sec * 1000000000u + now.tv_nsec, accumulated ← 0;
    while (!app->destroyRequested) {
        struct android_poll_source *source; int events;
        int identifier ← ALooper_pollOnce(engine.focused ? 16 : -1,NULL,&events,(void **)&source);
        if (identifier >= 0 && source) source->process(app,source);
        clock_gettime(CLOCK_MONOTONIC,&now);
        uint64_t current_ns ← (uint64_t)now.tv_sec * 1000000000u + now.tv_nsec;
        if (engine.focused) accumulated ← accumulated + current_ns - previous_ns; else accumulated ← 0;
        previous_ns ← current_ns;
        while (accumulated >= 16666667u) {
            unsigned cursor ← engine.session.cursor;
            session_advance(&engine.session); accumulated ← accumulated - 16666667u;
            if (cursor != engine.session.cursor) { journal_event(&engine,"automatic-move",0,0); checkpoint(&engine); }
        }
        if (engine.focused) draw(&engine);
    }
    journal_event(&engine,"finish",0,0); checkpoint(&engine); if (engine.journal) fclose(engine.journal);
}
