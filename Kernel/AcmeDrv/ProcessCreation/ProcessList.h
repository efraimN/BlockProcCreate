#pragma once

#include <IUserKernelUtils.h>
#include <IApcLib.h>

class CProcessList;

typedef struct _DllinjectionParams
{
	// for Dll injection
	BOOL m_InjectStarted;
	BOOL m_ImageExecutableLoaded;
	PETHREAD m_Kernel32LoaderThread;
	BOOL m_WhiteListProc;

	IApcLib* m_InjectionUserAPC;
	IApcLib* m_InjectionKernelAPC;

}DllinjectionParams, *PDllinjectionParams;

/* Do not use virtual members on this class to avoid Vtable; Also there is no way to instance the class, must use casting*/
struct ProcessDataElement : public UserKernelUtilsLib::ILinkListElement
{
	friend CProcessList;
	PEPROCESS m_ProcessObj;
	DWORD m_ProcessPid;


	// Some useful info about a process to keep
	UNICODE_STRING m_ProccesNtExecName;
	UNICODE_STRING m_ProccesDosExecName;
	UINT64 m_EntryPoint;
	BOOL m_IsWow64;
	BOOLEAN m_IsProtectedProccess;
	BOOLEAN m_IsProtectedProccessLight;


	DllinjectionParams m_DllinjectionParams;


private:
	ProcessDataElement() {};
	~ProcessDataElement() {};
	WCHAR m_ProccesNtExecNameBuff[MAX_PATH_];
	WCHAR m_ProccesDosExecNameBuff[MAX_PATH_];
	UINT m_RefCount;
};

class CProcessList
{
public:
	static CProcessList* GetInstance();

	BOOLEAN Start();
	VOID Stop();

	BOOL AddElement(PEPROCESS ProcessObj);
	BOOL RemoveElement(PEPROCESS ProcessObj);

	ProcessDataElement* GetElement(PEPROCESS ProcessObj);

	ProcessDataElement* GetElementEx(
		UserKernelUtilsLib::ILinkListLibInt::pfnFoundElementEx GetFoundElement,
		PVOID FindContext,
		BOOLEAN Removal);

	static
	__declspec(align(8))
	EX_RUNDOWN_REF m_ExitRunDown;


private:
	CProcessList();
	~CProcessList();
	VOID RemoveAllElements();
	BOOL InitElement(ProcessDataElement* Element);
	VOID ReleaseElement(ProcessDataElement* Element);

	UserKernelUtilsLib::ILinkListLibInt* m_pProcessList;
	UserKernelUtilsLib::IMemPoolMgr* m_ProcessListElementsPool;
	PVOID m_ListElementsPoolMem;
};
