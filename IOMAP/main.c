#include "ntddk.h"
#include "string.h"

#ifndef DWORD
#define DWORD unsigned int
#endif

#ifndef WORD
#define WORD unsigned short
#endif

#define IBUSY 9
#define LIMIT 0x20AB
#define EXPAN 0x2FFF

#pragma pack(1)
typedef struct _GDTR {
	WORD Limit;
	DWORD *dwBase;
} GDTR, *PGDTR;
typedef struct _GDTENTRY{
    DWORD dwLimit : 16;
    DWORD dwBaselo : 16;
    DWORD dwBasemid : 8;
    DWORD dwType : 4;
    DWORD dwSystem : 1;
    DWORD dwDpl : 2;
    DWORD dwPresent : 1;
    DWORD dwLimithi : 4;
    DWORD dwAvailable : 1;
    DWORD dwZero : 1;
    DWORD dwSize : 1;
    DWORD dwGranularity : 1;
    DWORD dwBasehi : 8;
} GDTENTRY, *PGDTENTRY;
#pragma pack()


void SetIO(int vv)
{
	GDTR gdtr;
	PGDTENTRY entry;
	WORD TSSSeg;

	__asm
	{
		cli;
		sgdt gdtr;
		str TSSSeg;
		movzx esi, TSSSeg;
		add esi, gdtr.dwBase;
		mov entry, esi
	}

	if (vv)
	{
		entry->dwLimit = EXPAN;
	} else
	{
		entry->dwLimit = LIMIT;
	}

	entry->dwType = IBUSY;
	__asm
	{
		ltr TSSSeg;
		sti;
	}
}

void Unload(PDRIVER_OBJECT driver)
{
	SetIO(0);
	DbgPrint("Unload!\n");
}

NTSTATUS DriverEntry(PDRIVER_OBJECT driver, PUNICODE_STRING szReg)
{
	SetIO(1);
	driver->DriverUnload = Unload;
	DbgPrint("Entry!\n");
	return STATUS_SUCCESS;
}