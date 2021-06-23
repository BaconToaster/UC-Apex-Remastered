#include "Player.h"
#include "Util.h"

struct visibleTime
{
	float lastTime[100]; // last visible time in float
	bool lastState[100]; // last visibility state
	uintptr_t lastCheck[100]; // last visibility check time
} lastVis;

int pics[100];

std::vector<uintptr_t> Player::GetPlayers()
{
	std::vector<uintptr_t> vec;
	for (uintptr_t i = 0; i <= 100; i++)
	{
		uintptr_t entity = Driver.rpm<uintptr_t>(globals.entityList + (i << 5));
		if (entity == globals.localPlayer || entity == 0) continue;

		// Player check
		if (Player::IsPlayer(entity))
			vec.push_back(entity);
	}
	return vec;
}

bool Player::IsPlayer(uintptr_t player)
{
	return (Driver.rpm<uintptr_t>(player + OFFSET_NAME) == 125780153691248);
}

bool Player::IsAlive(uintptr_t player)
{
	return (Driver.rpm<int>(player + OFFSET_BLEED_OUT_STATE) == 0 && Driver.rpm<int>(player + OFFSET_LIFE_STATE) == 0);
}

bool Player::IsValidPlayer(uintptr_t player)
{
	int health = Driver.rpm<int>(player + OFFSET_HEALTH);
	int teamID = Driver.rpm<int>(player + OFFSET_TEAM);

	if (!Player::IsAlive(player)) return false;
	if (health < 0 || health > 100 || teamID < 0 || teamID > 32) return false;

	return true;
}

bool Player::IsValidEnemy(uintptr_t player)
{
	int health = Driver.rpm<int>(player + OFFSET_HEALTH);
	int teamID = Driver.rpm<int>(player + OFFSET_TEAM);

	if (health < 0 || health > 100 || teamID < 0 || teamID > 32) return false;
	if (teamID == Driver.rpm<int>(globals.localPlayer + OFFSET_TEAM)) return false;
	if (!Player::IsAlive(player)) return false;

	return true;
}

void Player::PredictPos(uintptr_t target, vec3* bonePos)
{
	uintptr_t actWeaponID = Driver.rpm<uintptr_t>(globals.localPlayer + OFFSET_CURRENT_WEAPON) & 0xFFFF;
	uintptr_t currentWeapon = Driver.rpm<uintptr_t>(globals.entityList + (actWeaponID << 5));

	if (currentWeapon != 0)
	{
		float bulletSpeed = Driver.rpm<float>(currentWeapon + OFFSET_BULLET_SPEED);
		float bulletGravity = Driver.rpm<float>(currentWeapon + OFFSET_BULLET_GRAVITY);

		if (bulletSpeed > 1.f)
		{
			vec3 muzzle = Driver.rpm<vec3>(globals.localPlayer + OFFSET_CAMERAPOS);
			float time = bonePos->DistTo(muzzle) / bulletSpeed;
			bonePos->z += (700.f * bulletGravity * 0.5f) * (time * time);
			vec3 velDelta = (Driver.rpm<vec3>(target + OFFSET_ORIGIN - 0xC) * time);
			bonePos->x += velDelta.x;
			bonePos->y += velDelta.y;
			bonePos->z += velDelta.z;
		}
	}
}

bool Player::IsVisible(uintptr_t ent, int index)
{
	if (Util::GetMs() >= (lastVis.lastCheck[index] + 10)) // only check every 10ms else its gonna flicker
	{
		float visTime = Driver.rpm<float>(ent + OFFSET_VISIBLE_TIME);
		lastVis.lastState[index] = visTime > lastVis.lastTime[index] || visTime < 0.f && lastVis.lastTime[index] > 0.f;
		lastVis.lastTime[index] = visTime;
		lastVis.lastCheck[index] = Util::GetMs();
	}
	return lastVis.lastState[index];
}

uintptr_t Player::GetBestTarget()
{
	if (Player::IsPlayer(globals.localPlayer))
	{
		float oldDist = FLT_MAX;
		float newDist = 0;
		uintptr_t aimTarget = NULL;

		int index = 0;
		for (uintptr_t& player : Player::GetPlayers())
		{
			bool visible = Player::IsVisible(player, index);
			index++;

			if (!Player::IsValidEnemy(player)) continue;

			vec3 localHead = Driver.rpm<vec3>(globals.localPlayer + OFFSET_CAMERAPOS);
			vec3 targetHead = Util::GetBonePos(player, 8);
			Player::PredictPos(aimTarget, &targetHead);
			vec2 screenMiddle = { globals.windowWH.x / 2.f, globals.windowWH.y / 2.f };
			vec2 targetHeadScreen;
			if (!Util::WorldToScreen(targetHead, targetHeadScreen)) continue;
			newDist = abs(screenMiddle.Dist2D(targetHeadScreen));

			if (newDist < oldDist && newDist <= globals.aimbotFOV && Util::ToMeters(localHead.DistTo(targetHead)) <= globals.maxAimbotDistance)
			{
				if (globals.aimbotVisibleCheck && !visible) continue;

				oldDist = newDist;
				aimTarget = player;
			}
		}
		return aimTarget;
	}
	else
		return NULL;
}

std::string Player::GetName(uintptr_t player)
{
	uintptr_t nameIndex = Driver.rpm<uintptr_t>(player + OFFSET_NAME_INDEX);
	uintptr_t nameOffset = Driver.rpm<uintptr_t>(globals.nameList + ((nameIndex - 1) << 4));
	char buf[64] = { 0 };
	Driver.ReadRaw(globals.pID, nameOffset, (UINT_PTR)&buf, 64);
	return buf;
}

int Player::GetPic(int index) // for the ksk esp, its not getting the profile picture
{
	if (pics[index] == 0)
	{
		srand(Util::GetMs() * 69);
		pics[index] = ((rand() % 2) + 1); // generate a random number between 1 and 2
	}

	return pics[index];
}