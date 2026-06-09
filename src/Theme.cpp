/* Nintendulator - Dark/Light Theme engine implementation */

#include "stdafx.h"
#include "Theme.h"
#include "Nintendulator.h"
#include "resource.h"
#include "Lang.h"
#include "Debugger.h"
#include <commctrl.h>
#include <uxtheme.h>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "uxtheme.lib")

namespace Theme
{
    // ================================================================
    // State
    // ================================================================
    static Mode currentMode = MODE_LIGHT;

    // ================================================================
    // Color palettes
    // ================================================================
    struct ThemeColors
    {
        COLORREF bg;
        COLORREF text;
        COLORREF controlBg;
        COLORREF controlText;
        COLORREF buttonBg;
        COLORREF buttonText;
        COLORREF menuBg;
        COLORREF menuText;
        COLORREF menuHighlight;
        COLORREF menuHighlightText;
        COLORREF menuBorder;
        COLORREF menuGray;
        COLORREF menuBar;
        COLORREF menuBarText;
        COLORREF menuBarHighlight;
    };

    static const ThemeColors LightColors = {
        RGB(240, 240, 240),   // bg
        RGB(0,   0,   0),     // text
        RGB(255, 255, 255),   // controlBg
        RGB(0,   0,   0),     // controlText
        RGB(225, 225, 225),   // buttonBg
        RGB(0,   0,   0),     // buttonText
        RGB(255, 255, 255),   // menuBg
        RGB(0,   0,   0),     // menuText
        RGB(0,   120, 215),   // menuHighlight
        RGB(255, 255, 255),   // menuHighlightText
        RGB(200, 200, 200),   // menuBorder
        RGB(130, 130, 130),   // menuGray (disabled)
        RGB(240, 240, 240),   // menuBar
        RGB(0,   0,   0),     // menuBarText
        RGB(205, 227, 247),   // menuBarHighlight
    };

    static const ThemeColors DarkColors = {
        RGB(30,  30,  30),    // bg
        RGB(212, 212, 212),   // text
        RGB(45,  45,  45),    // controlBg
        RGB(212, 212, 212),   // controlText
        RGB(55,  55,  55),    // buttonBg
        RGB(212, 212, 212),   // buttonText
        RGB(40,  40,  40),    // menuBg
        RGB(212, 212, 212),   // menuText
        RGB(0,   122, 204),   // menuHighlight
        RGB(255, 255, 255),   // menuHighlightText
        RGB(80,  80,  80),    // menuBorder
        RGB(120, 120, 120),   // menuGray (disabled)
        RGB(30,  30,  30),    // menuBar
        RGB(212, 212, 212),   // menuBarText
        RGB(60,  60,  60),    // menuBarHighlight
    };

    static const ThemeColors& Colors(void)
    {
        return (currentMode == MODE_DARK) ? DarkColors : LightColors;
    }

    // ================================================================
    // GDI brushes
    // ================================================================
    static HBRUSH hBgBrush         = NULL;
    static HBRUSH hControlBgBrush  = NULL;
    static HBRUSH hButtonBgBrush   = NULL;
    static HBRUSH hMenuBgBrush     = NULL;
    static HBRUSH hMenuHiliteBrush = NULL;
    static HBRUSH hMenuBarBrush    = NULL;
    static HBRUSH hMenuBarHilBrush = NULL;

    static void CreateBrushes(void)
    {
        const ThemeColors& c = Colors();
        auto SafeDelete = [](HBRUSH& b) { if (b) { DeleteObject(b); b = NULL; } };

        SafeDelete(hBgBrush);
        SafeDelete(hControlBgBrush);
        SafeDelete(hButtonBgBrush);
        SafeDelete(hMenuBgBrush);
        SafeDelete(hMenuHiliteBrush);
        SafeDelete(hMenuBarBrush);
        SafeDelete(hMenuBarHilBrush);

        hBgBrush         = CreateSolidBrush(c.bg);
        hControlBgBrush  = CreateSolidBrush(c.controlBg);
        hButtonBgBrush   = CreateSolidBrush(c.buttonBg);
        hMenuBgBrush     = CreateSolidBrush(c.menuBg);
        hMenuHiliteBrush = CreateSolidBrush(c.menuHighlight);
        hMenuBarBrush    = CreateSolidBrush(c.menuBar);
        hMenuBarHilBrush = CreateSolidBrush(c.menuBarHighlight);
    }

