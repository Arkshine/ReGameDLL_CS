#include "precompiled.h"

#define C4_MAX_AMMO		1
#define C4_MAX_SPEED		250.0
#define C4_ARMING_ON_TIME	3.0

enum c4_e
{
	C4_IDLE1,
	C4_DRAW,
	C4_DROP,
	C4_ARM
};

/* <246a03> ../cstrike/dlls/wpn_shared/wpn_c4.cpp:48 */
LINK_ENTITY_TO_CLASS(weapon_c4, CC4);

/* <2469b9> ../cstrike/dlls/wpn_shared/wpn_c4.cpp:50 */
void CC4::__MAKE_VHOOK(Spawn)(void)
{
	SET_MODEL(edict(), "models/w_backpack.mdl");

	pev->frame = 0;
	pev->body = 3;
	pev->sequence = 0;
	pev->framerate = 0;

	m_iId = WEAPON_C4;
	m_iDefaultAmmo = C4_DEFAULT_GIVE;
	m_bStartedArming = false;
	m_fArmedTime = 0;

	if (!FStringNull(pev->targetname))
	{
		pev->effects |= EF_NODRAW;
		DROP_TO_FLOOR(edict());

		return;
	}

	FallInit();
	SetThink(&CBasePlayerItem::FallThink);
	pev->nextthink = UTIL_WeaponTimeBase() + 0.1;
}

/* <246418> ../cstrike/dlls/wpn_shared/wpn_c4.cpp:80 */
void CC4::__MAKE_VHOOK(Precache)(void)
{
	PRECACHE_MODEL("models/v_c4.mdl");
	PRECACHE_MODEL("models/w_backpack.mdl");

	PRECACHE_SOUND("weapons/c4_click.wav");
}

/* <24643f> ../cstrike/dlls/wpn_shared/wpn_c4.cpp:88 */
int CC4::__MAKE_VHOOK(GetItemInfo)(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "C4";
	p->iMaxAmmo1 = C4_MAX_AMMO;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 4;
	p->iPosition = 3;
	p->iId = m_iId = WEAPON_C4;
	p->iWeight = C4_WEIGHT;
	p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;

	return 1;
}

/* <2466d5> ../cstrike/dlls/wpn_shared/wpn_c4.cpp:105 */
BOOL CC4::__MAKE_VHOOK(Deploy)(void)
{
	pev->body = 0;

	m_bStartedArming = false;
	m_fArmedTime = 0;

	if (m_pPlayer->HasShield())
	{
		m_bHasShield = true;
		m_pPlayer->pev->gamestate = 1;
	}

	return DefaultDeploy("models/v_c4.mdl", "models/p_c4.mdl", C4_DRAW, "c4", UseDecrement() != FALSE);
}

/* <2466fc> ../cstrike/dlls/wpn_shared/wpn_c4.cpp:123 */
void CC4::__MAKE_VHOOK(Holster)(int skiplocal)
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	m_bStartedArming = false;

	if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		m_pPlayer->pev->weapons &= ~(1 << WEAPON_C4);
		DestroyItem();
	}

	if (m_bHasShield)
	{
		m_pPlayer->pev->gamestate = 0;
		m_bHasShield = false;
	}
}

