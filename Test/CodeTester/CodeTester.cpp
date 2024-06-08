#include "Precompiled.h"

#include <WppIncludes.h>

#include "CodeTester_Cmd.cpp"


/*!
Usage:
Help
Displays available functions

Help <function>
Displays help on function

Help all
Display help on all functions
*/
int RUN_FUNCTION help(CMD_LINE Args)
{
	return ParserHelpFunction(Args->argc, Args->argv);
}

int __cdecl wmain(int argc, LPCWSTR argv[])
{
	int RetVal;
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	WPP_INIT_TRACING(EXE_FILE_NAMEW);
	{
		auto ProcEntry = [](BOOL start)
		{
			if (start)
				LOG_OUT(DBG_TRACE_FUNCTIONS, ">>> Start file %!FILE! line: %!LINE!");
			else
				LOG_OUT(DBG_TRACE_FUNCTIONS, "<<< Finish file %!FILE! line: %!LINE!");

		};
		PROC_ENTRY;

		RetVal = ParserWmain(argc, argv);
	}
	WPP_CLEANUP();

	return RetVal;
}
