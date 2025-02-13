#include "precompiled.h"

#define SCOUT_MAX_SPEED			260
#define SCOUT_MAX_SPEED_ZOOM		220

#define SCOUT_DAMAGE			75
#define SCOUT_RANGE_MODIFER		0.98

#define SCOUT_RELOAD_TIME		2

enum scout_e
{
	SCOUT_IDLE,
	SCOUT_SHOOT,
	SCOUT_SHOOT2,
	SCOUT_RELOAD,
	SCOUT_DRAW
};

/* <29ba7b> ../cstrike/dlls/wpn_shared/wpn_scout.cpp:48 */
LINK_ENTITY_TO_CLASS(weapon_scout, CSCOUT);

/* <29b7ee> ../cstrike/dlls/wpn_shared/wpn_scout.cpp:50 */
void CSCOUT::__MAKE_VHOOK(Spawn)(void)
{
	Precache();
	m_iId = WEAPON_SCOUT;
	SET_MODEL(edict(), "models/w_scout.mdl");

	m_iDefaultAmmo = SCOUT_DEFAULT_GIVE;

	FallInit();
}

/* <29b70a> ../cstrike/dlls/wpn_shared/wpn_scout.cpp:62 */
void CSCOUT::__MAKE_VHOOK(Precache)(void)
{
	PRECACHE_MODEL("models/v_scout.mdl");
	PRECACHE_MODEL("models/w_scout.mdl");

	PRECACHE_SOUND("weapons/scout_fire-1.wav");
	PRECACHE_SOUND("weapons/scout_bolt.wav");
	PRECACHE_SOUND("weapons/scout_clipin.wav");
	PRECACHE_SOUND("weapons/scout_clipout.wav");
	PRECACHE_SOUND("weapons/zoom.wav");

	m_iShellId = m_iShell = PRECACHE_MODEL("models/rshell_big.mdl");
	m_usFireScout = PRECACHE_EVENT(1, "events/scout.sc");
}

/* <29b731> ../cstrike/dlls/wpn_shared/wpn_scout.cpp:79 */
int CSCOUT::__MAKE_VHOOK(GetItemInfo)(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "762Nato";
	p->iMaxAmmo1 = MAX_AMMO_762NATO;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = SCOUT_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 9;
	p->iId = m_iId = WEAPON_SCOUT;
	p->iFlags = 0;
	p->iWeight = SCOUT_WEIGHT;

	return 1;
}

/* <29b8f7> ../cstrike/dlls/wpn_shared/wpn_scout.cpp:96 */
BOOL CSCOUT::__MAKE_VHOOK(Deploy)(void)
{
	if (DefaultDeploy("models/v_scout.mdl", "models/p_scout.mdl", SCOUT_DRAW, "rifle", UseDecrement() != FALSE))
	{
		m_flNextPrimaryAttack = m_pPlayer->m_flNextAttack = GetNextAttackDelay(1.25);
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1;

		return TRUE;
	}

	return FALSE;
}

/* <29b78a> ../cstrike/dlls/wpn_shared/wpn_scout.cpp:109 */
void CSCOUT::__MAKE_VHOOK(SecondaryAttack)(void)
{
	switch (m_pPlayer->m_iFOV)
	{
	case 90: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 40; break;
	case 40: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 15; break;
#ifdef REGAMEDLL_FIXES
	default:
#else
	case 15:
#endif // REGAMEDLL_FIXES
		m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90; break;
	}

	if (TheBots != NULL)
	{
		TheBots->OnEvent(EVENT_WEAPON_ZOOMED, m_pPlayer);
	}

	m_pPlayer->ResetMaxSpeed();
	EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/zoom.wav", 0.2, 2.4);
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3;
}

/* <29ba31> ../cstrike/dlls/wpn_shared/wpn_scout.cpp:140 */
void CSCOUT::__MAKE_VHOOK(PrimaryAttack)(void)
{
	if (!(m_pPlayer->pev->flags & FL_ONGROUND))
	{
		SCOUTFire(0.2, 1.25, FALSE);
	}
	else if (m_pPlayer->pev->velocity.Length2D() > 170)
	{
		SCOUTFire(0.075, 1.25, FALSE);
	}
	else if (m_pPlayer->pev->flags & FL_DUCKING)
	{
		SCOUTFire(0, 1.25, FALSE);
	}
	else
	{
		SCOUTFire(0.007, 1.25, FALSE);
	}
}