/* <2464e8> ../cstrike/dlls/wpn_shared/wpn_c4.cpp:152 */
void CC4::__MAKE_VHOOK(PrimaryAttack)(void)
{
	BOOL PlaceBomb;
	int inBombZone, onGround;

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		return;
	}

	inBombZone = (m_pPlayer->m_signals.GetState() & SIGNAL_BOMB) == SIGNAL_BOMB;
	onGround = (m_pPlayer->pev->flags & FL_ONGROUND) == FL_ONGROUND;
	PlaceBomb = (onGround && inBombZone);

	if (!m_bStartedArming)
	{
		if (!inBombZone)
		{
			ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#C4_Plant_At_Bomb_Spot");
			m_flNextPrimaryAttack = GetNextAttackDelay(1.0);
			return;
		}

		if (!onGround)
		{
			ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#C4_Plant_Must_Be_On_Ground");
			m_flNextPrimaryAttack = GetNextAttackDelay(1);
			return;
		}

		m_bStartedArming = true;
		m_bBombPlacedAnimation = false;
		m_fArmedTime = gpGlobals->time + C4_ARMING_ON_TIME;

		SendWeaponAnim(C4_ARM, UseDecrement() != FALSE);

		SET_CLIENT_MAXSPEED(m_pPlayer->edict(), 1.0);

		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
		m_pPlayer->SetProgressBarTime(C4_ARMING_ON_TIME);
	}
	else
	{
		if (PlaceBomb)
		{
			CBaseEntity *pEntity = NULL;
			CBasePlayer *pTempPlayer = NULL;

			if (m_fArmedTime <= gpGlobals->time)
			{
				if (m_bStartedArming)
				{
					m_bStartedArming = false;
					m_fArmedTime = 0;

					Broadcast("BOMBPL");
					m_pPlayer->m_bHasC4 = false;

					if (pev->speed != 0 && g_pGameRules != NULL)
					{
						g_pGameRules->m_iC4Timer = (int)pev->speed;
					}

					CGrenade *pBomb = CGrenade::ShootSatchelCharge(m_pPlayer->pev, m_pPlayer->pev->origin, Vector(0, 0, 0));

					MESSAGE_BEGIN(MSG_SPEC, SVC_DIRECTOR);
						WRITE_BYTE(9);
						WRITE_BYTE(DRC_CMD_EVENT);
						WRITE_SHORT(m_pPlayer->entindex());
						WRITE_SHORT(0);
						WRITE_LONG(DRC_FLAG_FACEPLAYER | 11);
					MESSAGE_END();

					MESSAGE_BEGIN(MSG_ALL, gmsgBombDrop);
						WRITE_COORD(pBomb->pev->origin.x);
						WRITE_COORD(pBomb->pev->origin.y);
						WRITE_COORD(pBomb->pev->origin.z);
						WRITE_BYTE(1);
					MESSAGE_END();

					UTIL_ClientPrintAll(HUD_PRINTCENTER, "#Bomb_Planted");

					TheBots->OnEvent(EVENT_BOMB_PLANTED, m_pPlayer, pBomb);

					if (g_pGameRules->IsCareer() && !m_pPlayer->IsBot())
					{
						TheCareerTasks->HandleEvent(EVENT_BOMB_PLANTED, m_pPlayer);
					}

					UTIL_LogPrintf
					(
						"\"%s<%i><%s><TERRORIST>\" triggered \"Planted_The_Bomb\"\n",
						STRING(m_pPlayer->pev->netname),
						GETPLAYERUSERID(m_pPlayer->edict()),
						GETPLAYERAUTHID(m_pPlayer->edict())
					);

					g_pGameRules->m_bBombDropped = FALSE;
					EMIT_SOUND(edict(), CHAN_WEAPON, "weapons/c4_plant.wav", VOL_NORM, ATTN_NORM);

					m_pPlayer->pev->body = 0;
					m_pPlayer->ResetMaxSpeed();
					m_pPlayer->SetBombIcon(FALSE);

					m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

					if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
					{
						RetireWeapon();
						return;
					}
				}
			}
			else
			{
				if (m_fArmedTime - 0.75 <= gpGlobals->time && !m_bBombPlacedAnimation)
				{
					m_bBombPlacedAnimation = true;

					SendWeaponAnim(C4_DROP, UseDecrement() != FALSE);
					m_pPlayer->SetAnimation(PLAYER_HOLDBOMB);
				}
			}
		}
		else
		{
			if (inBombZone)
				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#C4_Plant_Must_Be_On_Ground");
			else
				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#C4_Arming_Cancelled");

			m_bStartedArming = false;
			m_flNextPrimaryAttack = GetNextAttackDelay(1.5);

			m_pPlayer->ResetMaxSpeed();
			m_pPlayer->SetProgressBarTime(0);
			m_pPlayer->SetAnimation(PLAYER_HOLDBOMB);

			SendWeaponAnim(m_bBombPlacedAnimation ? C4_DRAW : C4_IDLE1, UseDecrement() != FALSE);
			return;
		}
	}

	m_flNextPrimaryAttack = GetNextAttackDelay(0.3);
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT(10, 15);
}

