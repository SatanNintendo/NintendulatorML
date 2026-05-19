/*
 * NintendulatorML — система локализации
 * src/Lang.h
 */
#pragma once
#include <windows.h>
#include <tchar.h>
#include <map>
#include <vector>
#include <string>

// ============================================================
// ИДЕНТИФИКАТОРЫ СТРОК — один ID на каждую переводимую строку
// Добавляйте новые ID здесь, перед LANG_STRING_COUNT
// ============================================================
enum LangStringID
{
    // ---------- Главное меню ----------
    LANG_MENU_FILE = 0,
    LANG_MENU_FILE_OPEN,
    LANG_MENU_FILE_CLOSE,
    LANG_MENU_FILE_RECENT,
    LANG_MENU_FILE_RECENT_CLEAR,
    LANG_MENU_FILE_EXIT,
    LANG_MENU_NES,
    LANG_MENU_NES_RUN,
    LANG_MENU_NES_PAUSE,
    LANG_MENU_NES_RESET,
    LANG_MENU_NES_POWER,
    LANG_MENU_NES_STOP,
    LANG_MENU_SETTINGS,
    LANG_MENU_SETTINGS_VIDEO,
    LANG_MENU_SETTINGS_SOUND,
    LANG_MENU_SETTINGS_CONTROLLERS,
    LANG_MENU_SETTINGS_GAMEGENIE,
    LANG_MENU_SETTINGS_PATHS,
    LANG_MENU_SETTINGS_CLOCK,
    LANG_MENU_MOVIE,
    LANG_MENU_MOVIE_RECORD,
    LANG_MENU_MOVIE_PLAY,
    LANG_MENU_MOVIE_STOP,
    LANG_MENU_DEBUG,
    LANG_MENU_DEBUG_CPU,
    LANG_MENU_DEBUG_PPU,
    LANG_MENU_LANGUAGE,
    LANG_MENU_HELP,
    LANG_MENU_HELP_ABOUT,

    // ---------- Заголовок окна ----------
    LANG_WINDOW_TITLE,
    LANG_WINDOW_TITLE_PAUSED,
    LANG_WINDOW_TITLE_FPS,

    // ---------- Диалог открытия файла ----------
    LANG_OPEN_ROM_TITLE,
    LANG_OPEN_ROM_FILTER,
    LANG_OPEN_MOVIE_TITLE,
    LANG_SAVE_MOVIE_TITLE,
    LANG_OPEN_PALETTE_TITLE,
    LANG_SAVE_AVI_TITLE,

    // ---------- NES.cpp — ошибки ROM ----------
    LANG_ERR_ROM_OPEN,
    LANG_ERR_ROM_INVALID,
    LANG_ERR_ROM_MAPPER,
    LANG_ERR_ROM_UNIF,
    LANG_ERR_ROM_FDS_HEADER,
    LANG_ERR_ROM_FDS_DISKSIDE,
    LANG_ERR_ROM_NSF,
    LANG_MSG_ROM_LOADED,
    LANG_MSG_ROM_CLOSED,
    LANG_MSG_ROM_ALREADY_LOADED,

    // ---------- GFX.cpp — видео ----------
    LANG_ERR_GFX_DIRECTDRAW,
    LANG_ERR_GFX_SURFACE,
    LANG_ERR_GFX_PALETTE,
    LANG_ERR_GFX_RENDER,
    LANG_ERR_GFX_MODE,
    LANG_MSG_GFX_SCREENSHOT,

    // ---------- APU.cpp — звук ----------
    LANG_ERR_APU_DIRECTSOUND,
    LANG_ERR_APU_BUFFER,
    LANG_ERR_APU_FORMAT,
    LANG_MSG_APU_DISABLED,

    // ---------- Controllers.cpp ----------
    LANG_ERR_CTRL_KEYBOARD_INIT,
    LANG_ERR_CTRL_KEYBOARD_FORMAT,
    LANG_ERR_CTRL_KEYBOARD_LEVEL,
    LANG_ERR_CTRL_MOUSE_INIT,
    LANG_ERR_CTRL_MOUSE_LEVEL,
    LANG_ERR_CTRL_JOY_LEVEL,
    LANG_ERR_CTRL_INVALID_STD,
    LANG_ERR_CTRL_INVALID_EXP,

    // ---------- States.cpp — сохранения ----------
    LANG_MSG_STATE_SAVED,
    LANG_MSG_STATE_LOADED,
    LANG_ERR_STATE_SAVE,
    LANG_ERR_STATE_LOAD,
    LANG_ERR_STATE_VERSION,
    LANG_ERR_STATE_CORRUPT,
    LANG_MSG_STATE_SLOT,

