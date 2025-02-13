#include "precompiled.h"

#define SMOKEGRENADE_MAX_SPEED		250
#define SMOKEGRENADE_MAX_SPEED_SHIELD	180

enum smokegrenade_e
{
	SMOKEGRENADE_IDLE,
	SMOKEGRENADE_PINPULL,
	SMOKEGRENADE_THROW,
	SMOKEGRENADE_DRAW
};

/* <2ab333> ../cstrike/dlls/wpn_shared/wpn_smokegrenade.cpp:38 */
LINK_ENTITY_TO_CLASS(weapon_smokegrenade, CSmokeGrenade);

/* <2aafc7> ../cstrike/dlls/wpn_shared/wpn_smokegrenade.cpp:40 */
void CSmokeGrenade::__MAKE_VHOOK(Spawn)(void)
{
	Precache();
	m_iId = WEAPON_SMOKEGRENADE;

	SET_MODEL(edict(), "models/w_smokegrenade.mdl");

	pev->dmg = 4;

	m_iDefaultAmmo = SMOKEGRENADE_DEFAULT_GIVE;
	m_flStartThrow = 0;
	m_flReleaseThrow = -1;
	m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;

	// get ready to fall down.
	FallInit();
}

/* <2aaf12> ../cstrike/dlls/wpn_shared/wpn_smokegrenade.cpp:59 */
void CSmokeGrenade::__MAKE_VHOOK(Precache)(void)
{
	PRECACHE_MODEL("models/v_smokegrenade.mdl");
	PRECACHE_MODEL("models/shield/v_shield_smokegrenade.mdl");

	PRECACHE_SOUND("weapons/pinpull.wav");
	PRECACHE_SOUND("weapons/sg_explode.wav");

	m_usCreateSmoke = PRECACHE_EVENT(1, "events/createsmoke.sc");
}

/* <2aaf39> ../cstrike/dlls/wpn_shared/wpn_smokegrenade.cpp:71 */
int CSmokeGrenade::__MAKE_VHOOK(GetItemInfo)(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "SmokeGrenade";
	p->iMaxAmmo1 = MAX_AMMO_SMOKEGRENADE;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 3;
	p->iPosition = 3;
	p->iId = m_iId = WEAPON_SMOKEGRENADE;
	p->iWeight = SMOKEGRENADE_WEIGHT;
	p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;

	return 1;
}

/* <2ab077> ../cstrike/dlls/wpn_shared/wpn_smokegrenade.cpp:89 */
BOOL CSmokeGrenade::__MAKE_VHOOK(Deploy)(void)
{
	m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;

	m_flReleaseThrow = -1;
	m_fMaxSpeed = SMOKEGRENADE_MAX_SPEED;

	m_pPlayer->m_bShieldDrawn = false;

	if (m_pPlayer->HasShield())
		return DefaultDeploy("models/shield/v_shield_smokegrenade.mdl", "models/shield/p_shield_smokegrenade.mdl", SMOKEGRENADE_DRAW, "shieldgren", UseDecrement() != FALSE);
	else
		return DefaultDeploy("models/v_smokegrenade.mdl", "models/p_smokegrenade.mdl", SMOKEGRENADE_DRAW, "grenade", UseDecrement() != FALSE);
}

/* <2aaf92> ../cstrike/dlls/wpn_shared/wpn_smokegrenade.cpp:104 */
void CSmokeGrenade::__MAKE_VHOOK(Holster)(int skiplocal)
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	
	if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		// no more smokegrenades!
		// clear the smokegrenade of bits for HUD
		m_pPlayer->pev->weapons &= ~(1 << WEAPON_SMOKEGRENADE);
		DestroyItem();
	}

	m_flStartThrow = 0;
	m_flReleaseThrow = -1;
}

