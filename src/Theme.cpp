/* Nintendulator - Dark/Light Theme engine implementation */

#include "stdafx.h"
#include "Theme.h"
#include "Nintendulator.h"
#include "resource.h"
#include "Lang.h"
#include "Debugger.h"
#include <commctrl.h>
#include <uxtheme.h>
#include <dwmapi.h>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "uxtheme.lib")

namespace Theme
{
    // ============================================================
    // State
    // ============================================================
    static Mode currentMode = MODE_LIGHT;

    // ============================================================
    // Color palettes
    // ============================================================
    struct ThemeColors
    {
        COLORREF bg, text;
        COLORREF controlBg, controlText;
        COLORREF buttonBg, buttonText;
        COLORREF menuBg, menuText;
        COLORREF menuHilite, menuHiliteText;
        COLORREF menuBorder, menuDisabled;
        COLORREF menuBar, menuBarText;
        COLORREF menuBarHilite;
    };

    static const ThemeColors Light = {
        RGB(240,240,240), RGB(0,0,0),
        RGB(255,255,255), RGB(0,0,0),
        RGB(225,225,225), RGB(0,0,0),
        RGB(255,255,255), RGB(0,0,0),
        RGB(0,120,215),   RGB(255,255,255),
        RGB(200,200,200), RGB(130,130,130),
        RGB(240,240,240), RGB(0,0,0),
        RGB(205,227,247),
    };

    static const ThemeColors Dark = {
        RGB(30,30,30),    RGB(212,212,212),
        RGB(45,45,45),    RGB(212,212,212),
        RGB(55,55,55),    RGB(212,212,212),
        RGB(40,40,40),    RGB(212,212,212),
        RGB(0,122,204),   RGB(255,255,255),
        RGB(80,80,80),    RGB(120,120,120),
        RGB(30,30,30),    RGB(212,212,212),
        RGB(60,60,60),
    };

    static const ThemeColors& C() { return currentMode == MODE_DARK ? Dark : Light; }

    // ============================================================
    // GDI brushes
    // ============================================================
    static HBRUSH hBgBrush    = NULL;
    static HBRUSH hCtrlBrush  = NULL;
    static HBRUSH hBtnBrush   = NULL;
    static HBRUSH hMnuBrush   = NULL;
    static HBRUSH hMnuHilBrush= NULL;
    static HBRUSH hMnuBarBrush= NULL;
    static HBRUSH hMnuBarHil  = NULL;

    static void DeleteBrush(HBRUSH& b) { if (b) { DeleteObject(b); b = NULL; } }

    static void CreateBrushes()
    {
        const ThemeColors& c = C();
        DeleteBrush(hBgBrush);   DeleteBrush(hCtrlBrush); DeleteBrush(hBtnBrush);
        DeleteBrush(hMnuBrush);  DeleteBrush(hMnuHilBrush);
        DeleteBrush(hMnuBarBrush); DeleteBrush(hMnuBarHil);

        hBgBrush     = CreateSolidBrush(c.bg);
        hCtrlBrush   = CreateSolidBrush(c.controlBg);
        hBtnBrush    = CreateSolidBrush(c.buttonBg);
        hMnuBrush    = CreateSolidBrush(c.menuBg);
        hMnuHilBrush = CreateSolidBrush(c.menuHilite);
        hMnuBarBrush = CreateSolidBrush(c.menuBar);
        hMnuBarHil   = CreateSolidBrush(c.menuBarHilite);
    }

    // ============================================================
    // Undocumented uxtheme APIs (Win10 1903+ only, graceful fallback)
    // ============================================================
    typedef enum { APPMODE_DEFAULT=0, APPMODE_ALLOW_DARK=1 } PREFERRED_APP_MODE;
    typedef BOOL (WINAPI *pfnAllow)(HWND,BOOL);
    typedef PREFERRED_APP_MODE (WINAPI *pfnSetMode)(PREFERRED_APP_MODE);
    typedef void (WINAPI *pfnRefresh)(void);

    static pfnAllow   _Allow   = NULL;
    static pfnSetMode _SetMode = NULL;
    static pfnRefresh _Refresh = NULL;
    static BOOL       darkAPIs = FALSE;

