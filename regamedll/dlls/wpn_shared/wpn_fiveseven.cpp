#include "precompiled.h"

#define FIVESEVEN_MAX_SPEED		250

#define FIVESEVEN_DAMAGE		20
#define FIVESEVEN_RANGE_MODIFER		0.885

#define FIVESEVEN_RELOAD_TIME		2.7

enum fiveseven_e
{
	FIVESEVEN_IDLE,
	FIVESEVEN_SHOOT1,
	FIVESEVEN_SHOOT2,
	FIVESEVEN_SHOOT_EMPTY,
	FIVESEVEN_RELOAD,
	FIVESEVEN_DRAW
};

/* <25698d> ../cstrike/dlls/wpn_shared/wpn_fiveseven.cpp:49 */
LINK_ENTITY_TO_CLASS(weapon_fiveseven, CFiveSeven);

/* <256720> ../cstrike/dlls/wpn_shared/wpn_fiveseven.cpp:51 */
void CFiveSeven::__MAKE_VHOOK(Spawn)(void)
{
	Precache();
	m_iId = WEAPON_FIVESEVEN;
	SET_MODEL(edict(), "models/w_fiveseven.mdl");

	m_iDefaultAmmo = FIVESEVEN_DEFAULT_GIVE;
	m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
	m_flAccuracy = 0.92;

	FallInit();
}

/* <25669f> ../cstrike/dlls/wpn_shared/wpn_fiveseven.cpp:66 */
void CFiveSeven::__MAKE_VHOOK(Precache)(void)
{
	PRECACHE_MODEL("models/v_fiveseven.mdl");
	PRECACHE_MODEL("models/w_fiveseven.mdl");
	PRECACHE_MODEL("models/shield/v_shield_fiveseven.mdl");

	PRECACHE_SOUND("weapons/fiveseven-1.wav");
	PRECACHE_SOUND("weapons/fiveseven_clipout.wav");
	PRECACHE_SOUND("weapons/fiveseven_clipin.wav");
	PRECACHE_SOUND("weapons/fiveseven_sliderelease.wav");
	PRECACHE_SOUND("weapons/fiveseven_slidepull.wav");

	m_iShell = PRECACHE_MODEL("models/pshell.mdl");
	m_usFireFiveSeven = PRECACHE_EVENT(1, "events/fiveseven.sc");
}

/* <2566c6> ../cstrike/dlls/wpn_shared/wpn_fiveseven.cpp:84 */
int CFiveSeven::__MAKE_VHOOK(GetItemInfo)(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "57mm";
	p->iMaxAmmo1 = MAX_AMMO_57MM;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = FIVESEVEN_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 6;
	p->iId = m_iId = WEAPON_FIVESEVEN;
	p->iFlags = 0;
	p->iWeight = FIVESEVEN_WEIGHT;

	return 1;
}

/* <25680a> ../cstrike/dlls/wpn_shared/wpn_fiveseven.cpp:101 */
BOOL CFiveSeven::__MAKE_VHOOK(Deploy)(void)
{
	m_flAccuracy = 0.92;
	m_fMaxSpeed = FIVESEVEN_MAX_SPEED;
	m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
	m_pPlayer->m_bShieldDrawn = false;

	if (m_pPlayer->HasShield())
		return DefaultDeploy("models/shield/v_shield_fiveseven.mdl", "models/shield/p_shield_fiveseven.mdl", FIVESEVEN_DRAW, "shieldgun", UseDecrement() != FALSE);
	else
		return DefaultDeploy("models/v_fiveseven.mdl", "models/p_fiveseven.mdl", FIVESEVEN_DRAW, "onehanded", UseDecrement() != FALSE);
}

/* <256943> ../cstrike/dlls/wpn_shared/wpn_fiveseven.cpp:117 */
void CFiveSeven::__MAKE_VHOOK(PrimaryAttack)(void)
{
	if (!(m_pPlayer->pev->flags & FL_ONGROUND))
	{
		FiveSevenFire(1.5 * (1 - m_flAccuracy), 0.2, FALSE);
	}
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
	{
		FiveSevenFire(0.255 * (1 - m_flAccuracy), 0.2, FALSE);
	}
	else if (m_pPlayer->pev->flags & FL_DUCKING)
	{
		FiveSevenFire(0.075 * (1 - m_flAccuracy), 0.2, FALSE);
	}
	else
	{
		FiveSevenFire(0.15 * (1 - m_flAccuracy), 0.2, FALSE);
	}
}

