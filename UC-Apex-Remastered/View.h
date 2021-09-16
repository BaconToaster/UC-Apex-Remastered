#pragma once

#include "Drawings.h"
class View
{
public:
	void drawMenu();
	void drawMonitor();
	void DrawBoxESP(float x, float y, float width, float height, int thickness, D3DCOLOR fillColor, D3DCOLOR boxColor);
	void DrawHealthbars(float x, float y, float width, float height, int thickness, int health, int shieldHealth);
};

