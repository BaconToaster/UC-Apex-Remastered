#include "func-hook.h"
#include "Util.h"

#pragma warning (disable : 4022)

NTSTATUS NTAPI MmCopyVirtualMemory
(
	PEPROCESS SourceProcess,
	PVOID SourceAddress,
	PEPROCESS TargetProcess,
	PVOID TargetAddress,
	SIZE_T BufferSize,
	KPROCESSOR_MODE PreviousMode,
	PSIZE_T ReturnSize
);

NTKERNELAPI PPEB NTAPI PsGetProcessPeb
(
	IN PEPROCESS Process
);

NTSTATUS ZwQuerySystemInformation(
	IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
	OUT PVOID SystemInformation,
	IN ULONG SystemInformationLength,
	OUT PULONG ReturnLength OPTIONAL
);

PVOID NTAPI RtlFindExportedRoutineByName(
	PVOID ImageBase,
	PCCH RoutineName
);

UNICODE_STRING moduleName;

BOOLEAN IsValidAddr(ULONG64 ptr)
{
	ULONG64 min = 0x0001000;
	ULONG64 max = 0x7FFFFFFEFFFF;
	BOOLEAN result = (ptr > min && ptr < max);
	return result;
}

PVOID GetSystemModuleExport(LPCSTR modName, LPCSTR routineName)
{
	PVOID lpModule = GetSystemModBase(modName);

	if (!lpModule)
		return NULL;
	
	return RtlFindExportedRoutineByName(lpModule, routineName);
}

PVOID GetSystemModBase(LPCSTR modName)
{
	ULONG bytes = 0;
	NTSTATUS status = ZwQuerySystemInformation(SystemModuleInformation, NULL, bytes, &bytes);

	if (!bytes)
		return NULL;

	PRTL_PROCESS_MODULES modules = (PRTL_PROCESS_MODULES)ExAllocatePool(NonPagedPool, bytes);

	status = ZwQuerySystemInformation(SystemModuleInformation, modules, bytes, &bytes);

	if (!NT_SUCCESS(status))
		return NULL;



	PRTL_PROCESS_MODULE_INFORMATION module = modules->Modules;
	PVOID module_base = 0, module_size = 0;

	for (ULONG i = 0; i < modules->NumberOfModules; i++)
	{
		if (!strcmp((char*)module[i].FullPathName, modName))
		{
			module_base = module[i].ImageBase;
			module_size = (PVOID)module[i].ImageSize;
			break;
		}
	}

	if (modules)
		ExFreePool(modules);

	if (module_base <= NULL)
		return NULL;

	return module_base;
}

void WriteToReadOnly(PVOID dst, PVOID src, SIZE_T size)
{
	PMDL mdl = IoAllocateMdl(dst, (ULONG)size, FALSE, FALSE, NULL);

	if (!mdl)
		return;

	MmProbeAndLockPages(mdl, KernelMode, IoReadAccess);
	PVOID mapping = MmMapLockedPagesSpecifyCache(mdl, KernelMode, MmNonCached, NULL, FALSE, NormalPagePriority);
	MmProtectMdlSystemAddress(mdl, PAGE_READWRITE);

	RtlCopyMemory(mapping, src, size);

	MmUnmapLockedPages(mapping, mdl);
	MmUnlockPages(mdl);
	IoFreeMdl(mdl);
	
	return;
}

