#pragma hdrstop
#include "Precompile.h"
#include "dbg.h"

#include WPP_TO_INCLUDE(dbg.c.h)



#ifndef DBG_USE_WPP
UINT32 DBG_LEVEL;
#else
PDRIVER_OBJECT WPP_DriverObject;
#endif

void DebugInit(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
#ifdef DBG_USE_WPP
		WPP_DriverObject = DriverObject;
		WPP_INIT_TRACING(WPP_DriverObject,RegistryPath);
#else
		DBG_LEVEL = DBG_INIT_LEVEL;
#endif
}

void DebugStop()
{
#ifdef DBG_USE_WPP
	WPP_CLEANUP(WPP_DriverObject);
#else
#endif
}

