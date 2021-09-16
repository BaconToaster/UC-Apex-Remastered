#pragma once
#define OFFSET_ENTITYLIST  0x192f108
#define OFFSET_LOCAL_ENT 0x1cdea28		//LocalPlayer

#define OFFSET_NAME_INDEX 0x38//Œ¥’“µΩ
#define OFFSET_TEAM 0x0448		//m_iTeamNum
#define OFFSET_HEALTH 0x0438	//m_iHealth 
#define OFFSET_SHIELD 0x0170	//m_shieldHealth 
#define OFFSET_NAME 0x0589		//m_iName 
#define OFFSET_VISIBLE_TIME 0x1ad4		//VisibleTime

#define OFFSET_NAMELIST 0xb73ec60		//NAMELIST
#define OFFSET_LIFE_STATE 0x798		//m_lifeState
#define OFFSET_BLEED_OUT_STATE 0x26e8		//m_bleedoutState

#define OFFSET_BULLET_SPEED 0x1ee0		//ProjectileSpeed//1ed0
#define OFFSET_BULLET_GRAVITY 0x1ee8		//ProjectileScale

#define OFFSET_CURRENT_WEAPON 0x1A6C		//m_latestPrimaryWeapons
#define OFFSET_ORIGIN 0x014c			//m_vecAbsOrigin
#define OFFSET_BONES 0xf38			//m_bConstrainBetweenEndpoints
#define OFFSET_AIMPUNCH 0x2450			//m_currentFrameLocalPlayer.m_vecPunchWeapon_Angle
#define OFFSET_CAMERAPOS 0x1ef0//camera_origin
#define OFFSET_VIEWANGLES 0x2538		//ammoPoolCapacity
#define OFFSET_BREATH_ANGLES (OFFSET_VIEWANGLES - 0x10)
#define OFFSET_MATRIX 0x1b3bd0	//ViewMatrix 0
#define OFFSET_RENDER 0x75017e0 //ViewRender

//#define OFFSET_OBSERVER_MODE		0x339c			//m_iObserverMode
//#define OFFSET_OBSERVING_TARGET		0x33a8			//m_hObserverTarget

