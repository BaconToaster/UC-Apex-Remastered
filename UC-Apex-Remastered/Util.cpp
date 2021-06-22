#include "Util.h"
#include <random>

std::string Util::RandomString(const int len)
{
	const std::string alpha_numeric("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890");

	std::default_random_engine generator{ std::random_device{}() };
	const std::uniform_int_distribution< std::string::size_type > distribution{ 0, alpha_numeric.size() - 1 };

	std::string str(len, 0);
	for (auto& it : str)
		it = alpha_numeric[distribution(generator)];

	return str;
}

bool Util::RenameFile(std::string& path)
{
	std::string newPath = (Util::RandomString(16) + ".exe"); // generate a random string
	SetConsoleTitleA(Util::RandomString(16).c_str()); // set the console title to another random string

	if (std::rename(path.c_str(), newPath.c_str())) return false; // rename the file and return false if it failed

	path = newPath; // set the value of the path argument to the new path

	return true;
}

uintptr_t Util::GetPid(const wchar_t* processName)
{
	PROCESSENTRY32 procEntry32;
	uintptr_t pID = 0;

	procEntry32.dwSize = sizeof(PROCESSENTRY32);

	HANDLE hProcSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (!hProcSnap || hProcSnap == INVALID_HANDLE_VALUE)
		return 0;

	while (Process32Next(hProcSnap, &procEntry32))
	{
		if (!wcscmp(processName, procEntry32.szExeFile))
		{
			pID = procEntry32.th32ProcessID;

			CloseHandle(hProcSnap);
		}
	}

	CloseHandle(hProcSnap);
	return pID;
}

uintptr_t Util::GetMs()
{
	static LARGE_INTEGER s_frequency;
	static BOOL s_use_qpc = QueryPerformanceFrequency(&s_frequency);
	if (s_use_qpc) {
		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);
		return (1000LL * now.QuadPart) / s_frequency.QuadPart;
	}
	else {
		return GetTickCount64();
	}
}

D3DCOLOR Util::Vec4toARGB(ImVec4 src)
{
	return ARGB((int)(src.w * 255), (int)(src.x * 255), (int)(src.y * 255), (int)(src.z * 255));
}

bool Util::WorldToScreen(vec3 world, vec2& screen)
{
	float* m_vMatrix = globals.viewMatrix.matrix;
	float w = m_vMatrix[12] * world.x + m_vMatrix[13] * world.y + m_vMatrix[14] * world.z + m_vMatrix[15];

	if (w < 0.01f) return false;

	screen.x = m_vMatrix[0] * world.x + m_vMatrix[1] * world.y + m_vMatrix[2] * world.z + m_vMatrix[3];
	screen.y = m_vMatrix[4] * world.x + m_vMatrix[5] * world.y + m_vMatrix[6] * world.z + m_vMatrix[7];

	float invw = 1.0f / w;
	screen.x *= invw;
	screen.y *= invw;

	float x = globals.windowWH.x / 2;
	float y = globals.windowWH.y / 2;

	x += 0.5 * screen.x * globals.windowWH.x + 0.5;
	y -= 0.5 * screen.y * globals.windowWH.y + 0.5;

	screen.x = x;
	screen.y = y;

	return true;
}

vec3 Util::CalcAngle(vec3 src, vec3 dst)
{
	vec3 origin = dst - src;
	float dist = sqrt(origin.x * origin.x + origin.y * origin.y + origin.z * origin.z);
	vec3 angles = { 0.f, 0.f, 0.f };
	angles.x = -asinf(origin.z / dist) * (180 / M_PI);
	angles.y = atan2f(origin.y, origin.x) * (180 / M_PI);
	return angles;
}

vec3 Util::GetBonePos(uintptr_t ent, int id)
{
	vec3 pos = Driver.rpm<vec3>(ent + OFFSET_ORIGIN);
	uintptr_t bones = Driver.rpm<uintptr_t>(ent + OFFSET_BONES);
	vec3 bone = {};
	UINT32 boneloc = (id * 0x30);
	bone_t bo = {};
	bo = Driver.rpm<bone_t>(bones + boneloc);

	bone.x = bo.x + pos.x;
	bone.y = bo.y + pos.y;
	bone.z = bo.z + pos.z;
	return bone;
}

float Util::ToMeters(float x)
{
	return x / 39.62f;
}