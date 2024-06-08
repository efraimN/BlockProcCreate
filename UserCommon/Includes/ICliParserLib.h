#pragma once

/*
Following the possible values that can be used as 'type' for the arguments in test functions
{
	"INT8"
	"UINT8"
	"INT16"
	"UINT16"
	"INT32"
	"UINT32"
	"INT64"
	"UINT64"
	"char*"
	"CMD_LINE"
};
*/

typedef struct _PARSER_FUNC
{
	char* name;
	int(*ParserCmdFunc)(_PARSER_FUNC* Cmd, int argc, char* argv[]);
	int numParams;
	int* kind;
	char* help;
} PARSER_FUNC;

typedef struct _Arguments
{
    int argc;
    char** argv;
}Arguments, *PArguments;

#define RUN_FUNCTION
#define RUN_FUNCTION_DIS
#define CMD_LINE PArguments

#define MAX_CMD_LEN 500
#define MAX_COMMANDS 100

extern int ParserHelpFunction(int argc, char* argv[]);
extern int ParserWmain(int argc, LPCWSTR argv[]);

extern PCHAR CopyFileToMemBuff(PCHAR NtFilePathAndName, size_t* Filelength);
extern PCHAR ProfileGetJsonBuff(PCHAR JsonFile, size_t* Filelength, PCHAR JsonKey);
