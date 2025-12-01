#include "aoc2025.h"
#include "win32_resource.h"

#include <windows.h>

#include <stdio.h>

extern void *GetWindowHandle(void);

void
set_native_window_icon(void)
{
    HWND native_window  = GetWindowHandle();

    HICON big_icon = (HICON)LoadImage(GetModuleHandle(NULL),
                                      MAKEINTRESOURCE(IDI_APP_ICON),
                                      IMAGE_ICON,
                                      0, 0,
                                      LR_DEFAULTSIZE);

    HICON small_icon = (HICON)LoadImage(GetModuleHandle(NULL),
                                        MAKEINTRESOURCE(IDI_APP_ICON),
                                        IMAGE_ICON,
                                        GetSystemMetrics(SM_CXSMICON),
                                        GetSystemMetrics(SM_CYSMICON),
                                        LR_DEFAULTCOLOR);


    SendMessage(native_window, WM_SETICON, ICON_BIG,   (LPARAM)big_icon);
    SendMessage(native_window, WM_SETICON, ICON_SMALL, (LPARAM)small_icon);
}

int APIENTRY
WinMain(HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR     lpCmdLine,
        int       nShowCmd)
{
#ifndef NDEBUG
    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    freopen_s(&fp, "CONIN$",  "r", stdin);
#endif


    int exit_code = aoc2025_entry();

    return exit_code;
}
