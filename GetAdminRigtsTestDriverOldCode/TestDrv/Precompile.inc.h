#define _TO_STR(_N) #_N
#define TO_STR(_N) _TO_STR(_N)

#ifndef FORCEINLINE
#if (MSC_VER >= 1200)
#define FORCEINLINE __forceinline
#else
#define FORCEINLINE __inline
#endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif
// Common Includes
	#include <warning.h>
	#include <ntverp.h> // nt version info
   #include <ntifs.h>
	#include <ntddk.h>
	#include <windef.h>

// use efilter.h for W2k and under, use xfilter for whistler and above
#ifdef VER_PRODUCTBUILD
#	pragma message ("BUILDMSG: DDK version is: " TO_STR(VER_PRODUCTBUILD) )
#	if (VER_PRODUCTBUILD <= 2195)
		#include <efilter.h>   // this header file has been replaced in Whistler DDK
#	else
		#include <xfilter.h>
#	endif

#endif

#ifndef DBG_USE_WPP
#	include <stdarg.h>
#endif

#ifdef __cplusplus
}
#endif


//Beware DO NOT Include any header that uses WPP on the precompiled header!!!!
#include "oem.h"
#include "messages.h"

