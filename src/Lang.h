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
    LANG_MENU_CPU,
    LANG_MENU_PPU,
    LANG_MENU_SOUND,
    LANG_MENU_INPUT,
    LANG_MENU_DEBUG,
    LANG_MENU_DEBUG_CPU,
    LANG_MENU_DEBUG_PPU,
    LANG_MENU_MISC,
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
    // ---------- Controllers device list ----------
    LANG_CONT_UNCONNECTED,
    LANG_CONT_STDCONTROLLER,
    LANG_CONT_ZAPPER,
    LANG_CONT_ARKANOIDPADDLE,
    LANG_CONT_POWERPAD,
    LANG_CONT_FOURSCORE1,
    LANG_CONT_SNESCONTROLLER,
    LANG_CONT_VSZAPPER,
    LANG_CONT_SNESMOUSE,
    LANG_CONT_FOURSCORE2,
    LANG_CONT_FAMI4PLAY,
    LANG_CONT_EXP_ARKANOIDPADDLE,
    LANG_CONT_FAMILYBASICKEYBOARD,
    LANG_CONT_SUBORKEYBOARD,
    LANG_CONT_FAMTRAINER,
    LANG_CONT_TABLET,
    
    // ---------- Controllers dialog ----------
    LANG_CONT_LABEL_PORT1,
    LANG_CONT_LABEL_PORT2,
    LANG_CONT_LABEL_EXP,
    LANG_CONT_CONFIG,
    LANG_CONT_UDLR,
    LANG_CONT_POV,
    
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

    // ---------- PPU подменю ----------
    LANG_MENU_PPU_FRAMESKIP,
    LANG_MENU_PPU_SIZE,
    LANG_MENU_PPU_MODE,
    LANG_MENU_PPU_PALETTE,
    LANG_MENU_PPU_SLOWDOWN,
    LANG_MENU_PPU_FULLSCREEN,
    LANG_MENU_PPU_SCANLINES,
    LANG_MENU_PPU_BILINEAR,

    // ---------- Language menu ----------
    LANG_LANG_CHANGED,
    LANG_LANG_CHANGED_MSG,

    // ---------- CPU ----------
    LANG_ERR_CPU_BAD_OPCODE,
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
