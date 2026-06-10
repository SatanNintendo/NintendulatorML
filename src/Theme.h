/* Nintendulator - Dark/Light Theme engine
 * Works on Windows 7, 8, 10, 11.
 */

#pragma once
#include <windows.h>

namespace Theme
{
    enum Mode { MODE_LIGHT = 0, MODE_DARK = 1 };

    void Init(void);
    void Destroy(void);

    Mode GetMode(void);
    void SetMode(Mode mode);
    void Toggle(void);        // legacy

    void Reapply(void);

    void ApplyToDialog(HWND hDlg);
    void RemoveFromDialog(HWND hDlg);
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
    void EnableForWindow(HWND hWnd, BOOL enable); // legacy
    void RefreshMenuBar(void);                    // legacy

    // Owner-draw menu callbacks — call from main WndProc
    void HandleMeasureItem(HWND hWnd, MEASUREITEMSTRUCT *p);
    void HandleDrawItem(HWND hWnd, DRAWITEMSTRUCT *p);
}
