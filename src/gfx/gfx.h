#pragma once

#include <Uefi.h>

typedef struct {
    UINT8 blue;
    UINT8 green;
    UINT8 red;
    UINT8 res;
} __attribute__((packed)) GFXColor_t;

static const GFXColor_t GFX_COLOR_GRAY = { .red = 0x80, .green = 0x80, .blue = 0x80 };
static const GFXColor_t GFX_COLOR_DARKGRAY = { .red = 0x40, .green = 0x40, .blue = 0x40 };
static const GFXColor_t GFX_COLOR_LIGHTGRAY = { .red = 0xc0, .green = 0xc0, .blue = 0xc0 };
static const GFXColor_t GFX_COLOR_BLACK = { .red = 0x0, .green = 0x0, .blue = 0x0 };
static const GFXColor_t GFX_COLOR_WHITE = { .red = 0xff, .green = 0xff, .blue = 0xff };

void GFXInit();
void GFXDrawString(UINT16 x, UINT16 y, CONST char* s, ...);
void GFXSetBgColor(GFXColor_t c);
void GFXSetFgColor(GFXColor_t c);
void GFXFillRect(UINT16 x1, UINT16 y1, UINT16 x2, UINT16 y2);
void GFXDrawRect(UINT16 x1, UINT16 y1, UINT16 x2, UINT16 y2);
void GFXClear();
UINT16 GFXGetWidth();
UINT16 GFXGetHeight();
void GFXSetFontSize(int fontsize);