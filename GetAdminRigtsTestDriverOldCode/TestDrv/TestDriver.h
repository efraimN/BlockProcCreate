#pragma once

#include WPP_TO_INCLUDE(TestDriver.h.h)

class TestDriver
{
public:
	TestDriver();
	virtual~TestDriver();
NTSTATUS
Test_Dr_DriverEntry(
			IN PDRIVER_OBJECT DriverObject,
			IN PUNICODE_STRING RegistryPath
			);

static void Test_Dr_DriverExit(IN PDRIVER_OBJECT DriverObject);

UNICODE_STRING m_RegistryPath;
};


