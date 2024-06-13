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

	class IProcessUtils
	{
	public:

		static
		IProcessUtils
		*GetNewInstance(
			PEPROCESS ProcessObj
		);

		static
		VOID
		ReleaseInstance(
			IProcessUtils* Instance
		);

		virtual
		PUNICODE_STRING
		GetNtFullPathName(
		) = 0;

		virtual
		PUNICODE_STRING
		GetDosFullPathName(
		) = 0;

		virtual
		BOOLEAN
		NTAPI
		PsIsProtectedProcessLight(
			_In_ PEPROCESS Process
		) = 0;

	protected:
		IProcessUtils() {};
		virtual ~IProcessUtils() {};
	private:
	};
};
