#pragma once
namespace UserKernelUtilsLib
{
	class IMemPoolMgr
	{
	public:
		typedef BOOLEAN(_cdecl* pfnRetrieveUsedBlocks)(PVOID UsedBlock, PVOID additionalInfo);

		static
		IMemPoolMgr* GetNewInstance();

		static
		VOID FreeInstance(IMemPoolMgr* Interface);

		static
		size_t GetRequiredMemoryZoneLength(size_t DataBlockSize, size_t NumberOfBlocks);

		virtual
		BOOLEAN Start(PVOID MemoryZone, size_t Length, size_t DataBlockSize, size_t NumberOfBlocks) = 0;

		virtual
		PVOID AllocateFromPool() = 0;

		virtual
		BOOL FreeFromPool(PVOID DataBlock) = 0;


		virtual
		size_t UsedBlocks() = 0;

		static
		BOOLEAN RetrieveUsedBlocks(PVOID MemoryZone, size_t Length, size_t DataBlockSize, size_t NumberOfBlocks, pfnRetrieveUsedBlocks RetrieveUsedBlocksCallBack, PVOID additionalInfo);




	protected:
		IMemPoolMgr() {};
		virtual~IMemPoolMgr() {};

	private:
		IMemPoolMgr(const IMemPoolMgr& other);
		IMemPoolMgr& operator=(const IMemPoolMgr& other);
	};
}
