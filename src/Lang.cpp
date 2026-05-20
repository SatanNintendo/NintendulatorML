/*
 * NintendulatorML — система локализации
 * src/Lang.cpp
 */

#include "stdafx.h"
#include "Lang.h"
#include "Nintendulator.h"
#include <fstream>
#include <algorithm>

// ============================================================
// СТРОКИ ПО УМОЛЧАНИЮ (английские)
// ============================================================
static const TCHAR *DefaultStrings[LANG_STRING_COUNT] =
{
    // Меню
    _T("&File"),
    _T("&Open ROM..."),
    _T("&Close"),
    _T("Recent &Files"),
    _T("&Clear Recent Files"),
    _T("E&xit"),
    _T("&NES"),
    _T("&Run"),
    _T("&Pause"),
    _T("&Reset"),
    _T("Power &Cycle"),
    _T("&Stop"),
    _T("&Settings"),
    _T("&Video..."),
    _T("&Sound..."),
    _T("&Controllers..."),
    _T("&Game Genie..."),
    _T("&Paths..."),
    _T("&Clock speed..."),
    _T("&Movie"),
    _T("&Record..."),
    _T("&Play..."),
    _T("&Stop"),
    _T("&Debug"),
    _T("&CPU Debugger"),
    _T("&PPU Debugger"),
    _T("&Language"),
    _T("&Help"),
    _T("&About..."),

    // Заголовок окна
    _T("Nintendulator"),
    _T("Nintendulator [Paused]"),
    _T("Nintendulator - %s - %d FPS"),

    // Открытие файлов
    _T("Open ROM"),
    _T("NES ROMs (*.nes;*.unif;*.fds;*.nsf)\0*.nes;*.unif;*.fds;*.nsf\0All Files (*.*)\0*.*\0"),
    _T("Open Movie"),
    _T("Save Movie"),
    _T("Open Palette"),
    _T("Save AVI"),

    // Ошибки ROM
    _T("Failed to open ROM file"),
    _T("Invalid ROM file format"),
    _T("Unsupported mapper"),
    _T("Invalid UNIF ROM"),
    _T("Invalid FDS header"),
    _T("Invalid FDS disk side"),
    _T("Invalid NSF file"),
    _T("ROM loaded: %s"),
    _T("ROM closed"),
    _T("A ROM is already loaded. Close it first."),

    // Видео
    _T("Failed to initialize DirectDraw"),
    _T("Failed to create DirectDraw surface"),
    _T("Failed to set DirectDraw palette"),
    _T("DirectDraw render error"),
    _T("Unsupported video mode"),
    _T("Screenshot saved: %s"),

    // Звук
    _T("Failed to initialize DirectSound"),
    _T("Failed to create DirectSound buffer"),
    _T("Unsupported audio format"),
    _T("Sound disabled"),

    // Controllers device list
    _T("Unconnected"),
    _T("Standard Controller"),
    _T("Zapper"),
    _T("Arkanoid Paddle"),
    _T("Power Pad"),
    _T("Four Score (port 1 only)"),
    _T("SNES Controller"),
    _T("VS Unisystem Zapper"),
    _T("SNES Mouse"),
    _T("Four Score (port 2 only)"),
    _T("Famicom 4-Player Adapter"),
    _T("Famicom Arkanoid Paddle"),
    _T("Family Basic Keyboard"),
    _T("Subor Keyboard"),
    _T("Family Trainer"),
    _T("Oeka Kids Tablet"),

    // Controllers dialog
    _T("Controller Port &1:"),
    _T("Controller Port &2:"),
    _T("&Expansion Port:"),
    _T("&Config"),
    _T("&Allow simultaneous Left+Right and Up+Down"),
    _T("Configure &POV triggers as 8-way buttons"),
    
    // Контроллеры
    _T("Unable to initialize keyboard input device!"),
    _T("Unable to set keyboard input data format!"),
    _T("Unable to set keyboard input cooperative level!"),
    _T("Unable to initialize mouse input device!"),
    _T("Unable to set mouse input cooperative level!"),
    _T("Unable to restore joystick input cooperative level!"),
    _T("Error: selected invalid controller type for standard port!"),
    _T("Error: selected invalid controller type for expansion port!"),

    // Состояния
    _T("State saved to slot %d"),
    _T("State loaded from slot %d"),
    _T("Failed to save state"),
    _T("Failed to load state"),
    _T("State file version mismatch"),
    _T("State file is corrupted"),
    _T("Slot %d"),

    // Запись
    _T("Recording movie..."),
    _T("Playing movie..."),
    _T("Movie stopped"),
    _T("Failed to open movie file"),
    _T("Invalid movie file"),
    _T("Movie version mismatch"),
    _T("Recording AVI..."),
    _T("Failed to open AVI file"),
    _T("AVI recording stopped"),

    // Game Genie
    _T("Invalid Game Genie code"),
    _T("Too many Game Genie codes (maximum 3)"),
    _T("Game Genie code applied"),

    // Маппер
    _T("Failed to load %s - not a valid DLL file!"),
    _T("Mapper not found"),
    _T("Failed to load mapper pack %s - version mismatch!"),

    // Отладчик
    _T("Exec: $%04X"),
    _T("Read: $%04X"),
    _T("Write: $%04X"),
    _T("Access: $%04X"),
    _T("Opcode: $%02X"),
    _T("Interrupt: NMI"),
    _T("Interrupt: IRQ"),
    _T("Interrupt: BRK"),
    _T("Scanline: %i"),
    _T("UNDEFINED"),
    _T(" (+)"),
    _T(" (-)"),
    _T("Logging started"),
    _T("Logging stopped"),

    // Диалоги
    _T("OK"),
    _T("Cancel"),
    _T("Yes"),
    _T("No"),
    _T("Apply"),
    _T("Browse..."),
    _T("Nintendulator"),

    // About
    _T("About Nintendulator"),
    _T("Nintendulator\nNES Emulator\n\nCopyright (C) QMT Productions"),

    // Language
    _T("Language Changed"),
    _T("Language has been changed. Some elements will update on restart."),

    // === CPU ===
    _T("Bad opcode, CPU locked")   // ← Эта строка была пропущена
};