    static void LoadDarkAPIs()
    {
        HMODULE hUx = LoadLibrary(_T("uxtheme.dll"));
        if (!hUx) return;
        _Allow   = (pfnAllow)  GetProcAddress(hUx, MAKEINTRESOURCEA(133));
        _SetMode = (pfnSetMode)GetProcAddress(hUx, MAKEINTRESOURCEA(135));
        _Refresh = (pfnRefresh)GetProcAddress(hUx, MAKEINTRESOURCEA(104));
        if (_Allow && _SetMode) {
            darkAPIs = TRUE;
            _SetMode(APPMODE_ALLOW_DARK);
            if (_Refresh) _Refresh();
        }
    }

    // ============================================================
    // Dark title bar (Win10 1809+)
    // ============================================================
    void SetTitleBarDark(HWND hWnd, BOOL dark)
    {
        BOOL v = dark;
        // Attribute 20 = Win10 20H1+, attribute 19 = Win10 1809
        if (FAILED(DwmSetWindowAttribute(hWnd, 20, &v, sizeof(v))))
            DwmSetWindowAttribute(hWnd, 19, &v, sizeof(v));
    }

    // ============================================================
    // Fix up button controls: disable Visual Styles so
    // WM_CTLCOLORBTN is honoured by the OS
    // ============================================================
    static void FixButtons(HWND hParent, BOOL dark)
    {
        HWND h = GetWindow(hParent, GW_CHILD);
        while (h) {
            TCHAR cls[64] = {};
            GetClassName(h, cls, 63);
            if (_tcsicmp(cls, _T("Button")) == 0) {
                // Disabling visual styles makes the classic draw path active,
                // which then respects WM_CTLCOLORBTN from the parent dialog.
                SetWindowTheme(h, dark ? L"" : NULL, dark ? L"" : NULL);
                InvalidateRect(h, NULL, TRUE);
            }
            // Recurse (ComboBox contains a child edit/button etc.)
            FixButtons(h, dark);
            h = GetWindow(h, GW_HWNDNEXT);
        }
    }

    // ============================================================
    // Dialog subclass — handles all WM_CTLCOLOR* messages
    // ============================================================
    static LRESULT CALLBACK DlgProc(
        HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam,
        UINT_PTR, DWORD_PTR)
    {
        if (currentMode == MODE_DARK)
        {
            switch (msg)
            {
            case WM_CTLCOLORDLG:
            case WM_CTLCOLORSTATIC: {
                HDC dc = (HDC)wParam;
                SetTextColor(dc, Dark.text);
                SetBkColor(dc, Dark.bg);
                SetBkMode(dc, TRANSPARENT);
                return (LRESULT)hBgBrush;
            }
            case WM_CTLCOLOREDIT:
            case WM_CTLCOLORLISTBOX:
            case WM_CTLCOLORSCROLLBAR: {
                HDC dc = (HDC)wParam;
                SetTextColor(dc, Dark.controlText);
                SetBkColor(dc, Dark.controlBg);
                return (LRESULT)hCtrlBrush;
            }
            case WM_CTLCOLORBTN: {
                // Works only after SetWindowTheme(btn,"","")
                HDC dc = (HDC)wParam;
                SetTextColor(dc, Dark.buttonText);
                SetBkColor(dc, Dark.buttonBg);
                SetBkMode(dc, TRANSPARENT);
                return (LRESULT)hBtnBrush;
            }
            case WM_ERASEBKGND: {
                RECT rc;
                GetClientRect(hWnd, &rc);
                FillRect((HDC)wParam, &rc, hBgBrush);
                return TRUE;
            }
            }
        }
        if (msg == WM_DESTROY)
            RemoveWindowSubclass(hWnd, DlgProc, 0);
        return DefSubclassProc(hWnd, msg, wParam, lParam);
    }

    static void ForceRepaint(HWND hWnd)
    {
        RedrawWindow(hWnd, NULL, NULL,
            RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_ERASE);
    }

    // ============================================================
    // Owner-draw menu
    // ============================================================
    struct ItemData {
        TCHAR text[256];
        BOOL  topLevel;
    };

