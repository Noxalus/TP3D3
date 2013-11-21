// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <string>
#include <stdio.h>

#define IAENGINE_API
// avoid warning
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
typedef unsigned long ulong;
typedef unsigned int uint;
typedef unsigned _int32 uint32;
typedef unsigned short ushort;
typedef unsigned char uchar;

// TODO: reference additional headers (like DirectX one) your program requires here
