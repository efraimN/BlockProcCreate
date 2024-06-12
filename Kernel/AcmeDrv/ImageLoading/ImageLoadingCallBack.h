#pragma once

#include "ImageLoadingCallBackClient.h"

class CImageLoadingCallBack
{
public:

	static CImageLoadingCallBack* GetInstance();

	NTSTATUS  Start();
	NTSTATUS  Stop();

protected:

private:
	CImageLoadingCallBack(const CImageLoadingCallBack& other);
	CImageLoadingCallBack& operator=(const CImageLoadingCallBack& other);

	CImageLoadingCallBack();
	virtual~CImageLoadingCallBack();

	static
	void
	PloadImageNotifyRoutine_(
		PUNICODE_STRING FullImageName,
		HANDLE ProcessId,
		PIMAGE_INFO ImageInfo
	);

	void
	InitCallBacks();

	ImageLoadingCallBackClient** m_ImageLoadingCallBackClientArray;

	BOOL m_Inited;
};