// ============================================================
// Внутренние данные
// ============================================================
namespace
{
    std::wstring Strings[LANG_STRING_COUNT];
    std::vector<std::wstring> LanguageList;
    std::wstring CurrentLanguage = L"English";
    TCHAR LangDir[MAX_PATH] = { 0 };

    void ResetToDefaults()
    {
        for (int i = 0; i < LANG_STRING_COUNT; i++)
            Strings[i] = DefaultStrings[i];
    }

    std::wstring Trim(const std::wstring &s)
    {
        size_t a = s.find_first_not_of(L" \t\r\n");
        if (a == std::wstring::npos) return L"";
        size_t b = s.find_last_not_of(L" \t\r\n");
        return s.substr(a, b - a + 1);
    }

    void ScanLangDir()
    {
        LanguageList.clear();
        TCHAR mask[MAX_PATH];
        _tcscpy(mask, LangDir);
        _tcscat(mask, _T("*.lng"));
        WIN32_FIND_DATA fd;
        HANDLE h = FindFirstFile(mask, &fd);
        if (h == INVALID_HANDLE_VALUE) return;
        do {
            std::wstring name = fd.cFileName;
            size_t dot = name.rfind(L'.');
            if (dot != std::wstring::npos) name = name.substr(0, dot);
            LanguageList.push_back(name);
        } while (FindNextFile(h, &fd));
        FindClose(h);
    }
}

