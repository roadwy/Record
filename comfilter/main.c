#include "ntddk.h"

#define NTSTRSAFE_LIB
#include "ntstrsafe.h"

#define MAX_COMS 32
#define NAME_LENGTH 30
#define STOPTIME -5000

PDEVICE_OBJECT realCom[MAX_COMS] = { NULL };
PDEVICE_OBJECT nextCom[MAX_COMS] = { NULL };

void Unload(PDRIVER_OBJECT driver)
{
	ULONG i;
	LARGE_INTEGER internel;
	for (i=0; i<MAX_COMS; i++)
		if (nextCom[i] != NULL)
			IoDetachDevice(nextCom[i]);
	internel.QuadPart = STOPTIME;
	KeDelayExecutionThread(KernelMode, FALSE, &internel);
	for (i=0; i<MAX_COMS; i++)
		if (realCom[i] != NULL)
			IoDeleteDevice(realCom[i]);
}

void AttachAll(PDRIVER_OBJECT driver)
{
	ULONG i;
	NTSTATUS status;
	WCHAR name[NAME_LENGTH] = {0};
	UNICODE_STRING uniName;
	PDEVICE_OBJECT devObj;
	PFILE_OBJECT fileObj;
	for (i=0; i<MAX_COMS; i++)
	{
		RtlStringCchPrintfW( name, NAME_LENGTH, L"\\Device\\Serial%d", i);
		RtlInitUnicodeString(&uniName, name);
		status = IoGetDeviceObjectPointer(&uniName, FILE_ALL_ACCESS, &fileObj, &devObj);
		if (status != STATUS_SUCCESS) continue;
		ObDereferenceObject(fileObj);
		status = IoCreateDevice(driver,0,NULL,devObj->Type,0,FALSE, &realCom[i]);
		if (status != STATUS_SUCCESS) continue;
		realCom[i]->Flags = devObj->Flags | DO_DEVICE_INITIALIZING;
		nextCom[i] = IoAttachDeviceToDeviceStack( realCom[i], devObj);
		if (nextCom[i] == NULL)
		{
			IoDeleteDevice(realCom[i]);
			realCom[i] = NULL;
			continue;
		}
		realCom[i]->Type &= ~DO_DEVICE_INITIALIZING;
	}
}

NTSTATUS dispatch(PDEVICE_OBJECT device, PIRP irp)
{
	NTSTATUS status;
	PIO_STACK_LOCATION irpsp = IoGetCurrentIrpStackLocation(irp);
	ULONG i, j;
	for (i=0; i<MAX_COMS; i++)
	{
		if (realCom[i] == device)
		{
			if (irpsp->MajorFunction == IRP_MJ_PNP_POWER)
			{
				PoStartNextPowerIrp(irp);
				IoSkipCurrentIrpStackLocation(irp);
				return PoCallDriver(nextCom[i], irp);
			}
			if (irpsp->MajorFunction == IRP_MJ_WRITE)
			{
				ULONG len = irpsp->Parameters.Write.Length;
				PUCHAR buffer = NULL;
				if (irp->MdlAddress != NULL)
					buffer = (PUCHAR)MmGetSystemAddressForMdlSafe(
					irp->MdlAddress,
					NormalPagePriority);
				else
					buffer = (PUCHAR)irp->UserBuffer;
				if (buffer == NULL)
					buffer = (PUCHAR)irp->AssociatedIrp.SystemBuffer;
				for (j = 0; j<len; j++)
				{
					DbgPrint("data: %02x\n", buffer[j]);
				}
			}
			IoSkipCurrentIrpStackLocation(irp);
			return IoCallDriver(nextCom[i], irp);
		}
	}
	irp->IoStatus.Information = 0;
	irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
	IoCompleteRequest(irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT driver, PUNICODE_STRING szReg)
{
	ULONG i;
	for (i=0; i<IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		driver->MajorFunction[i] = dispatch;
	}
	driver->DriverUnload = Unload;
	AttachAll(driver);
	return STATUS_SUCCESS;
}