/* <2ab03d> ../cstrike/dlls/wpn_shared/wpn_smokegrenade.cpp:120 */
void CSmokeGrenade::__MAKE_VHOOK(PrimaryAttack)(void)
{
	if (m_iWeaponState & WPNSTATE_SHIELD_DRAWN)
		return;

	if (!m_flStartThrow && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
	{
		m_flReleaseThrow = 0;
		m_flStartThrow = gpGlobals->time;

		SendWeaponAnim(SMOKEGRENADE_PINPULL, UseDecrement() != FALSE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
	}
}

/* <2ab3fd> ../cstrike/dlls/wpn_shared/wpn_smokegrenade.cpp:135 */
bool CSmokeGrenade::ShieldSecondaryFire(int iUpAnim, int iDownAnim)
{
	if (!m_pPlayer->HasShield() || m_flStartThrow > 0)
	{
		return false;
	}

	if (m_iWeaponState & WPNSTATE_SHIELD_DRAWN)
	{
		m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
		SendWeaponAnim(iDownAnim, UseDecrement() != FALSE);

		Q_strcpy(m_pPlayer->m_szAnimExtention, "shieldgren");

		m_fMaxSpeed = SMOKEGRENADE_MAX_SPEED;
		m_pPlayer->m_bShieldDrawn = false;
	}
	else
	{
		m_iWeaponState |= WPNSTATE_SHIELD_DRAWN;
		SendWeaponAnim(iUpAnim, UseDecrement() != FALSE);

		Q_strcpy(m_pPlayer->m_szAnimExtention, "shielded");

		m_fMaxSpeed = SMOKEGRENADE_MAX_SPEED_SHIELD;
		m_pPlayer->m_bShieldDrawn = true;
	}

	m_pPlayer->UpdateShieldCrosshair((m_iWeaponState & WPNSTATE_SHIELD_DRAWN) != WPNSTATE_SHIELD_DRAWN);
	m_pPlayer->ResetMaxSpeed();

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.4;
	m_flNextPrimaryAttack = GetNextAttackDelay(0.4);
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.6;

	return true;
}

/* <2ab0b2> ../cstrike/dlls/wpn_shared/wpn_smokegrenade.cpp:173 */
void CSmokeGrenade::__MAKE_VHOOK(SecondaryAttack)(void)
{
	ShieldSecondaryFire(SHIELDGUN_DRAW, SHIELDGUN_DRAWN_IDLE);
}

/* <2ab42f> ../cstrike/dlls/wpn_shared/wpn_smokegrenade.cpp:178 */
void CSmokeGrenade::SetPlayerShieldAnim(void)
{
	if (!m_pPlayer->HasShield())
		return;

	if (m_iWeaponState & WPNSTATE_SHIELD_DRAWN)
		Q_strcpy(m_pPlayer->m_szAnimExtention, "shield");
	else
		Q_strcpy(m_pPlayer->m_szAnimExtention, "shieldgren");
}

/* <2ab451> ../cstrike/dlls/wpn_shared/wpn_smokegrenade.cpp:189 */
void CSmokeGrenade::ResetPlayerShieldAnim(void)
{
	if (!m_pPlayer->HasShield())
		return;

	if (m_iWeaponState & WPNSTATE_SHIELD_DRAWN)
	{
		Q_strcpy(m_pPlayer->m_szAnimExtention, "shieldgren");
	}
}

/* <2aa17b> ../cstrike/dlls/wpn_shared/wpn_smokegrenade.cpp:199 */
void CSmokeGrenade::__MAKE_VHOOK(WeaponIdle)(void)
{
	if (m_flReleaseThrow == 0)
		m_flReleaseThrow = gpGlobals->time;

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_flStartThrow)
	{
		m_pPlayer->Radio("%!MRAD_FIREINHOLE", "#Fire_in_the_hole");

		Vector angThrow = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

		if (angThrow.x < 0)
			angThrow.x = -10 + angThrow.x * ((90 - 10) / 90.0);
		else
			angThrow.x = -10 + angThrow.x * ((90 + 10) / 90.0);

		float_precision flVel = (90.0 - angThrow.x) * 6.0;

		if (flVel > 750.0f)
			flVel = 750.0f;

		UTIL_MakeVectors(angThrow);

		Vector vecSrc = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16;
		Vector vecThrow = gpGlobals->v_forward * flVel + m_pPlayer->pev->velocity;

		CGrenade::ShootSmokeGrenade(m_pPlayer->pev, vecSrc, vecThrow, 1.5, m_usCreateSmoke);

		SendWeaponAnim(SMOKEGRENADE_THROW, UseDecrement() != FALSE);
		SetPlayerShieldAnim();

		// player "shoot" animation
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

		m_flStartThrow = 0;
		m_flNextPrimaryAttack = GetNextAttackDelay(0.5);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75;

		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

		if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			// just threw last grenade
			// set attack times in the future, and weapon idle in the future so we can see the whole throw
			// animation, weapon idle will automatically retire the weapon for us.
			// ensure that the animation can finish playing
			m_flTimeWeaponIdle = m_flNextSecondaryAttack = m_flNextPrimaryAttack = GetNextAttackDelay(0.5);
		}

		ResetPlayerShieldAnim();
	}
	else if (m_flReleaseThrow > 0)
	{
		// we've finished the throw, restart.
		m_flStartThrow = 0;

		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			SendWeaponAnim(SMOKEGRENADE_DRAW, UseDecrement() != FALSE);
		}
		else
		{
			RetireWeapon();
			return;
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT(10, 15);
		m_flReleaseThrow = -1;
	}
	else if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		int iAnim;
		float flRand = RANDOM_FLOAT(0, 1);

		if (m_pPlayer->HasShield())
		{
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0;

			if (m_iWeaponState & WPNSTATE_SHIELD_DRAWN)
			{
				SendWeaponAnim(SHIELDREN_IDLE, UseDecrement() != FALSE);
			}
		}
		else
		{
			if (flRand <= 0.75)
			{
				iAnim = SMOKEGRENADE_IDLE;

				// how long till we do this again.
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT(10, 15);
			}
			else
			{
				iAnim = SMOKEGRENADE_IDLE;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 75.0 / 30.0;
			}

			SendWeaponAnim(iAnim, UseDecrement() != FALSE);
		}
	}
}

/* <2aaf6c> ../cstrike/dlls/wpn_shared/wpn_smokegrenade.cpp:310 */
BOOL CSmokeGrenade::__MAKE_VHOOK(CanDeploy)(void)
{
	return m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != 0;
}

#ifdef HOOK_GAMEDLL

void CSmokeGrenade::Spawn(void)
{
	Spawn_();
}

void CSmokeGrenade::Precache(void)
{
	Precache_();
}

int CSmokeGrenade::GetItemInfo(ItemInfo *p)
{
	return GetItemInfo_(p);
}

BOOL CSmokeGrenade::CanDeploy(void)
{
	return CanDeploy_();
}

BOOL CSmokeGrenade::Deploy(void)
{
	return Deploy_();
}

void CSmokeGrenade::Holster(int skiplocal)
{
	Holster_(skiplocal);
}

void CSmokeGrenade::PrimaryAttack(void)
{
	PrimaryAttack_();
}

void CSmokeGrenade::SecondaryAttack(void)
{
	SecondaryAttack_();
}

void CSmokeGrenade::WeaponIdle(void)
{
	WeaponIdle_();
}

#endif // HOOK_GAMEDLL