    // Get system menu font height — used for correct item sizing
    static int GetMenuFontHeight()
    {
        NONCLIENTMETRICS ncm = {};
        ncm.cbSize = sizeof(ncm);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
        // Create font and measure an actual character
        HDC hdc = GetDC(NULL);
        HFONT hf = CreateFontIndirect(&ncm.lfMenuFont);
        HFONT old = (HFONT)SelectObject(hdc, hf);
        TEXTMETRIC tm = {};
        GetTextMetrics(hdc, &tm);
        SelectObject(hdc, old);
        DeleteObject(hf);
        ReleaseDC(NULL, hdc);
        return tm.tmHeight;
    }

    static void ConvertToOwnerDraw(HMENU hM, BOOL top)
    {
        int n = GetMenuItemCount(hM);
        for (int i = 0; i < n; i++) {
            MENUITEMINFO mi = {}; mi.cbSize = sizeof(mi);
            mi.fMask = MIIM_FTYPE | MIIM_DATA | MIIM_STRING | MIIM_SUBMENU;
            if (!GetMenuItemInfo(hM, i, TRUE, &mi)) continue;
            if (mi.fType & MFT_SEPARATOR) continue;

            // Read text
            TCHAR buf[256] = {};
            mi.fMask = MIIM_STRING; mi.dwTypeData = buf; mi.cch = 255;
            GetMenuItemInfo(hM, i, TRUE, &mi);

            ItemData* d = new ItemData();
            _tcsncpy(d->text, buf, 255);
            d->topLevel = top;

            mi.fMask     = MIIM_FTYPE | MIIM_DATA;
            mi.fType     = MFT_OWNERDRAW;
            mi.dwItemData = (ULONG_PTR)d;
            SetMenuItemInfo(hM, i, TRUE, &mi);

            if (mi.hSubMenu)
                ConvertToOwnerDraw(mi.hSubMenu, FALSE);
        }
    }

    static void ConvertToString(HMENU hM)
    {
        int n = GetMenuItemCount(hM);
        for (int i = 0; i < n; i++) {
            MENUITEMINFO mi = {}; mi.cbSize = sizeof(mi);
            mi.fMask = MIIM_FTYPE | MIIM_DATA | MIIM_SUBMENU;
            if (!GetMenuItemInfo(hM, i, TRUE, &mi)) continue;
            if (mi.fType & MFT_SEPARATOR) continue;
            if (mi.fType & MFT_OWNERDRAW) {
                ItemData* d = (ItemData*)mi.dwItemData;
                TCHAR buf[256] = {};
                if (d) _tcsncpy(buf, d->text, 255);
                mi.fMask      = MIIM_FTYPE | MIIM_DATA | MIIM_STRING;
                mi.fType      = MFT_STRING;
                mi.dwTypeData = buf;
                mi.cch        = (UINT)_tcslen(buf);
                mi.dwItemData = 0;
                SetMenuItemInfo(hM, i, TRUE, &mi);
                delete d;
            }
            if (mi.hSubMenu) ConvertToString(mi.hSubMenu);
        }
    }

    // ============================================================
    // Public API
    // ============================================================

    void Init()
    {
        currentMode = MODE_LIGHT;
        CreateBrushes();
        LoadDarkAPIs();
    }

    void Destroy()
    {
        DeleteBrush(hBgBrush);  DeleteBrush(hCtrlBrush);
        DeleteBrush(hBtnBrush); DeleteBrush(hMnuBrush);
        DeleteBrush(hMnuHilBrush); DeleteBrush(hMnuBarBrush);
        DeleteBrush(hMnuBarHil);
    }

    Mode GetMode()  { return currentMode; }

    void SetMode(Mode mode)
    {
        if (currentMode == mode) return;
        currentMode = mode;
        CreateBrushes();
    }

    void ApplyToDialog(HWND hDlg)
    {
        BOOL dark = (currentMode == MODE_DARK);
        if (dark)
            SetWindowSubclass(hDlg, DlgProc, 0, 0);
        else
            RemoveWindowSubclass(hDlg, DlgProc, 0);

        SetTitleBarDark(hDlg, dark);
        if (darkAPIs) _Allow(hDlg, dark);
        FixButtons(hDlg, dark);
        ForceRepaint(hDlg);
    }

    void RemoveFromDialog(HWND hDlg)
    {
        RemoveWindowSubclass(hDlg, DlgProc, 0);
    }

