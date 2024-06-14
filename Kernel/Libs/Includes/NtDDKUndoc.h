#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

	NTKERNELAPI
	BOOLEAN
	NTAPI
	PsIsProtectedProcess(
		IN PEPROCESS Process
	);

	NTKERNELAPI
	NTSTATUS
	NTAPI
	PsReferenceProcessFilePointer(
		IN PEPROCESS Process,
		OUT PFILE_OBJECT *FileObject
	);

#ifdef _M_X64
	NTKERNELAPI
	PVOID
	NTAPI
	PsGetProcessWow64Process(
		IN PEPROCESS Process
	);
#endif

	typedef VOID(NTAPI * PFN_NORMAL_ROUTINE)
		(
			PVOID NormalContext,
			PVOID SystemArgument1,
			PVOID SystemArgument2
			);

	typedef VOID(NTAPI* PFN_KERNEL_ROUTINE)
		(
			PRKAPC Apc,
			PFN_NORMAL_ROUTINE *NormalRoutine,
			PVOID *NormalContext,
			PVOID *SystemArgument1,
			PVOID *SystemArgument2
			);

	typedef VOID(NTAPI * PFN_RUNDOWN_ROUTINE)(PRKAPC Apc);

	typedef enum _KAPC_ENVIRONMENT
	{
		OriginalApcEnvironment,
		AttachedApcEnvironment,
		CurrentApcEnvironment,
		InsertApcEnvironment
	} KAPC_ENVIRONMENT, *PKAPC_ENVIRONMENT;

	NTKERNELAPI
	VOID
	NTAPI
	KeInitializeApc(
		IN PKAPC Apc,
		IN PKTHREAD Thread,
		IN KAPC_ENVIRONMENT ApcStateIndex,
		IN PFN_KERNEL_ROUTINE KernelRoutine,
		IN PFN_RUNDOWN_ROUTINE RundownRoutine,
		IN PFN_NORMAL_ROUTINE NormalRoutine,
		IN KPROCESSOR_MODE ApcMode,
		IN PVOID NormalContext
	);

	NTKERNELAPI
	BOOLEAN
	NTAPI
	KeInsertQueueApc(
		PKAPC Apc,
		PVOID SystemArgument1,
		PVOID SystemArgument2,
		KPRIORITY Increment
	);

	NTKERNELAPI
	NTSTATUS
	NTAPI
	PsAcquireProcessExitSynchronization(
		IN PEPROCESS Process
	);

	NTKERNELAPI
	NTSTATUS
	NTAPI
	PsReleaseProcessExitSynchronization(
		IN PEPROCESS Process
	);

#ifdef __cplusplus
}
#endif
