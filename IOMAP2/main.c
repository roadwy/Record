#include "ntddk.h"

#define IOPM_SIZE 0x2000

typedef UCHAR IOPM[IOPM_SIZE];
IOPM* MyIOPM = NULL;

void Ke386SetIoAccessMap(int, IOPM *);
void Ke386QueryIoAccessMap(int, IOPM *);
void Ke386IoSetAccessProcess(PEPROCESS, int);
NTSTATUS PsLookupProcessByProcessId(
	IN ULONG ulProcId,
	OUT PEPROCESS* pEProcess
);

NTSTATUS Create(PDEVICE_OBJECT device, PIRP pIrp)
{
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS Dispatch(PDEVICE_OBJECT device, PIRP pIrp)
{
	PIO_STACK_LOCATION irpsp;
	NTSTATUS status = STATUS_SUCCESS;
	PUCHAR charBuffer;
	PUSHORT shortBuffer;
	PULONG longBuffer;
	PVOID ioBuffer;

	USHORT offset;
	UCHAR value;

	int a, b;
	ULONG processID;
	PEPROCESS process;

	irpsp = IoGetCurrentIrpStackLocation(pIrp);
	ioBuffer = pIrp->AssociatedIrp.SystemBuffer;
	charBuffer = (PUCHAR)ioBuffer;
	shortBuffer = (PUSHORT)ioBuffer;
	longBuffer = (PULONG)ioBuffer;

	switch (irpsp->Parameters.DeviceIoControl.IoControlCode)
	{
	case 0x04:
		DbgPrint("IOCTL: 0x04 - Set IOPM of ProcessID");
		processID = longBuffer[0];
		PsLookupProcessByProcessId(processID, &process);
		DbgPrint("Pointer to process is %08x\n", process);
		DbgPrint("Address = %08x\n", *(*MyIOPM + 0x6F));
		Ke386SetIoAccessMap(1, MyIOPM);
		Ke386IoSetAccessProcess(process, 1);
		break;
	case 0x08:
		DbgPrint("IOCTL 0x08 - READ_PORT_UCHAR 0x%X", shortBuffer[0]);
		(UCHAR)value = READ_PORT_UCHAR((PUCHAR)shortBuffer[0]);
		DbgPrint("Value read : %X", value);
		charBuffer[0] = value;
		break;
	case 0x0C:
		DbgPrint("IOCTL 0x0C - WRITE_PORT_UCHAR(0x%X <- 0x%X)", shortBuffer[0]);
		WRITE_PORT_UCHAR((PUCHAR)shortBuffer[0], charBuffer[2]);
		break;
	case 0x10:
		DbgPrint("IOCTL 0x10 - RTLFillMemory (Turn off all access)");
		RtlFillMemory(MyIOPM, sizeof(IOPM), 0xFF);
		break;
	case 0x14:
		DbgPrint("IOCTL 0x10 - Set IO Permission Bitmap");
		offset = shortBuffer[0];
		value = charBuffer[2];
		DbgPrint("Offset = %x, value = %x\n", offset, value);
		*(*MyIOPM + offset) = value;
		break;
	default:
		status = STATUS_UNSUCCESSFUL;
	}
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;
	return status;
}

void Unload(PDRIVER_OBJECT driver)
{
	WCHAR DOSNameBuffer[] = L"\\DosDevices\\PortTalk";
	UNICODE_STRING uniDosString;

	DbgPrint("Unload!\n");
	if (MyIOPM)
		MmFreeNonCachedMemory(MyIOPM, sizeof(IOPM));
	RtlInitUnicodeString(&uniDosString, DOSNameBuffer);
	IoDeleteSymbolicLink(&uniDosString);
	IoDeleteDevice(driver->DeviceObject);
}

NTSTATUS DriverEntry(PDRIVER_OBJECT driver, PUNICODE_STRING szReg)
{
	PDEVICE_OBJECT device;
	int a, b;
	NTSTATUS status;
	WCHAR NameBuffer[] = L"\\Device\\PortTalk";
	WCHAR DosNameBuffer[] = L"\\DosDevices\\PortTalk";
	UNICODE_STRING uniNameString, uniDosString;

	DbgPrint("PortTalk has loaded!\n");

	MyIOPM = MmAllocateNonCachedMemory(sizeof(IOPM));
	if (MyIOPM == NULL)
	{
		DbgPrint("MmAllocateNonCachedMemory error!\n");
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	RtlFillMemory(MyIOPM, sizeof(IOPM), 0xFF);
	DbgPrint("Memory Allocated at %x\n", MyIOPM);

	RtlInitUnicodeString(&uniNameString, NameBuffer);
	RtlInitUnicodeString(&uniDosString, DosNameBuffer);

	status = IoCreateDevice(
		driver, 
		0,
		&uniNameString,
		FILE_DEVICE_UNKNOWN, 
		0, 
		FALSE,
		&device );
	if (status != STATUS_SUCCESS)
		return status;

	status = IoCreateSymbolicLink(&uniDosString, &uniNameString);
	if (status != STATUS_SUCCESS)
		return status;

	driver->MajorFunction[IRP_MJ_CREATE] = Create;
	driver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = Dispatch;
	driver->DriverUnload = Unload;

	return STATUS_SUCCESS;
}