    // ---------- Movie.cpp — запись ----------
    LANG_MSG_MOVIE_RECORDING,
    LANG_MSG_MOVIE_PLAYING,
    LANG_MSG_MOVIE_STOPPED,
    LANG_ERR_MOVIE_OPEN,
    LANG_ERR_MOVIE_INVALID,
    LANG_ERR_MOVIE_VERSION,
    LANG_MSG_AVI_RECORDING,
    LANG_ERR_AVI_OPEN,
    LANG_MSG_AVI_STOPPED,

    // ---------- Genie.cpp — Game Genie ----------
    LANG_ERR_GENIE_INVALID,
    LANG_ERR_GENIE_TOO_MANY,
    LANG_MSG_GENIE_APPLIED,

    // ---------- MapperInterface.cpp ----------
    LANG_ERR_MAPPER_LOAD,
    LANG_ERR_MAPPER_NOTFOUND,
    LANG_ERR_MAPPER_VERSION,

    // ---------- Debugger.cpp ----------
    LANG_DBG_BREAK_EXEC,
    LANG_DBG_BREAK_READ,
    LANG_DBG_BREAK_WRITE,
    LANG_DBG_BREAK_ACCESS,
    LANG_DBG_BREAK_OPCODE,
    LANG_DBG_BREAK_NMI,
    LANG_DBG_BREAK_IRQ,
    LANG_DBG_BREAK_BRK,
    LANG_DBG_BREAK_SCANLINE,
    LANG_DBG_BREAK_UNDEF,
    LANG_DBG_BREAK_ENABLED,
    LANG_DBG_BREAK_DISABLED,
    LANG_MSG_DBG_LOG_STARTED,
    LANG_MSG_DBG_LOG_STOPPED,

    // ---------- Общие диалоги ----------
    LANG_DLG_OK,
    LANG_DLG_CANCEL,
    LANG_DLG_YES,
    LANG_DLG_NO,
    LANG_DLG_APPLY,
    LANG_DLG_BROWSE,
    LANG_DLG_NINTENDULATOR,

    // ---------- About dialog ----------
    LANG_ABOUT_TITLE,
    LANG_ABOUT_TEXT,

    // ---------- Language menu ----------
    LANG_LANG_CHANGED,
    LANG_LANG_CHANGED_MSG,

    // ---------- CPU ----------
    LANG_ERR_CPU_BAD_OPCODE,

    // ---------- Controllers dialog labels ----------
    LANG_CTRL_PORT1_LABEL,
    LANG_CTRL_PORT2_LABEL,
    LANG_CTRL_EXP_LABEL,
    LANG_CTRL_UDLR_TEXT,
    LANG_CTRL_POV_TEXT,
    LANG_CTRL_CONFIG,
    LANG_CTRL_CLOSE,

    // ---------- Titlebar ----------
    LANG_TITLEBAR_STOPPED,
    LANG_TITLEBAR_FPS_NOSKIP,
    LANG_TITLEBAR_FPS_SKIP,
    LANG_TITLEBAR_AUTO,

    // ---------- GFX errors (дополнительные) ----------
    LANG_ERR_GFX_PERF_COUNTER,
    LANG_ERR_GFX_DDRAW_DLL,
    LANG_ERR_GFX_DDRAW_ENTRY,
    LANG_ERR_GFX_REGION,
    LANG_ERR_GFX_INIT,
    LANG_ERR_GFX_NO_FULLSCREEN,
    LANG_ERR_GFX_COOP_LEVEL,
    LANG_ERR_GFX_COOP_LEVEL2,
    LANG_ERR_GFX_COOP_WINDOWED,
    LANG_ERR_GFX_PRIMARY,
    LANG_ERR_GFX_SECONDARY,
    LANG_ERR_GFX_PRIMARY2,
    LANG_ERR_GFX_SECONDARY2,
    LANG_ERR_GFX_CLIPPER,
    LANG_ERR_GFX_CLIPPER_WND,
    LANG_ERR_GFX_CLIPPER_SURF,
    LANG_ERR_GFX_SURF_DESC,
    LANG_ERR_GFX_BIT_DEPTH,
    LANG_MSG_GFX_DISPLAY,
    LANG_MSG_GFX_FULLSCREEN,
    LANG_MSG_GFX_WINDOWED,

    // ---------- File dialogs (дополнительные) ----------
    LANG_OPEN_ROM_TITLE2,
    LANG_OPEN_ROM_FILTER2,
    LANG_OPEN_HEADER_TITLE,
    LANG_MSG_NO_LANGUAGES,

    LANG_STRING_COUNT   // ← всегда последним!
};

#define LANG_MAX_STRING 1024

namespace Lang
{
    void Init(HINSTANCE hInst);
    bool Load(const TCHAR *langName);
    const TCHAR* GetString(LangStringID id);
    const std::vector<std::wstring>& GetLanguageList();
    const TCHAR* GetCurrentLanguage();
    void UpdateMenu(HMENU hMenu);
    void SaveToRegistry(const TCHAR *langName);
    std::wstring LoadFromRegistry();
}
