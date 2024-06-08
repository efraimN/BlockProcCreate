#pragma once

#ifdef NTDDI_VERSION
#undef NTDDI_VERSION
#endif

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif

#define NTDDI_VERSION NTDDI_WIN7
#define _WIN32_WINNT _WIN32_WINNT_WIN7

#define WINDOWS_ENABLE_CPLUSPLUS

#ifdef __cplusplus
extern "C"
{
#endif
// Common Includes
#include <warning.h>
#include <ntverp.h>
#include <ntifs.h>
#include <wdm.h>
#include <windef.h>
#include <stdarg.h>

#include <Ntstrsafe.h>
#include <ntimage.h>
#include <ntstatus.h>

#pragma warning(push)
#pragma warning(disable:4510 4512 4610)
#include <fltKernel.h>
#pragma warning(pop)

#include <NtDllInc.h>
#include <RtlNtIncludes.h>

#include <NtDDKUndoc.h>

#include <CommonMacros.h>
#include <CastMacros.h>

#include <oem.h>

#ifdef __cplusplus
}
#endif

#define NOEXCEPT

#ifdef VER_PRODUCTBUILD
#pragma message ("BUILDMSG: DDK version is: " TO_STR(VER_PRODUCTBUILD) )
#pragma message ("BUILDMSG: NTDDI_VERSION version is: " TO_STR(NTDDI_VERSION) )
#pragma message ("BUILDMSG: _WIN32_WINNT  version is: " TO_STR(_WIN32_WINNT) )
#endif

#include <WPPControl.h>

#include <Debug.h>

#include <KernelCrt.h>

#define MAX_PATH_ 512

// #define NTSYSAPI__ NTSYSAPI

// #include <KerneldllInc.h>
// #include <NtDllInc.h>

/*#define NOEXCEPT*/

#ifndef MIN
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#endif

#ifndef MAX
#define MAX(x,y) ((x)>(y) ? (x):(y))
#endif
