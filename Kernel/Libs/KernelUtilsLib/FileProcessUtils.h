#pragma once

#include <KernelUtilsLib.h>

namespace KernelUtilsLib
{
	class CProcessUtils : public IProcessUtils
	{
		friend IProcessUtils;
	public:

		virtual PUNICODE_STRING GetNtFullPathName()  { return m_FullNtPathName; };
		virtual PUNICODE_STRING GetDosFullPathName() { return m_FullDosPathName; };


	private:
		CProcessUtils();
		virtual ~CProcessUtils();

		PUNICODE_STRING m_FullNtPathName;
		PUNICODE_STRING m_FullDosPathName;


		PEPROCESS m_ProcessObj;
	};
};
