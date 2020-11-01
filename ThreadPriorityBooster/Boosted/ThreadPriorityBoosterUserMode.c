#include <Windows.h>
#include <stdio.h>
#include "..\ThreadPriorityBooster\ThreadPriorityBoosterHeader.h"

void DeviceControl(char* ThreadID, char* ThreadPriority, HANDLE hDevice);

int main(int argc, char** argv) {
	HANDLE hDevice = NULL;

	if (argc < 3) {
		printf("Usage: Boosted.exe <ThreadID> <PriorityLevel>");
		return 0;
	}

	hDevice = CreateFile(
		L"\\\\.\\ThreadPriorityBooster",
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL
	);
	if (hDevice == INVALID_HANDLE_VALUE) {
		printf("Failed to create a handle to device object. \nError - %d", GetLastError());
		return 0;
	}

	DeviceControl(argv[1], argv[2], hDevice);
	CloseHandle(hDevice);
	return 0;
}

/*
	This function pass the data to the device object using the handle
	input: threadID, thread priority, device handle
	output: none
*/

void DeviceControl(char* ThreadID, char* ThreadPriority, HANDLE hDevice) {
	DWORD returned = 0;
	BOOL success = 0;
	ThreadData data = { 0, 0 };

	data.ThreadId = atoi(ThreadID);
	data.PriorityLevel = atoi(ThreadPriority);

	success = DeviceIoControl(
		hDevice,
		IOCTL_PRIORITY_BOOSTER_SET_PRIORITY,
		&data,
		sizeof(data),
		NULL,
		0,
		&returned,
		NULL
	);
	if (success) {
		printf("Priority boost has been completed.");
	}
	else {
		printf("Priority boost has been failed.");
	}
}