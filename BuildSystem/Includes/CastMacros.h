#ifndef CASTMACRO
#define CASTMACRO
inline LONGLONG UPCAST(DWORD x)
{
	LARGE_INTEGER tmpCasting = { 0 };
	tmpCasting.LowPart = x;
	return tmpCasting.QuadPart;
 }
inline LONGLONG UPCASTHANDLE(DWORD x)
{
#ifdef _M_X64
	LARGE_INTEGER tmpCasting = { 0 };
	tmpCasting.LowPart = x;
	return tmpCasting.QuadPart;
#else
	return (DWORD)x;
#endif
}
inline DWORD DOWNCAST(LONGLONG x)
{
	LARGE_INTEGER tmpCasting = { 0 };
	tmpCasting.QuadPart = x;
	return tmpCasting.LowPart;
}
inline DWORD DOWNCASTHANDLE(PVOID x)
{
#ifdef _M_X64
	LARGE_INTEGER tmpCasting = { 0 };
	tmpCasting.QuadPart = (LONGLONG)x;
	return tmpCasting.LowPart;
#else
	return (DWORD)x;
#endif
}
inline DWORD DOWNCASTULONGPTR(ULONG_PTR x)
{
	return DOWNCASTHANDLE((PVOID)x);
}
#endif
