#include "Process.h"

typedef enum
{
	OriginalApcEnvironment,
	AttachedApcEnvironment,
	CurrentApcEnvironment
} KAPC_ENVIRONMENT;

void ApcKernelRoutine(
	IN struct _KAPC *Apc,
	IN OUT PKNORMAL_ROUTINE *NormalRoutine,
	IN OUT PVOID *NormalContext,
	IN OUT PVOID *SystemArgument1,
	IN OUT PVOID *SystemArgument2);
NTSTATUS InstallUserModeApc(LPSTR lpProcess, ULONG pTargetThread, ULONG pTargetProcess);
void ApcCreateProcess(PVOID NormalContext, PVOID SystemArgument1, PVOID SystemArgument2);
void ApcCreateProcessEnd();

void RunProcess(LPSTR lpProcess)
{
	char *name;
	ULONG pTargetProcess;
	ULONG pTargetThread;
	ULONG pNotAlertableThread;
	ULONG pSystemProcess;
	ULONG pTempThread;
	ULONG pNextEntry, pListHead, pThreadNextEntry, pThreadListHead;

	if (strlen(lpProcess) > 300) return;
	pSystemProcess = (ULONG)PsGetCurrentProcess();
	if (!pSystemProcess)
	{
		DbgPrint("APCExec -> Cannot find System process!");
		return;
	}

	pListHead = pSystemProcess + 0x88;
	pNextEntry = *(ULONG*)pListHead;
	if (!pNextEntry)
	{
		DbgPrint("KernelExec -> No processes found!");
		return;
	}
	
	while (pNextEntry != pListHead)
	{
		pSystemProcess = pNextEntry - 0x88;
		name = (char *)pSystemProcess + 0x174;
		DbgPrint("ProcessName %s\n", name);
		if (_strnicmp(name, "windbg.exe", 12) == 0)
		{
			pTargetProcess = pSystemProcess;
			DbgPrint("Found explorer.exe!\n");
			pTargetThread = pNotAlertableThread = 0;
			pThreadListHead = pSystemProcess + 0x50;
			pThreadNextEntry = *(ULONG*)pThreadListHead;
			while (pThreadNextEntry != pThreadListHead)
			{
				pTempThread = pThreadNextEntry - 0x1b0;
				DbgPrint("ETHREAD address is : 0x%08x\n", (ULONG*)pTempThread);
				DbgPrint("Thread ID is : %d\n", *(ULONG*)(pTempThread + 0x1f0));
				DbgPrint("Alertable is : 0x%08x", *(char*)(pTempThread + 0x164));
				if (*(char*)(pTempThread + 0x164))
				{
					pTargetThread = pTempThread;
					DbgPrint("Found alertable thread!\n");
					break;
				}
				else
				{
					pNotAlertableThread = pTempThread;
				}
				pThreadNextEntry = *(ULONG*)pThreadNextEntry;
			}
			break;
		}
		pNextEntry = *(ULONG*)pNextEntry;
	}
	
	if (!pTargetProcess)
	{
		DbgPrint("Could not find Explorer.exe");
		return;
	}
	if (!pTargetThread)
	{
		pTargetThread = pNotAlertableThread;
	}
	if (pTargetThread)
	{
		DbgPrint("Target thread: 0x%p", pTargetThread);
		InstallUserModeApc(lpProcess, pTargetThread, pTargetProcess);
	}
	else
	{
		DbgPrint("No thread found!\n");
	}
}

PMDL MyMDL = NULL;
void ApcKernelRoutine(IN struct _KAPC *Apc, 
					  IN OUT PKNORMAL_ROUTINE *NormalRoutine, 
					  IN OUT PVOID *NormalContext,
					  IN OUT PVOID *SystemArgument1, 
					  IN OUT PVOID *SystemArgument2)
{
	if (Apc) ExFreePool(Apc);
	if (MyMDL)
	{
		MmUnlockPages(MyMDL);
		IoFreeMdl(MyMDL);
		MyMDL = NULL;
	}
	DbgPrint("ApcKernelRoutine called!\n");
}

NTSTATUS InstallUserModeApc(LPSTR lpProcess, ULONG pTargetThread, ULONG pTargetProcess)
{
	PRKAPC pApc = NULL;
	PVOID pMappedAddress = NULL;
	ULONG dwSize = 0;
	KAPC_STATE ApcState;
	ULONG *data_addr = NULL;
	ULONG dwMappedAddress = 0;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	if (!pTargetProcess || !pTargetThread)
	{
		return status;
	}
	pApc = ExAllocatePool(NonPagedPool, sizeof(KAPC));
	if (!pApc)
	{
		DbgPrint("ExAllocatePool failed!\n");
		return status;
	}

	dwSize = (unsigned char*)ApcCreateProcessEnd - (unsigned char*)ApcCreateProcess;
	MyMDL = IoAllocateMdl(ApcCreateProcess, dwSize, FALSE, FALSE, NULL);
	if (!MyMDL)
	{
		DbgPrint("Failed to allocate MDL!\n");
		ExFreePool(pApc);
		return status;
	}

	__try
	{
		MmProbeAndLockPages(MyMDL, KernelMode, IoWriteAccess);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		DbgPrint("MmProbeAndLockPages error!\n");
		IoFreeMdl(MyMDL);
		ExFreePool(pApc);
		return status;
	}

	KeStackAttachProcess((PRKPROCESS)pTargetProcess, &ApcState);
	pMappedAddress = MmMapLockedPagesSpecifyCache(MyMDL,
		UserMode, MmCached, NULL, FALSE, NormalPagePriority);

	if (!pMappedAddress)
	{
		DbgPrint("Cannot map address");
		KeUnstackDetachProcess(&ApcState);
		IoFreeMdl(MyMDL);
		ExFreePool(pApc);
		return STATUS_UNSUCCESSFUL;
	}
	else
	{
		DbgPrint("Memory at %p", pMappedAddress);
	}

	dwMappedAddress = (ULONG)pMappedAddress;
	memset((unsigned char*)pMappedAddress + 0x14, 0, 50);
	memcpy((unsigned char*)pMappedAddress + 0x14, lpProcess, strlen(lpProcess));

	data_addr = (ULONG *)((char*)pMappedAddress + 0x9);
	*data_addr = dwMappedAddress + 0x14;

	KeUnstackDetachProcess(&ApcState);
	KeInitializeApc(pApc,
		(ULONG*)pTargetThread,
		OriginalApcEnvironment,
		&ApcKernelRoutine,
		NULL,
		pMappedAddress,
		UserMode,
		NULL);
	if (!KeInsertQueueApc(pApc, 0, NULL, 0))
	{
		DbgPrint("Failed to insert APC");
		MmUnlockPages(MyMDL);
		IoFreeMdl(MyMDL);
		ExFreePool(pApc);
		return STATUS_UNSUCCESSFUL;
	}
	else
	{
		DbgPrint("Apc delivered!\n");
	}

	if (!*(char*)(pTargetThread+0x4a))
	{
		*(char*)(pTargetThread+0x4a) = TRUE;
	}
	return 0;
}

__declspec(naked) void ApcCreateProcess(PVOID NormalContext, PVOID  SystemArgument1, PVOID SystemArgument2)
{
	__asm 
	{
		mov eax,0x7C8623AD
		push 1
		nop
		push 0xabcd
		call eax
		jmp end       
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
end:
		nop
		ret 0x0c
	}

}

void ApcCreateProcessEnd()
{

}
