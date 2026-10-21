#ifndef MATHIEU_RENDER_H
#define MATHIEU_RENDER_H
#include "session.h"
/* Pixels are RGBA8888 in native little-endian words. All paths share this raster. */
void mathieu_render(const mathieu_session *session, uint32_t *pixels, unsigned width, unsigned height, unsigned stride);
#endif