/* <2566f9> ../cstrike/dlls/wpn_shared/wpn_fiveseven.cpp:129 */
void CFiveSeven::__MAKE_VHOOK(SecondaryAttack)(void)
{
	ShieldSecondaryFire(SHIELDGUN_UP, SHIELDGUN_DOWN);
}

/* <256a57> ../cstrike/dlls/wpn_shared/wpn_fiveseven.cpp:134 */
void CFiveSeven::FiveSevenFire(float flSpread, float flCycleTime, BOOL fUseSemi)
{
	Vector vecAiming, vecSrc, vecDir;
	int flag;

	flCycleTime -= 0.05;

	if (++m_iShotsFired > 1)
	{
		return;
	}

	if (m_flLastFire != 0.0)
	{
		m_flAccuracy -= (0.275 - (gpGlobals->time - m_flLastFire)) * 0.25;

		if (m_flAccuracy > 0.92)
		{
			m_flAccuracy = 0.92;
		}
		else if (m_flAccuracy < 0.725)
		{
			m_flAccuracy = 0.725;
		}
	}

	m_flLastFire = gpGlobals->time;

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
	SetPlayerShieldAnim();

	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
	m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;

	vecSrc = m_pPlayer->GetGunPosition();
	vecAiming = gpGlobals->v_forward;

	vecDir = m_pPlayer->FireBullets3(vecSrc, vecAiming, flSpread, 4096, 1, BULLET_PLAYER_57MM, FIVESEVEN_DAMAGE, FIVESEVEN_RANGE_MODIFER, m_pPlayer->pev, false, // TODO: why fiveseven not is IsPistol?
		m_pPlayer->random_seed);

#ifdef CLIENT_WEAPONS
	flag = FEV_NOTHOST;
#else
	flag = 0;
#endif // CLIENT_WEAPONS

	PLAYBACK_EVENT_FULL(flag, m_pPlayer->edict(), m_usFireFiveSeven, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y,
		(int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_iClip == 0, FALSE);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay(flCycleTime);

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, FALSE);
	}
	
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2;
	m_pPlayer->pev->punchangle.x -= 2;
	ResetPlayerShieldAnim();
}

/* <2567d0> ../cstrike/dlls/wpn_shared/wpn_fiveseven.cpp:225 */
void CFiveSeven::__MAKE_VHOOK(Reload)(void)
{
	if (m_pPlayer->ammo_57mm <= 0)
	{
		return;
	}

	if (DefaultReload(FIVESEVEN_MAX_CLIP, FIVESEVEN_RELOAD, FIVESEVEN_RELOAD_TIME))
	{
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
		m_flAccuracy = 0.92;
	}
}

/* <256796> ../cstrike/dlls/wpn_shared/wpn_fiveseven.cpp:238 */
void CFiveSeven::__MAKE_VHOOK(WeaponIdle)(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
	{
		return;
	}

	if (m_pPlayer->HasShield())
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0;

		if (m_iWeaponState & WPNSTATE_SHIELD_DRAWN)
		{
			SendWeaponAnim(SHIELDGUN_DRAWN_IDLE, UseDecrement() != FALSE);
		}
	}
	else if (m_iClip)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0625;
		SendWeaponAnim(FIVESEVEN_IDLE, UseDecrement() != FALSE);
	}
}

#ifdef HOOK_GAMEDLL

void CFiveSeven::Spawn(void)
{
	Spawn_();
}

void CFiveSeven::Precache(void)
{
	Precache_();
}

int CFiveSeven::GetItemInfo(ItemInfo *p)
{
	return GetItemInfo_(p);
}

BOOL CFiveSeven::Deploy(void)
{
	return Deploy_();
}

void CFiveSeven::PrimaryAttack(void)
{
	PrimaryAttack_();
}

void CFiveSeven::SecondaryAttack(void)
{
	SecondaryAttack_();
}

void CFiveSeven::Reload(void)
{
	Reload_();
}

void CFiveSeven::WeaponIdle(void)
{
	WeaponIdle_();
}

#endif // HOOK_GAMEDLL
