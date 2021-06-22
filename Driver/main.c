#include "main.h"

#pragma warning (disable : 4047 4024 4311 4716 4020 4152 4113)

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING registryPath)
{
	UNREFERENCED_PARAMETER(pDriverObject);
	UNREFERENCED_PARAMETER(registryPath);

	HookFunction(&Hook, Harz4StrCrypt("JpOap?kilkoepekjOqnb]_aOp]peope_o"));

	return STATUS_SUCCESS;
}

NTSTATUS DriverUnload(PDRIVER_OBJECT pDriverObject)
{
	UNREFERENCED_PARAMETER(pDriverObject);

	return STATUS_SUCCESS;
}