    // ================================================================
    // Undocumented Windows 10+ uxtheme dark mode API
    // ================================================================
    typedef enum _PREFERRED_APP_MODE {
        APPMODE_DEFAULT    = 0,
        APPMODE_ALLOW_DARK = 1,
        APPMODE_FORCE_DARK = 2,
        APPMODE_FORCE_LIGHT = 3,
    } PREFERRED_APP_MODE;

    typedef BOOL  (WINAPI *pfnAllowDarkModeForWindow)(HWND, BOOL);
    typedef PREFERRED_APP_MODE (WINAPI *pfnSetPreferredAppMode)(PREFERRED_APP_MODE);
    typedef void  (WINAPI *pfnRefreshImmersiveColorPolicyState)(void);

    static pfnAllowDarkModeForWindow        _AllowDarkModeForWindow        = NULL;
    static pfnSetPreferredAppMode           _SetPreferredAppMode           = NULL;
    static pfnRefreshImmersiveColorPolicyState _RefreshImmersiveColorPolicyState = NULL;
    static BOOL darkModeAPIsAvailable = FALSE;

    static void LoadDarkModeAPIs(void)
    {
        HMODULE hUx = GetModuleHandle(_T("uxtheme.dll"));
        if (!hUx) hUx = LoadLibrary(_T("uxtheme.dll"));
        if (!hUx) return;

        _AllowDarkModeForWindow = (pfnAllowDarkModeForWindow)
            GetProcAddress(hUx, MAKEINTRESOURCEA(133));
        _SetPreferredAppMode = (pfnSetPreferredAppMode)
            GetProcAddress(hUx, MAKEINTRESOURCEA(135));
        _RefreshImmersiveColorPolicyState = (pfnRefreshImmersiveColorPolicyState)
            GetProcAddress(hUx, MAKEINTRESOURCEA(104));

        if (_SetPreferredAppMode && _AllowDarkModeForWindow)
        {
            darkModeAPIsAvailable = TRUE;
            _SetPreferredAppMode(APPMODE_ALLOW_DARK);
            if (_RefreshImmersiveColorPolicyState)
                _RefreshImmersiveColorPolicyState();
        }
    }

    // ================================================================
    // DWM dark title bar (Win10 1809+)
    // ================================================================
    void SetTitleBarDark(HWND hWnd, BOOL dark)
    {
        typedef HRESULT (WINAPI *pfnDwmSetWindowAttribute)(HWND, DWORD, LPCVOID, DWORD);
        static pfnDwmSetWindowAttribute pDwm = (pfnDwmSetWindowAttribute)-1;
        if (pDwm == (pfnDwmSetWindowAttribute)-1)
        {
            HMODULE hDwm = GetModuleHandle(_T("dwmapi.dll"));
            if (!hDwm) hDwm = LoadLibrary(_T("dwmapi.dll"));
            pDwm = hDwm ? (pfnDwmSetWindowAttribute)GetProcAddress(hDwm, "DwmSetWindowAttribute") : NULL;
        }
        if (!pDwm) return;

        BOOL val = dark ? TRUE : FALSE;
        // Try attribute 20 (Win10 20H1+), fall back to 19 (Win10 1809)
        if (FAILED(pDwm(hWnd, 20, &val, sizeof(val))))
            pDwm(hWnd, 19, &val, sizeof(val));
    }