/* <2464c1> ../cstrike/dlls/wpn_shared/wpn_c4.cpp:358 */
void CC4::__MAKE_VHOOK(WeaponIdle)(void)
{
	if (m_bStartedArming)
	{
		m_bStartedArming = false;

		m_pPlayer->ResetMaxSpeed();
		m_flNextPrimaryAttack = GetNextAttackDelay(1.0);
		m_pPlayer->SetProgressBarTime(0);

		SendWeaponAnim(m_bBombPlacedAnimation ? C4_DRAW : C4_IDLE1, UseDecrement() != FALSE);
	}

	if (m_flTimeWeaponIdle <= UTIL_WeaponTimeBase())
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		{
			RetireWeapon();
			return;
		}

		SendWeaponAnim(C4_DRAW, UseDecrement() != FALSE);
		SendWeaponAnim(C4_IDLE1, UseDecrement() != FALSE);
	}
}

/* <2468b7> ../cstrike/dlls/wpn_shared/wpn_c4.cpp:399 */
void CC4::__MAKE_VHOOK(KeyValue)(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "detonatedelay"))
	{
		pev->speed = Q_atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "detonatetarget"))
	{
		pev->noise1 = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "defusetarget"))
	{
		pev->target = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
	{
		CBaseEntity::KeyValue(pkvd);
	}
}

/* <24674f> ../cstrike/dlls/wpn_shared/wpn_c4.cpp:419 */
void CC4::__MAKE_VHOOK(Use)(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (m_pPlayer != NULL)
	{
		return;
	}

	CBasePlayer *pPlayer = reinterpret_cast<CBasePlayer *>(UTIL_PlayerByIndex(1));

	if (pPlayer != NULL)
	{
		edict_t *m_pentOldCurBombTarget = pPlayer->m_pentCurBombTarget;
		pPlayer->m_pentCurBombTarget = NULL;

		if (pev->speed != 0 && g_pGameRules)
		{
			g_pGameRules->m_iC4Timer = (int)pev->speed;
		}

		EMIT_SOUND(edict(), CHAN_WEAPON, "weapons/c4_plant.wav", VOL_NORM, ATTN_NORM);

		CGrenade::ShootSatchelCharge(m_pPlayer->pev, m_pPlayer->pev->origin, Vector(0, 0, 0));

		CGrenade *pC4 = NULL;
		while ((pC4 = (CGrenade *)UTIL_FindEntityByClassname(pC4, "grenade")) != NULL)
		{
			if (pC4->m_bIsC4 && pC4->m_flNextFreq == gpGlobals->time)
			{
				pC4->pev->target = pev->target;
				pC4->pev->noise1 = pev->noise1;
				break;
			}
		}

		pPlayer->m_pentCurBombTarget = m_pentOldCurBombTarget;
		SUB_Remove();
	}
}

/* <2463cc> ../cstrike/dlls/weapons.h:732 */
float CC4::GetMaxSpeed(void)
{
	return C4_MAX_SPEED;
}

#ifdef HOOK_GAMEDLL

void CC4::Spawn(void)
{
	Spawn_();
}

void CC4::Precache(void)
{
	Precache_();
}

void CC4::KeyValue(KeyValueData *pkvd)
{
	KeyValue_(pkvd);
}

void CC4::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	Use_(pActivator, pCaller, useType, value);
}

int CC4::GetItemInfo(ItemInfo *p)
{
	return GetItemInfo_(p);
}

BOOL CC4::Deploy(void)
{
	return Deploy_();
}

void CC4::Holster(int skiplocal)
{
	Holster_(skiplocal);
}

void CC4::PrimaryAttack(void)
{
	PrimaryAttack_();
}

void CC4::WeaponIdle(void)
{
	WeaponIdle_();
}

#endif // HOOK_GAMEDLL
