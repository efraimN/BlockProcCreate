;#pragma once
;//File Name: messages.mc
;//
;// WARNING....
;// The File messages.h is generated from messages.mc
;// Do not Edit the file messages.h directly; instead edit the file messages.mc
;//
;//  Note: comments in the .mc file must use both ";" and "//".
;//
;//  Status values are 32 bit values layed out as follows:
;//
;//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
;//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
;//  +---+-+-------------------------+-------------------------------+
;//  |Sev|C|       Facility          |        Code (MessageId)       |
;//  +---+-+-------------------------+-------------------------------+
;//
;//  where
;//
;//      Sev - is the severity code
;//
;//          00 - Success
;//          01 - Informational
;//          10 - Warning
;//          11 - Error
;//
;//      C - is the Customer code flag
;//
;//      Facility - is the facility code
;//
;//      Code - is the facility's status code
;//
;
MessageIdTypedef=NTSTATUS

SeverityNames=(Success=0x0:STATUS_SEVERITY_SUCCESS
               Informational=0x1:STATUS_SEVERITY_INFORMATIONAL
               Warning=0x2:STATUS_SEVERITY_WARNING
               Error=0x3:STATUS_SEVERITY_ERROR
              )

FacilityNames=(System=0x0
               RpcRuntime=0x2:FACILITY_RPC_RUNTIME
               RpcStubs=0x3:FACILITY_RPC_STUBS
               Io=0x4:FACILITY_IO_ERROR_CODE
               Custom=0x7:FACILITY_CUSTOM_ERROR_CODE
              )

;
;//
;// %1 is reserved by the IO Manager. If IoAllocateErrorLogEntry is
;// called with a device, the name of the device will be inserted into
;// the message at %1. Otherwise, the place of %1 will be left empty.
;// In either case, the insertion strings from the driver's error log
;// entry starts at %2. In other words, the first insertion string goes
;// to %2, the second to %3 and so on.  Fixed strings can be added anywhere.
;//
;

;// MESSAGE ID USAGE
;//
;// MessageID field is 16-bit wide.  It usage is described as the following:
;//
;//   1 1 1 1 1 1
;//   5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
;//  +-----+-------------------------+
;//  |Cust.|       MessageID         |
;//  +-----+-------------------------+
;//
;// The Cust. is the custom ID that is used to group messages together.
;// The Custom ID is specified as a registry parameter under the name of CustomMessages
;// CustomMessages specifies which message group.
;// 
;// If CustomMessages equals 0, use Microsoft/System Messages.
;// Otherwise, use messages defined in this file according to the Custom ID specified.
;// *** Custom ID = CustomMessages - 1 ***
;//
;// Custom ID:
;// 0:		Intel Specific Messages
;// 1:		Custom Generic Messages
;// 2-7:	Reserved
;//
;// For each Custom ID group, there can be zero or upto 8192 messages (13 bits).
;//
;// IMPORTANT:
;// - Each message is added to a Custom ID group must also be added to other groups
;// - Each message must conform to the context of its Custom ID group

;//*********************************************************************************
;// Custom ID 0: Intel Specific Messages
;//*********************************************************************************

MessageId=0x0007 Facility=Io Severity=Error             SymbolicName=MSG_ERROR_ADAPTER_START_FAILURE
Language=English
%2
 PROBLEM: Could not start the adapter.
 ACTION: Reinstall the driver.
.

MessageId=0x0009 Facility=Io Severity=Error             SymbolicName=MSG_ERROR_RESURCES
Language=English
%2
 PROBLEM: Not enough resources. 
 ACTION: Reinstall the driver.
.

MessageId=0x000A Facility=Io Severity=Error             SymbolicName=MSG_ERROR_IVALID_REG_PARAMS
Language=English
%2
 PROBLEM: Couldn't read the registry parameters. 
 ACTION: Reinstall the driver.
.

MessageId=0x000B Facility=Io Severity=Error             SymbolicName=MSG_ERROR_LOW_MENORY
Language=English
%2
 PROBLEM: Coudn't allocate enough memory. 
 ACTION: Reinstall the driver.
.


;//EOF