    // ================================================================
    // Disable Visual Styles for a control so WM_CTLCOLOR* is honoured
    // ================================================================
    static void DisableVisualStylesForControl(HWND hCtrl)
    {
        // SetWindowTheme with empty strings disables theming for this control,
        // which makes the OS honour WM_CTLCOLORBTN for buttons.
        SetWindowTheme(hCtrl, L"", L"");
    }

    // ================================================================
    // Enumerate child controls and fix them up for dark mode
    // ================================================================
    static void ApplyToChildControls(HWND hParent, BOOL dark)
    {
        HWND hChild = GetWindow(hParent, GW_CHILD);
        while (hChild)
        {
            TCHAR cls[64] = {0};
            GetClassName(hChild, cls, 63);

            if (_tcsicmp(cls, _T("Button")) == 0)
            {
                if (dark)
                    DisableVisualStylesForControl(hChild);
                else
                    SetWindowTheme(hChild, NULL, NULL); // restore default
            }
            else if (_tcsicmp(cls, _T("Edit")) == 0 ||
                     _tcsicmp(cls, _T("ListBox")) == 0 ||
                     _tcsicmp(cls, _T("ComboBox")) == 0)
            {
                // For these controls, AllowDarkModeForWindow handles
                // scrollbar thumb colour on Win10+
                if (darkModeAPIsAvailable)
                    _AllowDarkModeForWindow(hChild, dark);
            }

            // Recurse into combo-box dropdown list etc.
            ApplyToChildControls(hChild, dark);

            hChild = GetWindow(hChild, GW_HWNDNEXT);
        }
    }

    // ================================================================
    // Owner-draw menu support
    // ================================================================

    // Per-item data stored with each owner-draw menu item
    struct MenuItemData
    {
        TCHAR text[256];
        BOOL  isTopLevel; // TRUE = item on the menu bar itself
    };

    // Recursively convert all items in a menu to MF_OWNERDRAW
    static void ConvertMenuToOwnerDraw(HMENU hM, BOOL topLevel)
    {
        int count = GetMenuItemCount(hM);
        for (int i = 0; i < count; i++)
        {
            MENUITEMINFO mii = {0};
            mii.cbSize = sizeof(mii);
            mii.fMask  = MIIM_TYPE | MIIM_SUBMENU | MIIM_DATA | MIIM_STATE;
            if (!GetMenuItemInfo(hM, i, TRUE, &mii))
                continue;

            // Separators stay as-is (they are drawn automatically)
            if (mii.fType & MFT_SEPARATOR)
                continue;

            // Retrieve display text
            TCHAR buf[256] = {0};
            mii.fMask     = MIIM_STRING;
            mii.dwTypeData = buf;
            mii.cch        = 255;
            GetMenuItemInfo(hM, i, TRUE, &mii);

            // Allocate data for the item (freed in RebuildOwnerDrawMenu cleanup
            // or when the menu is destroyed - Windows does not free it for us)
            MenuItemData* pData = new MenuItemData();
            _tcsncpy(pData->text, buf, 255);
            pData->isTopLevel = topLevel;

            // Apply OWNERDRAW
            mii.fMask      = MIIM_TYPE | MIIM_DATA;
            mii.fType      = (mii.fType & ~MFT_STRING) | MFT_OWNERDRAW;
            mii.dwItemData = (ULONG_PTR)pData;
            SetMenuItemInfo(hM, i, TRUE, &mii);

            // Recurse into submenus
            HMENU hSub = GetSubMenu(hM, i);
            if (hSub)
                ConvertMenuToOwnerDraw(hSub, FALSE);
        }
    }