// ============================================================
// Таблица сопоставления ключей
// ============================================================
struct KeyMapping { const wchar_t *key; LangStringID id; };
static const KeyMapping KeyMap[] =
{
    { L"MENU_FILE",                 LANG_MENU_FILE },
    { L"MENU_FILE_OPEN",            LANG_MENU_FILE_OPEN },
    { L"MENU_FILE_CLOSE",           LANG_MENU_FILE_CLOSE },
    { L"MENU_FILE_RECENT",          LANG_MENU_FILE_RECENT },
    { L"MENU_FILE_RECENT_CLEAR",    LANG_MENU_FILE_RECENT_CLEAR },
    { L"MENU_FILE_EXIT",            LANG_MENU_FILE_EXIT },
    { L"MENU_NES",                  LANG_MENU_NES },
    { L"MENU_NES_RUN",              LANG_MENU_NES_RUN },
    { L"MENU_NES_PAUSE",            LANG_MENU_NES_PAUSE },
    { L"MENU_NES_RESET",            LANG_MENU_NES_RESET },
    { L"MENU_NES_POWER",            LANG_MENU_NES_POWER },
    { L"MENU_NES_STOP",             LANG_MENU_NES_STOP },
    { L"MENU_SETTINGS",             LANG_MENU_SETTINGS },
    { L"MENU_SETTINGS_VIDEO",       LANG_MENU_SETTINGS_VIDEO },
    { L"MENU_SETTINGS_SOUND",       LANG_MENU_SETTINGS_SOUND },
    { L"MENU_SETTINGS_CONTROLLERS", LANG_MENU_SETTINGS_CONTROLLERS },
    { L"MENU_SETTINGS_GAMEGENIE",   LANG_MENU_SETTINGS_GAMEGENIE },
    { L"MENU_SETTINGS_PATHS",       LANG_MENU_SETTINGS_PATHS },
    { L"MENU_SETTINGS_CLOCK",       LANG_MENU_SETTINGS_CLOCK },
    { L"MENU_MOVIE",                LANG_MENU_MOVIE },
    { L"MENU_MOVIE_RECORD",         LANG_MENU_MOVIE_RECORD },
    { L"MENU_MOVIE_PLAY",           LANG_MENU_MOVIE_PLAY },
    { L"MENU_MOVIE_STOP",           LANG_MENU_MOVIE_STOP },
    { L"MENU_DEBUG",                LANG_MENU_DEBUG },
    { L"MENU_DEBUG_CPU",            LANG_MENU_DEBUG_CPU },
    { L"MENU_DEBUG_PPU",            LANG_MENU_DEBUG_PPU },
    { L"MENU_LANGUAGE",             LANG_MENU_LANGUAGE },
    { L"MENU_HELP",                 LANG_MENU_HELP },
    { L"MENU_HELP_ABOUT",           LANG_MENU_HELP_ABOUT },
    { L"WINDOW_TITLE",              LANG_WINDOW_TITLE },
    { L"WINDOW_TITLE_PAUSED",       LANG_WINDOW_TITLE_PAUSED },
    { L"WINDOW_TITLE_FPS",          LANG_WINDOW_TITLE_FPS },
    { L"OPEN_ROM_TITLE",            LANG_OPEN_ROM_TITLE },
    { L"OPEN_ROM_FILTER",           LANG_OPEN_ROM_FILTER },
    { L"OPEN_MOVIE_TITLE",          LANG_OPEN_MOVIE_TITLE },
    { L"SAVE_MOVIE_TITLE",          LANG_SAVE_MOVIE_TITLE },
    { L"OPEN_PALETTE_TITLE",        LANG_OPEN_PALETTE_TITLE },
    { L"SAVE_AVI_TITLE",            LANG_SAVE_AVI_TITLE },
    { L"ERR_ROM_OPEN",              LANG_ERR_ROM_OPEN },
    { L"ERR_ROM_INVALID",           LANG_ERR_ROM_INVALID },
    { L"ERR_ROM_MAPPER",            LANG_ERR_ROM_MAPPER },
    { L"ERR_ROM_UNIF",              LANG_ERR_ROM_UNIF },
    { L"ERR_ROM_FDS_HEADER",        LANG_ERR_ROM_FDS_HEADER },
    { L"ERR_ROM_FDS_DISKSIDE",      LANG_ERR_ROM_FDS_DISKSIDE },
    { L"ERR_ROM_NSF",               LANG_ERR_ROM_NSF },
    { L"MSG_ROM_LOADED",            LANG_MSG_ROM_LOADED },
    { L"MSG_ROM_CLOSED",            LANG_MSG_ROM_CLOSED },
    { L"MSG_ROM_ALREADY_LOADED",    LANG_MSG_ROM_ALREADY_LOADED },
    { L"ERR_GFX_DIRECTDRAW",        LANG_ERR_GFX_DIRECTDRAW },
    { L"ERR_GFX_SURFACE",           LANG_ERR_GFX_SURFACE },
    { L"ERR_GFX_PALETTE",           LANG_ERR_GFX_PALETTE },
    { L"ERR_GFX_RENDER",            LANG_ERR_GFX_RENDER },
    { L"ERR_GFX_MODE",              LANG_ERR_GFX_MODE },
    { L"MSG_GFX_SCREENSHOT",        LANG_MSG_GFX_SCREENSHOT },
    { L"ERR_APU_DIRECTSOUND",       LANG_ERR_APU_DIRECTSOUND },
    { L"ERR_APU_BUFFER",            LANG_ERR_APU_BUFFER },
    { L"ERR_APU_FORMAT",            LANG_ERR_APU_FORMAT },
    { L"MSG_APU_DISABLED",          LANG_MSG_APU_DISABLED },
    { L"CONT_UNCONNECTED",          LANG_CONT_UNCONNECTED },
    { L"CONT_STDCONTROLLER",        LANG_CONT_STDCONTROLLER },
    { L"CONT_ZAPPER",               LANG_CONT_ZAPPER },
    { L"CONT_ARKANOIDPADDLE",       LANG_CONT_ARKANOIDPADDLE },
    { L"CONT_POWERPAD",             LANG_CONT_POWERPAD },
    { L"CONT_FOURSCORE1",           LANG_CONT_FOURSCORE1 },
    { L"CONT_SNESCONTROLLER",       LANG_CONT_SNESCONTROLLER },
    { L"CONT_VSZAPPER",             LANG_CONT_VSZAPPER },
    { L"CONT_SNESMOUSE",            LANG_CONT_SNESMOUSE },
    { L"CONT_FOURSCORE2",           LANG_CONT_FOURSCORE2 },
    { L"CONT_FAMI4PLAY",            LANG_CONT_FAMI4PLAY },
    { L"CONT_EXP_ARKANOIDPADDLE",   LANG_CONT_EXP_ARKANOIDPADDLE },
    { L"CONT_FAMILYBASICKEYBOARD",  LANG_CONT_FAMILYBASICKEYBOARD },
    { L"CONT_SUBORKEYBOARD",        LANG_CONT_SUBORKEYBOARD },
    { L"CONT_FAMTRAINER",           LANG_CONT_FAMTRAINER },
    { L"CONT_TABLET",               LANG_CONT_TABLET },
    { L"CONT_LABEL_PORT1",          LANG_CONT_LABEL_PORT1 },
    { L"CONT_LABEL_PORT2",          LANG_CONT_LABEL_PORT2 },
    { L"CONT_LABEL_EXP",            LANG_CONT_LABEL_EXP },
    { L"CONT_CONFIG",               LANG_CONT_CONFIG },
    { L"CONT_UDLR",                 LANG_CONT_UDLR },
    { L"CONT_POV",                  LANG_CONT_POV },    
    { L"ERR_CTRL_KEYBOARD_INIT",    LANG_ERR_CTRL_KEYBOARD_INIT },
    { L"ERR_CTRL_KEYBOARD_FORMAT",  LANG_ERR_CTRL_KEYBOARD_FORMAT },
    { L"ERR_CTRL_KEYBOARD_LEVEL",   LANG_ERR_CTRL_KEYBOARD_LEVEL },
    { L"ERR_CTRL_MOUSE_INIT",       LANG_ERR_CTRL_MOUSE_INIT },
    { L"ERR_CTRL_MOUSE_LEVEL",      LANG_ERR_CTRL_MOUSE_LEVEL },
    { L"ERR_CTRL_JOY_LEVEL",        LANG_ERR_CTRL_JOY_LEVEL },
    { L"ERR_CTRL_INVALID_STD",      LANG_ERR_CTRL_INVALID_STD },
    { L"ERR_CTRL_INVALID_EXP",      LANG_ERR_CTRL_INVALID_EXP },
    { L"MSG_STATE_SAVED",           LANG_MSG_STATE_SAVED },
    { L"MSG_STATE_LOADED",          LANG_MSG_STATE_LOADED },
    { L"ERR_STATE_SAVE",            LANG_ERR_STATE_SAVE },
    { L"ERR_STATE_LOAD",            LANG_ERR_STATE_LOAD },
    { L"ERR_STATE_VERSION",         LANG_ERR_STATE_VERSION },
    { L"ERR_STATE_CORRUPT",         LANG_ERR_STATE_CORRUPT },
    { L"MSG_STATE_SLOT",            LANG_MSG_STATE_SLOT },
    { L"MSG_MOVIE_RECORDING",       LANG_MSG_MOVIE_RECORDING },
    { L"MSG_MOVIE_PLAYING",         LANG_MSG_MOVIE_PLAYING },
    { L"MSG_MOVIE_STOPPED",         LANG_MSG_MOVIE_STOPPED },
    { L"ERR_MOVIE_OPEN",            LANG_ERR_MOVIE_OPEN },
    { L"ERR_MOVIE_INVALID",         LANG_ERR_MOVIE_INVALID },
    { L"ERR_MOVIE_VERSION",         LANG_ERR_MOVIE_VERSION },
    { L"MSG_AVI_RECORDING",         LANG_MSG_AVI_RECORDING },
    { L"ERR_AVI_OPEN",              LANG_ERR_AVI_OPEN },
    { L"MSG_AVI_STOPPED",           LANG_MSG_AVI_STOPPED },
    { L"ERR_GENIE_INVALID",         LANG_ERR_GENIE_INVALID },
    { L"ERR_GENIE_TOO_MANY",        LANG_ERR_GENIE_TOO_MANY },
    { L"MSG_GENIE_APPLIED",         LANG_MSG_GENIE_APPLIED },
    { L"ERR_MAPPER_LOAD",           LANG_ERR_MAPPER_LOAD },
    { L"ERR_MAPPER_NOTFOUND",       LANG_ERR_MAPPER_NOTFOUND },
    { L"ERR_MAPPER_VERSION",        LANG_ERR_MAPPER_VERSION },
    { L"DBG_BREAK_EXEC",            LANG_DBG_BREAK_EXEC },
    { L"DBG_BREAK_READ",            LANG_DBG_BREAK_READ },
    { L"DBG_BREAK_WRITE",           LANG_DBG_BREAK_WRITE },
    { L"DBG_BREAK_ACCESS",          LANG_DBG_BREAK_ACCESS },
    { L"DBG_BREAK_OPCODE",          LANG_DBG_BREAK_OPCODE },
    { L"DBG_BREAK_NMI",             LANG_DBG_BREAK_NMI },
    { L"DBG_BREAK_IRQ",             LANG_DBG_BREAK_IRQ },
    { L"DBG_BREAK_BRK",             LANG_DBG_BREAK_BRK },
    { L"DBG_BREAK_SCANLINE",        LANG_DBG_BREAK_SCANLINE },
    { L"DBG_BREAK_UNDEF",           LANG_DBG_BREAK_UNDEF },
    { L"DBG_BREAK_ENABLED",         LANG_DBG_BREAK_ENABLED },
    { L"DBG_BREAK_DISABLED",        LANG_DBG_BREAK_DISABLED },
    { L"MSG_DBG_LOG_STARTED",       LANG_MSG_DBG_LOG_STARTED },
    { L"MSG_DBG_LOG_STOPPED",       LANG_MSG_DBG_LOG_STOPPED },
    { L"DLG_OK",                    LANG_DLG_OK },
    { L"DLG_CANCEL",                LANG_DLG_CANCEL },
    { L"DLG_YES",                   LANG_DLG_YES },
    { L"DLG_NO",                    LANG_DLG_NO },
    { L"DLG_APPLY",                 LANG_DLG_APPLY },
    { L"DLG_BROWSE",                LANG_DLG_BROWSE },
    { L"DLG_NINTENDULATOR",         LANG_DLG_NINTENDULATOR },
    { L"ABOUT_TITLE",               LANG_ABOUT_TITLE },
    { L"ABOUT_TEXT",                LANG_ABOUT_TEXT },
    { L"LANG_CHANGED",              LANG_LANG_CHANGED },
    { L"LANG_CHANGED_MSG",          LANG_LANG_CHANGED_MSG },
    { L"ERR_CPU_BAD_OPCODE",        LANG_ERR_CPU_BAD_OPCODE },   // ← Добавлено
    { NULL, LANG_STRING_COUNT }
};

