#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

	NTKERNELAPI
	NTSTATUS
	NTAPI
	PsReferenceProcessFilePointer(
		IN PEPROCESS Process,
		OUT PFILE_OBJECT *FileObject
	);

#ifdef __cplusplus
}
#endif
