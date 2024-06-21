#include "Precompile.h"
#include "dbg.h"

#include WPP_TO_INCLUDE(message.cpp.h)

#include "message.h"

//-----------------------------------------------------------------------------
// Procedure:   LogEvent
//
// Description: Writes a event to the system event log.
//
// Arguments:
//      AdapterHandle - adapter handle received in Initialize
//		ErrorCode - one of the error codes defined in the messages.h
// Returns:
//     VOID
//-----------------------------------------------------------------------------
/*VOID LogEvent(IN NDIS_HANDLE AdapterHandle, IN NTSTATUS ErrorCode)
{
	PROC_ENTRY
    if (AdapterHandle)
	{
		if (ErrorCode)
		{
			NdisWriteErrorLogEntry(
				AdapterHandle,
				(NDIS_ERROR_CODE)ErrorCode,    // Message Name
				(ULONG)1,                      // Number of data arguments to follow
				(UINT32)(ErrorCode));            // Event Number
		}
	}
}
*/