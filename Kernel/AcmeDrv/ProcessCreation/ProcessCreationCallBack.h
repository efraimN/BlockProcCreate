#pragma once

class CProcCreateCallback
{
public:

	static CProcCreateCallback* GetInstance();

	NTSTATUS Start();
	NTSTATUS Stop();

protected:

private:
	static
	VOID ProcessNotifyCallBack_(
		_Inout_  PEPROCESS              Process,
		_In_     HANDLE                 ProcessId,
		_In_opt_ PPS_CREATE_NOTIFY_INFO CreateInfo
	);

	VOID ProcessNotifyCallBack(
		_Inout_  PEPROCESS              Process,
		_In_     HANDLE                 ProcessId,
		_In_opt_ PPS_CREATE_NOTIFY_INFO CreateInfo
	);

	CProcCreateCallback();
	virtual ~CProcCreateCallback();
};