    // Recursively restore menu items to normal MF_STRING
    static void ConvertMenuToString(HMENU hM)
    {
        int count = GetMenuItemCount(hM);
        for (int i = 0; i < count; i++)
        {
            MENUITEMINFO mii = {0};
            mii.cbSize = sizeof(mii);
            mii.fMask  = MIIM_TYPE | MIIM_DATA | MIIM_SUBMENU;
            if (!GetMenuItemInfo(hM, i, TRUE, &mii))
                continue;

            if (mii.fType & MFT_SEPARATOR)
                continue;

            if (mii.fType & MFT_OWNERDRAW)
            {
                MenuItemData* pData = (MenuItemData*)mii.dwItemData;
                TCHAR buf[256]      = {0};
                if (pData) _tcsncpy(buf, pData->text, 255);

                mii.fMask      = MIIM_TYPE | MIIM_DATA;
                mii.fType      = (mii.fType & ~MFT_OWNERDRAW) | MFT_STRING;
                mii.dwTypeData = buf;
                mii.cch        = (UINT)_tcslen(buf);
                mii.dwItemData = 0;
                SetMenuItemInfo(hM, i, TRUE, &mii);

                delete pData;
            }

            HMENU hSub = GetSubMenu(hM, i);
            if (hSub)
                ConvertMenuToString(hSub);
        }
    }

    // ================================================================
    // Subclass WndProc for dialogs (handles background + controls)
    // ================================================================
    static LRESULT CALLBACK ThemeDialogProc(
        HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
        UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
    {
        if (currentMode == MODE_DARK)
        {
            switch (uMsg)
            {
            case WM_CTLCOLORDLG:
            case WM_CTLCOLORSTATIC:
            {
                HDC hdc = (HDC)wParam;
                SetTextColor(hdc, DarkColors.text);
                SetBkColor(hdc, DarkColors.bg);
                SetBkMode(hdc, TRANSPARENT);
                return (LRESULT)hBgBrush;
            }
            case WM_CTLCOLOREDIT:
            {
                HDC hdc = (HDC)wParam;
                SetTextColor(hdc, DarkColors.controlText);
                SetBkColor(hdc, DarkColors.controlBg);
                return (LRESULT)hControlBgBrush;
            }
            case WM_CTLCOLORLISTBOX:
            case WM_CTLCOLORSCROLLBAR:
            {
                HDC hdc = (HDC)wParam;
                SetTextColor(hdc, DarkColors.controlText);
                SetBkColor(hdc, DarkColors.controlBg);
                return (LRESULT)hControlBgBrush;
            }
            case WM_CTLCOLORBTN:
            {
                // Honoured only after SetWindowTheme(btn,"","") has been called
                HDC hdc = (HDC)wParam;
                SetTextColor(hdc, DarkColors.buttonText);
                SetBkColor(hdc, DarkColors.buttonBg);
                SetBkMode(hdc, TRANSPARENT);
                return (LRESULT)hButtonBgBrush;
            }
            case WM_ERASEBKGND:
            {
                HDC hdc = (HDC)wParam;
                RECT rc;
                GetClientRect(hWnd, &rc);
                FillRect(hdc, &rc, hBgBrush);
                return TRUE;
            }
            }
        }

        if (uMsg == WM_DESTROY)
            RemoveWindowSubclass(hWnd, ThemeDialogProc, 0);

        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }

    // ================================================================
    // Force repaint of a window and its children
    // ================================================================
    static void ForceRepaint(HWND hWnd)
    {
        InvalidateRect(hWnd, NULL, TRUE);
        UpdateWindow(hWnd);

        HWND hChild = GetWindow(hWnd, GW_CHILD);
        while (hChild)
        {
            InvalidateRect(hChild, NULL, TRUE);
            UpdateWindow(hChild);
            hChild = GetWindow(hChild, GW_HWNDNEXT);
        }
    }

    // ================================================================
    // Public: Init
    // ================================================================
    void Init(void)
    {
        currentMode = MODE_LIGHT;
        CreateBrushes();
        LoadDarkModeAPIs();
    }

    // ================================================================
    // Public: Destroy
    // ================================================================
    void Destroy(void)
    {
        auto SafeDelete = [](HBRUSH& b) { if (b) { DeleteObject(b); b = NULL; } };
        SafeDelete(hBgBrush);
        SafeDelete(hControlBgBrush);
        SafeDelete(hButtonBgBrush);
        SafeDelete(hMenuBgBrush);
        SafeDelete(hMenuHiliteBrush);
        SafeDelete(hMenuBarBrush);
        SafeDelete(hMenuBarHilBrush);
    }

    // ================================================================
    // Public: GetMode / SetMode
    // ================================================================
    Mode GetMode(void) { return currentMode; }

    void SetMode(Mode mode)
    {
        if (currentMode == mode) return;
        currentMode = mode;
        CreateBrushes();
    }

    // ================================================================
    // Public: ApplyToDialog
    // ================================================================
    void ApplyToDialog(HWND hDlg)
    {
        BOOL dark = (currentMode == MODE_DARK);

        // Subclass for WM_CTLCOLOR* handling (install if dark, remove if light)
        if (dark)
            SetWindowSubclass(hDlg, ThemeDialogProc, 0, 0);
        else
            RemoveWindowSubclass(hDlg, ThemeDialogProc, 0);

        // Dark title bar (Win10+)
        SetTitleBarDark(hDlg, dark);

        // Fix up child controls
        ApplyToChildControls(hDlg, dark);

        // uxtheme for scroll bars etc. (Win10+)
        if (darkModeAPIsAvailable)
            _AllowDarkModeForWindow(hDlg, dark);

        ForceRepaint(hDlg);
    }

    // ================================================================
    // Public: RemoveFromDialog
    // ================================================================
    void RemoveFromDialog(HWND hDlg)
    {
        RemoveWindowSubclass(hDlg, ThemeDialogProc, 0);
    }

    // ================================================================
    // Public: ApplyToMainWindow
    // ================================================================
    void ApplyToMainWindow(HWND hWnd)
    {
        BOOL dark = (currentMode == MODE_DARK);

        // Window class background
        SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND,
            dark ? (LONG_PTR)hBgBrush
                 : (LONG_PTR)(HBRUSH)(COLOR_APPWORKSPACE + 1));

        // Dark title bar (Win10+)
        SetTitleBarDark(hWnd, dark);

        // uxtheme for the window and its controls (Win10+)
        if (darkModeAPIsAvailable)
        {
            _AllowDarkModeForWindow(hWnd, dark);
            if (_RefreshImmersiveColorPolicyState)
                _RefreshImmersiveColorPolicyState();
        }

        InvalidateRect(hWnd, NULL, TRUE);
        UpdateWindow(hWnd);
    }