/* <29bb45> ../cstrike/dlls/wpn_shared/wpn_scout.cpp:154 */
void CSCOUT::SCOUTFire(float flSpread, float flCycleTime, BOOL fUseAutoAim)
{
	Vector vecAiming, vecSrc, vecDir;
	int flag;

	if (m_pPlayer->pev->fov != DEFAULT_FOV)
	{
		m_pPlayer->m_bResumeZoom = true;
		m_pPlayer->m_iLastZoom = m_pPlayer->m_iFOV;

		// reset a fov
		m_pPlayer->m_iFOV = DEFAULT_FOV;
#ifdef REGAMEDLL_FIXES
		m_pPlayer->pev->fov = DEFAULT_FOV;
#endif // REGAMEDLL_FIXES
	}
	else
	{
		flSpread += 0.025;
	}

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = GetNextAttackDelay(0.2);
		}

		if (TheBots != NULL)
		{
			TheBots->OnEvent(EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer);
		}

		return;
	}

	m_iClip--;
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	m_pPlayer->m_flEjectBrass = gpGlobals->time + 0.56;
	m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	vecSrc = m_pPlayer->GetGunPosition();
	vecAiming = gpGlobals->v_forward;

	vecDir = m_pPlayer->FireBullets3(vecSrc, vecAiming, flSpread, 8192, 3, BULLET_PLAYER_762MM, SCOUT_DAMAGE, SCOUT_RANGE_MODIFER, m_pPlayer->pev, true, m_pPlayer->random_seed);

#ifdef CLIENT_WEAPONS
	flag = FEV_NOTHOST;
#else
	flag = 0;
#endif // CLIENT_WEAPONS

	PLAYBACK_EVENT_FULL(flag, m_pPlayer->edict(), m_usFireScout, 0, (float *)&g_vecZero, (float *)&m_pPlayer->pev->angles, (vecDir.x * 1000), (vecDir.y * 1000),
		(int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.x * 100), FALSE, FALSE);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay(flCycleTime);

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.8;
	m_pPlayer->pev->punchangle.x -= 2;
}

/* <29b89e> ../cstrike/dlls/wpn_shared/wpn_scout.cpp:232 */
void CSCOUT::__MAKE_VHOOK(Reload)(void)
{
#ifdef REGAMEDLL_FIXES
	// to prevent reload if not enough ammo
	if (m_pPlayer->ammo_762nato <= 0)
	{
		return;
	}
#endif // REGAMEDLL_FIXES

	if (DefaultReload(SCOUT_MAX_CLIP, SCOUT_RELOAD, SCOUT_RELOAD_TIME))
	{
		if (m_pPlayer->pev->fov != DEFAULT_FOV)
		{
			m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 15;
			SecondaryAttack();
		}

		m_pPlayer->SetAnimation(PLAYER_RELOAD);
	}
}

/* <29b864> ../cstrike/dlls/wpn_shared/wpn_scout.cpp:250 */
void CSCOUT::__MAKE_VHOOK(WeaponIdle)(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
	{
		return;
	}

	if (m_iClip)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60;
		SendWeaponAnim(SCOUT_IDLE, UseDecrement() != FALSE);
	}
}

/* <29b764> ../cstrike/dlls/wpn_shared/wpn_scout.cpp:267 */
float CSCOUT::__MAKE_VHOOK(GetMaxSpeed)(void)
{
	return (m_pPlayer->m_iFOV == DEFAULT_FOV) ? SCOUT_MAX_SPEED : SCOUT_MAX_SPEED_ZOOM;
}

#ifdef HOOK_GAMEDLL

void CSCOUT::Spawn(void)
{
	Spawn_();
}

void CSCOUT::Precache(void)
{
	Precache_();
}

int CSCOUT::GetItemInfo(ItemInfo *p)
{
	return GetItemInfo_(p);
}

BOOL CSCOUT::Deploy(void)
{
	return Deploy_();
}

float CSCOUT::GetMaxSpeed(void)
{
	return GetMaxSpeed_();
}

void CSCOUT::PrimaryAttack(void)
{
	PrimaryAttack_();
}

void CSCOUT::SecondaryAttack(void)
{
	SecondaryAttack_();
}

void CSCOUT::Reload(void)
{
	Reload_();
}

void CSCOUT::WeaponIdle(void)
{
	WeaponIdle_();
}

#endif // HOOK_GAMEDLL
