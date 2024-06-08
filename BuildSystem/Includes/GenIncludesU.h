#pragma once

#ifdef NTDDI_VERSION
#undef NTDDI_VERSION
#endif

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif

#define NTDDI_VERSION NTDDI_WIN7
#define _WIN32_WINNT _WIN32_WINNT_WIN7

#ifdef __cplusplus
extern "C"
{
#endif

#include <WinSDKVer.h>

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN

#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1 //Behind the scenes, the call to strcpy is changed to a call to strcpy_s with the size argument supplied automatically... and so on
#define _CRT_SECURE_CPP_OVERLOAD_SECURE_NAMES 1 // enables template overloads of the secure variants
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT 1


#define _CRTDBG_MAP_ALLOC

#ifdef DEBUG_NEW
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#endif


#define _WINSOCKAPI_    // stops windows.h including winsock.h
#define WIN32_NO_STATUS
#include <Windows.h>
#undef WIN32_NO_STATUS

#include <ntstatus.h>

#include <WInternl.h>
#define _NTDEF_

#include <NtDllInc.h>
#include <RtlNtIncludes.h>

#include <CommonMacros.h>
#include <CastMacros.h>

#include <oem.h>

#ifdef __cplusplus
}
#endif

#define NOEXCEPT noexcept

#include <WPPControl.h>
#include <WppUseNtdllFunctions.h>

#include <Debug.h>

#include<new>
#include <crtdbg.h>

#include <stdio.h>
#include <stdarg.h>

#define MAX_PATH_ 512

#ifndef PAGE_SIZE
#define PAGE_SIZE 0x1000
#endif

#ifndef MIN
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#endif

#ifndef MAX
#define MAX(x,y) ((x)>(y) ? (x):(y))
#endif
