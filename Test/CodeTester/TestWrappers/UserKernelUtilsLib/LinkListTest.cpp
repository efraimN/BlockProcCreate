#include "Precompiled.h"

#include <WppIncludes.h>

#include <ILinkListLibInt.h>

using namespace UserKernelUtilsLib;

#define TestLootp 10

class MyTestElelment : public ILinkListElement
{
public:
	DWORD MagicTest;

private:

};

MyTestElelment ArrayTest[TestLootp+5];

/*!
Usage:
Tester -LinkListTest


for debug purposes

*/
int RUN_FUNCTION LinkListTest()
{
	int RetVal = -1;
	ILinkListLibInt* pLinkListLibInt;
	MyTestElelment* Element;

	for (UINT i=0;i< TestLootp;i++)
	{
		ArrayTest[i].MagicTest = i;
	}

	pLinkListLibInt = ILinkListLibInt::GetNewInstance();
	if (!pLinkListLibInt)
	{
		goto Leave;
	}

	if (!pLinkListLibInt->Start())
	{
		goto Leave;
	}

	// Test PushItemHead vs PopItemHead
	for (UINT i = 0; i < TestLootp; i++)
	{
		pLinkListLibInt->PushItemHead(&ArrayTest[i]);
	}

	pLinkListLibInt->GetElement(
		[](ILinkListElement* Element, PVOID /*FindContext*/)->BOOLEAN
		{
			printf("get element: %d\n",
				((MyTestElelment*)Element)->MagicTest
			);

			return FALSE; // to continue the enumeration
		},
		NULL,
		FALSE
	);
	printf("\n");

	pLinkListLibInt->GetElement(
		[](ILinkListElement* Element, PVOID FindContext)->BOOLEAN
		{
			printf("get element: %d\n",
				((MyTestElelment*)Element)->MagicTest
			);

			if (((MyTestElelment*)Element)->MagicTest == (DWORD)DOWNCASTHANDLE(FindContext))
			{
				return TRUE;
			}
			return FALSE; // to continue the enumeration
		},
		(PVOID)4,
		TRUE
	);
	printf("\n");

	pLinkListLibInt->GetElement(
		[](ILinkListElement* Element, PVOID /*FindContext*/)->BOOLEAN
		{
			printf("get element: %d\n",
				((MyTestElelment*)Element)->MagicTest
			);

			return FALSE; // to continue the enumeration
		},
		NULL,
		FALSE
		);

	printf("\n");

	pLinkListLibInt->GetElement(
		[](ILinkListElement* Element, PVOID FindContext)->BOOLEAN
		{
			printf("get element: %d\n",
				((MyTestElelment*)Element)->MagicTest
			);

			if (((MyTestElelment*)Element)->MagicTest == (DWORD)DOWNCASTHANDLE(FindContext))
			{
				return TRUE;
			}
			return FALSE; // to continue the enumeration
		},
		(PVOID)7,
		TRUE
		);
	printf("\n");

	pLinkListLibInt->GetElement(
		[](ILinkListElement* Element, PVOID /*FindContext*/)->BOOLEAN
		{
			printf("get element: %d\n",
				((MyTestElelment*)Element)->MagicTest
			);

			return FALSE; // to continue the enumeration
		},
		NULL,
		FALSE
		);

	printf("\n");

	for (UINT i = 0; i < TestLootp; i++)
	{
		Element = (MyTestElelment*)pLinkListLibInt->PopItemHead();
		if (!Element)
		{
			printf("no element at index %d\n", i);
		}
		else
		{
			printf("get element index: %d Value: %d\n",
				i,
				Element->MagicTest
			);
		}
// 		if (Element->MagicTest != (TestLootp -i -1))
// 		{
// 			printf("Element->MagicTest != i **** TEST FAILED *****\n");
// 			goto Leave;
// 		}
	}

	printf("\n");

	// Test PushItemHead vs PopItemTail

	// Test PushItemTail vs PopItemTail
	
	// Test PushItemTail vs PopItemHead

	printf("***** TEST PASS *****\n");
	RetVal = 0;
Leave:
	if (pLinkListLibInt)
	{
		pLinkListLibInt->Stop();
		ILinkListLibInt::FreeInstance(pLinkListLibInt);
	}

	if (RetVal != 0)
	{
		printf("***** TEST FAILED *****\n");
	}
	return RetVal;
}