void HookFunction(PVOID src, LPCSTR funcName)
{
	RtlInitUnicodeString(&moduleName, Harz4StrCryptW(L"n1]lat*ata"));

	if (!src)
		return;

	// \\SystemRoot\\System32\\drivers\\dxgkrnl.sys
	PVOID* origFunction = (PVOID*)GetSystemModuleExport(Harz4StrCrypt("XOuopaiNkkpXOuopai/.X`neranoX`tcgnjh*ouo"), funcName);

	if (!origFunction)
		return;

	UINT_PTR hookAddr = (UINT_PTR)src;

	BYTE movInst[2] = { 0x48, 0xBA }; // mov rdx,  
	BYTE jmpInst[2] = { 0xFF, 0xE2 }; // jmp rdx

	BYTE originalInstructions[] = { 0x48, 0x8B, 0xC4, 0x48, 0x89, 0x48, 0x08, 0x53, 0x56, 0x57 };
	BYTE shellcodeEnd[] = { 0x5B, 0x5E, 0x5F, 0x48, 0x83, 0xC0, 0x19, 0x48, 0xFF, 0xC0, 0x48, 0x39, 0xD0, 0x48, 0x83, 0xE8, 0x19, 0x48, 0x39, 0xD8, 0x48, 0xFF, 0xC8, 0x48, 0x39, 0xC1, 0x48, 0x81, 0xE9,
		0x69, 0x69, 0x00, 0x00, 0x48, 0x39, 0xD9, 0x48, 0x81, 0xC1, 0x69, 0x69, 0x00, 0x00, 0xFF, 0xE2 };
	BYTE newInstructions[68] = { 0x0 };

	RtlSecureZeroMemory(&newInstructions, sizeof(newInstructions));

	memcpy((PVOID)((UINT_PTR)newInstructions), &originalInstructions, sizeof(originalInstructions));
	memcpy((PVOID)((UINT_PTR)newInstructions + sizeof(originalInstructions)), &movInst, sizeof(movInst));
	memcpy((PVOID)((UINT_PTR)newInstructions + sizeof(originalInstructions) + sizeof(movInst)), &hookAddr, sizeof(hookAddr));
	memcpy((PVOID)((UINT_PTR)newInstructions + sizeof(originalInstructions) + sizeof(movInst) + sizeof(hookAddr)), &shellcodeEnd, sizeof(shellcodeEnd));
	memcpy((PVOID)((UINT_PTR)newInstructions + sizeof(originalInstructions) + sizeof(movInst) + sizeof(hookAddr) + sizeof(shellcodeEnd)), &jmpInst, sizeof(jmpInst));

	WriteToReadOnly(origFunction, &newInstructions, sizeof(newInstructions));

	return;
}

NTSTATUS Hook(PVOID param)
{
	if (param == NULL)
		return STATUS_UNSUCCESSFUL;

	PKERNEL_REQUEST request = (PKERNEL_REQUEST)param;

	switch (request->instructionID)
	{
	case INST_GETMODBASE:
	{
		PEPROCESS process = 0;
		PVOID result = 0;

		if (NT_SUCCESS(PsLookupProcessByProcessId(request->pID, &process)))
		{
			PPEB64 peb = (PPEB64)PsGetProcessPeb(process);

			if (peb > 0)
			{
				KAPC_STATE state;
				KeStackAttachProcess(process, &state);

				// InLoadOrderLinks will have main executable first, ntdll.dll second, kernel32.dll
				for (PLIST_ENTRY pListEntry = peb->Ldr->InLoadOrderLinks.Flink; pListEntry != &peb->Ldr->InLoadOrderLinks; pListEntry = pListEntry->Flink)
				{
					if (!pListEntry)
						continue;

					PLDR_DATA_TABLE_ENTRY module_entry = CONTAINING_RECORD(pListEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

					if (RtlCompareUnicodeString(&module_entry->BaseDllName, &moduleName, TRUE) == 0)
						result = module_entry->DllBase;
				}

				KeUnstackDetachProcess(&state);
			}
		}
		request->response = result;
		break;
	}

	case INST_READ:
	{
		SIZE_T tmp = 0;
		PEPROCESS proc;
		PEPROCESS srcProc;
		if (NT_SUCCESS(PsLookupProcessByProcessId(request->pID, &proc)) && NT_SUCCESS(PsLookupProcessByProcessId(request->srcPID, &srcProc)) && IsValidAddr(request->srcAddr) && IsValidAddr(request->targetAddr))
			MmCopyVirtualMemory(proc, (PVOID)request->srcAddr, srcProc, (PVOID)request->targetAddr, request->size, KernelMode, &tmp);

		break;
	}

	case INST_WRITE:
	{
		SIZE_T tmp = 0;
		PEPROCESS proc;
		PEPROCESS srcProc;
		if (NT_SUCCESS(PsLookupProcessByProcessId(request->pID, &proc)) && NT_SUCCESS(PsLookupProcessByProcessId(request->srcPID, &srcProc)) && IsValidAddr(request->targetAddr) && IsValidAddr(request->srcAddr))
			MmCopyVirtualMemory(srcProc, (PVOID)request->srcAddr, proc, (PVOID)request->targetAddr, request->size, KernelMode, &tmp);

		break;
	}

	case INST_ISRUNNING:
	{
		request->response = (PVOID)0x6969;

		break;
	}

	default:
		break;
	}

	return STATUS_SUCCESS;
}