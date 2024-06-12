#include <GenIncludesU.h>


#include <vector>
#include <filesystem>
#include <IUserKernelUtils.h>
#include <regex>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "Shlwapi.h"

using namespace std;

// experimental is due we are using vs15. As soon we start to use vs19 and up, we can remove the "experimental"
namespace filesys = std::experimental::filesystem; 

BOOL GetMemPtrAndLengthFromPath(char* FilePath, PVOID* FileMemPtr, ULONG* FileLength)
{
	int RetVal = FALSE;
	WCHAR FilePathW[1024];
	MultiByteToWideChar(CP_ACP, 0, FilePath, -1, FilePathW, 1023);

	UNICODE_STRING FilePathU;

	RtlInitUnicodeString(&FilePathU, FilePathW);

	if (!UserKernelUtilsLib::IMapFileInt::GetInstance()->MapFileIntoMem(&FilePathU, FileMemPtr, FileLength, TRUE))
	{
		goto Leave;
	}

	RetVal = TRUE;
Leave:
	return RetVal;
}

std::vector<std::string> GetAllFilesInDir(const std::string &dirPath, const std::string &Filter)
{

	// Create a vector of string
	CHAR DirPath[1024];
	std::vector<std::string> listOfFiles;
	filesys::path Path;
	if(!GetFullPathNameA(dirPath.c_str(), 1023, DirPath, NULL))
	{
		printf("Internal error GetFullPathName dirPath doesn't exist %s\n", dirPath.c_str());
		goto Leave;
	}
	Path = DirPath;
	try
	{
		// Check if given path exists and points to a directory
		if (!filesys::exists(Path) || !filesys::is_directory(Path))
		{
			printf("Internal error dirPath doesn't exist %s\n", dirPath.c_str());
			goto Leave;
		}
		// Create a Recursive Directory Iterator object and points to the starting of directory
		filesys::recursive_directory_iterator iter(Path);

		// Create a Recursive Directory Iterator object pointing to end.
		filesys::recursive_directory_iterator end;

		// Iterate till end
		while (iter != end)
		{
			// Check if current entry is a directory and if exists in skip list
			if (!filesys::is_directory(iter->path()) &&
				(iter->path().filename().string().find(Filter) != std::string::npos)
				)
			{
				// Add the name in vector
				string a = iter->path().string();
				size_t s = a.size();
				for (UINT i = 0; i < s; i++)
				{
					if (a[i] =='/')
					{
						a[i] = '\\';
					}
				}
				listOfFiles.push_back(a);
			}

			// Increment the iterator to point to next entry in recursive iteration
			iter++;
		}
}
	catch (std::system_error & e)
	{
		UNREFERENCED_PARAMETER(e);
	}


Leave:
	return listOfFiles;
}

typedef struct _TRACE_FUNCTION_DATA
{
	string FuncHlp;
	string FuncName;
	string FuncParm;

}SPLITED_FUNCTION_DATA , *PSPLITED_FUNCTION_DATA;

std::vector<SPLITED_FUNCTION_DATA> FindParserFuncs(std::string File)
{
	static BOOL DoOnce = FALSE;
	PVOID FileMemPtr = NULL;
	ULONG FileLength;
	std::vector<SPLITED_FUNCTION_DATA> RetVal;
	cmatch Match;
	PVOID SearchStart = NULL;
	static regex rx;
	SPLITED_FUNCTION_DATA Data;
	if (!DoOnce)
	{
		rx = "(/\\*!([^!]+)\\*/)?\\s*int\\s[\\s\\w]*RUN_FUNCTION\\s+(\\w+)\\s*\\(([^}{\\(]*?)\\)\\s*?";
		DoOnce = TRUE;
	}

	if (!GetMemPtrAndLengthFromPath((char*)File.c_str(), &FileMemPtr, &FileLength))
	{
		goto Leave;
	}

	SearchStart = FileMemPtr;
	while (regex_search((const char*)SearchStart, (const char*)((PBYTE)FileMemPtr + FileLength), Match, rx))
	{
		SearchStart = (PVOID)Match.suffix().first;
		if (Match[2].matched)
		{
			Data.FuncHlp = Match[2].str();
		}
		else
		{
			Data.FuncHlp = "None";
		}
		Data.FuncName = Match[3].str();
		Data.FuncParm = Match[4].str();
		RetVal.insert(RetVal.end(), Data);
	}
	

Leave:
	if (NULL != FileMemPtr)
	{
		UserKernelUtilsLib::IMapFileInt::GetInstance()->UnMapFile(FileMemPtr, GetCurrentProcess());
	}
	return RetVal;
}

