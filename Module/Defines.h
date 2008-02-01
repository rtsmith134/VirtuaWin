/*
//  VirtuaWin - Virtual Desktop Manager (virtuawin.sourceforge.net)
//  ConfigParameters.h - Constant definitions used.
// 
//  Copyright (c) 1999-2005 Johan Piculell
//  Copyright (c) 2006-2007 VirtuaWin (VirtuaWin@home.se)
// 
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, 
//  USA.
*/

#ifndef _DEFINES_H_
#define _DEFINES_H

#include <windows.h>

/* Application name and version defines */
#define vwVIRTUAWIN_NAME         _T("VirtuaWin")
#define vwVIRTUAWIN_CLASSNAME    _T("VirtuaWinMainClass")
#define vwVIRTUAWIN_EMAIL        _T("VirtuaWin@home.se")
#define vwVIRTUAWIN_NAME_VERSION _T("VirtuaWin v4.0 beta1")
#define vwVIRTUAWIN_WEBSITE      _T("http://virtuawin.sourceforge.net/")

/* Various defines used on several places */
#define vwHOTKEY_MAX      40       /* max number of hotkeys */
#define vwWINHASH_SIZE   509       /* size of the window hash table */
#define vwDESKTOP_MAX     20       /* max number of desktops */
#define vwDESKTOP_SIZE    (vwDESKTOP_MAX + 2)
#define vwCLASSNAME_MAX   64       /* class name buffer size */
#define vwWINDOWNAME_MAX 128       /* window name buffer size */
#define vwMODULENAME_MAX  79       /* Maximum length of a module name (buffer needs to be n+1 long) */

#define MAXMODULES        10       /* max number of modules to handle */

/* Internal windows messages */
#define VW_SYSTRAY        (WM_USER + 1)  /* Sent to us by the systray */
#define VW_MOUSEWARP      (WM_USER + 9)  /* Mouse thread message */

/* Some base types used to make code more readable */
typedef unsigned int   vwUInt ;
typedef unsigned short vwUShort ;
typedef unsigned char  vwUByte ;


#define vwWTNAME_COUNT            3
#define vwWTFLAGS_CN_SVAR         0x00000001
#define vwWTFLAGS_CN_EVAR         0x00000002
#define vwWTFLAGS_WN_SVAR         0x00000004
#define vwWTFLAGS_WN_EVAR         0x00000008
#define vwWTFLAGS_PN_SVAR         0x00000010
#define vwWTFLAGS_PN_EVAR         0x00000020
#define vwWTFLAGS_STICKY          0x00001000
#define vwWTFLAGS_MOVE            0x00002000
#define vwWTFLAGS_MOVE_IMMEDIATE  0x00004000
#define vwWTFLAGS_HIDEWIN_MASK    0x000f0000
#define vwWTFLAGS_HIDEWIN_BITROT  16
#define vwWTFLAGS_HIDEWIN_HIDE    0x00000000
#define vwWTFLAGS_HIDEWIN_MOVE    0x00010000
#define vwWTFLAGS_HIDEWIN_MINIM   0x00020000
#define vwWTFLAGS_HIDETSK_MASK    0x00f00000
#define vwWTFLAGS_HIDETSK_BITROT  20
#define vwWTFLAGS_HIDETSK_HIDE    0x00000000
#define vwWTFLAGS_HIDETSK_DONT    0x00100000
#define vwWTFLAGS_HIDETSK_TOOLWN  0x00200000
#define vwWTFLAGS_MANAGE          0x20000000
#define vwWTFLAGS_DONT_MANAGE     0x40000000
#define vwWTFLAGS_ENABLED         0x80000000

typedef struct vwWindowType {
    struct vwWindowType *next;
    TCHAR               *name[vwWTNAME_COUNT] ;
    vwUInt               flags ; 
    vwUByte              desk ;
    vwUByte              nameLen[vwWTNAME_COUNT] ;
} vwWindowType ;

