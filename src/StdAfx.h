// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
#define AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

// Target Windows 7 and later
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#ifndef WINVER
#define WINVER 0x0601
#endif

#pragma warning(disable:4100)	// "unreferenced formal parameter"
#pragma warning(disable:4127)	// "conditional expression is constant"
#pragma warning(disable:4201)	// "nonstandard extension used : nameless struct/union"
#pragma warning(disable:4244)	// "conversion from 'foo' to 'bar', possible loss of data"

// Windows Header Files:
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// Local Header Files
#include <stdio.h>
#include <commdlg.h>
#include <time.h>

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