std::vector<std::string> Split(std::string const &in, char sep)
{
	std::string::size_type b = 0;
	std::vector<std::string> result;

	while ((b = in.find_first_not_of(sep, b)) != std::string::npos)
	{
		auto e = in.find_first_of(sep, b);
		result.push_back(in.substr(b, e - b));
		b = e;
	}
	return result;
}

BOOL CreateFunctiosWrappersVars(std::vector<std::string>& Parameters, std::string &ParamCall)
{
	BOOL RetVal = FALSE;

	size_t ParametersSize = Parameters.size();
	std::vector<std::string> Par;
	if (!ParametersSize)
	{
		goto Leave;
	}
	Par = Split(Parameters[0], ' ');
	ParamCall += "#pragma pack( push )\r\n";
	ParamCall += "#pragma pack( 1 )\r\n";
	ParamCall += "    struct _Params\r\n";
	ParamCall += "    {\r\n";
	if (Par[0] == "CMD_LINE")
	{
		ParamCall += "        Arguments arg1;\r\n";
	}
	else
	{
		for (UINT i = 0; i < ParametersSize; i++)
		{
			Par = Split(Parameters[i], ' ');
			if (Par.size() != 2)
			{
				goto Leave;
			}
			ParamCall += "        " + Par[0] + " " + Par[1] + ";\r\n";
		}
	}
	ParamCall += "    };\r\n";
	ParamCall += "    _Params Params;\r\n";
	ParamCall += "#pragma pack( pop )\r\n";
	ParamCall += "\r\n";


	RetVal = TRUE;

Leave:
	return RetVal;
}

BOOL CreateFunctionsWrappersCallArgs(SPLITED_FUNCTION_DATA &Params, std::string &ParamCall)
{
	BOOL RetVal = FALSE;
	BOOL First = TRUE;
	std::vector<std::string> Parameters;
	Parameters = Split(Params.FuncParm, ',');
	size_t ParametersSize = Parameters.size();
	for (UINT i = 0; i < ParametersSize; i++)
	{
		std::vector<std::string> Par;
		Par = Split(Parameters[i], ' ');
		if (Par.size() != 2)
		{
			goto Leave;
		}
		std::string ParName = Par[1];
		std::string ParType = Par[0];
		if (ParType == "CMD_LINE")
		{
			ParamCall += "&Params.arg1";
		}
		else
		{
			if (First)
			{
				First = FALSE;
			}
			else
			{
				ParamCall += ", ";
			}
			ParamCall += "Params." + ParName;
		}
	}
	RetVal = TRUE;
Leave:
	return RetVal;
}

BOOL CreateParserFunctionsWrappers(SPLITED_FUNCTION_DATA &Params, std::string &ParamCall)
{
	ParamCall = "static int Parser_call_";
	ParamCall += Params.FuncName;
	ParamCall += "(PARSER_FUNC* Cmd, int argc, char* argv[])\r\n{\r\n";
	std::vector<std::string> Parameters = Split(Params.FuncParm, ',');
	size_t ParametersSize = Parameters.size();

	CreateFunctiosWrappersVars(Parameters, ParamCall);

	if (ParametersSize)
	{
		ParamCall += "   if (!ProcessParameters(&Params, Cmd, argc, argv))\r\n";
	}
	else
	{
		ParamCall += "   if (!ProcessParameters(NULL, Cmd, argc, argv))\r\n";
	}

	ParamCall += "   {\r\n";
	ParamCall += "        return -1;\r\n";
	ParamCall += "   }\r\n";
	ParamCall += "\r\n";

	ParamCall += "    return " + Params.FuncName + "(";
	CreateFunctionsWrappersCallArgs(Params, ParamCall);
	ParamCall += ");\r\n}\r\n\r\n\r\n";
	return TRUE;
}

 PCHAR VarTypes[] = { "INT8", "UINT8", "INT16", "UINT16", "INT32", "UINT32", "INT64", "UINT64", "char*", "CMD_LINE" };
 UINT MaxVarTypes = sizeof(VarTypes) / sizeof(PCHAR);

UINT GetNumOfVars(string FuncParm)
{
	for (UINT i = 0; i< MaxVarTypes; i++)
	{
		if (FuncParm == std::string(VarTypes[i]))
		{
			return i;
		}
	}
	return 0;
}

