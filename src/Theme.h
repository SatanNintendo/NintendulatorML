/* Nintendulator - Dark/Light Theme engine
 *
 * Works on Windows 7, 8, 10, 11.
 *
 * - Dialogs/backgrounds : WM_CTLCOLOR* subclassing (all Windows)
 * - Buttons             : SetWindowTheme("","") + WM_CTLCOLORBTN (all Windows)
 * - Menu                : MF_OWNERDRAW owner-draw (all Windows)
 *                         + uxtheme ordinals on Win10 1903+ as bonus
 * - Title bar           : DWMWA_USE_IMMERSIVE_DARK_MODE (Win10 1809+)
 */

#pragma once
#include <windows.h>

namespace Theme
{
    enum Mode { MODE_LIGHT = 0, MODE_DARK = 1 };

    void Init(void);          // call once before window creation
    void Destroy(void);       // call at shutdown

    Mode GetMode(void);
    void SetMode(Mode mode);

    void Reapply(void);       // call after SetMode

    // Call from WM_INITDIALOG of every dialog
    void ApplyToDialog(HWND hDlg);
    void RemoveFromDialog(HWND hDlg);

    // Call after main window is created and after every menu rebuild
    void ApplyToMainWindow(HWND hWnd);
    void RebuildOwnerDrawMenu(HMENU hMenu);

    void SaveSettings(HKEY base);
    void LoadSettings(HKEY base);
    void SyncMenuChecks(void);

    // Accessors used by WndProc
    HBRUSH   GetBackgroundBrush(void);
    HBRUSH   GetControlBrush(void);
    COLORREF GetBgColor(void);
    COLORREF GetTextColor(void);
    COLORREF GetControlBgColor(void);
    COLORREF GetControlTextColor(void);
    bool     IsDark(void);

    void SetTitleBarDark(HWND hWnd, BOOL dark);

    // Owner-draw menu callbacks — call from WndProc
    void HandleMeasureItem(HWND hWnd, MEASUREITEMSTRUCT* p);
    void HandleDrawItem(HWND hWnd, DRAWITEMSTRUCT* p);
}
