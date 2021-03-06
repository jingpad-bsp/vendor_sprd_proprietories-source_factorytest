/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _MINUI_H_
#define _MINUI_H_

#include <sys/types.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int width;
    int height;
    int row_bytes;
    int pixel_bytes;
    unsigned char* data;
} GRSurface;

typedef GRSurface* gr_surface;

typedef struct minui_backend {
    // Initializes the backend and returns a gr_surface to draw into.
    gr_surface (*init)(struct minui_backend*);

    // Sync display surface buffer.
    void (*sync)(GRSurface*);

    // Causes the current drawing surface (returned by the most recent
    // call to flip() or init()) to be displayed, and returns a new
    // drawing surface.
    gr_surface (*flip)(struct minui_backend*);

    // Blank (or unblank) the screen.
    void (*blank)(struct minui_backend*, bool);

    // Device cleanup when drawing is done.
    void (*exit)(struct minui_backend*);
} minui_backend;

void gr_flip_only(void);
int gr_init();
void gr_exit(void);
int gr_pixel_bytes(void);
minui_backend* gr_backend_get(void);
GRSurface* gr_draw_get(void);
int gr_fb_width(void);
int gr_fb_height(void);
void gr_tp_flag(int flag);

void gr_sync(void);
void gr_flip(void);
void gr_fb_blank(bool blank);

void gr_clear();  // clear entire surface to current color
void gr_color(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
void gr_fill(int x1, int y1, int x2, int y2);
char* gr_text(int x, int y, const char *s);
void set_render_mode(int mode);
void gr_texticon(int x, int y, gr_surface icon);

void gr_blit(gr_surface source, int sx, int sy, int w, int h, int dx, int dy);
unsigned int gr_get_width(gr_surface surface);
unsigned int gr_get_height(gr_surface surface);
void gr_draw_point(int x, int y);
int get_string_room_size(const char *);
void freetype_setsize(unsigned int);
void gr_set_fontsize(int fontSize);

// Resources

// res_create_*_surface() functions return 0 if no error, else
// negative.
//
// A "display" surface is one that is intended to be drawn to the
// screen with gr_blit().  An "alpha" surface is a grayscale image
// interpreted as an alpha mask used to render text in the current
// color (with gr_text() or gr_texticon()).
//
// All these functions load PNG images from "/res/images/${name}.png".

// Load a single display surface from a PNG image.
int res_create_display_surface(const char* name, gr_surface* pSurface);

// Load an array of display surfaces from a single PNG image.  The PNG
// should have a 'Frames' text chunk whose value is the number of
// frames this image represents.  The pixel data itself is interlaced
// by row.
int res_create_multi_display_surface(const char* name,
                                     int* frames, gr_surface** pSurface);

// Load a single alpha surface from a grayscale PNG image.
int res_create_alpha_surface(const char* name, gr_surface* pSurface);

// Load part of a grayscale PNG image that is the first match for the
// given locale.  The image is expected to be a composite of multiple
// translations of the same text, with special added rows that encode
// the subimages' size and intended locale in the pixel data.  See
// development/tools/recovery_l10n for an app that will generate these
// specialized images from Android resources.
int res_create_localized_alpha_surface(const char* name, const char* locale,
                                       gr_surface* pSurface);

// Free a surface allocated by any of the res_create_*_surface()
// functions.
void res_free_surface(gr_surface surface);

#ifdef __cplusplus
}
#endif

//#include <utils/Log.h>
#include <log/log.h>

#undef LOG_TAG
#define LOG_TAG  "FACTORY"
#define LOGD(format, ...)  ALOGD("%s: "  format "\n" , __func__, ## __VA_ARGS__)
#define LOGE(format, ...)  ALOGE("%s: "  format "[%d][%s][%d]\n", __func__,  ## __VA_ARGS__, errno,strerror(errno),__LINE__)
#define LOGI(format, ...)  ALOGI("%s: "  format "\n", __func__,  ## __VA_ARGS__)
#define LOGV(format, ...)  ALOGV("%s: "  format "\n", __func__,  ## __VA_ARGS__)

#define SPRD_DBG       LOGV


#endif
