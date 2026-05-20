/*
 * NintendulatorML — система локализации
 * src/Lang.cpp
 */
#include "stdafx.h"
#include "Lang.h"
#include "resource.h"
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
   
    // Контроллеры ошибки
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
    // CPU
    _T("Bad opcode, CPU locked")
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
    // ... (остальные функции ScanLangDir, Trim и т.д. оставляем как были)
}

// ============================================================
// Таблица сопоставления ключей (KeyMap) — оставляем как было
// ============================================================
struct KeyMapping { const wchar_t *key; LangStringID id; };
static const KeyMapping KeyMap[] =
{
    // ... (весь KeyMap оставляем как у тебя был)
    { L"MENU_FILE", LANG_MENU_FILE },
    { L"MENU_FILE_OPEN", LANG_MENU_FILE_OPEN },
    // ... (все остальные строки KeyMap)
    { L"ERR_CPU_BAD_OPCODE", LANG_ERR_CPU_BAD_OPCODE },
    { NULL, LANG_STRING_COUNT }
};

// ============================================================
// Реализация функций (Init, Load и т.д.) — оставляем как было
// ============================================================

// ... (весь код Init, Load, GetString, GetLanguageList, GetCurrentLanguage, SaveToRegistry, LoadFromRegistry оставляем без изменений)

// ============================================================
// ИСПРАВЛЕННАЯ ФУНКЦИЯ UpdateMenu
// ============================================================
void Lang::UpdateMenu(HMENU hMenu)
{
    if (!hMenu) return;

    // === Главные пункты меню (по позиции) ===
    ModifyMenu(hMenu, 0, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)GetSubMenu(hMenu, 0), GetString(LANG_MENU_FILE));
    ModifyMenu(hMenu, 1, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)GetSubMenu(hMenu, 1), GetString(LANG_MENU_NES));
    ModifyMenu(hMenu, 2, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)GetSubMenu(hMenu, 2), GetString(LANG_MENU_SETTINGS));
    ModifyMenu(hMenu, 3, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)GetSubMenu(hMenu, 3), GetString(LANG_MENU_MOVIE));
    ModifyMenu(hMenu, 4, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)GetSubMenu(hMenu, 4), GetString(LANG_MENU_DEBUG));
    ModifyMenu(hMenu, 5, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)GetSubMenu(hMenu, 5), GetString(LANG_MENU_LANGUAGE));
    ModifyMenu(hMenu, 6, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)GetSubMenu(hMenu, 6), GetString(LANG_MENU_HELP));

    // === Подменю NES ===
    HMENU hNES = GetSubMenu(hMenu, 1);
    if (hNES)
    {
        ModifyMenu(hNES, ID_CPU_RUN,       MF_BYCOMMAND, ID_CPU_RUN,       GetString(LANG_MENU_NES_RUN));
        ModifyMenu(hNES, ID_CPU_STOP,      MF_BYCOMMAND, ID_CPU_STOP,      GetString(LANG_MENU_NES_STOP));
        ModifyMenu(hNES, ID_CPU_SOFTRESET, MF_BYCOMMAND, ID_CPU_SOFTRESET, GetString(LANG_MENU_NES_RESET));
        ModifyMenu(hNES, ID_CPU_HARDRESET, MF_BYCOMMAND, ID_CPU_HARDRESET, GetString(LANG_MENU_NES_POWER));
    }

    // === Подменю Movie ===
    HMENU hMovie = GetSubMenu(hMenu, 3);
    if (hMovie)
    {
        ModifyMenu(hMovie, ID_MISC_RECORDMOVIE, MF_BYCOMMAND, ID_MISC_RECORDMOVIE, GetString(LANG_MENU_MOVIE_RECORD));
        ModifyMenu(hMovie, ID_MISC_PLAYMOVIE,   MF_BYCOMMAND, ID_MISC_PLAYMOVIE,   GetString(LANG_MENU_MOVIE_PLAY));
        ModifyMenu(hMovie, ID_MISC_STOPMOVIE,   MF_BYCOMMAND, ID_MISC_STOPMOVIE,   GetString(LANG_MENU_MOVIE_STOP));
    }

    // === Подменю Debug ===
    HMENU hDebug = GetSubMenu(hMenu, 4);
    if (hDebug)
    {
        ModifyMenu(hDebug, ID_DEBUG_CPU, MF_BYCOMMAND, ID_DEBUG_CPU, GetString(LANG_MENU_DEBUG_CPU));
        ModifyMenu(hDebug, ID_DEBUG_PPU, MF_BYCOMMAND, ID_DEBUG_PPU, GetString(LANG_MENU_DEBUG_PPU));
    }

    DrawMenuBar(hMainWnd);
}
