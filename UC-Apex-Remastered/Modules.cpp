#include "Modules.h"

vec3 oldPunch = { 0.f, 0.f, 0.f };
int lastKey = 0;

void Modules::Run()
{
	// run all of them every ms
	Modules::UpdateVars();
	Modules::UpdateKeys();
	Modules::NoRecoil();
	Modules::Aimbot();
}

void Modules::UpdateVars()
{
	// update important global variables every ms
	globals.localPlayer = Driver.rpm<uintptr_t>(globals.moduleBase + OFFSET_LOCAL_ENT);
	globals.viewRenderer = Driver.rpm<uintptr_t>(globals.moduleBase + OFFSET_RENDER);
}

void Modules::UpdateKeys()
{
	// i made a custom GetAsyncKeyState & 1 becuase the & 1 flag doesn't work on apex for me
	while (GetAsyncKeyState(VK_INSERT))
	{
		lastKey = VK_INSERT;
	}

	if (lastKey == VK_INSERT)
	{
		lastKey = 0;
		globals.menuActive = !globals.menuActive;
	}
}

void Modules::NoRecoil()
{
	// check if localplayer is valid and rcs is on
	if (!globals.rcs || !Player::IsPlayer(globals.localPlayer)) return;

	if (!(GetAsyncKeyState(VK_RBUTTON) && globals.aimbot && globals.currentAimTarget != NULL))
	{
		// get current angles and recoil (punch angle)
		vec3 viewAngles = Driver.rpm<vec3>(globals.localPlayer + OFFSET_VIEWANGLES);
		vec3 punchAngle = Driver.rpm<vec3>(globals.localPlayer + OFFSET_AIMPUNCH);

		// calculate the new angles by adding the punchangle to the viewangles, it is important to subtract the old punch angle
		vec3 newAngle = viewAngles + (oldPunch - punchAngle) * (globals.rcsIntensity / 100.f);

		newAngle.Normalize(); // clamp angles
		Driver.wpm<vec2>(globals.localPlayer + OFFSET_VIEWANGLES, { newAngle.x, newAngle.y }); // overwrite old angles

		oldPunch = punchAngle;
	}
}

void Modules::Aimbot()
{
	// check if localplayer is valid and aimbot is on
	if (!globals.aimbot || !Player::IsPlayer(globals.localPlayer)) return;

	globals.currentAimTarget = Player::GetBestTarget(); // find closest enemy to crosshair
	if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) // aimbot key
	{
		if (globals.currentAimTarget != NULL)
		{
			if (Player::IsAlive(globals.currentAimTarget))
			{
				// get positions and predict the enemy position
				vec3 localHead = Driver.rpm<vec3>(globals.localPlayer + OFFSET_CAMERAPOS);
				vec3 targetHead = Util::GetBonePos(globals.currentAimTarget, 8);
				Player::PredictPos(globals.currentAimTarget, &targetHead);

				// get all the angles
				vec3 oldAngle = Driver.rpm<vec3>(globals.localPlayer + OFFSET_VIEWANGLES);
				vec3 punchAngle = Driver.rpm<vec3>(globals.localPlayer + OFFSET_AIMPUNCH);
				vec3 breathAngle = Driver.rpm<vec3>(globals.localPlayer + OFFSET_BREATH_ANGLES);

				// calculate the new angles
				vec3 newAngles = Util::CalcAngle(localHead, targetHead);

				// subtracting punchangles and breath angles
				newAngles -= breathAngle * (globals.aimRcsIntensity / 100.f);
				newAngles -= (punchAngle * 0.05f) * (globals.aimRcsIntensity / 100.f);
				newAngles += oldAngle * (globals.aimRcsIntensity / 100.f);
				oldPunch = punchAngle; // do this so the rcs doesnt jump down after unlocking from the enemy

				newAngles.Normalize(); // clamp angles

				Driver.wpm<vec2>(globals.localPlayer + OFFSET_VIEWANGLES, { newAngles.x, newAngles.y }); // overwrite old angles
			}
		}
	}
}

void Modules::DrawBoxESP(float x, float y, float width, float height, int thickness, D3DCOLOR fillColor, D3DCOLOR boxColor)
{
	// self explaining
	if (globals.fillBox)
		DrawFilledRectangle(x, y, width, height, fillColor);
	DrawBorderBox(x, y, width, height, thickness, boxColor);
}

void Modules::DrawHealthbars(float x, float y, float width, float height, int thickness, int health, int shieldHealth)
{
							// shitty calculation but it works
	DrawFilledRectangle(x - (thickness + 5), y, thickness, height, ARGB(255, 0, 255, 0));
	DrawFilledRectangle(x - (thickness + 5), y, thickness, height - ((height / 100) * health), ARGB(120, 255, 0, 0));
	DrawFilledRectangle(x - (thickness * 2 + 12), y, thickness, height, ARGB(255, 155, 0, 252));
	DrawFilledRectangle(x - (thickness * 2 + 12), y, thickness, height - ((height / 125) * shieldHealth), ARGB(0, 0, 0, 0));

	if (shieldHealth > 0)
		DrawBorderBox(x - (thickness * 2 + 12) - 1, y - 1, thickness + 1, height + 1, 1, ARGB(255, 0, 0, 0));
	DrawBorderBox(x - (thickness + 5) - 1, y - 1, thickness + 1, height + 1, 1, ARGB(255, 0, 0, 0));
}