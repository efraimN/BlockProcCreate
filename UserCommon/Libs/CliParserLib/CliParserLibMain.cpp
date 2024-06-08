#include "Precompiled.h"

#include "CliParserLib.h"

#define MAX_LINE 2048

void ParserInit();

HANDLE hStdInput = NULL;

void DECLSPEC_NORETURN FatalError(__in DWORD Error)
{
    PWCHAR text;

    if (FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        Error,
        0,
        (PTCHAR)&text,
        0,
        NULL))
    {
        wprintf(text);
        LocalFree(text);
    }

    RaiseException(Error, 0, 0, NULL);
    __debugbreak();
}

int ParserProcessLineWithTry(int argc, char* argv[])
{
	LONG ret = ERROR_SUCCESS;
	__try
	{
		ret = ParserProcessLine(argc, argv);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		ret = GetExceptionCode();
		FatalError(ret);
	}
	return ret;
}

int __cdecl ParserWmain(int argc, LPCWSTR argv[])
{

	LONG ret = ERROR_SUCCESS;

	DWORD mode;
	int i;
	char cLine[MAX_LINE];
	char* carg[1024];
	char* cPtr;
	DWORD LineLeft = MAX_LINE;

	hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	GetConsoleMode(hStdInput, &mode);
	SetConsoleMode(hStdInput, mode & (~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT)));

	ParserInit();

	cLine[0] = 0;
	cPtr = cLine;
	for (i = 0; i < argc; i++)
	{
		carg[i] = cPtr;
		LineLeft -= WideCharToMultiByte(CP_ACP, 0, argv[i], -1, cPtr, LineLeft, NULL, NULL);
		cPtr = cLine + MAX_LINE - LineLeft;
	}
	carg[i] = NULL;
#ifdef _DEBUG
	ret = ParserProcessLine(argc, carg);
#else
	ret = ParserProcessLineWithTry(argc, carg);
#endif

	SetConsoleMode(hStdInput, mode);

	return ret;
}