BOOL CreateParamsVarTypeTableArray(SPLITED_FUNCTION_DATA &FunctionData, std::string &ParamCall)
{
	ParamCall = "";
	BOOL RetVal = FALSE;

	std::vector<std::string> Parameters;
	Parameters = Split(FunctionData.FuncParm, ',');
	size_t ParametersSize = Parameters.size();
	ParamCall = "int " + FunctionData.FuncName + "_Params[] = \r\n{\r\n";
	if (ParametersSize)
	{
		for (UINT i = 0; i < ParametersSize; i++)
		{
			std::vector<std::string> Par;
			Par = Split(Parameters[i], ' ');
			if (Par.size() != 2)
			{
				printf("Unsupported: parameter type without variable in Parser function '%s'", FunctionData.FuncName.c_str());
				goto Leave;
			}
			std::string ParName = Par[1];
			std::string ParType = Par[0];
			UINT VarNumber = GetNumOfVars(ParType);
			if (!VarNumber)
			{
				printf("Unsupported parameter type '%s' in Parser function '%s'", ParType.c_str(), FunctionData.FuncName.c_str());
				goto Leave;
			}
			if (std::string("CMD_LINE") == ParType)
			{
				if (ParametersSize > 1)
				{
					printf("when using CMD_LINE no extra parameter format allowed in Parser function '%s'", FunctionData.FuncName.c_str());
					goto Leave;
				}
			}
			ParamCall += "    /*" + ParName + "*/ " + std::to_string(VarNumber + 1) + ",\r\n";
		}
	}
	else
	{
		ParamCall += "    0\r\n";
	}

	ParamCall += "};\r\n\r\n\r\n";

	RetVal = TRUE;
Leave:
	return RetVal;
}

std::string EscapeRetLf(std::string In)
{
	std::string RetVal = "";
	UINT Length = (UINT)In.length();
	for (UINT i = 0; i < Length; i++)
	{
		if ( (In[i] != '\r') && (In[i] != '\n') )
		{
			RetVal += In[i];
		}
		else
		{
			RetVal += '\\';
			if (In[i] == '\r')
			{
				RetVal += 'r';
			}
			else
			{
				RetVal += 'n';
			}
		}
	}
	return RetVal;
}

BOOL MakeParserFileData(std::vector<SPLITED_FUNCTION_DATA> &TotalFunctions, std::string *FileData, std::string &FileName)
{
	BOOL RetVal = FALSE;
	std::string ParamCall;
	std::string raw_str;
	printf("Generating ParserFIleData\n");
	*FileData = *FileData +
		"\r\n"
		+ "// This file is auto generated\r\n"
		+ "// DO NOT MODIFY IT MANUALLY\r\n"
		+ "// ANY modification to this file will be lost\r\n"
		+ "\r\n"
		"\r\n"

		+ "extern void ParserRegisterModule(PARSER_FUNC* Func, int FuncCount);\r\n"
		+ "extern void ParserCallError(int err);\r\n"
		+ "extern BOOL ProcessParameters(PVOID Params, PARSER_FUNC * Cmd, int argc, char* argv[]);\r\n"
	+"\r\n"
		;

	UINT TotalFunctionssize = (UINT)TotalFunctions.size();
	for (UINT i = 0; i< TotalFunctionssize;i++)
	{
		*FileData += "extern int " + TotalFunctions[i].FuncName + " (" + TotalFunctions[i].FuncParm + ");\r\n";
	}

	*FileData += "\r\n\r\n";

	for (UINT i = 0; i < TotalFunctionssize; i++)
	{
		CreateParserFunctionsWrappers(TotalFunctions[i], ParamCall);
		*FileData += ParamCall;
	}

	for (UINT i = 0; i < TotalFunctionssize; i++)
	{
		if(!CreateParamsVarTypeTableArray(TotalFunctions[i], ParamCall))
		{
			goto Leave;
		}
		*FileData += ParamCall;
	}


	*FileData += "PARSER_FUNC " + FileName + "_Funcs[] =\r\n{\r\n";
	for (UINT i = 0; i < TotalFunctionssize; i++)
	{
		
		*FileData += 
			"    {\"-"
			+ TotalFunctions[i].FuncName
			+ "\", Parser_call_"
			+ TotalFunctions[i].FuncName
			+ ", sizeof("
			+ TotalFunctions[i].FuncName
			+ "_Params)/sizeof(int), "
			+ TotalFunctions[i].FuncName
			+ "_Params, \""
			+ EscapeRetLf(TotalFunctions[i].FuncHlp)
			+ "\"},\r\n";
	}

	*FileData += "};\r\n\r\n\r\n";

	*FileData += "int " + FileName + "_FuncsCount = sizeof(" + FileName + "_Funcs)/sizeof(PARSER_FUNC);\r\n\r\n\r\n";

	*FileData += "void  ParserInit()\r\n{\r\n";
	*FileData += "    ParserRegisterModule(" + FileName + "_Funcs, " + FileName + "_FuncsCount);\r\n";
	*FileData += "}\r\n\r\n\r\n";

	RetVal = TRUE;
Leave:
	return RetVal;;
}

