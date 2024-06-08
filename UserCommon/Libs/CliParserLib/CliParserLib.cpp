#include "Precompiled.h"

#include "CliParserLib.h"
extern void DECLSPEC_NORETURN FatalError(__in DWORD Error);

#define MAX_VAL_BUF_LEN 128

#define whitespace(c) (((c) == ' ') || ((c) == '\t'))

#define ROUND_LEN(x) ((x + 3) & ~3)


/* GLOBALS ********************************************************************/


static int  G_ParserCommandsCount = 0;
PARSER_FUNC *G_ParserCommands = NULL;

typedef struct
{
	char* name;
	int     size;
	int     isSigned;
	char* format;
} PARSER_VAR_KIND;

/* FUNCTIONS ******************************************************************/

PARSER_VAR_KIND var_kinds[] =
{
    {"INT8",        sizeof(INT8),   1, "%hhi"},     // kind = 1
    {"UINT8",       sizeof(UINT8),  0, "%hhu"},     // kind = 2
    {"INT16",       sizeof(INT16),  1, "%hi"},      // kind = 3
    {"UINT16",      sizeof(UINT16), 0, "%hu"},      // kind = 4
    {"INT32",       sizeof(INT32),  1, "%li"},      // kind = 5
    {"UINT32",      sizeof(UINT32), 0, "%lu"},      // kind = 6
    {"INT64",       sizeof(INT64),  1, "%lli"},     // kind = 7
    {"UINT64",      sizeof(UINT64), 0, "%llu"},     // kind = 8
    {"char*",       sizeof(char*),  0, "%s"},       // kind = 9
    { "CMD_LINE", 0, 0, NULL },                     // kind = 10
};

static
void WaitForInput()
{
	printf_s("press any key to continue");
	while (!_kbhit());
    if (_getch()) {};
}

bool CaseInSensStringCompare(std::string& str1, std::string& str2)
{
    auto CmpChar = [](char& c1, char& c2)
    {
        return (c1 == c2 || std::toupper(c1) == std::toupper(c2));
    };

	return ((str1.size() == str2.size()) && std::equal(str1.begin(), str1.end(), str2.begin(), CmpChar));
}

void ParserRegisterModule(PARSER_FUNC* Func, int FuncCount)
{
    G_ParserCommands = Func;
    G_ParserCommandsCount = FuncCount;
}

static PARSER_FUNC* ParserFindCommand(char* InCmdName)
{
	std::string CmdName = InCmdName;

	int i;

	if (CmdName == "?")
	{
		CmdName = "-help";
	}

	for (i = 0; i < G_ParserCommandsCount; i++)
	{
        std::string tmp =G_ParserCommands[i].name;
		if (CaseInSensStringCompare(CmdName, tmp))
		{
			return &G_ParserCommands[i];
		}
	}

	return NULL;
}

static int ParserPrintCmdHelp(char *CmdName)
{
    PARSER_FUNC 	*func;

    func = ParserFindCommand(CmdName);
    if(func == NULL)
    {
        printf_s("No such command: %s\r\n", CmdName);
		WaitForInput();
        return -1;
    }

    if (func->help)
    {
        printf_s("    help for: %s\r\n", func->name);
        printf_s("%s\r\n", func->help);
    }
    return 0;
}

int ParserHelpFunction(int argc, char* argv[])
{
    char CmdName[MAX_CMD_LEN+1]={0};

    if((argc - 2) == 0)
    {
        /* If no argument then print all the available cmd names*/
        int i;
        printf_s("Available commands are:\r\n");
        for(i=0; i< G_ParserCommandsCount; i++)
        {
            printf_s("    %s\r\n",G_ParserCommands[i].name);
        }
        printf_s("type -help <command> for help on specific command:\r\n");
        printf_s("type -help all for help on all the commands:\r\n");
    }
    else
    {
        /* Print specific cmd help*/
		strcpy_s(CmdName, MAX_CMD_LEN, argv[2]);
        if (0 == _stricmp(CmdName,"all"))
        {
            int i;
            for(i=0; i< G_ParserCommandsCount; i++)
            {
                ParserPrintCmdHelp(G_ParserCommands[i].name);
            }
        }
        else
        {
            ParserPrintCmdHelp(CmdName);
        }
    }
    return 0;
}

char Buffer[2048];
BOOL ProcessParameters(PVOID Params, PARSER_FUNC* Cmd, int argc, char* argv[])
{
	INT i;
    PARSER_VAR_KIND Kind;
    PCHAR VarStructPtr = (PCHAR)Params;
	CHAR Fmt[16];
	UINT n;
	int err;
	char* Line = (char*)Buffer;
	Line[0] = 0;

	if ((Cmd->kind) && Cmd->kind[0] == 10)
	{
        PArguments Args = (PArguments)Params;

        Args->argc = argc;
		Args->argv = argv;
		return TRUE;
	}

	argc -= 2;

	if ((Cmd->kind) && (Cmd->kind[0] == 0))
	{
		if (argc == 0)
		{
			return TRUE; // is a void functions
		}
		else
		{
			Line[0] = 0;
			for (i = 0; i < argc; i++)
			{
				strcat_s(Line, 2048, argv[i + 2]);
				strcat_s(Line, 2048, " ");
			}
			printf_s("extra parameter(s) found %s\r\n", Line);
			WaitForInput();
			return FALSE;
		}
	}

	if (argc < Cmd->numParams)
	{
		printf_s("not enough parameters\r\n");
		WaitForInput();
		return FALSE;
	}

    for (i = 0; i <= Cmd->numParams; i++)
    {
		if ((i == Cmd->numParams) && (argc > Cmd->numParams))
		{
			for (; i < argc; i++)
			{
				strcat_s(Line, 2048, argv[i + 2]);
				strcat_s(Line, 2048, " ");
			}
			printf_s("extra parameter(s) found %s\r\n", Line);
			WaitForInput();
			return FALSE;
		}
		if (i == Cmd->numParams)
		{
			return TRUE; // finished
		}
		Kind = var_kinds[Cmd->kind[i] - 1];
		if (!strcmp(Kind.format, "%s"))
		{
			memcpy(VarStructPtr, &argv[i + 2], Kind.size);
            VarStructPtr += Kind.size;
		}
        else
        {
			strcpy_s(Fmt, 16, Kind.format);
			strcat_s(Fmt, 16, "%n"); // This will tell us how many chars we read
			err = sscanf_s(argv[i + 2], Fmt, VarStructPtr, &n);
			if ((err != 1) || (n == 0))
			{
				printf_s("Invalid parameter %s\r\n", argv[i + 2]);
				WaitForInput();
				return FALSE;
			}
			if (n != strlen(argv[i + 2]))
			{
				printf_s("Invalid parameter %s\r\n", argv[i + 2]);
				WaitForInput();
				return FALSE;
			}
			VarStructPtr += Kind.size;
        }
	}

    return FALSE;
}

int ParserProcessLine(int argc, char* argv[])
{
    PARSER_FUNC *cmd;
    if (!argv[1])
    {
        printf_s("Type -help or ? for help\r\n");
        return -1;
    }

	cmd = ParserFindCommand(argv[1]);
	if (cmd == NULL)
	{
		printf_s("No such command\r\n");
		WaitForInput();
		return -1;
	}

    return cmd->ParserCmdFunc(cmd, argc, argv);
}

void ParserCallError(int err)
{
    FatalError(err);
}

