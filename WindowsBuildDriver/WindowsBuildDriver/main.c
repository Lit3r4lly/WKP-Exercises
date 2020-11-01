#include <ntddk.h>

int getSysInfo();
void sampleUnload(_In_ PDRIVER_OBJECT DriverObject);

/*
	This function consider like 'main' function
	input: Driver object, Registry path
	output: None
*/

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(RegistryPath);
	DriverObject->DriverUnload = sampleUnload;

	getSysInfo();
	KdPrint(("SampleDriver has loaded and initialized successfully!!\n"));
	return STATUS_SUCCESS;
}

/*
	This function get the system version (major, minor, build number)
	input: None
	output: None
*/

int getSysInfo() {
	RTL_OSVERSIONINFOW sysVersionInfo = { 0 };
	NTSTATUS status = RtlGetVersion(&sysVersionInfo);

	if (!NT_SUCCESS(status)) {
		KdPrint(("Error occurred: 0x%08X\n", status));
		return status;
	}

	KdPrint(("Major: %ul \nMinor: %ul \nBuild: %ul", sysVersionInfo.dwMajorVersion, sysVersionInfo.dwMinorVersion, sysVersionInfo.dwBuildNumber));
	return STATUS_SUCCESS;
}

/*
	This function unloads the driver
	input: Driver object
	output: None
*/

void sampleUnload(_In_ PDRIVER_OBJECT DriverObject) {
	KdPrint(("Unload has been started!\n"));
	UNREFERENCED_PARAMETER(DriverObject);
}