// ============================================================
// Реализация функций
// ============================================================
void Lang::Init(HINSTANCE hInst)
{
    ResetToDefaults();

    GetModuleFileName(hInst, LangDir, MAX_PATH);
    TCHAR *sl = _tcsrchr(LangDir, _T('\\'));
    if (sl) *(sl + 1) = 0;
    _tcscat(LangDir, _T("Lang\\"));
    CreateDirectory(LangDir, NULL);

    ScanLangDir();

    if (LanguageList.empty())
    {
        TCHAR ep[MAX_PATH];
        _tcscpy(ep, LangDir);
        _tcscat(ep, _T("English.lng"));
        FILE *f = NULL;
        _tfopen_s(&f, ep, _T("wt,ccs=UTF-8"));
        if (f)
        {
            fputs("# NintendulatorML Language File\n# Format: KEY=Value\n\n", f);
            fputs("[Info]\nLanguage=English\nAuthor=\nVersion=1.0\n\n[Strings]\n", f);
            for (int i = 0; KeyMap[i].key != NULL; i++)
            {
                char key8[128];
                WideCharToMultiByte(CP_UTF8, 0, KeyMap[i].key, -1, key8, 128, NULL, NULL);
                char val8[LANG_MAX_STRING * 2];
                WideCharToMultiByte(CP_UTF8, 0, DefaultStrings[KeyMap[i].id], -1, val8, sizeof(val8), NULL, NULL);
                fprintf(f, "%s=%s\n", key8, val8);
            }
            fclose(f);
        }
        ScanLangDir();
    }

    std::wstring saved = LoadFromRegistry();
    if (!saved.empty())
        Load(saved.c_str());
    else if (!LanguageList.empty())
        Load(LanguageList[0].c_str());
}