    // ================================================================
    // Public: RebuildOwnerDrawMenu
    // Must be called AFTER the menu is fully built (including language
    // items and checkmarks), and AFTER SetMode has been called.
    // Pass the HMENU returned by GetMenu(hMainWnd).
    // ================================================================
    void RebuildOwnerDrawMenu(HMENU hMenu)
    {
        if (!hMenu) return;

        // First, strip any existing owner-draw to free old data
        ConvertMenuToString(hMenu);

        if (currentMode == MODE_DARK)
        {
            // Re-apply owner-draw for dark mode
            ConvertMenuToOwnerDraw(hMenu, TRUE);
        }

        // Force the menu bar to repaint
        DrawMenuBar(hMainWnd);
    }

    // ================================================================
    // Public: Reapply
    // Call after SetMode or after language change
    // ================================================================
    void Reapply(void)
    {
        ApplyToMainWindow(hMainWnd);

        if (hDebug)
            ApplyToDialog(hDebug);

#ifdef ENABLE_DEBUGGER
        if (Debugger::CPUWnd)
            ApplyToDialog(Debugger::CPUWnd);
        if (Debugger::PPUWnd)
            ApplyToDialog(Debugger::PPUWnd);
#endif

        // Rebuild owner-draw menu
        RebuildOwnerDrawMenu(GetMenu(hMainWnd));
    }

