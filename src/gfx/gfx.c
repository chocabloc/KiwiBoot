#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/GraphicsOutput.h>
#include <util/GfxUtils.h>
#include "gfx.h"

#include "stbtt_helper.h"
#include "stb_truetype.h"

// framebuffer information
static UINT32* fb;
static UINT32 width;
static UINT32 height;
static UINT32 pitch;

// font information
static stbtt_fontinfo fontinfo;
static float fscale;
static int fascent, fdescent, flinegap;
static int fsize = 20;

// current color
static GFXColor_t fgcolor, bgcolor;

// temporary buffer for drawing
static UINT8* tbuffer;

// the actual font data
extern const unsigned char font_ttf;

// set foreground color
void GFXSetFgColor(GFXColor_t c)
{
    fgcolor = c;
}

// set background color
void GFXSetBgColor(GFXColor_t c)
{
    bgcolor = c;
}

// clear screen
void GFXClear()
{
    for (int x = 0; x < width; x++)
        for (int y = 0; y < height; y++)
            fb[y * pitch + x] = *((UINT64*)((void*)&bgcolor));
}

// fills a rectangle
void GFXFillRect(UINT16 x1, UINT16 y1, UINT16 x2, UINT16 y2)
{
    for (int x = x1; x < x2; x++)
        for (int y = y1; y < y2; y++)
            fb[pitch * y + x] = *((UINT64*)((void*)&fgcolor));
}

// draws a rectangle
void GFXDrawRect(UINT16 x1, UINT16 y1, UINT16 x2, UINT16 y2)
{
    for (int x = x1 + 1; x < x2; x++) {
        fb[pitch * y1 + x] = *((UINT64*)((void*)&fgcolor));
        fb[pitch * y2 + x] = *((UINT64*)((void*)&fgcolor));
    }
    for (int y = y1 + 1; y < y2; y++) {
        fb[pitch * y + x1] = *((UINT64*)((void*)&fgcolor));
        fb[pitch * y + x2] = *((UINT64*)((void*)&fgcolor));
    }
}

UINT16 GFXGetWidth()
{
    return width;
}

UINT16 GFXGetHeight()
{
    return height;
}

// draw a character, advances x position
void GFXDrawChar(UINT16* x, UINT16* y, int c, BOOLEAN dryrun)
{
    int ax, lsb;
    stbtt_GetCodepointHMetrics(&fontinfo, c, &ax, &lsb);

    if (dryrun)
        goto done;

    int bx1, by1, bx2, by2;
    stbtt_GetCodepointBitmapBox(&fontinfo, c, fscale, fscale, &bx1, &by1, &bx2, &by2);
    int cwidth = bx2 - bx1, cheight = by2 - by1;

    stbtt_MakeCodepointBitmap(&fontinfo, (UINT8*)tbuffer, cwidth, cheight, fsize, fscale, fscale, c);

    for (int dx = 0; dx < cwidth; dx++) {
        for (int dy = 0; dy < cheight; dy++) {
            UINT16 mx = *x + dx + (lsb * fscale);
            UINT16 my = *y + dy + (fascent + by1);

            UINT8 r1 = tbuffer[dx + (dy * fsize)], r2 = 255 - r1;
            GFXColor_t bgclr = *((GFXColor_t*)&(fb[my * pitch + mx]));
            GFXColor_t clr = { .red = (fgcolor.red * r1 + bgclr.red * r2) / 255,
                .green = (fgcolor.green * r1 + bgclr.green * r2) / 255,
                .blue = (fgcolor.blue * r1 + bgclr.blue * r2) / 255 };

            fb[my * pitch + mx] = *((UINT64*)((void*)&clr));
        }
    }

done:
    *x += (ax * fscale);
}

// draw string with mid at x, y
void GFXDrawString(UINT16 x, UINT16 y, CONST char* s, ...)
{
    VA_LIST(args);
    VA_START(args, s);

    BOOLEAN dryrun = TRUE;

start:
    UINT16 tx = x, ty = y;
    for (int i = 0; s[i] != '\0'; i++) {
        switch (s[i]) {
        case '%': {
            switch (s[i + 1]) {
            case 'd': {
                int n = VA_ARG(args, int);
                int nsi = 29;
                char ns[30];
                while (n > 0) {
                    ns[nsi] = (n % 10) + '0';
                    n /= 10;
                    nsi--;
                }
                for (nsi = nsi + 1; nsi < 30; nsi++)
                    GFXDrawChar(&tx, &ty, ns[nsi], dryrun);
            } break;

            default: {
                GFXDrawChar(&tx, &ty, s[i + 1], dryrun);
            } break;
            }
            i++;
        } break;

        case ' ': {
            tx += fsize / 4;
            continue;
        } break;

        default: {
            GFXDrawChar(&tx, &ty, s[i], dryrun);
        } break;
        }
    }

    if (dryrun) {
        x = x - ((tx - x) / 2);
        y = y - (fsize / 2);
        dryrun = FALSE;
        goto start;
    }
}

void GFXSetFontSize(int fontsize)
{
    fsize = fontsize;
    fscale = stbtt_ScaleForPixelHeight(&fontinfo, fsize);
    stbtt_GetFontVMetrics(&fontinfo, &fascent, &fdescent, &flinegap);
    fascent *= fscale;
    fdescent *= fscale;

    FreePool(tbuffer);
    tbuffer = AllocatePool(fsize * fsize);
}

void GFXInit()
{
    INT32 mode = GetBestGfxMode(800, 600);
    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = NULL;
    ASSERT_EFI_ERROR(gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID**)&gop));
    ASSERT_EFI_ERROR(gop->SetMode(gop, (UINT32)mode));

    fb = (UINT32*)(gop->Mode->FrameBufferBase);
    width = gop->Mode->Info->HorizontalResolution;
    height = gop->Mode->Info->VerticalResolution;
    pitch = gop->Mode->Info->PixelsPerScanLine;

    // initialize font
    stbtt_InitFont(&fontinfo, &font_ttf, 0);
    fscale = stbtt_ScaleForPixelHeight(&fontinfo, fsize);
    stbtt_GetFontVMetrics(&fontinfo, &fascent, &fdescent, &flinegap);
    fascent *= fscale;
    fdescent *= fscale;

    // allocate temporary buffer
    tbuffer = AllocatePool(fsize * fsize);
}