bool Lang::Load(const TCHAR *langName)
{
    ResetToDefaults();

    TCHAR path[MAX_PATH];
    _tcscpy(path, LangDir);
    _tcscat(path, langName);
    _tcscat(path, _T(".lng"));

    FILE *f = NULL;
    _tfopen_s(&f, path, _T("rt,ccs=UTF-8"));
    if (!f)
    {
        _tfopen_s(&f, path, _T("rt"));
        if (!f) return false;
    }

    bool inStrings = false;
    wchar_t line[LANG_MAX_STRING * 2];

    while (fgetws(line, _countof(line), f))
    {
        std::wstring s = Trim(std::wstring(line));
        if (s.empty() || s[0] == L'#') continue;
        if (s == L"[Strings]") { inStrings = true; continue; }
        if (s[0] == L'[')     { inStrings = false; continue; }
        if (!inStrings) continue;

        size_t eq = s.find(L'=');
        if (eq == std::wstring::npos) continue;

        std::wstring key = Trim(s.substr(0, eq));
        std::wstring val = s.substr(eq + 1);

        std::wstring out;
        for (size_t i = 0; i < val.size(); i++)
        {
            if (val[i] == L'\\' && i + 1 < val.size() && val[i+1] == L'n')
            { out += L'\n'; i++; }
            else out += val[i];
        }

        for (int i = 0; KeyMap[i].key != NULL; i++)
        {
            if (_wcsicmp(key.c_str(), KeyMap[i].key) == 0)
            { Strings[KeyMap[i].id] = out; break; }
        }
    }
    fclose(f);
    CurrentLanguage = langName;
    return true;
}

