#ifndef _FREETYPE_H_
#define _FREETYPE_H_

#include <sys/types.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int top;
    int height;
    int left;
    int width;
    char *map;
} FontGraph;

enum Render_type
{
    Render_DEFAULT = 0,
    Render_BOLD = 0x01,
    Render_ITALIC = 0x02,
    Render_NARROW = 0x04,
    Render_ENLARGE = 0x08,
    Render_Underline = 0x10,
};


#ifdef __cplusplus
}
#endif

#endif