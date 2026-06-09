/* Nintendulator - Dark/Light Theme engine
 *
 * Supports all Windows versions from Windows 7+.
 *
 * Technique:
 *   - Dialogs/backgrounds: WM_CTLCOLOR* subclassing (all Windows).
 *   - Buttons: SetWindowTheme(btn, L"", L"") to disable Visual Styles,
 *     then WM_CTLCOLORBTN handles color (all Windows with comctl32 v6).
 *   - Menu: Owner-draw (MF_OWNERDRAW) for full color control on all
 *     Windows versions. Additionally uses undocumented uxtheme APIs
 *     on Windows 10 1903+ for scrollbars and system controls.
 *   - Title bar: DwmSetWindowAttribute (Windows 10 1809+).
 */

#pragma once
#include <windows.h>

namespace Theme
{
    enum Mode
    {
        MODE_LIGHT = 0,
        MODE_DARK  = 1
    };

    // Call once at startup (before window creation)
    void Init(void);

    // Call at shutdown
    void Destroy(void);

    // Get/set theme mode
    Mode GetMode(void);
    void SetMode(Mode mode);

    // Apply theme to all open UI (call after SetMode)
    void Reapply(void);

    // Apply theme to a dialog and all its child controls
    // Call from WM_INITDIALOG
    void ApplyToDialog(HWND hDlg);

    // Remove subclassing from a dialog (call before dialog destruction)
    void RemoveFromDialog(HWND hDlg);

    // Apply theme to the main window (background, title bar, menu)
    void ApplyToMainWindow(HWND hWnd);

    // Rebuild the main menu as owner-draw (call after menu is created/rebuilt)
    void RebuildOwnerDrawMenu(HMENU hMenu);

    // Save/load theme setting from registry
    void SaveSettings(HKEY SettingsBase);
    void LoadSettings(HKEY SettingsBase);

    // Sync checkmarks in the View/Theme menu
    void SyncMenuChecks(void);

    // GDI accessors (used by WM_CTLCOLOR handlers in main WndProc)
    HBRUSH GetBackgroundBrush(void);
    HBRUSH GetControlBrush(void);

    // Color accessors
    COLORREF GetBgColor(void);
    COLORREF GetTextColor(void);
    COLORREF GetControlBgColor(void);
    COLORREF GetControlTextColor(void);

    // Convenience check
    bool IsDark(void);

    // Apply dark title bar attribute (Win10+ DWM)
    void SetTitleBarDark(HWND hWnd, BOOL dark);

    // Owner-draw menu handlers (call from main WndProc)
    void HandleMeasureItem(HWND hWnd, MEASUREITEMSTRUCT* pMIS);
    void HandleDrawItem(HWND hWnd, DRAWITEMSTRUCT* pDIS);
}
