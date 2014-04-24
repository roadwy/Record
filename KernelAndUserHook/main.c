#include "ntddk.h"
#include "winnt.h"

DWORD UsersharedMemory = 0x7FFE0800;
DWORD KerlsharedMemory = 0xFFDF0800;

NTSTATUS Hook(PIMAGE_DOS_HEADER base, HANDLE hProcesss)
{
	PIMAGE_DOS_HEADER dosHeader;
	PIMAGE_NT_HEADERS pNTHeader;
	PIMAGE_IMPORT_DESCRIPTOR pIID;
	PIMAGE_IMPORT_BY_NAME pImportByName;
	DWORD RVA;
	PDWORD pIAT, pINT;

	int count, index;
	char *dllName = NULL;
	char *dllTarget = "kernel32.dll";
	char *funcTarget = "GetProcAddress";
	PMDL MyMDL;
	PDWORD MappedTable;
	
	dosHeader = base;
	pNTHeader = (PIMAGE_NT_HEADERS)(dosHeader->e_lfanew + (DWORD)base);

	if (pNTHeader->Signature != IMAGE_NT_SIGNATURE)
		return STATUS_INVALID_IMAGE_FORMAT;

	RVA = pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	pIID = (PIMAGE_IMPORT_DESCRIPTOR)(RVA + (DWORD)base);

	for (count = 0; count < pIID[count].Characteristics!=0; count++)
	{
		dllName = (char *)(pIID[count].Name + (DWORD)base);
		pIAT = (PDWORD)((DWORD)base + (DWORD)pIID[count].FirstThunk);
		PINT = (PDWORD)((DWORD)base + (DWORD)pIID[count].OriginalFirstThunk);
		for (index=0; pIAT[index]!=0; index++)
		{
			if ((pINT[index] & IMAGE_ORDINAL_FLAG) != IMAGE_ORDINAL_FLAG)
			{
				pImportByName = (PIMAGE_IMPORT_BY_NAME)(pINT[index] + (DWORD)base);
				if ((_stricmp(dllName, dllTarget) == 0) &&
					(_stricmp(pImportByName->Name, funcTarget) == 0))
				{
					MyMDL = MmCreateMdl(NULL, &pIAT[index], 4);
					if (!MyMDL) return STATUS_UNSUCCESSFUL;
					MmBuildMdlForNonPagedPool(MyMDL);
					MyMDL->MdlFlags |= MDL_MAPPED_TO_SYSTEM_VA;
					MappedTable = MmMapLockedPages(MyMDL, KernelMode);
					*MappedTable = UsersharedMemory;
					MmUnmapLockedPages(MappedTable,MyMDL);
					IoFreeMdl(MyMDL);
				}
			}
		}
	}
	return STATUS_SUCCESS;
}

void MyFunc(IN PUNICODE_STRING dllName,
			IN HANDLE hProcess,
			IN PIMAGE_INFO pImageInfo)
{
	UNICODE_STRING targetDll;
	RtlInitUnicodeString(&targetDll, L"\\WINDOWS\\system32\\kernel32.dll");
	if (RtlCompareUnicodeString(dllName, &targetDll, FALSE) == 0)
	{
		Hook(pImageInfo->ImageBase, hProcess);
	}
}