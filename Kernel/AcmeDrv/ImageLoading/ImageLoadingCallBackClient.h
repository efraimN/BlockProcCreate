#pragma once

class ImageLoadingCallBackClient
{
public:

	virtual
		NTSTATUS  Start() = 0;

	virtual
		NTSTATUS  Stop() = 0;

	virtual
		void
		ImageLoadingCallBack(
			PUNICODE_STRING FullImageName,
			HANDLE ProcessId,
			PIMAGE_INFO ImageInfo
		) = 0;

protected:
	ImageLoadingCallBackClient() {};
	virtual~ImageLoadingCallBackClient() {};

private:
	ImageLoadingCallBackClient(const ImageLoadingCallBackClient& other);
	ImageLoadingCallBackClient& operator=(const ImageLoadingCallBackClient& other);

};