    // ================================================================
    // Public: SaveSettings / LoadSettings
    // ================================================================
    void SaveSettings(HKEY SettingsBase)
    {
        DWORD val = (DWORD)currentMode;
        RegSetValueEx(SettingsBase, _T("ThemeMode"), 0, REG_DWORD, (LPBYTE)&val, sizeof(DWORD));
    }

    void LoadSettings(HKEY SettingsBase)
    {
        DWORD val  = 0;
        DWORD size = sizeof(DWORD);
        RegQueryValueEx(SettingsBase, _T("ThemeMode"), 0, NULL, (LPBYTE)&val, &size);
        currentMode = (Mode)val;
        CreateBrushes();
    }

    // ================================================================
    // Public: SyncMenuChecks
    // ================================================================
    void SyncMenuChecks(void)
    {
        if (!hMenu) return;
        CheckMenuItem(hMenu, ID_PPU_THEME_DARK,
            (currentMode == MODE_DARK) ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_PPU_THEME_LIGHT,
            (currentMode == MODE_LIGHT) ? MF_CHECKED : MF_UNCHECKED);
    }

    // ================================================================
    // Public: Accessors
    // ================================================================
    HBRUSH   GetBackgroundBrush(void)    { return hBgBrush; }
    HBRUSH   GetControlBrush(void)       { return hControlBgBrush; }
    COLORREF GetBgColor(void)            { return Colors().bg; }
    COLORREF GetTextColor(void)          { return Colors().text; }
    COLORREF GetControlBgColor(void)     { return Colors().controlBg; }
    COLORREF GetControlTextColor(void)   { return Colors().controlText; }
    bool     IsDark(void)                { return currentMode == MODE_DARK; }

    // ================================================================
    // Owner-draw menu message handlers
    // These must be called from the main WndProc (WM_MEASUREITEM,
    // WM_DRAWITEM).  See Step 3 for where to call them.
    // ================================================================

    // Call from WM_MEASUREITEM in WndProc
    void HandleMeasureItem(HWND /*hWnd*/, MEASUREITEMSTRUCT* pMIS)
    {
        if (pMIS->CtlType != ODT_MENU) return;
        MenuItemData* pData = (MenuItemData*)pMIS->itemData;
        if (!pData) return;

        HDC hdc = GetDC(hMainWnd);
        HFONT hOldFont = NULL;
        NONCLIENTMETRICS ncm = {0};
        ncm.cbSize = sizeof(ncm);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
        HFONT hFont = CreateFontIndirect(&ncm.lfMenuFont);
        if (hFont) hOldFont = (HFONT)SelectObject(hdc, hFont);

        SIZE sz = {0};
        // Remove accelerator tab character for measurement
        TCHAR displayText[256];
        _tcsncpy(displayText, pData->text, 255);
        TCHAR* tab = _tcschr(displayText, _T('\t'));
        if (tab) *tab = 0;

        GetTextExtentPoint32(hdc, displayText, (int)_tcslen(displayText), &sz);

        if (hOldFont) SelectObject(hdc, hOldFont);
        if (hFont) DeleteObject(hFont);
        ReleaseDC(hMainWnd, hdc);

        if (pData->isTopLevel)
        {
            pMIS->itemWidth  = sz.cx + 12;
            pMIS->itemHeight = sz.cy + 6;
        }
        else
        {
            pMIS->itemWidth  = sz.cx + 50; // room for check/icon on left, accel on right
            pMIS->itemHeight = sz.cy + 4;
        }
    }

