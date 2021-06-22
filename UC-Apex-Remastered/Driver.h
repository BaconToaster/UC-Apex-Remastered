#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdint>
#include <iostream>
#include "XorStr.hpp"

enum
{
	INST_GETMODBASE = 1,
	INST_READ,
	INST_WRITE,
	INST_ISRUNNING
};

typedef struct
{
	ULONG srcPID;
	ULONG pID;
	UINT_PTR srcAddr;
	UINT_PTR targetAddr;
	ULONG size;
	PVOID response;
	ULONG instructionID;
} KERNEL_REQUEST, * PKERNEL_REQUEST;

class KDriver
{
private:
	uintptr_t currentPID;
	uintptr_t targetPID;
	void* hookedFunc;

public:
	template<typename ... arg>
	uint64_t CallHook(const arg ... args)
	{
		// setting up a function template
		auto func = static_cast<uint64_t(_stdcall*)(arg...)>(hookedFunc);
		return func(args ...); // call the function
	}

	uintptr_t GetModuleBase(uintptr_t pID)
	{
		// set private pid variable to the passed pid so we dont need to pass it in the read and write function
		targetPID = pID;
		KERNEL_REQUEST modRequest; // setting up a kernel request struct
		modRequest.instructionID = INST_GETMODBASE;
		modRequest.pID = targetPID;
		CallHook(&modRequest);

		uintptr_t base = 0;
		base = reinterpret_cast<uintptr_t>(modRequest.response);
		return base;
	}

	bool Init()
	{
		// loading the library the hooked function is in and getting the address of it
		HMODULE m = LoadLibrary(xorstr_(L"win32u.dll"));
		if (!m) return false;

		hookedFunc = GetProcAddress(m, xorstr_("NtSetCompositionSurfaceStatistics"));
		currentPID = GetCurrentProcessId();

		if (!hookedFunc || !currentPID) return false;

		return true;
	}

	bool ReadRaw(uintptr_t pID, UINT_PTR readAddress, UINT_PTR targetAddress, ULONG size)
	{
		// setting up a request and calling the hook
		KERNEL_REQUEST rpmRequest;
		rpmRequest.instructionID = INST_READ;
		rpmRequest.srcPID = currentPID;
		rpmRequest.pID = pID;
		rpmRequest.srcAddr = readAddress;
		rpmRequest.targetAddr = targetAddress;
		rpmRequest.size = size;
		CallHook(&rpmRequest);
		return true;
	}

	template<class type>
	type rpm(UINT_PTR readAddress)
	{
		// use the ReadRaw function we defined earlier
		type tmp;
		if (ReadRaw(targetPID, readAddress, (UINT_PTR)&tmp, sizeof(type)))
			return tmp;
		else
			return { 0 };
	}

	template<class type>
	bool wpm(UINT_PTR writeAddress, const type& value)
	{
		// setting up a request and calling the hook
		KERNEL_REQUEST wpmRequest;
		wpmRequest.instructionID = INST_WRITE;
		wpmRequest.srcPID = currentPID;
		wpmRequest.pID = targetPID;
		wpmRequest.srcAddr = (UINT_PTR)&value;
		wpmRequest.targetAddr = writeAddress;
		wpmRequest.size = sizeof(value);
		CallHook(&wpmRequest);
		return true;
	}

	std::string ReadString(uintptr_t address)
	{
		// creating a buffer and copying memory to it
		char buf[100] = { 0 };
		ReadRaw(targetPID, address, (UINT_PTR)&buf, 100);
		return buf;
	}
};