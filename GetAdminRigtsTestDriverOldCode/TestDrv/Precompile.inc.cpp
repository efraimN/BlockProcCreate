#include "Precompile.h"
#include "dbg.h"

#include WPP_TO_INCLUDE(Globals.cpp.h)

void * __cdecl ::operator new(size_t nSize)
{ 
	void* retval; 
	retval = ExAllocatePoolWithTag(NonPagedPool,nSize,'PPWF');
	return retval; 
}

void __cdecl ::operator delete(void* p)
{ 
	if (p) ExFreePool(p);
	p = NULL;
}


