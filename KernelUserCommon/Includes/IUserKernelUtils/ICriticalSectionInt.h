	/*****************************************************************************************
		 Note For kernel code:
		 When using CriticalSectionShared the thread CAN BE at DISPATCH_LEVEL or less
			In this case for threads at DISPATCH_LEVEL the call will be a blocking call
		 
		 But when using CriticalSectionExclusive the thread MUST BE at APC_LEVEL or less
		 In other words, CriticalSectionExclusive CAN'T be used for a thread that is running at a level greater than APC_LEVEL

		 Note For User mode:
		 The parameter CriticalSectionShared/CriticalSectionExclusive is ignored
	******************************************************************************************/
namespace UserKernelUtilsLib
{
	class CriticalSection
	{
	public:
		enum class CriticalSectionType : BYTE
		{
			CriticalSectionShared = 1,
			CriticalSectionExclusive
		};
		typedef class _CriticalSectionLock CriticalSectionLock, * pCriticalSectionLock;

		CriticalSection(pCriticalSectionLock Lock, CriticalSectionType LockType);
		~CriticalSection();

		static NTSTATUS InitLockObject(pCriticalSectionLock* Lock);
		static NTSTATUS DeleteLockObject(pCriticalSectionLock Lock);

	private:
		// delete copy and move constructors and assign operators
		CriticalSection(CriticalSection const&);             // Copy construct
		CriticalSection(CriticalSection&&);                  // Move construct
		CriticalSection& operator=(CriticalSection const&);  // Copy assign
		CriticalSection& operator=(CriticalSection&&);       // Move assign

		pCriticalSectionLock m_Lock;
#ifdef _NTDDK_
		CriticalSectionType m_LockType;
		KLOCK_QUEUE_HANDLE m_LocalData;
#endif
	};
}