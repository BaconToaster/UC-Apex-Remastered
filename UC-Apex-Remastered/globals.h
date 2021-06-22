#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "Driver.h"
#include "offsets.h"
#include "imgui/imgui.h"
#include "defs.h"

struct globalVars
{
	HWND hWnd;
	uintptr_t pID;
	uintptr_t moduleBase;

	uintptr_t localPlayer;
	uintptr_t entityList;
	uintptr_t nameList;
	uintptr_t currentAimTarget;
	uintptr_t viewRenderer;
	viewMatrix_t viewMatrix;

	POINT windowWH = { 0, 0 };

	ImVec4 currentRainbowColor = { 1.f, 0.f, 0.f, 1.f };
	ImVec4 fovCircleColor = { 1.f, 0.f, 0.f, 1.f };
	ImVec4 espColorVisible = { 0.f, 1.f, 0.f, 1.f };
	ImVec4 espColorInvisible = { 1.f, 0.f, 0.f, 1.f };
	ImVec4 espFillColorVisible = { 0.f, 1.f, 0.f, .039f };
	ImVec4 espFillColorInvisible = { 1.f, 0.f, 0.f, .039f };
	ImVec4 textColorVisible = { 0.f, 1.f, 0.f, 1.f };
	ImVec4 textColorInvisible = { 1.f, 0.f, 0.f, 1.f };
	ImVec4 headCircleColorVisible = { 0.f, 1.f, .1f, 1.f };
	ImVec4 headCircleColorInvisible = { 1.f, .1f, 0.f, 1.f };
	ImVec4 headLineColor = { 1.f, 0.f, 0.f, 1.f };

	float rainbowSpeed = 0.005f;

	bool rcs = true; // recoil control system toggle
	bool aimbot = false; // aimbot toggle
	bool aimbotVisibleCheck = true; // aimbot visibility check toggle
	bool esp = true; // esp toggle
	bool menuActive = true; // menu toggle
	bool drawAimbotFOV = true;
	bool rainbowFOV = true;
	bool fillBox = false;
	bool espColorVisibleRainbow = false;
	bool espColorInvisibleRainbow = false;
	bool textColorVisibleRainbow = false;
	bool textColorInvisibleRainbow = false;
	bool headCircleColorVisibleRainbow = false;
	bool headCircleColorInvisibleRainbow = false;
	bool headLineRainbow = false;

	float rcsIntensity = 100.f; // recoil control system intensity in %
	float aimRcsIntensity = 100.f; // aimbot recoil control intensity in %
	float maxAimbotDistance = 200.f; // maximum aimbot distance
	float aimbotFOV = 200.f; // aimbot FOV
	float maxBoxDistance = 1000.f;
	float maxTextDistance = 300.f;
	float maxHealthDistance = 150.f;
	float maxHeadCircleDistance = 150.f;
	float maxHeadLineDistance = 150.f;
	float maxKskDistance = 30.f;
	int espThickness = 2;
	int headLineThickness = 2;
};

extern KDriver Driver;
extern globalVars globals;