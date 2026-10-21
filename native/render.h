#ifndef MATHIEU_RENDER_H
#define MATHIEU_RENDER_H
#include "session.h"
/* Pixels are RGBA8888 in native little-endian words. All paths share this raster. */
typedef struct { unsigned controls_visible; } mathieu_render_options;
/* Presentation policy belongs to the renderer, independently of session state.
 * Hidden controls use a scene viewport fitted to the complete dance, not a crop.
 * Labels, selection rings and cycle arrows remain mathematical scene content. */
void mathieu_render_with_options(const mathieu_session *session, uint32_t *pixels, unsigned width, unsigned height, unsigned stride, mathieu_render_options options);
/* Scene clears its target; controls is a separate transparent overlay pass.
 * These two passes share the interactive 360x720 coordinate transform. */
void mathieu_render_scene(const mathieu_session *session, uint32_t *pixels, unsigned width, unsigned height, unsigned stride);
void mathieu_render_controls(const mathieu_session *session, uint32_t *pixels, unsigned width, unsigned height, unsigned stride);
/* Compatibility compositor: interactive controls remain visible. */
void mathieu_render(const mathieu_session *session, uint32_t *pixels, unsigned width, unsigned height, unsigned stride);
#endif
