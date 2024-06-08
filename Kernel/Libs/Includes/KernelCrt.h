#pragma once

void CrtInit();
void CrtExit();
void * __cdecl operator new(size_t nSize, ULONG Tag);
// void * __cdecl operator new(size_t nSize);
void __cdecl operator delete(void * pVoid);