static
int WriteToFile(PVOID Data, UINT DataLength, PWCHAR FullFilePathAndName)
{
	int RetVal = -1;
	FILE* fptr = NULL;

#pragma warning( push )
#pragma warning( disable : 4996 )
	fptr = _wfopen(FullFilePathAndName, L"wb");
#pragma warning( pop )
	if (!fptr)
	{
		goto Leave;
	}

	fwrite(Data, sizeof(char), DataLength, fptr);

	fflush(fptr);

Leave:
	if (fptr)
	{
		fclose(fptr);
	}

	RetVal = 0;
	return RetVal;
}

int __cdecl wmain(int argc, LPCWSTR argv[])

{
	if (argc !=4)
	{
		printf("Bad number of parameters received %d\n", argc);
		return -1;
	}

	int RetVal = -1;
	size_t size = wcslen(argv[1]) + 1;
	char* cArgv_1 = new char[size];
	if (!WideCharToMultiByte(CP_UTF8, 0, argv[1], -1, cArgv_1, (int)size, NULL, NULL))
	{
		printf("Internal error cant convert argument to utf %S\n", argv[1]);
		delete[] cArgv_1;
		return -1;
	}
	string Argv_1(cArgv_1);
	delete[] cArgv_1;
	string Path(Argv_1.begin(), Argv_1.end());
	std::vector<std::string> Files;
	std::vector<SPLITED_FUNCTION_DATA> Functions;
	std::vector<SPLITED_FUNCTION_DATA> TotalFunctions;
	std::string FileData = "";

	WCHAR _Filename[1024];

// 	wcscpy(_Filename, argv[3]);

	UINT FilenameSize = (UINT)wcslen(argv[3]);
	for (UINT i = 0, j=0; i < FilenameSize; i++, j++)
	{
		if(argv[3][i] == L'\"')
		{
			i++;
		}
		_Filename[j] = argv[3][i];
	}


	if (!PathAppendW(_Filename, argv[2]))
	{
		return -1;
	}
	std::wstring Filename;

	Filename = _Filename;
	Filename += L"_Cmd.cpp";

	DeleteFile(Filename.c_str());
	size = wcslen(argv[2]) + 1;
	char* cArgv_2 = new char[size];
	if (!WideCharToMultiByte(CP_UTF8, 0, argv[2], -1, cArgv_2, (int)size, NULL, NULL))
	{
		printf("Internal error cant convert argument to utf %S\n", argv[2]);
		delete[] cArgv_2;
		return -1;
	}
	string Argv_2(cArgv_2);
	delete[] cArgv_2;
	string NAME(Argv_2.begin(), Argv_2.end());
	Files = GetAllFilesInDir(Path, ".cpp");

	std::string f;
	UINT i;
	size_t NumFiles = Files.size();

	for (i = 0; i < NumFiles; ++i)
	{
		f = "\\dosdevices\\";
		f.append(Files[i]);

		try
		{
			Functions = FindParserFuncs(f);
		}
		catch(...)
		{
			printf("Error in Parsing files\n");
			goto Leave;
		}
// 		printf("Files: %s %d Parser functions found\n", Files[i].c_str(), (int)Functions.size());
		TotalFunctions.insert(TotalFunctions.end(), Functions.begin(), Functions.end());
	}

	printf("Total %d Parser functions found\n", (int)TotalFunctions.size());
	if (!TotalFunctions.size())
	{
		goto Leave;
	}

	if (!MakeParserFileData(TotalFunctions, &FileData, NAME))
	{
		goto Leave;
	}

	RetVal = 0;

Leave:
	if (RetVal != 0)
	{
		FileData = "";
		RetVal = 0;
	}
	if (WriteToFile((PVOID)FileData.c_str(), (UINT)FileData.size(), (PWCHAR)Filename.c_str()))
	{
		printf("Error in WriteToFile\n");
	}

	return RetVal;
}

