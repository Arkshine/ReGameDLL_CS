#include "precompiled.h"

#define M4A1_MAX_SPEED		230

#define M4A1_DAMAGE		32
#define M4A1_DAMAGE_SIL		33

#define M4A1_RANGE_MODIFER      0.97
#define M4A1_RANGE_MODIFER_SIL  0.95

#define M4A1_RELOAD_TIME	3.05

enum m4a1_e
{
	M4A1_IDLE,
	M4A1_SHOOT1,
	M4A1_SHOOT2,
	M4A1_SHOOT3,
	M4A1_RELOAD,
	M4A1_DRAW,
	M4A1_ATTACH_SILENCER,
	M4A1_UNSIL_IDLE,
	M4A1_UNSIL_SHOOT1,
	M4A1_UNSIL_SHOOT2,
	M4A1_UNSIL_SHOOT3,
	M4A1_UNSIL_RELOAD,
	M4A1_UNSIL_DRAW,
	M4A1_DETACH_SILENCER
};

/* <281342> ../cstrike/dlls/wpn_shared/wpn_m4a1.cpp:60 */
LINK_ENTITY_TO_CLASS(weapon_m4a1, CM4A1);

/* <281059> ../cstrike/dlls/wpn_shared/wpn_m4a1.cpp:62 */
void CM4A1::__MAKE_VHOOK(Spawn)(void)
{
	Precache();
	m_iId = WEAPON_M4A1;
	SET_MODEL(edict(), "models/w_m4a1.mdl");

	m_iDefaultAmmo = M4A1_DEFAULT_GIVE;
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;
	m_bDelayFire = true;

	FallInit();
}

/* <280fb2> ../cstrike/dlls/wpn_shared/wpn_m4a1.cpp:77 */
void CM4A1::__MAKE_VHOOK(Precache)(void)
{
	PRECACHE_MODEL("models/v_m4a1.mdl");
	PRECACHE_MODEL("models/w_m4a1.mdl");

	PRECACHE_SOUND("weapons/m4a1-1.wav");
	PRECACHE_SOUND("weapons/m4a1_unsil-1.wav");
	PRECACHE_SOUND("weapons/m4a1_unsil-2.wav");
	PRECACHE_SOUND("weapons/m4a1_clipin.wav");
	PRECACHE_SOUND("weapons/m4a1_clipout.wav");
	PRECACHE_SOUND("weapons/m4a1_boltpull.wav");
	PRECACHE_SOUND("weapons/m4a1_deploy.wav");
	PRECACHE_SOUND("weapons/m4a1_silencer_on.wav");
	PRECACHE_SOUND("weapons/m4a1_silencer_off.wav");

	m_iShell = PRECACHE_MODEL("models/rshell.mdl");
	m_usFireM4A1 = PRECACHE_EVENT(1, "events/m4a1.sc");
}

/* <280fd9> ../cstrike/dlls/wpn_shared/wpn_m4a1.cpp:97 */
int CM4A1::__MAKE_VHOOK(GetItemInfo)(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556Nato";
	p->iMaxAmmo1 = MAX_AMMO_556NATO;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = M4A1_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 6;
	p->iId = m_iId = WEAPON_M4A1;
	p->iFlags = 0;
	p->iWeight = M4A1_WEIGHT;

	return 1;
}

/* <281169> ../cstrike/dlls/wpn_shared/wpn_m4a1.cpp:114 */
BOOL CM4A1::__MAKE_VHOOK(Deploy)(void)
{
	m_bDelayFire = true;
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;

	iShellOn = 1;

	if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
		return DefaultDeploy("models/v_m4a1.mdl", "models/p_m4a1.mdl", M4A1_DRAW, "rifle", UseDecrement() != FALSE);
	else
		return DefaultDeploy("models/v_m4a1.mdl", "models/p_m4a1.mdl", M4A1_UNSIL_DRAW, "rifle", UseDecrement() != FALSE);
}

/* <281032> ../cstrike/dlls/wpn_shared/wpn_m4a1.cpp:127 */
void CM4A1::__MAKE_VHOOK(SecondaryAttack)(void)
{
	if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
	{
		m_iWeaponState &= ~WPNSTATE_M4A1_SILENCED;
		SendWeaponAnim(M4A1_DETACH_SILENCER, UseDecrement() != FALSE);
		Q_strcpy(m_pPlayer->m_szAnimExtention, "rifle");
	}
	else
	{
		m_iWeaponState |= WPNSTATE_M4A1_SILENCED;
		SendWeaponAnim(M4A1_ATTACH_SILENCER, UseDecrement() != FALSE);
		Q_strcpy(m_pPlayer->m_szAnimExtention, "rifle");
	}

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.0;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0;
	m_flNextPrimaryAttack = GetNextAttackDelay(2.0);
}

/* <2812fd> ../cstrike/dlls/wpn_shared/wpn_m4a1.cpp:149 */
void CM4A1::__MAKE_VHOOK(PrimaryAttack)(void)
{
	if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
	{
		if (!(m_pPlayer->pev->flags & FL_ONGROUND))
		{
			M4A1Fire(0.035 + (0.4 * m_flAccuracy), 0.0875, FALSE);
		}
		else if (m_pPlayer->pev->velocity.Length2D() > 140)
		{
			M4A1Fire(0.035 + (0.07 * m_flAccuracy), 0.0875, FALSE);
		}
		else
		{
			M4A1Fire(0.025 * m_flAccuracy, 0.0875, FALSE);
		}
	}
	else
	{
		if (!(m_pPlayer->pev->flags & FL_ONGROUND))
		{
			M4A1Fire(0.035 + (0.4 * m_flAccuracy), 0.0875, FALSE);
		}
		else if (m_pPlayer->pev->velocity.Length2D() > 140)
		{
			M4A1Fire(0.035 + (0.07 * m_flAccuracy), 0.0875, FALSE);
		}
		else
		{
			M4A1Fire(0.02 * m_flAccuracy, 0.0875, FALSE);
		}
	}
}

