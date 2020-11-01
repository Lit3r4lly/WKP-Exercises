#include <ntifs.h>
#include <ntddk.h>
#include "ThreadPriorityBoosterHeader.h"

void UnloadDriver(PDRIVER_OBJECT DriverObject);
NTSTATUS DispatchCreateClose(PDEVICE_OBJECT DeviceObject,PIRP Irp);
NTSTATUS DisptachControlDevice(PDEVICE_OBJECT DeviceObject, PIRP Irp);

/*
	This function considered as the main of the driver
	input: driver object, registry path
	output: native kernel success status
*/

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(RegistryPath);

	NTSTATUS status = 0;
	PDEVICE_OBJECT DeviceObject = NULL;
	UNICODE_STRING deviceName = RTL_CONSTANT_STRING(L"\\Device\\ThreadPriorityBooster");
	UNICODE_STRING symbolicLink = RTL_CONSTANT_STRING(L"\\??\\ThreadPriorityBooster");

	DriverObject->DriverUnload = UnloadDriver;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = DispatchCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = DispatchCreateClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DisptachControlDevice;

	status = IoCreateDevice(
		DriverObject,
		0,
		&deviceName,
		FILE_DEVICE_UNKNOWN,
		0,
		FALSE,
		&DeviceObject
	);
	if (!NT_SUCCESS(status)) {
		KdPrint(("Device initialization failed. \nError - 0x%08X", status));
		return status;
	}

	status = IoCreateSymbolicLink(&symbolicLink, &deviceName);
	if (!NT_SUCCESS(status)) {
		KdPrint(("Failed to create a symbolic link. \nError - %0x08X", status));
		IoDeleteDevice(DeviceObject);

		return status;
	}

	return STATUS_SUCCESS;
}

/*
	This function controls the unload routine of the driver
	input: driver object
	output: none
*/

void UnloadDriver(PDRIVER_OBJECT DriverObject) {
	UNICODE_STRING symbolicLink = RTL_CONSTANT_STRING(L"\\??\\ThreadPriorityBooster");

	IoDeleteSymbolicLink(&symbolicLink);
	IoDeleteDevice(DriverObject->DeviceObject);
}

/*
	This function controling the dispatch routine of the create / close major functions
	input: device object, irp
	output: native kernel success status
*/

NTSTATUS DispatchCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	
	return STATUS_SUCCESS;
}

/*
	This function controls the dispatch routine of the control device major function
	input: device object, irp
	output: native kernel success status
*/

NTSTATUS DisptachControlDevice(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);

	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
	NTSTATUS status = STATUS_SUCCESS;
	ThreadData* data = { 0 };
	PETHREAD thread = NULL;

	switch (stack->Parameters.DeviceIoControl.IoControlCode)
	{
	case IOCTL_PRIORITY_BOOSTER_SET_PRIORITY:
		if (stack->Parameters.DeviceIoControl.InputBufferLength < sizeof(ThreadData)) {
			status = STATUS_BUFFER_TOO_SMALL;
			return status;
		}

		data = stack->Parameters.DeviceIoControl.Type3InputBuffer;
		if (data == NULL) {
			status = STATUS_INVALID_PARAMETER;
			return status;
		}

		if (data->PriorityLevel < 1 || data->PriorityLevel > 31) {
			status = STATUS_INVALID_PARAMETER;
			return status;
		}

		status = PsLookupThreadByThreadId(UlongToHandle(data->ThreadId), &thread);
		if (!NT_SUCCESS(status)) {
			break;
		}

		KeSetPriorityThread((PKTHREAD)thread, data->PriorityLevel);
		ObDereferenceObject(thread);

		KdPrint(("Thread priority boost has been completed! \nThreadID - %d \nNew Priority Level - %d", data->ThreadId, data->PriorityLevel));
		break;
	
	default:
		status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}