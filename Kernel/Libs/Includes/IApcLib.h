#pragma once

class IApcLib
{
public:
	static IApcLib* GetNewInstance();
	static VOID ReleaseInstance(IApcLib* Instance);

	virtual
	BOOL
	Start(
		PDRIVER_OBJECT DriverObject,
		KPROCESSOR_MODE eProcessorMode,
		PFN_KERNEL_ROUTINE KernelRoutine,
		PFN_RUNDOWN_ROUTINE RundownRoutine,
		// if NormalRoutine is NULL then the eProcessorMode is ignored and set to kernel
		// (coz it will be an special kernel apc)
		PFN_NORMAL_ROUTINE NormalRoutine
	) = 0;

	virtual
	VOID
	Stop(
	) = 0;

	virtual
	BOOL
	InsertApc(
		PETHREAD ptThread,
		PVOID SystemArgument1,
		PVOID SystemArgument2,
		PVOID NormalRoutineContext,
		KPRIORITY Increment
	) = 0;

protected:
	IApcLib(){};
	virtual~IApcLib(){};

private:
	// delete copy and move constructors and assign operators
	IApcLib(IApcLib const&);             // Copy construct
	IApcLib(IApcLib&&);                  // Move construct
	IApcLib& operator=(IApcLib const&);  // Copy assign
	IApcLib& operator=(IApcLib&&);       // Move assign

};