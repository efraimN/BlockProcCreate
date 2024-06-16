#include "Precompiled.h"
#include <WppIncludes.h>

WCHAR ProcName[1024];

BOOL
APIENTRY
DllMain(
	_In_  HINSTANCE hModule,
	_In_  DWORD ul_reason_for_call,
	_In_  LPVOID lpReserved
)
{
	UNREFERENCED_PARAMETER(hModule);
	UNREFERENCED_PARAMETER(lpReserved);

	HMODULE Tmp;
	BOOL DoHooks = TRUE;

	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
#if 0
			while (!IsDebuggerPresent())
			{
				Sleep(100);
			}
			DebugBreak();
#endif
			_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
			_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
			_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
			_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

		// Since we use static links with the crt, we MUST NOT call DisableThreadLibraryCalls
		// look at MSDN DisableThreadLibraryCalls remarks section
// 			DisableThreadLibraryCalls(hModule);

			WPP_INIT_TRACING(NULL);
			GetModuleFileNameW(NULL, ProcName, 1023);
			LOG_OUT(DBG_INFO, "WppInit Called for hdll %S", ProcName);

			// be sure we never unload....
			GetModuleHandleExW(
				GET_MODULE_HANDLE_EX_FLAG_PIN | GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
				(LPCWSTR)DllMain,
				&Tmp
			);

			auto ForDebug =
			[&]()
			{
				DoHooks = FALSE;
				WCHAR FileName[255];
				GetModuleFileNameW(
					NULL,
					FileName,
					255
				);
				for (UINT i = 0; i < 255; i++)
				{
					if (FileName[i] == 0)
					{
						break;
					}
					towlower(FileName[i]);
				}
				if (wcsstr(FileName, L"notepad"))
				{
					DoHooks = TRUE;
				}
			};

// 			ForDebug();

			if (DoHooks)
			{
			}
		}
		break;

// 		case DLL_THREAD_ATTACH:
// 		{
// 			LOG_OUT(DBG_INFO, "WppStop DLL_THREAD_ATTACH %S", ProcName);
// 		}
// 		break;
// 
// 		case DLL_THREAD_DETACH:
// 		{
// 			LOG_OUT(DBG_INFO, "WppStop DLL_THREAD_DETACH %S", ProcName);
// 		}
// 		break;

		case DLL_PROCESS_DETACH:
		{

			LOG_OUT(DBG_INFO, "WppStop Called %S", ProcName);
			WPP_CLEANUP();
		}
		break;

		default:
		break;
	}

	return TRUE;
}