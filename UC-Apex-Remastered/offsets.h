#pragma once
#define OFFSET_ENTITYLIST 0x1923578    //changed
#define OFFSET_LOCAL_ENT 0x1cd2df8		//LocalPlayer //changed

#define OFFSET_NAME_INDEX 0x38
#define OFFSET_TEAM 0x0448		//m_iTeamNum
#define OFFSET_HEALTH 0x0438	//m_iHealth 
#define OFFSET_SHIELD 0x0170	//m_shieldHealth 
#define OFFSET_NAME 0x0589		//m_iName 
#define OFFSET_VISIBLE_TIME 0x1ACC		//VisibleTime

#define OFFSET_NAMELIST 0x84067a0		//NAMELIST //changed
#define OFFSET_LIFE_STATE 0x798		//m_lifeState
#define OFFSET_BLEED_OUT_STATE 0x26d8		//m_bleedoutState //changed

#define OFFSET_BULLET_SPEED 0x1ED0		//ProjectileSpeed
#define OFFSET_BULLET_GRAVITY 0x1ED8		//ProjectileScale

#define OFFSET_CURRENT_WEAPON 0x1A6C		//m_latestPrimaryWeapons
#define OFFSET_ORIGIN 0x014c			//m_vecAbsOrigin
#define OFFSET_BONES 0xf38			//m_bConstrainBetweenEndpoints
#define OFFSET_AIMPUNCH 0x2440			//m_currentFrameLocalPlayer.m_vecPunchWeapon_Angle //changed
#define OFFSET_CAMERAPOS 0x1EE0//camera_origin //changed
#define OFFSET_VIEWANGLES 0x253c  -  0x0014		//ammoPoolCapacity //changed
#define OFFSET_BREATH_ANGLES (OFFSET_VIEWANGLES - 0x10)
#define OFFSET_MATRIX 0x1b3bd0	//ViewMatrix 0
#define OFFSET_RENDER 0x41f5e98 //ViewRender //changed