    // Call from WM_DRAWITEM in WndProc
    void HandleDrawItem(HWND /*hWnd*/, DRAWITEMSTRUCT* pDIS)
    {
        if (pDIS->CtlType != ODT_MENU) return;
        MenuItemData* pData = (MenuItemData*)pDIS->itemData;
        if (!pData) return;

        const ThemeColors& c = Colors();
        HDC    hdc  = pDIS->hDC;
        RECT   rc   = pDIS->rcItem;
        BOOL   sel  = (pDIS->itemState & ODS_SELECTED)  != 0;
        BOOL   gray = (pDIS->itemState & ODS_GRAYED)    != 0;
        BOOL   chk  = (pDIS->itemState & ODS_CHECKED)   != 0;

        // ---- Background ----
        HBRUSH hBg;
        COLORREF clrText;

        if (pData->isTopLevel)
        {
            hBg     = sel ? hMenuBarHilBrush : hMenuBarBrush;
            clrText = gray ? c.menuGray : (sel ? c.menuHighlightText : c.menuBarText);
        }
        else
        {
            hBg     = sel ? hMenuHiliteBrush : hMenuBgBrush;
            clrText = gray ? c.menuGray : (sel ? c.menuHighlightText : c.menuText);
        }

        FillRect(hdc, &rc, hBg);

        // ---- Checkmark (non-top-level only) ----
        if (!pData->isTopLevel && chk)
        {
            RECT rcCheck = rc;
            rcCheck.right = rcCheck.left + 20;

            SetTextColor(hdc, clrText);
            SetBkMode(hdc, TRANSPARENT);

            // Draw a simple checkmark using the Marlett font (available on all Windows)
            LOGFONT lf = {0};
            lf.lfHeight    = rc.bottom - rc.top - 2;
            lf.lfCharSet   = SYMBOL_CHARSET;
            _tcscpy(lf.lfFaceName, _T("Marlett"));
            HFONT hMarlettFont = CreateFontIndirect(&lf);
            HFONT hOld = (HFONT)SelectObject(hdc, hMarlettFont);
            DrawText(hdc, _T("a"), 1, &rcCheck, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            SelectObject(hdc, hOld);
            DeleteObject(hMarlettFont);
        }

        // ---- Text ----
        NONCLIENTMETRICS ncm = {0};
        ncm.cbSize = sizeof(ncm);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
        HFONT hMenuFont = CreateFontIndirect(&ncm.lfMenuFont);
        HFONT hOldFont  = (HFONT)SelectObject(hdc, hMenuFont);

        SetTextColor(hdc, clrText);
        SetBkMode(hdc, TRANSPARENT);

        RECT rcText = rc;
        if (!pData->isTopLevel)
            rcText.left += 24; // leave room for check/icon

        // Split text on tab (accelerator)
        TCHAR main[256] = {0}, accel[256] = {0};
        _tcsncpy(main, pData->text, 255);
        TCHAR* tab = _tcschr(main, _T('\t'));
        if (tab) { *tab = 0; _tcsncpy(accel, tab + 1, 255); }

        // Draw main label
        DrawText(hdc, main, -1, &rcText,
            DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_HIDEPREFIX);

        // Draw accelerator right-aligned
        if (accel[0])
        {
            RECT rcAccel = rc;
            rcAccel.right -= 8;
            DrawText(hdc, accel, -1, &rcAccel,
                DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
        }

        SelectObject(hdc, hOldFont);
        DeleteObject(hMenuFont);

        // ---- Border for top-level selected/focused ----
        if (pData->isTopLevel && sel)
        {
            HPEN hPen  = CreatePen(PS_SOLID, 1, c.menuBorder);
            HPEN hOldP = (HPEN)SelectObject(hdc, hPen);
            POINT pts[5] = {
                {rc.left, rc.top}, {rc.right-1, rc.top},
                {rc.right-1, rc.bottom-1}, {rc.left, rc.bottom-1},
                {rc.left, rc.top}
            };
            Polyline(hdc, pts, 5);
            SelectObject(hdc, hOldP);
            DeleteObject(hPen);
        }
    }

} // namespace Theme
