#include "precompiled.h"

#define P228_MAX_SPEED		250

#define P228_DAMAGE		32
#define P228_RANGE_MODIFER	0.8

#define P228_RELOAD_TIME	2.7

enum p228_e
{
	P228_IDLE,
	P228_SHOOT1,
	P228_SHOOT2,
	P228_SHOOT3,
	P228_SHOOT_EMPTY,
	P228_RELOAD,
	P228_DRAW
};

enum p228_shield_e
{
	P228_SHIELD_IDLE,
	P228_SHIELD_SHOOT1,
	P228_SHIELD_SHOOT2,
	P228_SHIELD_SHOOT_EMPTY,
	P228_SHIELD_RELOAD,
	P228_SHIELD_DRAW,
	P228_SHIELD_IDLE_UP,
	P228_SHIELD_UP,
	P228_SHIELD_DOWN
};

/* <291149> ../cstrike/dlls/wpn_shared/wpn_p228.cpp:63 */
LINK_ENTITY_TO_CLASS(weapon_p228, CP228);

/* <290eb8> ../cstrike/dlls/wpn_shared/wpn_p228.cpp:65 */
void CP228::__MAKE_VHOOK(Spawn)(void)
{
	Precache();
	m_iId = WEAPON_P228;
	SET_MODEL(ENT(pev), "models/w_p228.mdl");

	m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
	m_iDefaultAmmo = P228_DEFAULT_GIVE;
	m_flAccuracy = 0.9;

	FallInit();
}

/* <290e37> ../cstrike/dlls/wpn_shared/wpn_p228.cpp:80 */
void CP228::__MAKE_VHOOK(Precache)(void)
{
	PRECACHE_MODEL("models/v_p228.mdl");
	PRECACHE_MODEL("models/w_p228.mdl");
	PRECACHE_MODEL("models/shield/v_shield_p228.mdl");

	PRECACHE_SOUND("weapons/p228-1.wav");
	PRECACHE_SOUND("weapons/p228_clipout.wav");
	PRECACHE_SOUND("weapons/p228_clipin.wav");
	PRECACHE_SOUND("weapons/p228_sliderelease.wav");
	PRECACHE_SOUND("weapons/p228_slidepull.wav");

	m_iShell = PRECACHE_MODEL("models/pshell.mdl");
	m_usFireP228 = PRECACHE_EVENT(1, "events/p228.sc");
}

/* <290e5e> ../cstrike/dlls/wpn_shared/wpn_p228.cpp:98 */
int CP228::__MAKE_VHOOK(GetItemInfo)(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "357SIG";
	p->iMaxAmmo1 = MAX_AMMO_357SIG;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = P228_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 3;
	p->iId = m_iId = WEAPON_P228;
	p->iFlags = 0;
	p->iWeight = P228_WEIGHT;

	return 1;
}

/* <290fc6> ../cstrike/dlls/wpn_shared/wpn_p228.cpp:115 */
BOOL CP228::__MAKE_VHOOK(Deploy)(void)
{
	m_flAccuracy = 0.9;
	m_fMaxSpeed = P228_MAX_SPEED;
	m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
	m_pPlayer->m_bShieldDrawn = false;

	if (m_pPlayer->HasShield())
		return DefaultDeploy("models/shield/v_shield_p228.mdl", "models/shield/p_shield_p228.mdl", P228_SHIELD_DRAW, "shieldgun", UseDecrement() != FALSE);
	else
		return DefaultDeploy("models/v_p228.mdl", "models/p_p228.mdl", P228_DRAW, "onehanded", UseDecrement() != FALSE);
}

/* <2910ff> ../cstrike/dlls/wpn_shared/wpn_p228.cpp:130 */
void CP228::__MAKE_VHOOK(PrimaryAttack)(void)
{
	if (!(m_pPlayer->pev->flags & FL_ONGROUND))
	{
		P228Fire(1.5 * (1 - m_flAccuracy), 0.2, FALSE);
	}
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
	{
		P228Fire(0.255 * (1 - m_flAccuracy), 0.2, FALSE);
	}
	else if (m_pPlayer->pev->flags & FL_DUCKING)
	{
		P228Fire(0.075 * (1 - m_flAccuracy), 0.2, FALSE);
	}
	else
	{
		P228Fire(0.15 * (1 - m_flAccuracy), 0.2, FALSE);
	}
}

/* <290e91> ../cstrike/dlls/wpn_shared/wpn_p228.cpp:142 */
void CP228::__MAKE_VHOOK(SecondaryAttack)(void)
{
	ShieldSecondaryFire(SHIELDGUN_UP, SHIELDGUN_DOWN);
}

/* <291213> ../cstrike/dlls/wpn_shared/wpn_p228.cpp:147 */
void CP228::P228Fire(float flSpread, float flCycleTime, BOOL fUseSemi)
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
		m_flAccuracy -= (0.325 - (gpGlobals->time - m_flLastFire)) * 0.3;

		if (m_flAccuracy > 0.9)
		{
			m_flAccuracy = 0.9;
		}
		else if (m_flAccuracy < 0.6)
		{
			m_flAccuracy = 0.6;
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

	vecDir = m_pPlayer->FireBullets3(vecSrc, vecAiming, flSpread, 4096, 1, BULLET_PLAYER_357SIG, P228_DAMAGE, P228_RANGE_MODIFER, m_pPlayer->pev, true, m_pPlayer->random_seed);

#ifdef CLIENT_WEAPONS
	flag = FEV_NOTHOST;
#else
	flag = 0;
#endif // CLIENT_WEAPONS

	PLAYBACK_EVENT_FULL(flag, m_pPlayer->edict(), m_usFireP228, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y,
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

/* <290f69> ../cstrike/dlls/wpn_shared/wpn_p228.cpp:241 */
void CP228::__MAKE_VHOOK(Reload)(void)
{
	if (m_pPlayer->ammo_357sig <= 0)
	{
		return;
	}

	if (DefaultReload(P228_MAX_CLIP, m_pPlayer->HasShield() ? P228_SHIELD_RELOAD : P228_RELOAD, P228_RELOAD_TIME))
	{
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
		m_flAccuracy = 0.9;
	}
}

/* <290f2e> ../cstrike/dlls/wpn_shared/wpn_p228.cpp:261 */
void CP228::__MAKE_VHOOK(WeaponIdle)(void)
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
			SendWeaponAnim(P228_SHIELD_IDLE_UP, UseDecrement() != FALSE);
		}
	}
	else if (m_iClip)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0625;
		SendWeaponAnim(P228_IDLE, UseDecrement() != FALSE);
	}
}

#ifdef HOOK_GAMEDLL

void CP228::Spawn(void)
{
	Spawn_();
}

void CP228::Precache(void)
{
	Precache_();
}

int CP228::GetItemInfo(ItemInfo *p)
{
	return GetItemInfo_(p);
}

BOOL CP228::Deploy(void)
{
	return Deploy_();
}

void CP228::PrimaryAttack(void)
{
	PrimaryAttack_();
}

void CP228::SecondaryAttack(void)
{
	SecondaryAttack_();
}

void CP228::Reload(void)
{
	Reload_();
}

void CP228::WeaponIdle(void)
{
	WeaponIdle_();
}

#endif // HOOK_GAMEDLL
