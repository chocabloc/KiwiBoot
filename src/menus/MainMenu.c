#include "Menus.h"

#include <util/DrawUtils.h>

#include <config/BootConfig.h>
#include <config/BootEntries.h>

#include <Uefi.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/DevicePathToText.h>
#include <Library/DebugLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/CpuLib.h>
#include <loaders/Loaders.h>
#include <menus/Menus.h>
#include <gfx/gfx.h>
#include <config/BootEntries.h>

static UINT16 xmid, ymid;
static UINT16 box_x1, box_y1, box_x2, box_y2;

static const GFXColor_t COLOR_LIGHTRED = (GFXColor_t) { .red = 0x88, .green = 0x22, .blue = 0x22 };
static const GFXColor_t COLOR_HIGHLIGHT = (GFXColor_t) { .red = 0x22, .green = 0x66, .blue = 0x22 };

MENU EnterMainMenu(BOOLEAN first)
{
    xmid = GFXGetWidth() / 2;
    ymid = GFXGetHeight() / 2;

    GFXSetBgColor(GFX_COLOR_LIGHTGRAY);
    GFXSetFgColor(GFX_COLOR_BLACK);
    GFXClear();

    GFXSetFontSize(29);
    GFXDrawString(xmid, 30, "KiwiBoot");
    GFXSetFontSize(19);
    GFXDrawString(xmid, 60, "Version 0.1a");

    UINTN numentries = 0;
    for (LIST_ENTRY* link = gBootEntries.ForwardLink; link != &gBootEntries; link = link->ForwardLink, numentries++)
        ;

    box_y1 = ymid - (numentries * 15);
    box_y2 = ymid + (numentries * 15);
    box_x1 = xmid - 200;
    box_x2 = xmid + 200;

    if (numentries == 0) {
        GFXSetFgColor(COLOR_LIGHTRED);
        GFXDrawRect(box_x1, ymid - 50, box_x2, ymid + 50);
        GFXDrawString(xmid, ymid, "No Operating Systems Found!");
        while (TRUE)
            CpuPause();
    }

    GFXSetFontSize(23);
    GFXDrawString(xmid, box_y1 - 30, "Choose an Option");
    GFXSetFontSize(17);

    int selected = 0;
    BOOT_ENTRY* selectedEntry = NULL;

    while (TRUE) {
        UINTN i = 0;
        for (LIST_ENTRY* link = gBootEntries.ForwardLink; i < numentries; link = link->ForwardLink, i++) {
            GFXSetFgColor(GFX_COLOR_GRAY);
            GFXDrawRect(box_x1, box_y1 + (30 * i), box_x2, box_y1 + (30 * (i + 1)));
            GFXSetFgColor(GFX_COLOR_BLACK);

            BOOT_ENTRY* entry = BASE_CR(link, BOOT_ENTRY, Link);

            if (i == selected) {
                selectedEntry = entry;
                GFXSetFgColor(COLOR_HIGHLIGHT);
                GFXFillRect(box_x1 + 1, box_y1 + (30 * i) + 1, box_x2, box_y1 + (30 * (i + 1)));
                GFXSetFgColor(GFX_COLOR_WHITE);
            } else {
                GFXSetFgColor(GFX_COLOR_LIGHTGRAY);
                GFXFillRect(box_x1 + 1, box_y1 + (30 * i) + 1, box_x2, box_y1 + (30 * (i + 1)));
                GFXSetFgColor(GFX_COLOR_BLACK);
            }

            // convert unicode string to ascii string for drawing
            CHAR8* str = AllocatePool(StrLen(entry->Name) + 1);
            UnicodeStrToAsciiStr(entry->Name, str);
            GFXDrawString(xmid, box_y1 + (30 * i) + 15, str);
        }
        // draw the shutdown entry
        GFXSetFgColor(GFX_COLOR_GRAY);
        GFXDrawRect(box_x1, box_y1 + (30 * i) + 10, box_x2, box_y1 + (30 * (i + 1)) + 10);
        GFXSetFgColor(GFX_COLOR_BLACK);

        if (i == selected) {
            GFXSetFgColor(COLOR_LIGHTRED);
            GFXFillRect(box_x1 + 1, box_y1 + (30 * i) + 11, box_x2, box_y1 + (30 * (i + 1)) + 10);
            GFXSetFgColor(GFX_COLOR_WHITE);
        } else {
            GFXSetFgColor(GFX_COLOR_LIGHTGRAY);
            GFXFillRect(box_x1 + 1, box_y1 + (30 * i) + 11, box_x2, box_y1 + (30 * (i + 1)) + 10);
            GFXSetFgColor(GFX_COLOR_BLACK);
        }

        GFXDrawString(xmid, box_y1 + (30 * i) + 25, "Shutdown");

        UINTN which = 0;
        EFI_INPUT_KEY key = {};
        ASSERT_EFI_ERROR(gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &which));
        EFI_STATUS status = gST->ConIn->ReadKeyStroke(gST->ConIn, &key);
        if (status == EFI_NOT_READY) {
            continue;
        }
        ASSERT_EFI_ERROR(status);

        // decrease value
        if (key.ScanCode == SCAN_DOWN) {
            selected++;
            if (selected > i) {
                selected = 0;
            }

            // prev potion
        } else if (key.ScanCode == SCAN_UP) {
            selected--;
            if (selected < 0) {
                selected = i;
            }

            // save and exit
        } else if (key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            // shutdown
            if(i==selected)
                return MENU_SHUTDOWN;
                
            // choose an os to starts
            LoadKernel(selectedEntry);
            while (1)
                CpuSleep();
        }
    }
}