    void ApplyToMainWindow(HWND hWnd)
    {
        BOOL dark = (currentMode == MODE_DARK);
        SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND,
            dark ? (LONG_PTR)hBgBrush
                 : (LONG_PTR)(HBRUSH)(COLOR_APPWORKSPACE + 1));
        SetTitleBarDark(hWnd, dark);
        if (darkAPIs) {
            _Allow(hWnd, dark);
            if (_Refresh) _Refresh();
        }
        RedrawWindow(hWnd, NULL, NULL,
            RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_FRAME);
    }

    void RebuildOwnerDrawMenu(HMENU hMenu)
    {
        if (!hMenu) return;
        ConvertToString(hMenu);
        if (currentMode == MODE_DARK)
            ConvertToOwnerDraw(hMenu, TRUE);
        DrawMenuBar(hMainWnd);
    }

    void Reapply()
    {
        ApplyToMainWindow(hMainWnd);
        if (hDebug) ApplyToDialog(hDebug);
#ifdef ENABLE_DEBUGGER
        if (Debugger::CPUWnd) ApplyToDialog(Debugger::CPUWnd);
        if (Debugger::PPUWnd) ApplyToDialog(Debugger::PPUWnd);
#endif
        RebuildOwnerDrawMenu(GetMenu(hMainWnd));
    }

    void SaveSettings(HKEY base)
    {
        DWORD v = currentMode;
        RegSetValueEx(base, _T("ThemeMode"), 0, REG_DWORD, (LPBYTE)&v, sizeof(v));
    }

    void LoadSettings(HKEY base)
    {
        DWORD v = 0, sz = sizeof(v);
        RegQueryValueEx(base, _T("ThemeMode"), 0, NULL, (LPBYTE)&v, &sz);
        currentMode = (Mode)v;
        CreateBrushes();
    }

    void SyncMenuChecks()
    {
        if (!hMenu) return;
        CheckMenuItem(hMenu, ID_PPU_THEME_DARK,
            currentMode == MODE_DARK ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem(hMenu, ID_PPU_THEME_LIGHT,
            currentMode == MODE_LIGHT ? MF_CHECKED : MF_UNCHECKED);
    }

    HBRUSH   GetBackgroundBrush()   { return hBgBrush;  }
    HBRUSH   GetControlBrush()      { return hCtrlBrush; }
    COLORREF GetBgColor()           { return C().bg;     }
    COLORREF GetTextColor()         { return C().text;   }
    COLORREF GetControlBgColor()    { return C().controlBg;   }
    COLORREF GetControlTextColor()  { return C().controlText; }
    bool     IsDark()               { return currentMode == MODE_DARK; }

    // ============================================================
    // WM_MEASUREITEM — fixes the vertical spacing of menu items
    // ============================================================
    void HandleMeasureItem(HWND /*hWnd*/, MEASUREITEMSTRUCT* p)
    {
        if (p->CtlType != ODT_MENU) return;
        ItemData* d = (ItemData*)p->itemData;
        if (!d) return;

        // Use the actual system menu font metrics for correct height
        NONCLIENTMETRICS ncm = {}; ncm.cbSize = sizeof(ncm);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);

        HDC hdc  = GetDC(hMainWnd);
        HFONT hf = CreateFontIndirect(&ncm.lfMenuFont);
        HFONT old= (HFONT)SelectObject(hdc, hf);

        // Measure text (strip tab/accelerator part)
        TCHAR txt[256]; _tcsncpy(txt, d->text, 255);
        TCHAR* tab = _tcschr(txt, _T('\t')); if (tab) *tab = 0;

        SIZE sz = {};
        GetTextExtentPoint32(hdc, txt, (int)_tcslen(txt), &sz);
        SelectObject(hdc, old);
        DeleteObject(hf);
        ReleaseDC(hMainWnd, hdc);

        // --- KEY FIX: match system menu item proportions exactly ---
        // GetSystemMetrics(SM_CYMENU) gives the standard menu bar height.
        // For popup items we want font-height + 2px padding (top+bottom).
        int fontH = sz.cy;

        if (d->topLevel) {
            // Top-level bar items: width = text + 16px padding, height = SM_CYMENU
            p->itemWidth  = (UINT)(sz.cx + 16);
            p->itemHeight = (UINT)GetSystemMetrics(SM_CYMENU);
        } else {
            // Popup items: height = font height + 4px (2 top, 2 bottom)
            // Width accounts for: check column (20) + text + accel gap (30)
            p->itemHeight = (UINT)(fontH + 4);
            p->itemWidth  = (UINT)(sz.cx + 50);
        }
    }

    // ============================================================
    // WM_DRAWITEM — paints menu items
    // ============================================================
    void HandleDrawItem(HWND /*hWnd*/, DRAWITEMSTRUCT* p)
    {
        if (p->CtlType != ODT_MENU) return;
        ItemData* d = (ItemData*)p->itemData;
        if (!d) return;

        const ThemeColors& c = C();
        HDC  dc  = p->hDC;
        RECT rc  = p->rcItem;
        BOOL sel  = (p->itemState & ODS_SELECTED) != 0;
        BOOL gray = (p->itemState & ODS_GRAYED)   != 0;
        BOOL chk  = (p->itemState & ODS_CHECKED)  != 0;

        // --- Background ---
        HBRUSH bgBrush;
        COLORREF fgColor;
        if (d->topLevel) {
            bgBrush = sel ? hMnuBarHil : hMnuBarBrush;
            fgColor = gray ? c.menuDisabled
                           : (sel ? c.menuHiliteText : c.menuBarText);
        } else {
            bgBrush = sel ? hMnuHilBrush : hMnuBrush;
            fgColor = gray ? c.menuDisabled
                           : (sel ? c.menuHiliteText : c.menuText);
        }
        FillRect(dc, &rc, bgBrush);

        // --- System menu font ---
        NONCLIENTMETRICS ncm = {}; ncm.cbSize = sizeof(ncm);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
        HFONT hf  = CreateFontIndirect(&ncm.lfMenuFont);
        HFONT old = (HFONT)SelectObject(dc, hf);
        SetTextColor(dc, fgColor);
        SetBkMode(dc, TRANSPARENT);

        // --- Checkmark (popup items only) ---
        if (!d->topLevel && chk) {
            LOGFONT lf = {}; lf.lfCharSet = SYMBOL_CHARSET;
            lf.lfHeight = rc.bottom - rc.top - 2;
            _tcscpy(lf.lfFaceName, _T("Marlett"));
            HFONT hm  = CreateFontIndirect(&lf);
            HFONT oldm = (HFONT)SelectObject(dc, hm);
            RECT rcChk = { rc.left, rc.top, rc.left + 20, rc.bottom };
            DrawText(dc, _T("a"), 1, &rcChk,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            SelectObject(dc, oldm);
            DeleteObject(hm);
            SelectObject(dc, hf); // restore menu font
            SetTextColor(dc, fgColor);
            SetBkMode(dc, TRANSPARENT);
        }

        // --- Text ---
        TCHAR main[256] = {}, accel[256] = {};
        _tcsncpy(main, d->text, 255);
        TCHAR* tab = _tcschr(main, _T('\t'));
        if (tab) { *tab = 0; _tcsncpy(accel, tab + 1, 255); }

        RECT rcText = rc;
        if (!d->topLevel) rcText.left += 24;   // space for check column
        else              rcText.left += 6;    // small left padding on bar

        DrawText(dc, main, -1, &rcText,
            DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_HIDEPREFIX);

        if (accel[0]) {
            RECT rcA = rc; rcA.right -= 8;
            DrawText(dc, accel, -1, &rcA,
                DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
        }

        SelectObject(dc, old);
        DeleteObject(hf);

        // --- Thin border around selected top-level item ---
        if (d->topLevel && sel) {
            HPEN pen  = CreatePen(PS_SOLID, 1, c.menuBorder);
            HPEN oldp = (HPEN)SelectObject(dc, pen);
            POINT pts[5] = {
                {rc.left,rc.top},{rc.right-1,rc.top},
                {rc.right-1,rc.bottom-1},{rc.left,rc.bottom-1},
                {rc.left,rc.top}
            };
            Polyline(dc, pts, 5);
            SelectObject(dc, oldp);
            DeleteObject(pen);
        }
    }

} // namespace Theme
