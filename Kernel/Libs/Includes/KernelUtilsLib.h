#pragma once

namespace KernelUtilsLib
{
	namespace CRegistryUtil
	{

		NTSTATUS
		InitDriverRegPath(
			IN PUNICODE_STRING RegistryPath
		);

		void ReleaseDriverRegPath();

		extern UNICODE_STRING m_RegistryPath;
		extern UNICODE_STRING m_CurrentControlSetPath;
		extern UNICODE_STRING m_DriverImageFileName;
		extern UNICODE_STRING m_Attitude;
	};

};