const TCHAR* Lang::GetString(LangStringID id)
{
    if (id < 0 || id >= LANG_STRING_COUNT) return _T("");
    return Strings[id].c_str();
}

const std::vector<std::wstring>& Lang::GetLanguageList()
{
    return LanguageList;
}

const TCHAR* Lang::GetCurrentLanguage()
{
    return CurrentLanguage.c_str();
}

void Lang::UpdateMenu(HMENU hMenu)
{
    if (!hMenu)
        return;

    ModifyMenu(
        hMenu,
        0,
        MF_BYPOSITION | MF_POPUP | MF_STRING,
        (UINT_PTR)GetSubMenu(hMenu, 0),
        GetString(LANG_MENU_FILE)
    );

    ModifyMenu(
        hMenu,
        1,
        MF_BYPOSITION | MF_POPUP | MF_STRING,
        (UINT_PTR)GetSubMenu(hMenu, 1),
        GetString(LANG_MENU_NES)
    );

    ModifyMenu(
        hMenu,
        2,
        MF_BYPOSITION | MF_POPUP | MF_STRING,
        (UINT_PTR)GetSubMenu(hMenu, 2),
        GetString(LANG_MENU_SETTINGS)
    );

    ModifyMenu(
        hMenu,
        3,
        MF_BYPOSITION | MF_POPUP | MF_STRING,
        (UINT_PTR)GetSubMenu(hMenu, 3),
        GetString(LANG_MENU_MOVIE)
    );

    ModifyMenu(
        hMenu,
        4,
        MF_BYPOSITION | MF_POPUP | MF_STRING,
        (UINT_PTR)GetSubMenu(hMenu, 4),
        GetString(LANG_MENU_DEBUG)
    );

    ModifyMenu(
        hMenu,
        8,
        MF_BYPOSITION | MF_POPUP | MF_STRING,
        (UINT_PTR)GetSubMenu(hMenu, 8),
        GetString(LANG_MENU_LANGUAGE)
    );

    ModifyMenu(
        hMenu,
        9,
        MF_BYPOSITION | MF_POPUP | MF_STRING,
        (UINT_PTR)GetSubMenu(hMenu, 9),
        GetString(LANG_MENU_HELP)
    );

    DrawMenuBar(hMainWnd);
}

void Lang::SaveToRegistry(const TCHAR *langName)
{
    HKEY hk;
    if (RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\Nintendulator"), 0, NULL, 0, KEY_WRITE, NULL, &hk, NULL) == ERROR_SUCCESS)
    {
        RegSetValueEx(hk, _T("Language"), 0, REG_SZ, (const BYTE*)langName, (DWORD)((_tcslen(langName)+1)*sizeof(TCHAR)));
        RegCloseKey(hk);
    }
}

std::wstring Lang::LoadFromRegistry()
{
    HKEY hk;
    std::wstring r;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Nintendulator"), 0, KEY_READ, &hk) == ERROR_SUCCESS)
    {
        TCHAR buf[256] = {0};
        DWORD sz = sizeof(buf), type = REG_SZ;
        if (RegQueryValueEx(hk, _T("Language"), NULL, &type, (LPBYTE)buf, &sz) == ERROR_SUCCESS)
            r = buf;
        RegCloseKey(hk);
    }
    return r;
}