/* <28140c> ../cstrike/dlls/wpn_shared/wpn_m4a1.cpp:173 */
void CM4A1::M4A1Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim)
{
	Vector vecAiming, vecSrc, vecDir;
	int flag;

	m_bDelayFire = true;
	m_iShotsFired++;

	m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 220) + 0.3;

	if (m_flAccuracy > 1)
		m_flAccuracy = 1;

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

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	vecSrc = m_pPlayer->GetGunPosition();
	vecAiming = gpGlobals->v_forward;

	if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
	{
		vecDir = m_pPlayer->FireBullets3(vecSrc, vecAiming, flSpread, 8192, 2, BULLET_PLAYER_556MM,
			M4A1_DAMAGE_SIL, M4A1_RANGE_MODIFER_SIL, m_pPlayer->pev, false, m_pPlayer->random_seed);
	}
	else
	{
		vecDir = m_pPlayer->FireBullets3(vecSrc, vecAiming, flSpread, 8192, 2, BULLET_PLAYER_556MM,
			M4A1_DAMAGE, M4A1_RANGE_MODIFER, m_pPlayer->pev, false, m_pPlayer->random_seed);

		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
	}

#ifdef CLIENT_WEAPONS
	flag = FEV_NOTHOST;
#else
	flag = 0;
#endif // CLIENT_WEAPONS

	m_pPlayer->ammo_556nato--;

	PLAYBACK_EVENT_FULL(flag, m_pPlayer->edict(), m_usFireM4A1, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y,
		(int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), (m_iWeaponState & WPNSTATE_M4A1_SILENCED) == WPNSTATE_M4A1_SILENCED, FALSE);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay(flCycleTime);

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;

	if (m_pPlayer->pev->velocity.Length2D() > 0)
	{
		KickBack(1.0, 0.45, 0.28, 0.045, 3.75, 3.0, 7);
	}
	else if (!(m_pPlayer->pev->flags & FL_ONGROUND))
	{
		KickBack(1.2, 0.5, 0.23, 0.15, 5.5, 3.5, 6);
	}
	else if (m_pPlayer->pev->flags & FL_DUCKING)
	{
		KickBack(0.6, 0.3, 0.2, 0.0125, 3.25, 2.0, 7);
	}
	else
	{
		KickBack(0.65, 0.35, 0.25, 0.015, 3.5, 2.25, 7);
	}
}

/* <28110b> ../cstrike/dlls/wpn_shared/wpn_m4a1.cpp:257 */
void CM4A1::__MAKE_VHOOK(Reload)(void)
{
	int iResult;
	if (m_pPlayer->ammo_556nato <= 0)
		return;

	if ((m_iWeaponState & WPNSTATE_M4A1_SILENCED) == WPNSTATE_M4A1_SILENCED)
	{
		iResult = DefaultReload(M4A1_MAX_CLIP, M4A1_RELOAD, M4A1_RELOAD_TIME);
	}
	else
		iResult = DefaultReload(M4A1_MAX_CLIP, M4A1_UNSIL_RELOAD, M4A1_RELOAD_TIME);

	if (iResult)
	{
		m_pPlayer->SetAnimation(PLAYER_RELOAD);

		m_flAccuracy = 0.2;
		m_iShotsFired = 0;
		m_bDelayFire = false;
	}
}

/* <2810cf> ../cstrike/dlls/wpn_shared/wpn_m4a1.cpp:279 */
void CM4A1::__MAKE_VHOOK(WeaponIdle)(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
	{
		return;
	}

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0;
	SendWeaponAnim((m_iWeaponState & WPNSTATE_M4A1_SILENCED) == WPNSTATE_M4A1_SILENCED ? M4A1_IDLE : M4A1_UNSIL_IDLE, UseDecrement() != FALSE);
}

/* <28100c> ../cstrike/dlls/wpn_shared/wpn_m4a1.cpp:296 */
float CM4A1::__MAKE_VHOOK(GetMaxSpeed)(void)
{
	return M4A1_MAX_SPEED;
}

#ifdef HOOK_GAMEDLL

void CM4A1::Spawn(void)
{
	Spawn_();
}

void CM4A1::Precache(void)
{
	Precache_();
}

int CM4A1::GetItemInfo(ItemInfo *p)
{
	return GetItemInfo_(p);
}

BOOL CM4A1::Deploy(void)
{
	return Deploy_();
}

float CM4A1::GetMaxSpeed(void)
{
	return GetMaxSpeed_();
}

void CM4A1::PrimaryAttack(void)
{
	PrimaryAttack_();
}

void CM4A1::SecondaryAttack(void)
{
	SecondaryAttack_();
}

void CM4A1::Reload(void)
{
	Reload_();
}

void CM4A1::WeaponIdle(void)
{
	WeaponIdle_();
}

#endif // HOOK_GAMEDLL
