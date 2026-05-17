/* Lang.cpp - Language support for Nintendulator
 * Copyright (C) QMT Productions
 */

#include "stdafx.h"
#include "Nintendulator.h"
#include "resource.h"
#include "Lang.h"

namespace Lang
{
	TCHAR *Strings[LANG_STRING_COUNT];

	static const TCHAR *DefaultStrings[LANG_STRING_COUNT] =
	{
		// === Main Menu ===
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

		// === Window title ===
		_T("Nintendulator"),
		_T("Nintendulator [Paused]"),
		_T("Nintendulator - %s - %d FPS"),

		// === File dialogs ===
		_T("Open ROM"),
		_T("NES ROMs (*.nes;*.unif;*.fds;*.nsf)\0*.nes;*.unif;*.fds;*.nsf\0All Files (*.*)\0*.*\0"),
		_T("Open Movie"),
		_T("Save Movie"),
		_T("Open Palette"),
		_T("Save AVI"),

		// === ROM errors ===
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

		// === Video ===
		_T("Failed to initialize DirectDraw"),
		_T("Failed to create DirectDraw surface"),
		_T("Failed to set DirectDraw palette"),
		_T("DirectDraw render error"),
		_T("Unsupported video mode"),
		_T("Screenshot saved: %s"),

		// === Sound ===
		_T("Failed to initialize DirectSound"),
		_T("Failed to create DirectSound buffer"),
		_T("Unsupported audio format"),
		_T("Sound disabled"),

		// === Controllers ===
		_T("Unable to initialize keyboard input device!"),
		_T("Unable to set keyboard input data format!"),
		_T("Unable to set keyboard input cooperative level!"),
		_T("Unable to initialize mouse input device!"),
		_T("Unable to set mouse input cooperative level!"),
		_T("Unable to restore joystick input cooperative level!"),
		_T("Error: selected invalid controller type for standard port!"),
		_T("Error: selected invalid controller type for expansion port!"),

		// === Save states ===
		_T("State saved to slot %d"),
		_T("State loaded from slot %d"),
		_T("Failed to save state"),
		_T("Failed to load state"),
		_T("State file version mismatch"),
		_T("State file is corrupted"),
		_T("Slot %d"),

		// === Movie recording ===
		_T("Recording movie..."),
		_T("Playing movie..."),
		_T("Movie stopped"),
		_T("Failed to open movie file"),
		_T("Invalid movie file"),
		_T("Movie version mismatch"),
		_T("Recording AVI..."),
		_T("Failed to open AVI file"),
		_T("AVI recording stopped"),

		// === Game Genie ===
		_T("Invalid Game Genie code"),
		_T("Too many Game Genie codes (maximum 3)"),
		_T("Game Genie code applied"),

		// === Mapper ===
		_T("Failed to load %s - not a valid DLL file!"),
		_T("Mapper not found"),
		_T("Failed to load mapper pack %s - version mismatch!"),

		// === Debugger breakpoints ===
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

		// === Dialogs ===
		_T("OK"),
		_T("Cancel"),
		_T("Yes"),
		_T("No"),
		_T("Apply"),
		_T("Browse..."),
		_T("Nintendulator"),

		// === About ===
		_T("About Nintendulator"),
		_T("Nintendulator\nNES Emulator\n\nCopyright (C) QMT Productions"),

		// === Language ===
		_T("Language Changed"),
		_T("Language has been changed. Some elements will update on restart."),

		// === CPU ===
		_T("Bad opcode, CPU locked")
	};

	void	Init (void)
	{
		int i;
		for (i = 0; i < LANG_STRING_COUNT; i++)
			Strings[i] = _tcsdup(DefaultStrings[i]);
	}

	void	ResetToDefaults (void)
	{
		int i;
		for (i = 0; i < LANG_STRING_COUNT; i++)
		{
			free(Strings[i]);
			Strings[i] = _tcsdup(DefaultStrings[i]);
		}
	}

	void	Shutdown (void)
	{
		int i;
		for (i = 0; i < LANG_STRING_COUNT; i++)
		{
			free(Strings[i]);
			Strings[i] = NULL;
		}
	}
}
