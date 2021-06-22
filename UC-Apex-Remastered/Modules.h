#pragma once
#include "offsets.h"
#include "globals.h"
#include "Util.h"
#include "Drawings.h"
#include "Player.h"

namespace Modules
{
	void Run();
	void UpdateVars();
	void UpdateKeys();
	void NoRecoil();
	void Aimbot();
	void DrawBoxESP(float x, float y, float width, float height, int thickness, D3DCOLOR fillColor, D3DCOLOR boxColor);
	void DrawHealthbars(float x, float y, float width, float height, int thickness, int health, int shieldHealth);
}