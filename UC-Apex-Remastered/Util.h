#pragma once
#include "globals.h"
#include <TlHelp32.h>
#include "defs.h"
#include <string>

#define ARGB(a, r, g, b) D3DCOLOR_ARGB(a, r, g, b)

namespace Util
{
	std::string RandomString(const int len);
	bool RenameFile(std::string& path);
	uintptr_t GetPid(const wchar_t* processName);
	uintptr_t GetMs();
	D3DCOLOR Vec4toARGB(ImVec4 src);

	vec3 CalcAngle(vec3 src, vec3 dst);
	bool WorldToScreen(vec3 world, vec2& screen);
	vec3 GetBonePos(uintptr_t ent, int id);
	float ToMeters(float x);
}