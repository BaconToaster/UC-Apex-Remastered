#include "Modules.h"
#include "Util.h"
#include <random>

vec3 oldPunch = { 0.f, 0.f, 0.f };
int lastKey = 0;

int targetBone[5] = {8,7,5,3,2};//rand boneId 随机选择瞄准位置
std::random_device randomEngine;

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

	//强行瞄准开关
	while (GetAsyncKeyState(VK_LEFT))
	{
		lastKey = (VK_LEFT);
	}

	if (lastKey == (VK_LEFT))
	{
		lastKey = 0;
		globals.forceAim = !globals.forceAim;
	}

	//加延迟
	while (GetAsyncKeyState(VK_PRIOR))
	{
		lastKey = VK_PRIOR;
	}

	if (lastKey == VK_PRIOR)
	{
		lastKey = 0;
		globals.delayTime += 5;
	}
	//减延迟
	while (GetAsyncKeyState(VK_NEXT))
	{
		lastKey = VK_NEXT;
	}

	if (lastKey == VK_NEXT)
	{
		lastKey = 0;
		if (globals.delayTime >= 5)
			globals.delayTime -= 5;
	}

	//加压枪强度
	while (GetAsyncKeyState(VK_UP))
	{
		lastKey = VK_UP;
	}

	if (lastKey == VK_UP)
	{
		lastKey = 0;
		if (globals.smoothAimIntensity <= 95)
			globals.smoothAimIntensity += 5;
	}
	//减压枪强度
	while (GetAsyncKeyState(VK_DOWN))
	{
		lastKey = VK_DOWN;
	}

	if (lastKey == VK_DOWN)
	{
		lastKey = 0;
		if (globals.smoothAimIntensity >= 6)
			globals.smoothAimIntensity -= 5;
	}
}

void Modules::NoRecoil()//无后座力
{
	// check if localplayer is valid and rcs is on
	if (!globals.rcs || !Player::IsPlayer(globals.localPlayer)) return;

	if (!(GetAsyncKeyState(VK_RBUTTON) && globals.aimbot && globals.currentAimTarget != NULL))
	{
		// get current angles and recoil (punch angle)
		vec3 viewAngles = Driver.rpm<vec3>(globals.localPlayer + OFFSET_VIEWANGLES);
		vec3 punchAngle = Driver.rpm<vec3>(globals.localPlayer + OFFSET_AIMPUNCH);//冲击角度

		// calculate the new angles by adding the punchangle to the viewangles, it is important to subtract the old punch angle
		vec3 newAngle = viewAngles + (oldPunch - punchAngle) * (globals.rcsIntensity / 100.f);

		newAngle.Normalize(); // clamp angles
		Driver.wpm<vec2>(globals.localPlayer + OFFSET_VIEWANGLES, { newAngle.x, newAngle.y }); // overwrite old angles
		oldPunch = punchAngle;

		Util::Delay(globals.delayTime);
	}
}

void Modules::Aimbot()
{
	// check if localplayer is valid and aimbot is on
	if (!globals.aimbot || !Player::IsPlayer(globals.localPlayer)) return;

	globals.currentAimTarget = Player::GetBestTarget(); // find closest enemy to crosshair 找到最近的敌人
	if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) // aimbot key
	{
		if (globals.currentAimTarget != NULL)
		{
			if (Player::IsAlive(globals.currentAimTarget))
			{
				if (globals.smoothMode)
				{
					// 获取位置并预测敌人位置
					vec3 localHead = Driver.rpm<vec3>(globals.localPlayer + OFFSET_CAMERAPOS);
					//rand boneId 随机选择瞄准位置
					vec3 targetHead = Util::GetBonePos(globals.currentAimTarget, targetBone[randomEngine() % 5]);//8是头，5是胸。8 is head,5 is chest 
					if (globals.forceAim == false)//预判
					{
						Player::PredictPos(globals.currentAimTarget, &targetHead);
					}

					// get all the angles
					// 得到所有的角度
					vec3 oldAngle = Driver.rpm<vec3>(globals.localPlayer + OFFSET_VIEWANGLES);
					vec3 punchAngle = Driver.rpm<vec3>(globals.localPlayer + OFFSET_AIMPUNCH);
					vec3 breathAngle = Driver.rpm<vec3>(globals.localPlayer + OFFSET_BREATH_ANGLES);

					// calculate the new angles
					// 计算新的角度
					vec3 newAngles = Util::CalcAngle(localHead, targetHead);

					vec3 clampAngles = newAngles - oldAngle;//夹角
					clampAngles.Normalize();//夹角
					//float clampDistance = clampAngles.Length2D();//夹角两点间2d距离

					newAngles = oldAngle + (clampAngles * (globals.smoothAimIntensity / 100.f));





					newAngles.Normalize(); // clamp angles 获取夹角

					Driver.wpm<vec2>(globals.localPlayer + OFFSET_VIEWANGLES, { newAngles.x, newAngles.y }); // overwrite old angles
					Util::Delay(globals.delayTime);//使每次自瞄位置有间隔，防止变成激光枪
				}
				else
				{
					// get positions and predict the enemy position
				// 获取位置并预测敌人位置

					vec3 localHead = Driver.rpm<vec3>(globals.localPlayer + OFFSET_CAMERAPOS);
					//rand boneId 随机选择瞄准位置
					vec3 targetHead = Util::GetBonePos(globals.currentAimTarget, targetBone[randomEngine() % 5]);//8是头，5是胸。8 is head,5 is chest 
					if (globals.forceAim == false)//预判
					{
						Player::PredictPos(globals.currentAimTarget, &targetHead);
					}


					// get all the angles
					// 得到所有的角度
					vec3 oldAngle = Driver.rpm<vec3>(globals.localPlayer + OFFSET_VIEWANGLES);
					vec3 punchAngle = Driver.rpm<vec3>(globals.localPlayer + OFFSET_AIMPUNCH);
					vec3 breathAngle = Driver.rpm<vec3>(globals.localPlayer + OFFSET_BREATH_ANGLES);





					// calculate the new angles
					// 计算新的角度
					vec3 newAngles = Util::CalcAngle(localHead, targetHead);







					// subtracting punchangles and breath angles
					// 减去出拳角度和呼吸角度
					//每次修正角度时乘一个百分值用来调整自瞄程度
					newAngles -= breathAngle * (globals.aimRcsIntensity / 100.f);
					newAngles -= (punchAngle * 0.05f) * (globals.aimRcsIntensity / 100.f);
					newAngles += oldAngle * (globals.aimRcsIntensity / 100.f);
					oldPunch = punchAngle; // do this so the rcs doesnt jump down after unlocking from the enemy 这样做是为了让 rcs 在从敌人身上解锁后不会跳下来

					newAngles.Normalize(); // clamp angles 夹角

					Driver.wpm<vec2>(globals.localPlayer + OFFSET_VIEWANGLES, { newAngles.x, newAngles.y }); // overwrite old angles
					Util::Delay(globals.delayTime);//使每次自瞄位置有间隔，防止变成激光枪
				}
			}
		}
	}
}
