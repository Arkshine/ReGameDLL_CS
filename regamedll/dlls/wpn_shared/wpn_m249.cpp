#include "precompiled.h"

#define M249_MAX_SPEED			220

#define M249_DAMAGE			32
#define M249_RANGE_MODIFER		0.97

#define M249_RELOAD_TIME		4.7

enum m249_e
{
	M249_IDLE1,
	M249_SHOOT1,
	M249_SHOOT2,
	M249_RELOAD,
	M249_DRAW
};

/* <276bbd> ../cstrike/dlls/wpn_shared/wpn_m249.cpp:49 */
LINK_ENTITY_TO_CLASS(weapon_m249, CM249);

/* <276951> ../cstrike/dlls/wpn_shared/wpn_m249.cpp:51 */
void CM249::__MAKE_VHOOK(Spawn)(void)
{
	Precache();
	m_iId = WEAPON_M249;
	SET_MODEL(edict(), "models/w_m249.mdl");

	m_iDefaultAmmo = M249_DEFAULT_GIVE;
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;

	FallInit();
}

/* <2768d0> ../cstrike/dlls/wpn_shared/wpn_m249.cpp:65 */
void CM249::__MAKE_VHOOK(Precache)(void)
{
	PRECACHE_MODEL("models/v_m249.mdl");
	PRECACHE_MODEL("models/w_m249.mdl");

	PRECACHE_SOUND("weapons/m249-1.wav");
	PRECACHE_SOUND("weapons/m249-2.wav");
	PRECACHE_SOUND("weapons/m249_boxout.wav");
	PRECACHE_SOUND("weapons/m249_boxin.wav");
	PRECACHE_SOUND("weapons/m249_chain.wav");
	PRECACHE_SOUND("weapons/m249_coverup.wav");
	PRECACHE_SOUND("weapons/m249_coverdown.wav");

	m_iShell = PRECACHE_MODEL("models/rshell.mdl");
	m_usFireM249 = PRECACHE_EVENT(1, "events/m249.sc");
}

/* <2768f7> ../cstrike/dlls/wpn_shared/wpn_m249.cpp:84 */
int CM249::__MAKE_VHOOK(GetItemInfo)(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556NatoBox";
	p->iMaxAmmo1 = MAX_AMMO_556NATOBOX;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = M249_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 4;
	p->iId = m_iId = WEAPON_M249;
	p->iFlags = 0;
	p->iWeight = M249_WEIGHT;

	return 1;
}

/* <27692a> ../cstrike/dlls/wpn_shared/wpn_m249.cpp:101 */
BOOL CM249::__MAKE_VHOOK(Deploy)(void)
{
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;
	iShellOn = 1;

	return DefaultDeploy("models/v_m249.mdl", "models/p_m249.mdl", M249_DRAW, "m249", UseDecrement() != FALSE);
}

/* <276b73> ../cstrike/dlls/wpn_shared/wpn_m249.cpp:111 */
void CM249::__MAKE_VHOOK(PrimaryAttack)(void)
{
	if (!(m_pPlayer->pev->flags & FL_ONGROUND))
	{
		M249Fire(0.045 + (0.5 * m_flAccuracy), 0.1, FALSE);
	}
	else if (m_pPlayer->pev->velocity.Length2D() > 140)
	{
		M249Fire(0.045 + (0.095 * m_flAccuracy), 0.1, FALSE);
	}
	else
	{
		M249Fire(0.03 * m_flAccuracy, 0.1, FALSE);
	}
}

/* <276c87> ../cstrike/dlls/wpn_shared/wpn_m249.cpp:121 */
void CM249::M249Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim)
{
	Vector vecAiming, vecSrc, vecDir;
	int flag;

	m_bDelayFire = true;
	m_iShotsFired++;

	m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 175) + 0.4;

	if (m_flAccuracy > 0.9)
		m_flAccuracy = 0.9;

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
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	vecSrc = m_pPlayer->GetGunPosition();
	vecAiming = gpGlobals->v_forward;

	vecDir = m_pPlayer->FireBullets3(vecSrc, vecAiming, flSpread, 8192, 2, BULLET_PLAYER_556MM,
		M249_DAMAGE, M249_RANGE_MODIFER, m_pPlayer->pev, false, m_pPlayer->random_seed);

#ifdef CLIENT_WEAPONS
	flag = FEV_NOTHOST;
#else
	flag = 0;
#endif // CLIENT_WEAPONS

	PLAYBACK_EVENT_FULL(flag, m_pPlayer->edict(), m_usFireM249, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y,
		(int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay(flCycleTime);

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.6;

	if (!(m_pPlayer->pev->flags & FL_ONGROUND))
	{
		KickBack(1.8, 0.65, 0.45, 0.125, 5.0, 3.5, 8);
	}
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
	{
		KickBack(1.1, 0.5, 0.3, 0.06, 4.0, 3.0, 8);
	}
	else if (m_pPlayer->pev->flags & FL_DUCKING)
	{
		KickBack(0.75, 0.325, 0.25, 0.025, 3.5, 2.5, 9);
	}
	else
	{
		KickBack(0.8, 0.35, 0.3, 0.03, 3.75, 3.0, 9);
	}
}

/* <276a02> ../cstrike/dlls/wpn_shared/wpn_m249.cpp:201 */
void CM249::__MAKE_VHOOK(Reload)(void)
{
#ifdef REGAMEDLL_FIXES
	// to prevent reload if not enough ammo
	if (m_pPlayer->ammo_556natobox <= 0)
	{
		return;
	}
#endif // REGAMEDLL_FIXES

	if (DefaultReload(M249_MAX_CLIP, M249_RELOAD, M249_RELOAD_TIME))
	{
		m_pPlayer->SetAnimation(PLAYER_RELOAD);

		m_flAccuracy = 0.2;
		m_bDelayFire = false;
		m_iShotsFired = 0;
	}
}

/* <2769c7> ../cstrike/dlls/wpn_shared/wpn_m249.cpp:222 */
void CM249::__MAKE_VHOOK(WeaponIdle)(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
	{
		return;
	}

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20;
	SendWeaponAnim(M249_IDLE1, UseDecrement() != FALSE);
}

#ifdef HOOK_GAMEDLL

void CM249::Spawn(void)
{
	Spawn_();
}

void CM249::Precache(void)
{
	Precache_();
}

int CM249::GetItemInfo(ItemInfo *p)
{
	return GetItemInfo_(p);
}

BOOL CM249::Deploy(void)
{
	return Deploy_();
}

void CM249::PrimaryAttack(void)
{
	PrimaryAttack_();
}

void CM249::Reload(void)
{
	Reload_();
}

void CM249::WeaponIdle(void)
{
	WeaponIdle_();
}

#endif // HOOK_GAMEDLL
