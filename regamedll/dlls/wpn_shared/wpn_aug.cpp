#include "precompiled.h"

#define AUG_MAX_SPEED			240

#define AUG_DAMAGE			32
#define AUG_RANGE_MODIFER		0.96

#define AUG_RELOAD_TIME			3.3

enum aug_e
{
	AUG_IDLE1,
	AUG_RELOAD,
	AUG_DRAW,
	AUG_SHOOT1,
	AUG_SHOOT2,
	AUG_SHOOT3
};

/* <23a81f> ../cstrike/dlls/wpn_shared/wpn_aug.cpp:50 */
LINK_ENTITY_TO_CLASS(weapon_aug, CAUG);

/* <23a711> ../cstrike/dlls/wpn_shared/wpn_aug.cpp:52 */
void CAUG::__MAKE_VHOOK(Spawn)(void)
{
	Precache();
	m_iId = WEAPON_AUG;
	SET_MODEL(edict(), "models/w_aug.mdl");

	m_iDefaultAmmo = AUG_DEFAULT_GIVE;
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;

	FallInit();
}

/* <23a66a> ../cstrike/dlls/wpn_shared/wpn_aug.cpp:66 */
void CAUG::__MAKE_VHOOK(Precache)(void)
{
	PRECACHE_MODEL("models/v_aug.mdl");
	PRECACHE_MODEL("models/w_aug.mdl");

	PRECACHE_SOUND("weapons/aug-1.wav");
	PRECACHE_SOUND("weapons/aug_clipout.wav");
	PRECACHE_SOUND("weapons/aug_clipin.wav");
	PRECACHE_SOUND("weapons/aug_boltpull.wav");
	PRECACHE_SOUND("weapons/aug_boltslap.wav");
	PRECACHE_SOUND("weapons/aug_forearm.wav");

	m_iShell = PRECACHE_MODEL("models/rshell.mdl");
	m_usFireAug = PRECACHE_EVENT(1, "events/aug.sc");
}

/* <23a691> ../cstrike/dlls/wpn_shared/wpn_aug.cpp:83 */
int CAUG::__MAKE_VHOOK(GetItemInfo)(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556Nato";
	p->iMaxAmmo1 = MAX_AMMO_556NATO;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = AUG_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 14;
	p->iId = m_iId = WEAPON_AUG;
	p->iFlags = 0;
	p->iWeight = AUG_WEIGHT;

	return 1;
}

/* <23a6ea> ../cstrike/dlls/wpn_shared/wpn_aug.cpp:100 */
BOOL CAUG::__MAKE_VHOOK(Deploy)(void)
{
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;
	iShellOn = 1;

	return DefaultDeploy("models/v_aug.mdl", "models/p_aug.mdl", AUG_DRAW, "carbine", UseDecrement() != FALSE);
}

/* <23a6c4> ../cstrike/dlls/wpn_shared/wpn_aug.cpp:109 */
void CAUG::__MAKE_VHOOK(SecondaryAttack)(void)
{
	if (m_pPlayer->m_iFOV == DEFAULT_FOV)
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 55;
	else
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3;
}

/* <23aa31> ../cstrike/dlls/wpn_shared/wpn_aug.cpp:130 */
void CAUG::__MAKE_VHOOK(PrimaryAttack)(void)
{
	if (!(m_pPlayer->pev->flags & FL_ONGROUND))
	{
		AUGFire(0.035 + (0.4 * m_flAccuracy), 0.0825, FALSE);
	}
	else if (m_pPlayer->pev->velocity.Length2D() > 140)
	{
		AUGFire(0.035 + (0.07 * m_flAccuracy), 0.0825, FALSE);
	}
	else if (m_pPlayer->pev->fov == DEFAULT_FOV)
	{
		AUGFire(0.02 * m_flAccuracy, 0.0825, FALSE);
	}
	else
	{
		AUGFire(0.02 * m_flAccuracy, 0.135, FALSE);
	}
}

/* <23a8e9> ../cstrike/dlls/wpn_shared/wpn_aug.cpp:143 */
void CAUG::AUGFire(float flSpread, float flCycleTime, BOOL fUseAutoAim)
{
	Vector vecAiming, vecSrc, vecDir;
	int flag;

	m_bDelayFire = true;
	m_iShotsFired++;

	m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 215) + 0.3;

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
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	vecSrc = m_pPlayer->GetGunPosition();
	vecAiming = gpGlobals->v_forward;

	vecDir = m_pPlayer->FireBullets3(vecSrc, vecAiming, flSpread, 8192, 2, BULLET_PLAYER_556MM,
		AUG_DAMAGE, AUG_RANGE_MODIFER, m_pPlayer->pev, false, m_pPlayer->random_seed);

#ifdef CLIENT_WEAPONS
	flag = FEV_NOTHOST;
#else
	flag = 0;
#endif // CLIENT_WEAPONS

	PLAYBACK_EVENT_FULL(flag, m_pPlayer->edict(), m_usFireAug, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y,
		(int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay(flCycleTime);

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.9;

	if (m_pPlayer->pev->velocity.Length2D() > 0)
	{
		KickBack(1.0, 0.45, 0.275, 0.05, 4.0, 2.5, 7);
	}
	else if (!(m_pPlayer->pev->flags & FL_ONGROUND))
	{
		KickBack(1.25, 0.45, 0.22, 0.18, 5.5, 4.0, 5);
	}
	else if (m_pPlayer->pev->flags & FL_DUCKING)
	{
		KickBack(0.575, 0.325, 0.2, 0.011, 3.25, 2.0, 8);
	}
	else
	{
		KickBack(0.625, 0.375, 0.25, 0.0125, 3.5, 2.25, 8);
	}
}

/* <23a7c2> ../cstrike/dlls/wpn_shared/wpn_aug.cpp:225 */
void CAUG::__MAKE_VHOOK(Reload)(void)
{
	if (m_pPlayer->ammo_556nato <= 0)
	{
		return;
	}

	if (DefaultReload(AUG_MAX_CLIP, AUG_RELOAD, AUG_RELOAD_TIME))
	{
		m_pPlayer->SetAnimation(PLAYER_RELOAD);

		if (m_pPlayer->m_iFOV != DEFAULT_FOV)
		{
			SecondaryAttack();
		}

		m_flAccuracy = 0;
		m_iShotsFired = 0;
		m_bDelayFire = false;
	}
}

/* <23a787> ../cstrike/dlls/wpn_shared/wpn_aug.cpp:245 */
void CAUG::__MAKE_VHOOK(WeaponIdle)(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
	{
		return;
	}

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20;
	SendWeaponAnim(AUG_IDLE1, UseDecrement() != FALSE);
}

#ifdef HOOK_GAMEDLL

void CAUG::Spawn(void)
{
	Spawn_();
}

void CAUG::Precache(void)
{
	Precache_();
}

int CAUG::GetItemInfo(ItemInfo *p)
{
	return GetItemInfo_(p);
}

BOOL CAUG::Deploy(void)
{
	return Deploy_();
}

void CAUG::PrimaryAttack(void)
{
	PrimaryAttack_();
}

void CAUG::SecondaryAttack(void)
{
	SecondaryAttack_();
}

void CAUG::Reload(void)
{
	Reload_();
}

void CAUG::WeaponIdle(void)
{
	WeaponIdle_();
}

#endif // HOOK_GAMEDLL