/* vwWindow structures for storing information about one window */
#define vwWINFLAGS_INITIALIZED     0x00000001
#define vwWINFLAGS_FOUND           0x00000002
#define vwWINFLAGS_ACTIVATED       0x00000004
#define vwWINFLAGS_VISIBLE         0x00000008
#define vwWINFLAGS_MINIMIZED       0x00000010
#define vwWINFLAGS_WINDOW          0x00000020
#define vwWINFLAGS_MANAGED         0x00000040
#define vwWINFLAGS_TRICKY_POS      0x00000080
#define vwWINFLAGS_SHOWN           0x00000100
#define vwWINFLAGS_SHOW            0x00000200
#define vwWINFLAGS_NO_TASKBAR_BUT  0x00000400
#define vwWINFLAGS_RM_TASKBAR_BUT  0x00000800
#define vwWINFLAGS_STICKY          vwWTFLAGS_STICKY        
#define vwWINFLAGS_MOVE            vwWTFLAGS_MOVE          
#define vwWINFLAGS_MOVE_IMMEDIATE  vwWTFLAGS_MOVE_IMMEDIATE
#define vwWINFLAGS_MAXIMIZED       0x00008000
#define vwWINFLAGS_HIDEWIN_MASK    vwWTFLAGS_HIDEWIN_MASK  
#define vwWINFLAGS_HIDEWIN_HIDE    vwWTFLAGS_HIDEWIN_HIDE  
#define vwWINFLAGS_HIDEWIN_MOVE    vwWTFLAGS_HIDEWIN_MOVE  
#define vwWINFLAGS_HIDEWIN_MINIM   vwWTFLAGS_HIDEWIN_MINIM 
#define vwWINFLAGS_HIDETSK_MASK    vwWTFLAGS_HIDETSK_MASK  
#define vwWINFLAGS_HIDETSK_HIDE    vwWTFLAGS_HIDETSK_HIDE  
#define vwWINFLAGS_HIDETSK_DONT    vwWTFLAGS_HIDETSK_DONT  
#define vwWINFLAGS_HIDETSK_TOOLWN  vwWTFLAGS_HIDETSK_TOOLWN

/* vwWindowBase - Holds data far a non-managed window */
typedef struct vwWindowBase { 
    struct vwWindowBase *next ;
    struct vwWindowBase *hash ;
    HWND                 handle ;
    vwUInt               flags ;
} vwWindowBase;

/* vwWindow - Holds data far a managed window, start must be the same as vwWindowBase */
typedef struct vwWindow { 
    /* same as vwWindowBase - start */
    struct vwWindow     *next ;
    struct vwWindow     *hash ;
    HWND                 handle ;
    vwUInt               flags ;
    /* same as vwWindowBase - end */
    HWND                 owner;
    long                 exStyle;
    vwUInt               zOrder[vwDESKTOP_SIZE] ;
    vwUByte              menuId ;
    vwUByte              desk;
} vwWindow ;

/* Holds data for modules */
typedef struct {
    HWND      handle;
    TCHAR     description[vwMODULENAME_MAX+1];
    vwUByte   disabled;
} moduleType;

/* Holds disabled modules */
typedef struct {
    TCHAR     moduleName[vwMODULENAME_MAX+1];
} disModules;

/* vwMenuItem - Structure used by the window list menu */
typedef struct {
    TCHAR    *name;
    HICON     icon; 
    vwUInt    zOrder ;
    vwUShort  id;
    vwUByte   desk;
    vwUByte   sticky;
} vwMenuItem ;

/* vwHotkey - Structure to store a hotkey binding */
#define vwHOTKEY_ALT     MOD_ALT
#define vwHOTKEY_CONTROL MOD_CONTROL
#define vwHOTKEY_SHIFT   MOD_SHIFT
#define vwHOTKEY_WIN     MOD_WIN
#define vwHOTKEY_EXT     0x10

typedef struct {
    ATOM     atom ;
    vwUByte  key ;
    vwUByte  modifier ;
    vwUByte  command ;
    vwUByte  desk ;
} vwHotkey ;

#endif
