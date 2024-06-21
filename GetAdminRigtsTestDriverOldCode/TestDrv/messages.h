#pragma once
//File Name: messages.mc
//
// WARNING....
// The File messages.h is generated from messages.mc
// Do not Edit the file messages.h directly; instead edit the file messages.mc
//
//  Note: comments in the .mc file must use both ";" and "//".
//
//  Status values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-------------------------+-------------------------------+
//  |Sev|C|       Facility          |        Code (MessageId)       |
//  +---+-+-------------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//


//
// %1 is reserved by the IO Manager. If IoAllocateErrorLogEntry is
// called with a device, the name of the device will be inserted into
// the message at %1. Otherwise, the place of %1 will be left empty.
// In either case, the insertion strings from the driver's error log
// entry starts at %2. In other words, the first insertion string goes
// to %2, the second to %3 and so on.  Fixed strings can be added anywhere.
//

// MESSAGE ID USAGE
//
// MessageID field is 16-bit wide.  It usage is described as the following:
//
//   1 1 1 1 1 1
//   5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +-----+-------------------------+
//  |Cust.|       MessageID         |
//  +-----+-------------------------+
//
// The Cust. is the custom ID that is used to group messages together.
// The Custom ID is specified as a registry parameter under the name of CustomMessages
// CustomMessages specifies which message group.
// 
// If CustomMessages equals 0, use Microsoft/System Messages.
// Otherwise, use messages defined in this file according to the Custom ID specified.
// *** Custom ID = CustomMessages - 1 ***
//
// Custom ID:
// 0:		Intel Specific Messages
// 1:		Custom Generic Messages
// 2-7:	Reserved
//
// For each Custom ID group, there can be zero or upto 8192 messages (13 bits).
//
// IMPORTANT:
// - Each message is added to a Custom ID group must also be added to other groups
// - Each message must conform to the context of its Custom ID group
//*********************************************************************************
// Custom ID 0: Intel Specific Messages
//*********************************************************************************
//
//  Values are 32 bit values laid out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//
#define FACILITY_RPC_STUBS               0x3
#define FACILITY_RPC_RUNTIME             0x2
#define FACILITY_IO_ERROR_CODE           0x4
#define FACILITY_CUSTOM_ERROR_CODE       0x7


//
// Define the severity codes
//
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3


//
// MessageId: MSG_ERROR_ADAPTER_START_FAILURE
//
// MessageText:
//
// %2
//  PROBLEM: Could not start the adapter.
//  ACTION: Reinstall the driver.
//
#define MSG_ERROR_ADAPTER_START_FAILURE  ((NTSTATUS)0xC0040007L)

//
// MessageId: MSG_ERROR_RESURCES
//
// MessageText:
//
// %2
//  PROBLEM: Not enough resources. 
//  ACTION: Reinstall the driver.
//
#define MSG_ERROR_RESURCES               ((NTSTATUS)0xC0040009L)

//
// MessageId: MSG_ERROR_IVALID_REG_PARAMS
//
// MessageText:
//
// %2
//  PROBLEM: Couldn't read the registry parameters. 
//  ACTION: Reinstall the driver.
//
#define MSG_ERROR_IVALID_REG_PARAMS      ((NTSTATUS)0xC004000AL)

//
// MessageId: MSG_ERROR_LOW_MENORY
//
// MessageText:
//
// %2
//  PROBLEM: Coudn't allocate enough memory. 
//  ACTION: Reinstall the driver.
//
#define MSG_ERROR_LOW_MENORY             ((NTSTATUS)0xC004000BL)

//EOF