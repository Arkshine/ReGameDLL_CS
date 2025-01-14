#include "precompiled.h"

/*
* Globals initialization
*/
#ifndef HOOK_GAMEDLL

ItemInfo CBasePlayerItem::ItemInfoArray[32];
AmmoInfo CBasePlayerItem::AmmoInfoArray[32];

TYPEDESCRIPTION CBasePlayerItem::m_SaveData[] =
{
	DEFINE_FIELD(CBasePlayerItem, m_pPlayer, FIELD_CLASSPTR),
	DEFINE_FIELD(CBasePlayerItem, m_pNext, FIELD_CLASSPTR),
	DEFINE_FIELD(CBasePlayerItem, m_iId, FIELD_INTEGER),
};

TYPEDESCRIPTION CBasePlayerWeapon::m_SaveData[] =
{
	DEFINE_FIELD(CBasePlayerWeapon, m_flNextPrimaryAttack, FIELD_TIME),
	DEFINE_FIELD(CBasePlayerWeapon, m_flNextSecondaryAttack, FIELD_TIME),
	DEFINE_FIELD(CBasePlayerWeapon, m_flTimeWeaponIdle, FIELD_TIME),
	DEFINE_FIELD(CBasePlayerWeapon, m_iPrimaryAmmoType, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayerWeapon, m_iSecondaryAmmoType, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayerWeapon, m_iClip, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayerWeapon, m_iDefaultAmmo, FIELD_INTEGER),
};

TYPEDESCRIPTION CWeaponBox::m_SaveData[] =
{
	DEFINE_ARRAY(CWeaponBox, m_rgAmmo, FIELD_INTEGER, MAX_AMMO_SLOTS),
	DEFINE_ARRAY(CWeaponBox, m_rgiszAmmo, FIELD_STRING, MAX_AMMO_SLOTS),
	DEFINE_ARRAY(CWeaponBox, m_rgpPlayerItems, FIELD_CLASSPTR, MAX_ITEM_TYPES),
	DEFINE_FIELD(CWeaponBox, m_cAmmoTypes, FIELD_INTEGER),
};

const char *g_pModelNameLaser = "sprites/laserbeam.spr";

#else // HOOK_GAMEDLL

ItemInfo IMPLEMENT_ARRAY_CLASS(CBasePlayerItem, ItemInfoArray)[32];
AmmoInfo IMPLEMENT_ARRAY_CLASS(CBasePlayerItem, AmmoInfoArray)[32];

TYPEDESCRIPTION IMPLEMENT_ARRAY_CLASS(CBasePlayerItem, m_SaveData)[3];
TYPEDESCRIPTION IMPLEMENT_ARRAY_CLASS(CBasePlayerWeapon, m_SaveData)[7];
TYPEDESCRIPTION IMPLEMENT_ARRAY_CLASS(CWeaponBox, m_SaveData)[4];

const char *g_pModelNameLaser;

#endif // HOOK_GAMEDLL

short g_sModelIndexLaser;		// holds the index for the laser beam
short g_sModelIndexLaserDot;		// holds the index for the laser beam dot
short g_sModelIndexFireball;		// holds the index for the fireball
short g_sModelIndexSmoke;		// holds the index for the smoke cloud
short g_sModelIndexWExplosion;		// holds the index for the underwater explosion
short g_sModelIndexBubbles;		// holds the index for the bubbles model
short g_sModelIndexBloodDrop;		// holds the sprite index for the initial blood
short g_sModelIndexBloodSpray;		// holds the sprite index for splattered blood
short g_sModelIndexSmokePuff;
short g_sModelIndexFireball2;
short g_sModelIndexFireball3;
short g_sModelIndexFireball4;
short g_sModelIndexRadio;

short int g_sModelIndexCTGhost;
short int g_sModelIndexTGhost;
short int g_sModelIndexC4Glow;

int giAmmoIndex;

MULTIDAMAGE gMultiDamage;

// MaxAmmoCarry - pass in a name and this function will tell
// you the maximum amount of that type of ammunition that a
// player can carry.

/* <1d018e> ../cstrike/dlls/weapons.cpp:82 */
int MaxAmmoCarry(int iszName)
{
	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		ItemInfo *info = &IMPLEMENT_ARRAY_CLASS(CBasePlayerItem, ItemInfoArray)[ i ];

		if (info->pszAmmo1 && !Q_strcmp(STRING(iszName), info->pszAmmo1))
		{
			return info->iMaxAmmo1;
		}

		if (info->pszAmmo2 && !Q_strcmp(STRING(iszName), info->pszAmmo2))
		{
			return info->iMaxAmmo2;
		}
	}

	ALERT(at_console, "MaxAmmoCarry() doesn't recognize '%s'!\n", STRING(iszName));
	return -1;
}

// ClearMultiDamage - resets the global multi damage accumulator

/* <1d2a29> ../cstrike/dlls/weapons.cpp:110 */
void ClearMultiDamage(void)
{
	gMultiDamage.pEntity = NULL;
	gMultiDamage.amount = 0;
	gMultiDamage.type = 0;
}

// ApplyMultiDamage - inflicts contents of global multi damage register on gMultiDamage.pEntity

/* <1d2a41> ../cstrike/dlls/weapons.cpp:124 */
void ApplyMultiDamage(entvars_t *pevInflictor, entvars_t *pevAttacker)
{
	//Vector vecSpot1;//where blood comes from
	//Vector vecDir;//direction blood should go
	//TraceResult tr;

	if (!gMultiDamage.pEntity)
		return;

	gMultiDamage.pEntity->TakeDamage(pevInflictor, pevAttacker, gMultiDamage.amount, gMultiDamage.type);

}

/* <1d2ad3> ../cstrike/dlls/weapons.cpp:140 */
void AddMultiDamage(entvars_t *pevInflictor, CBaseEntity *pEntity, float flDamage, int bitsDamageType)
{
	if (!pEntity)
		return;

	gMultiDamage.type |= bitsDamageType;

	if (pEntity != gMultiDamage.pEntity)
	{
		// UNDONE: wrong attacker!
		ApplyMultiDamage(pevInflictor, pevInflictor);
		gMultiDamage.pEntity = pEntity;
		gMultiDamage.amount = 0;
	}

	gMultiDamage.amount += flDamage;
}

/* <1d2b6f> ../cstrike/dlls/weapons.cpp:162 */
void SpawnBlood(Vector vecSpot, int bloodColor, float flDamage)
{
	UTIL_BloodDrips(vecSpot, g_vecAttackDir, bloodColor, (int)flDamage);
}

/* <1d2bbb> ../cstrike/dlls/weapons.cpp:168 */
NOXREF int DamageDecal(CBaseEntity *pEntity, int bitsDamageType)
{
	if (pEntity)
	{
		return pEntity->DamageDecal(bitsDamageType);
	}

	return RANDOM_LONG(DECAL_GUNSHOT4, DECAL_GUNSHOT5);
}

/* <1d2bfa> ../cstrike/dlls/weapons.cpp:176 */
void DecalGunshot(TraceResult *pTrace, int iBulletType, bool ClientOnly, entvars_t *pShooter, bool bHitMetal)
{
	;
}

// EjectBrass - tosses a brass shell from passed origin at passed velocity

/* <1d07b3> ../cstrike/dlls/weapons.cpp:184 */
void EjectBrass(const Vector &vecOrigin, const Vector &vecLeft, const Vector &vecVelocity, float rotation, int model, int soundtype, int entityIndex)
{
	//CBaseEntity *ent = UTIL_PlayerByIndex(entityIndex);	// unused
	bool useNewBehavior = UTIL_IsGame("czero");

	MESSAGE_BEGIN(MSG_PVS, gmsgBrass, vecOrigin);
		if (!useNewBehavior)
		{
			// noxref
			WRITE_BYTE(TE_MODEL);
		}
		WRITE_COORD(vecOrigin.x);	// origin
		WRITE_COORD(vecOrigin.y);
		WRITE_COORD(vecOrigin.z);
		if (!useNewBehavior)
		{
			// noxref
			// it parses the client side, but does not use it
			WRITE_COORD(vecLeft.x);
			WRITE_COORD(vecLeft.y);
			WRITE_COORD(vecLeft.z);
		}
		WRITE_COORD(vecVelocity.x);	// velocity
		WRITE_COORD(vecVelocity.y);
		WRITE_COORD(vecVelocity.z);
		WRITE_ANGLE(rotation);
		WRITE_SHORT(model);
		WRITE_BYTE(soundtype);
		if (!useNewBehavior)
		{
			// noxref
			WRITE_BYTE(25);// 2.5 seconds
		}
		WRITE_BYTE(entityIndex);
	MESSAGE_END();
}

/* <1d2cfd> ../cstrike/dlls/weapons.cpp:220 */
NOXREF void EjectBrass2(const Vector &vecOrigin, const Vector &vecVelocity, float rotation, int model, int soundtype, entvars_t *pev)
{
	MESSAGE_BEGIN(MSG_ONE, SVC_TEMPENTITY, NULL, pev);
		WRITE_BYTE(TE_MODEL);
		WRITE_COORD(vecOrigin.x);
		WRITE_COORD(vecOrigin.y);
		WRITE_COORD(vecOrigin.z);
		WRITE_COORD(vecVelocity.x);
		WRITE_COORD(vecVelocity.y);
		WRITE_COORD(vecVelocity.z);
		WRITE_ANGLE(rotation);
		WRITE_SHORT(model);
		WRITE_BYTE(0);
		WRITE_BYTE(5);// 0.5 seconds
	MESSAGE_END();
}

// Precaches the ammo and queues the ammo info for sending to clients

/* <1d020f> ../cstrike/dlls/weapons.cpp:242 */
void AddAmmoNameToAmmoRegistry(const char *szAmmoname)
{
	// make sure it's not already in the registry
	for (int i = 0; i < MAX_AMMO_SLOTS; i++)
	{
		if (!IMPLEMENT_ARRAY_CLASS(CBasePlayerItem, AmmoInfoArray)[ i ].pszName)
			continue;

		if (!Q_stricmp(IMPLEMENT_ARRAY_CLASS(CBasePlayerItem, AmmoInfoArray)[ i ].pszName, szAmmoname))
		{
			// ammo already in registry, just quite
			return;
		}
	}

	giAmmoIndex++;
	assert(giAmmoIndex < MAX_AMMO_SLOTS);

	if (giAmmoIndex >= MAX_AMMO_SLOTS)
	{
		giAmmoIndex = 0;
	}

	IMPLEMENT_ARRAY_CLASS(CBasePlayerItem, AmmoInfoArray)[ giAmmoIndex ].pszName = szAmmoname;

	// yes, this info is redundant
	IMPLEMENT_ARRAY_CLASS(CBasePlayerItem, AmmoInfoArray)[ giAmmoIndex ].iId = giAmmoIndex;
}

// Precaches the weapon and queues the weapon info for sending to clients

/* <1d2e01> ../cstrike/dlls/weapons.cpp:265 */
void UTIL_PrecacheOtherWeapon(const char *szClassname)
{
	edict_t *pent = CREATE_NAMED_ENTITY(MAKE_STRING(szClassname));

	if (FNullEnt(pent))
	{
		ALERT(at_console, "NULL Ent in UTIL_PrecacheOtherWeapon\n");
		return;
	}

	CBaseEntity *pEntity = CBaseEntity::Instance(VARS(pent));

	if (pEntity != NULL)
	{
		ItemInfo II;
		Q_memset(&II, 0, sizeof(II));

		pEntity->Precache();
		if (((CBasePlayerItem *)pEntity)->GetItemInfo(&II))
		{
			IMPLEMENT_ARRAY_CLASS(CBasePlayerItem, ItemInfoArray)[ II.iId ] = II;

			if (II.pszAmmo1 != NULL && *II.pszAmmo1 != '\0')
			{
				AddAmmoNameToAmmoRegistry(II.pszAmmo1);
			}

			if (II.pszAmmo2 != NULL && *II.pszAmmo2 != '\0')
			{
				AddAmmoNameToAmmoRegistry(II.pszAmmo2);
			}
		}
	}

	REMOVE_ENTITY(pent);
}

/* <1d2fc9> ../cstrike/dlls/weapons.cpp:304 */
NOXREF void UTIL_PrecacheOtherWeapon2(const char *szClassname)
{
	edict_t *pent = CREATE_NAMED_ENTITY(MAKE_STRING(szClassname));

	if (FNullEnt(pent))
	{
		ALERT(at_console, "NULL Ent in UTIL_PrecacheOtherWeapon\n");
		return;
	}

	CBaseEntity *pEntity = CBaseEntity::Instance(VARS(pent));

	if (pEntity != NULL)
	{
		ItemInfo II;
		Q_memset(&II, 0, sizeof(II));

		pEntity->Precache();

		if (((CBasePlayerItem *)pEntity)->GetItemInfo(&II))
		{
			IMPLEMENT_ARRAY_CLASS(CBasePlayerItem, ItemInfoArray)[ II.iId ] = II;

			if (II.pszAmmo1 != NULL && *II.pszAmmo1 != '\0')
			{
				AddAmmoNameToAmmoRegistry(II.pszAmmo1);
			}

			if (II.pszAmmo2 != NULL && *II.pszAmmo2 != '\0')
			{
				AddAmmoNameToAmmoRegistry(II.pszAmmo2);
			}
		}
	}

	REMOVE_ENTITY(pent);
}

// called by worldspawn

/* <1d3191> ../cstrike/dlls/weapons.cpp:345 */
void W_Precache(void)
{
	Q_memset(IMPLEMENT_ARRAY_CLASS(CBasePlayerItem, ItemInfoArray), 0, ARRAYSIZE(IMPLEMENT_ARRAY_CLASS(CBasePlayerItem, ItemInfoArray)));
	Q_memset(IMPLEMENT_ARRAY_CLASS(CBasePlayerItem, AmmoInfoArray), 0, ARRAYSIZE(IMPLEMENT_ARRAY_CLASS(CBasePlayerItem, AmmoInfoArray)));
	giAmmoIndex = 0;

	// custom items...

	// common world objects
	UTIL_PrecacheOther("item_suit");
	UTIL_PrecacheOther("item_battery");
	UTIL_PrecacheOther("item_antidote");
	UTIL_PrecacheOther("item_security");
	UTIL_PrecacheOther("item_longjump");
	UTIL_PrecacheOther("item_kevlar");
	UTIL_PrecacheOther("item_assaultsuit");
	UTIL_PrecacheOther("item_thighpack");

	// awp magnum
	UTIL_PrecacheOtherWeapon("weapon_awp");
	UTIL_PrecacheOther("ammo_338magnum");

	UTIL_PrecacheOtherWeapon("weapon_g3sg1");
	UTIL_PrecacheOtherWeapon("weapon_ak47");
	UTIL_PrecacheOtherWeapon("weapon_scout");
	UTIL_PrecacheOther("ammo_762nato");

	// m249
	UTIL_PrecacheOtherWeapon("weapon_m249");
	UTIL_PrecacheOther("ammo_556natobox");

	UTIL_PrecacheOtherWeapon("weapon_m4a1");
	UTIL_PrecacheOtherWeapon("weapon_sg552");
	UTIL_PrecacheOtherWeapon("weapon_aug");
	UTIL_PrecacheOtherWeapon("weapon_sg550");
	UTIL_PrecacheOther("ammo_556nato");

	// shotgun
	UTIL_PrecacheOtherWeapon("weapon_m3");
	UTIL_PrecacheOtherWeapon("weapon_xm1014");
	UTIL_PrecacheOther("ammo_buckshot");

	UTIL_PrecacheOtherWeapon("weapon_usp");
	UTIL_PrecacheOtherWeapon("weapon_mac10");
	UTIL_PrecacheOtherWeapon("weapon_ump45");
	UTIL_PrecacheOther("ammo_45acp");

	UTIL_PrecacheOtherWeapon("weapon_fiveseven");
	UTIL_PrecacheOtherWeapon("weapon_p90");
	UTIL_PrecacheOther("ammo_57mm");

	// deagle
	UTIL_PrecacheOtherWeapon("weapon_deagle");
	UTIL_PrecacheOther("ammo_50ae");

	// p228
	UTIL_PrecacheOtherWeapon("weapon_p228");
	UTIL_PrecacheOther("ammo_357sig");

	// knife
	UTIL_PrecacheOtherWeapon("weapon_knife");

	UTIL_PrecacheOtherWeapon("weapon_glock18");
	UTIL_PrecacheOtherWeapon("weapon_mp5navy");
	UTIL_PrecacheOtherWeapon("weapon_tmp");
	UTIL_PrecacheOtherWeapon("weapon_elite");
	UTIL_PrecacheOther("ammo_9mm");

	UTIL_PrecacheOtherWeapon("weapon_flashbang");
	UTIL_PrecacheOtherWeapon("weapon_hegrenade");
	UTIL_PrecacheOtherWeapon("weapon_smokegrenade");
	UTIL_PrecacheOtherWeapon("weapon_c4");
	UTIL_PrecacheOtherWeapon("weapon_galil");
	UTIL_PrecacheOtherWeapon("weapon_famas");

	if (g_pGameRules->IsDeathmatch())
	{
		// container for dropped deathmatch weapons
		UTIL_PrecacheOther("weaponbox");
	}

	g_sModelIndexFireball = PRECACHE_MODEL("sprites/zerogxplode.spr");	// fireball
	g_sModelIndexWExplosion = PRECACHE_MODEL("sprites/WXplo1.spr");		// underwater fireball
	g_sModelIndexSmoke = PRECACHE_MODEL("sprites/steam1.spr");		// smoke
	g_sModelIndexBubbles = PRECACHE_MODEL("sprites/bubble.spr");		// bubbles
	g_sModelIndexBloodSpray = PRECACHE_MODEL("sprites/bloodspray.spr");	// initial blood
	g_sModelIndexBloodDrop = PRECACHE_MODEL("sprites/blood.spr");		// splattered blood

	g_sModelIndexSmokePuff = PRECACHE_MODEL("sprites/smokepuff.spr");
	g_sModelIndexFireball2 = PRECACHE_MODEL("sprites/eexplo.spr");
	g_sModelIndexFireball3 = PRECACHE_MODEL("sprites/fexplo.spr");
	g_sModelIndexFireball4 = PRECACHE_MODEL("sprites/fexplo1.spr");
	g_sModelIndexRadio = PRECACHE_MODEL("sprites/radio.spr");

	g_sModelIndexCTGhost = PRECACHE_MODEL("sprites/b-tele1.spr");
	g_sModelIndexTGhost = PRECACHE_MODEL("sprites/c-tele1.spr");
	g_sModelIndexC4Glow = PRECACHE_MODEL("sprites/ledglow.spr");

	g_sModelIndexLaser = PRECACHE_MODEL((char *)g_pModelNameLaser);
	g_sModelIndexLaserDot = PRECACHE_MODEL("sprites/laserdot.spr");

	// used by explosions
	PRECACHE_MODEL("models/grenade.mdl");
	PRECACHE_MODEL("sprites/explode1.spr");

	PRECACHE_SOUND("weapons/debris1.wav");		// explosion aftermaths
	PRECACHE_SOUND("weapons/debris2.wav");		// explosion aftermaths
	PRECACHE_SOUND("weapons/debris3.wav");		// explosion aftermaths

	PRECACHE_SOUND("weapons/grenade_hit1.wav");	// grenade
	PRECACHE_SOUND("weapons/grenade_hit2.wav");	// grenade
	PRECACHE_SOUND("weapons/grenade_hit3.wav");	// grenade

	PRECACHE_SOUND("weapons/bullet_hit1.wav");	// hit by bullet
	PRECACHE_SOUND("weapons/bullet_hit2.wav");	// hit by bullet

	PRECACHE_SOUND("items/weapondrop1.wav");	// weapon falls to the ground
	PRECACHE_SOUND("weapons/generic_reload.wav");
}

/* <1d20a8> ../cstrike/dlls/weapons.cpp:458 */
IMPLEMENT_SAVERESTORE(CBasePlayerItem, CBaseAnimating);

/* <1d27b2> ../cstrike/dlls/weapons.cpp:472 */
IMPLEMENT_SAVERESTORE(CBasePlayerWeapon, CBasePlayerItem);

/* <1d1730> ../cstrike/dlls/weapons.cpp:475 */
void CBasePlayerItem::__MAKE_VHOOK(SetObjectCollisionBox)(void)
{
	pev->absmin = pev->origin + Vector(-24, -24, 0);
	pev->absmax = pev->origin + Vector(24, 24, 16);
}

// Sets up movetype, size, solidtype for a new weapon.

/* <1d31ab> ../cstrike/dlls/weapons.cpp:485 */
void CBasePlayerItem::FallInit(void)
{
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_BBOX;

	UTIL_SetOrigin(pev, pev->origin);

	//pointsize until it lands on the ground.
	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

	SetTouch(&CBasePlayerItem::DefaultTouch);
	SetThink(&CBasePlayerItem::FallThink);

	pev->nextthink = gpGlobals->time + 0.1f;
}

// FallThink - Items that have just spawned run this think
// to catch them when they hit the ground. Once we're sure
// that the object is grounded, we change its solid type
// to trigger and set it in a large box that helps the
// player get it.

/* <1d32bc> ../cstrike/dlls/weapons.cpp:506 */
void CBasePlayerItem::FallThink(void)
{
	pev->nextthink = gpGlobals->time + 0.1f;

	if (pev->flags & FL_ONGROUND)
	{
		// clatter if we have an owner (i.e., dropped by someone)
		// don't clatter if the gun is waiting to respawn (if it's waiting, it is invisible!)
		if (!FNullEnt(pev->owner))
		{
			int pitch = RANDOM_LONG(0, 29) + 95;
			EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "items/weapondrop1.wav", VOL_NORM, ATTN_NORM, 0, pitch);
		}

		// lie flat
		pev->angles.x = 0.0f;
		pev->angles.z = 0.0f;

		Materialize();
	}
}

// Materialize - make a CBasePlayerItem visible and tangible

/* <1d3252> ../cstrike/dlls/weapons.cpp:531 */
void CBasePlayerItem::Materialize(void)
{
	if (pev->effects & EF_NODRAW)
	{
		// changing from invisible state to visible.
		if (g_pGameRules->IsMultiplayer())
		{
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "items/suitchargeok1.wav", VOL_NORM, ATTN_NORM, 0, 150);
		}

		pev->effects &= ~EF_NODRAW;
		pev->effects |= EF_MUZZLEFLASH;
	}

	pev->solid = SOLID_TRIGGER;

	// link into world.
	UTIL_SetOrigin(pev, pev->origin);
	SetTouch(&CBasePlayerItem::DefaultTouch);

	if (g_pGameRules->IsMultiplayer())
	{
		if (!CanDrop())
		{
			SetTouch(NULL);
		}

		SetThink(&CBaseEntity::SUB_Remove);
		pev->nextthink = gpGlobals->time + 1.0f;
	}
	else
		SetThink(NULL);
}

// AttemptToMaterialize - the item is trying to rematerialize,
// should it do so now or wait longer?

/* <1d327b> ../cstrike/dlls/weapons.cpp:567 */
void CBasePlayerItem::AttemptToMaterialize(void)
{
	float time = g_pGameRules->FlWeaponTryRespawn(this);

	if (time == 0)
	{
		Materialize();
		return;
	}

	pev->nextthink = gpGlobals->time + time;
}

// CheckRespawn - a player is taking this weapon, should
// it respawn?

/* <1d3348> ../cstrike/dlls/weapons.cpp:584 */
void CBasePlayerItem::CheckRespawn(void)
{
	switch (g_pGameRules->WeaponShouldRespawn(this))
	{
		case GR_WEAPON_RESPAWN_YES:
			return;
		case GR_WEAPON_RESPAWN_NO:
			return;
	}
}

// Respawn- this item is already in the world, but it is
// invisible and intangible. Make it visible and tangible.

/* <1d1e09> ../cstrike/dlls/weapons.cpp:616 */
CBaseEntity *CBasePlayerItem::__MAKE_VHOOK(Respawn)(void)
{
	// make a copy of this weapon that is invisible and inaccessible to players (no touch function). The weapon spawn/respawn code
	// will decide when to make the weapon visible and touchable.
	CBaseEntity *pNewWeapon = CBaseEntity::Create((char *)STRING(pev->classname), g_pGameRules->VecWeaponRespawnSpot(this), pev->angles, pev->owner);

	if (pNewWeapon != NULL)
	{
		// invisible for now
		pNewWeapon->pev->effects |= EF_NODRAW;

		// no touch
		pNewWeapon->SetTouch(NULL);
		pNewWeapon->SetThink(&CBasePlayerItem::AttemptToMaterialize);

		DROP_TO_FLOOR(ENT(pev));

		// not a typo! We want to know when the weapon the player just picked up should respawn! This new entity we created is the replacement,
		// but when it should respawn is based on conditions belonging to the weapon that was taken.
		pNewWeapon->pev->nextthink = g_pGameRules->FlWeaponRespawnTime(this);
	}
	else
	{
		ALERT(at_console, "Respawn failed to create %s!\n", STRING(pev->classname));
	}

	return pNewWeapon;
}

/* <1d26f0> ../cstrike/dlls/weapons.cpp:642 */
void CBasePlayerItem::DefaultTouch(CBaseEntity *pOther)
{
	// if it's not a player, ignore
	if (!pOther->IsPlayer())
	{
		return;
	}

	CBasePlayer *pPlayer = reinterpret_cast<CBasePlayer *>(pOther);

	if (pPlayer->m_bIsVIP
		&& m_iId != WEAPON_USP
		&& m_iId != WEAPON_GLOCK18
		&& m_iId != WEAPON_P228
		&& m_iId != WEAPON_DEAGLE
		&& m_iId != WEAPON_KNIFE)
	{
		return;
	}

	// can I have this?
	if (!g_pGameRules->CanHavePlayerItem(pPlayer, this))
	{
		if (gEvilImpulse101)
		{
			UTIL_Remove(this);
		}

		return;
	}

	if (pOther->AddPlayerItem(this))
	{
		AttachToPlayer(pPlayer);
		SetThink(NULL);
		EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, "items/gunpickup2.wav", VOL_NORM, ATTN_NORM);
	}

	SUB_UseTargets(pOther, USE_TOGGLE, 0);
}

/* <1d3371> ../cstrike/dlls/weapons.cpp:678 */
void CBasePlayerWeapon::SetPlayerShieldAnim(void)
{
	if (!m_pPlayer->HasShield())
		return;

	if (m_iWeaponState & WPNSTATE_SHIELD_DRAWN)
	{
		Q_strcpy(m_pPlayer->m_szAnimExtention, "shield");
	}
	else
	{
		Q_strcpy(m_pPlayer->m_szAnimExtention, "shieldgun");
	}
}

/* <1d339a> ../cstrike/dlls/weapons.cpp:689 */
void CBasePlayerWeapon::ResetPlayerShieldAnim(void)
{
	if (m_pPlayer->HasShield())
	{
		if (m_iWeaponState & WPNSTATE_SHIELD_DRAWN)
		{
			Q_strcpy(m_pPlayer->m_szAnimExtention, "shieldgun");
		}
	}
}

/* <1d33c3> ../cstrike/dlls/weapons.cpp:699 */
void CBasePlayerWeapon::EjectBrassLate(void)
{
	int soundType;
	Vector vecUp, vecRight, vecShellVelocity;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	vecUp = RANDOM_FLOAT(100, 150) * gpGlobals->v_up;
	vecRight = RANDOM_FLOAT(50, 70) * gpGlobals->v_right;

	vecShellVelocity = (m_pPlayer->pev->velocity + vecRight + vecUp) + gpGlobals->v_forward * 25;
	soundType = (m_iId == WEAPON_XM1014 || m_iId == WEAPON_M3) ? 2 : 1;

	EjectBrass(pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_up * -9 + gpGlobals->v_forward * 16, gpGlobals->v_right * -9,
		vecShellVelocity, pev->angles.y, m_iShellId, soundType, m_pPlayer->entindex());
}

/* <1d372a> ../cstrike/dlls/weapons.cpp:717 */
bool CBasePlayerWeapon::ShieldSecondaryFire(int iUpAnim, int iDownAnim)
{
	if (!m_pPlayer->HasShield())
		return false;

	if (m_iWeaponState & WPNSTATE_SHIELD_DRAWN)
	{
		m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
		SendWeaponAnim(iDownAnim, UseDecrement() != FALSE);
		Q_strcpy(m_pPlayer->m_szAnimExtention, "shieldgun");
		m_fMaxSpeed = 250.0f;
		m_pPlayer->m_bShieldDrawn = false;
	}
	else
	{
		m_iWeaponState |= WPNSTATE_SHIELD_DRAWN;
		SendWeaponAnim(iUpAnim, UseDecrement() != FALSE);
		Q_strcpy(m_pPlayer->m_szAnimExtention, "shielded");
		m_fMaxSpeed = 180.0f;
		m_pPlayer->m_bShieldDrawn = true;
	}

	m_pPlayer->UpdateShieldCrosshair((m_iWeaponState & WPNSTATE_SHIELD_DRAWN) != WPNSTATE_SHIELD_DRAWN);
	m_pPlayer->ResetMaxSpeed();

	m_flNextSecondaryAttack = 0.4f;
	m_flNextPrimaryAttack = 0.4f;
	m_flTimeWeaponIdle = 0.6f;

	return true;
}

/* <1d3773> ../cstrike/dlls/weapons.cpp:752 */
void CBasePlayerWeapon::KickBack(float up_base, float lateral_base, float up_modifier, float lateral_modifier, float up_max, float lateral_max, int direction_change)
{
	float_precision flKickUp;
	float flKickLateral;

	if (m_iShotsFired == 1)
	{
		flKickUp = up_base;
		flKickLateral = lateral_base;
	}
	else
	{
		flKickUp = m_iShotsFired * up_modifier + up_base;
		flKickLateral = m_iShotsFired * lateral_modifier + lateral_base;
	}

	m_pPlayer->pev->punchangle.x -= flKickUp;

	if (m_pPlayer->pev->punchangle.x < -up_max)
	{
		m_pPlayer->pev->punchangle.x = -up_max;
	}

	if (m_iDirection == 1)
	{
		m_pPlayer->pev->punchangle.y += flKickLateral;

		if (m_pPlayer->pev->punchangle.y > lateral_max)
			m_pPlayer->pev->punchangle.y = lateral_max;
	}
	else
	{
		m_pPlayer->pev->punchangle.y -= flKickLateral;

		if (m_pPlayer->pev->punchangle.y < -lateral_max)
			m_pPlayer->pev->punchangle.y = -lateral_max;
	}

	if (!RANDOM_LONG(0, direction_change))
	{
		m_iDirection = !m_iDirection;
	}
}

/* <1d242e> ../cstrike/dlls/weapons.cpp:792 */
void CBasePlayerWeapon::FireRemaining(int &shotsFired, float &shootTime, BOOL bIsGlock)
{
	m_iClip--;

	if (m_iClip < 0)
	{
		m_iClip = 0;
		shotsFired = 3;
		shootTime = 0;
		return;
	}

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir;

	int flag;
#ifdef CLIENT_WEAPONS
	flag = FEV_NOTHOST;
#else
	flag = 0;
#endif // CLIENT_WEAPONS

	if (bIsGlock)
	{
		vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, 0.05, 8192, 1, BULLET_PLAYER_9MM, 18, 0.9, m_pPlayer->pev, true, m_pPlayer->random_seed);
		m_pPlayer->ammo_9mm--;

		PLAYBACK_EVENT_FULL(flag, m_pPlayer->edict(), m_usFireGlock18, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y,
			(int)(m_pPlayer->pev->punchangle.x * 10000), (int)(m_pPlayer->pev->punchangle.y * 10000), m_iClip == 0, FALSE);
	}
	else
	{

		vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, m_fBurstSpread, 8192, 2, BULLET_PLAYER_556MM, 30, 0.96, m_pPlayer->pev, false, m_pPlayer->random_seed);
		m_pPlayer->ammo_556nato--;

		PLAYBACK_EVENT_FULL(flag, m_pPlayer->edict(), m_usFireFamas, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y,
			(int)(m_pPlayer->pev->punchangle.x * 10000000), (int)(m_pPlayer->pev->punchangle.y * 10000000), FALSE, FALSE);
	}

	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
	shotsFired++;

	float nexttime = 0;

	if (shotsFired != 3)
	{
		nexttime = gpGlobals->time + 0.1f;
	}

	shootTime = nexttime;
}

/* <1d389e> ../cstrike/dlls/weapons.cpp:876 */
BOOL CanAttack(float attack_time, float curtime, BOOL isPredicted)
{
#ifdef CLIENT_WEAPONS
	if (!isPredicted)
#else
	if (1)
#endif // CLIENT_WEAPONS
	{
		return (attack_time <= curtime) ? TRUE : FALSE;
	}
	else
	{
		return (attack_time <= 0.0f) ? TRUE : FALSE;
	}
}

/* <1d38f0> ../cstrike/dlls/weapons.cpp:890 */
bool CBasePlayerWeapon::HasSecondaryAttack(void)
{
	if (m_pPlayer->HasShield())
	{
		return true;
	}

	switch (m_iId)
	{
	case WEAPON_AK47:
	case WEAPON_XM1014:
	case WEAPON_MAC10:
	case WEAPON_ELITE:
	case WEAPON_FIVESEVEN:
	case WEAPON_MP5N:
	case WEAPON_M249:
	case WEAPON_M3:
	case WEAPON_TMP:
	case WEAPON_DEAGLE:
	case WEAPON_P228:
	case WEAPON_P90:
	case WEAPON_C4:
	case WEAPON_GALIL:
		return false;
	default:
		break;
	}

	return true;
}

/* <1d3919> ../cstrike/dlls/weapons.cpp:915 */
void CBasePlayerWeapon::__MAKE_VHOOK(ItemPostFrame)(void)
{
	int usableButtons = m_pPlayer->pev->button;

	if (!HasSecondaryAttack())
	{
		usableButtons &= ~IN_ATTACK2;
	}

	if (m_flGlock18Shoot != 0)
	{
		FireRemaining(m_iGlock18ShotsFired, m_flGlock18Shoot, TRUE);
	}
	else if (gpGlobals->time > m_flFamasShoot && m_flFamasShoot != 0)
	{
		FireRemaining(m_iFamasShotsFired, m_flFamasShoot, FALSE);
	}

	if (m_flNextPrimaryAttack <= UTIL_WeaponTimeBase())
	{
		if (m_pPlayer->m_bResumeZoom)
		{
			m_pPlayer->m_iFOV = m_pPlayer->m_iLastZoom;
			m_pPlayer->pev->fov = m_pPlayer->m_iFOV;

			if (m_pPlayer->m_iFOV == m_pPlayer->m_iLastZoom)
			{
				m_pPlayer->m_bResumeZoom = false;
			}
		}
	}

	if (m_pPlayer->m_flEjectBrass != 0 && m_pPlayer->m_flEjectBrass <= gpGlobals->time)
	{
		m_pPlayer->m_flEjectBrass = 0;
		EjectBrassLate();
	}

	if (!(m_pPlayer->pev->button & IN_ATTACK))
	{
		m_flLastFireTime = 0;
	}

	if (m_pPlayer->HasShield())
	{
		if (m_fInReload && (m_pPlayer->pev->button & IN_ATTACK2))
		{
			SecondaryAttack();
			m_pPlayer->pev->button &= ~IN_ATTACK2;
			m_fInReload = FALSE;
			m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase();
		}
	}

	if (m_fInReload && m_pPlayer->m_flNextAttack <= UTIL_WeaponTimeBase())
	{
		// complete the reload.
		int j = Q_min(iMaxClip() - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);

		// Add them to the clip
		m_iClip += j;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= j;
		m_pPlayer->TabulateAmmo();
		m_fInReload = FALSE;
	}

	if ((usableButtons & IN_ATTACK2) && CanAttack(m_flNextSecondaryAttack, UTIL_WeaponTimeBase(), UseDecrement()))
	{
		if (pszAmmo2() && !m_pPlayer->m_rgAmmo[SecondaryAmmoIndex()])
		{
			m_fFireOnEmpty = TRUE;
		}

		SecondaryAttack();
		m_pPlayer->pev->button &= ~IN_ATTACK2;
	}
	else if ((m_pPlayer->pev->button & IN_ATTACK) && CanAttack(m_flNextPrimaryAttack, UTIL_WeaponTimeBase(), UseDecrement()))
	{
		if ((m_iClip == 0 && pszAmmo1()) || (iMaxClip() == WEAPON_NOCLIP && !m_pPlayer->m_rgAmmo[PrimaryAmmoIndex()]))
		{
			m_fFireOnEmpty = TRUE;
		}

		m_pPlayer->TabulateAmmo();

		if ((m_pPlayer->m_bCanShoot && g_pGameRules->IsMultiplayer() && !g_pGameRules->IsFreezePeriod() && !m_pPlayer->m_bIsDefusing) || !g_pGameRules->IsMultiplayer())
		{
			PrimaryAttack();
		}
	}
	else if ((m_pPlayer->pev->button & IN_RELOAD) && iMaxClip() != WEAPON_NOCLIP && !m_fInReload && m_flNextPrimaryAttack < UTIL_WeaponTimeBase())
	{
		if (m_flFamasShoot == 0 && m_flGlock18Shoot == 0)
		{
			if (!(m_iWeaponState & WPNSTATE_SHIELD_DRAWN))
			{
				// reload when reload is pressed, or if no buttons are down and weapon is empty.
				Reload();
			}
		}
	}
	else if (!(usableButtons & (IN_ATTACK | IN_ATTACK2)))
	{
		// no fire buttons down
		if (m_bDelayFire)
		{
			m_bDelayFire = false;

			if (m_iShotsFired > 15)
			{
				m_iShotsFired = 15;
			}

			m_flDecreaseShotsFired = gpGlobals->time + 0.4f;
		}

		m_fFireOnEmpty = FALSE;

		if (m_iId != WEAPON_USP && m_iId != WEAPON_GLOCK18 && m_iId != WEAPON_P228 && m_iId != WEAPON_DEAGLE && m_iId != WEAPON_ELITE && m_iId != WEAPON_FIVESEVEN)
		{
			if (m_iShotsFired > 0 && m_flDecreaseShotsFired < gpGlobals->time)
			{
				m_iShotsFired--;
				m_flDecreaseShotsFired = gpGlobals->time + 0.0225;
			}
		}
		else
			m_iShotsFired = 0;

		if (!IsUseable() && m_flNextPrimaryAttack < UTIL_WeaponTimeBase())
		{
#if 0
			// weapon isn't useable, switch.
			if (!(iFlags() & ITEM_FLAG_NOAUTOSWITCHEMPTY) && g_pGameRules->GetNextBestWeapon(m_pPlayer, this))
			{
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.3;
				return;
			}
#endif
		}
		else
		{
			if (!(m_iWeaponState & WPNSTATE_SHIELD_DRAWN))
			{
				// weapon is useable. Reload if empty and weapon has waited as long as it has to after firing
				if (!m_iClip && !(iFlags() & ITEM_FLAG_NOAUTORELOAD) && m_flNextPrimaryAttack < UTIL_WeaponTimeBase())
				{
					if (m_flFamasShoot == 0 && m_flGlock18Shoot == 0)
					{
						Reload();
						return;
					}
				}
			}
		}

		WeaponIdle();
		return;
	}

	// catch all
	if (ShouldWeaponIdle())
	{
		WeaponIdle();
	}
}

/* <1d3aac> ../cstrike/dlls/weapons.cpp:1069 */
void CBasePlayerItem::DestroyItem(void)
{
	if (m_pPlayer != NULL)
	{
		// if attached to a player, remove.
		m_pPlayer->RemovePlayerItem(this);
	}

	Kill();
}

/* <1d17d4> ../cstrike/dlls/weapons.cpp:1081 */
int CBasePlayerItem::__MAKE_VHOOK(AddToPlayer)(CBasePlayer *pPlayer)
{
	m_pPlayer = pPlayer;

	MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev);
		WRITE_BYTE(m_iId);
	MESSAGE_END();

	return TRUE;
}

/* <1d183d> ../cstrike/dlls/weapons.cpp:1092 */
void CBasePlayerItem::__MAKE_VHOOK(Drop)(void)
{
	SetTouch(NULL);
	SetThink(&CBaseEntity::SUB_Remove);
	pev->nextthink = gpGlobals->time + 0.1f;
}

/* <1d1866> ../cstrike/dlls/weapons.cpp:1099 */
void CBasePlayerItem::__MAKE_VHOOK(Kill)(void)
{
	SetTouch(NULL);
	SetThink(&CBaseEntity::SUB_Remove);
	pev->nextthink = gpGlobals->time + 0.1f;
}

/* <1d188f> ../cstrike/dlls/weapons.cpp:1106 */
void CBasePlayerItem::__MAKE_VHOOK(Holster)(int skiplocal)
{
	m_pPlayer->pev->viewmodel = 0;
	m_pPlayer->pev->weaponmodel = 0;
}

/* <1d18c7> ../cstrike/dlls/weapons.cpp:1112 */
void CBasePlayerItem::__MAKE_VHOOK(AttachToPlayer)(CBasePlayer *pPlayer)
{
	pev->movetype = MOVETYPE_FOLLOW;
	pev->solid = SOLID_NOT;
	pev->aiment = pPlayer->edict();
	pev->effects = EF_NODRAW;

	// server won't send down to clients if modelindex == 0
	pev->modelindex = 0;
	pev->model = 0;
	pev->owner = pPlayer->edict();
	pev->nextthink = gpGlobals->time + 0.1;

	SetTouch(NULL);
}

// CALLED THROUGH the newly-touched weapon's instance. The existing player weapon is pOriginal

/* <1d1e4a> ../cstrike/dlls/weapons.cpp:1126 */
int CBasePlayerWeapon::__MAKE_VHOOK(AddDuplicate)(CBasePlayerItem *pOriginal)
{
	if (m_iDefaultAmmo)
	{
		return ExtractAmmo((CBasePlayerWeapon *)pOriginal);
	}
	else
	{
		// a dead player dropped this.
		return ExtractClipAmmo((CBasePlayerWeapon *)pOriginal);
	}
}

/* <1d237f> ../cstrike/dlls/weapons.cpp:1140 */
int CBasePlayerWeapon::__MAKE_VHOOK(AddToPlayer)(CBasePlayer *pPlayer)
{
	m_pPlayer = pPlayer;
	pPlayer->pev->weapons |= (1 << m_iId);

	if (!m_iPrimaryAmmoType)
	{
		m_iPrimaryAmmoType = pPlayer->GetAmmoIndex(pszAmmo1());
		m_iSecondaryAmmoType = pPlayer->GetAmmoIndex(pszAmmo2());
	}

	if (AddWeapon())
	{
		return CBasePlayerItem::AddToPlayer(pPlayer);
	}

	return FALSE;
}

/* <1d24a3> ../cstrike/dlls/weapons.cpp:1164 */
int CBasePlayerWeapon::__MAKE_VHOOK(UpdateClientData)(CBasePlayer *pPlayer)
{
	BOOL bSend = FALSE;
	int state = 0;

	if (pPlayer->m_pActiveItem == this)
	{
		if (pPlayer->m_fOnTarget)
			state = WEAPON_IS_ONTARGET;
		else
			state = 1;
	}

	if (!pPlayer->m_fWeapon)
		bSend = TRUE;

	if (this == pPlayer->m_pActiveItem || this == pPlayer->m_pClientActiveItem)
	{
		if (pPlayer->m_pActiveItem != pPlayer->m_pClientActiveItem)
			bSend = TRUE;
	}

	if (m_iClip != m_iClientClip || state != m_iClientWeaponState || pPlayer->m_iFOV != pPlayer->m_iClientFOV)
		bSend = TRUE;

	if (bSend)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgCurWeapon, NULL, pPlayer->pev);
			WRITE_BYTE(state);
			WRITE_BYTE(m_iId);
			WRITE_BYTE(m_iClip);
		MESSAGE_END();

		m_iClientClip = m_iClip;
		m_iClientWeaponState = state;
		pPlayer->m_fWeapon = TRUE;
	}

	if (m_pNext != NULL)
	{
		m_pNext->UpdateClientData(pPlayer);
	}

	return 1;
}

/* <1d22c7> ../cstrike/dlls/weapons.cpp:1218 */
void CBasePlayerWeapon::__MAKE_VHOOK(SendWeaponAnim)(int iAnim, int skiplocal)
{
	m_pPlayer->pev->weaponanim = iAnim;

#ifdef CLIENT_WEAPONS
	if (skiplocal && ENGINE_CANSKIP(m_pPlayer->edict()))
		return;
#endif // CLIENT_WEAPONS

	MESSAGE_BEGIN(MSG_ONE, SVC_WEAPONANIM, NULL, m_pPlayer->pev);
		WRITE_BYTE(iAnim);	// sequence number
		WRITE_BYTE(pev->body);		// weaponmodel bodygroup.
	MESSAGE_END();
}

/* <1d3ad5> ../cstrike/dlls/weapons.cpp:1231 */
BOOL CBasePlayerWeapon::AddPrimaryAmmo(int iCount, char *szName, int iMaxClip, int iMaxCarry)
{
	int iIdAmmo;

	if (iMaxClip < 1)
	{
		m_iClip = -1;
		iIdAmmo = m_pPlayer->GiveAmmo(iCount, szName, iMaxCarry);
	}
	else if (m_iClip == 0)
	{
		int i;
		i = Q_min(m_iClip + iCount, iMaxClip);
		m_iClip += i;

		iIdAmmo = m_pPlayer->GiveAmmo(iCount - i, szName, iMaxCarry);
	}
	else
	{
		iIdAmmo = m_pPlayer->GiveAmmo(iCount, szName, iMaxCarry);
	}

	if (iIdAmmo > 0)
	{
		m_iPrimaryAmmoType = iIdAmmo;
		if (m_pPlayer->HasPlayerItem(this))
		{
			// play the "got ammo" sound only if we gave some ammo to a player that already had this gun.
			// if the player is just getting this gun for the first time, DefaultTouch will play the "picked up gun" sound for us.
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
		}
	}

	return iIdAmmo > 0 ? TRUE : FALSE;
}

/* <1d3cd7> ../cstrike/dlls/weapons.cpp:1267 */
BOOL CBasePlayerWeapon::AddSecondaryAmmo(int iCount, char *szName, int iMax)
{
	int iIdAmmo;

	iIdAmmo = m_pPlayer->GiveAmmo(iCount, szName, iMax);

	if (iIdAmmo > 0)
	{
		m_iSecondaryAmmoType = iIdAmmo;
		EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	}

	return iIdAmmo > 0 ? TRUE : FALSE;
}

// IsUseable - this function determines whether or not a
// weapon is useable by the player in its current state.
// (does it have ammo loaded? do I have any ammo for the
// weapon?, etc)

/* <1d19ba> ../cstrike/dlls/weapons.cpp:1287 */
BOOL CBasePlayerWeapon::__MAKE_VHOOK(IsUseable)(void)
{
	if (m_iClip <= 0)
	{
		if (m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] <= 0 && iMaxAmmo1() != -1)
		{
			// clip is empty (or nonexistant) and the player has no more ammo of this type.
			return FALSE;
		}
	}

	return TRUE;
}

/* <1d19f4> ../cstrike/dlls/weapons.cpp:1301 */
BOOL CBasePlayerWeapon::__MAKE_VHOOK(CanDeploy)(void)
{
	return TRUE;
}

/* <1d3d7a> ../cstrike/dlls/weapons.cpp:1306 */
BOOL CBasePlayerWeapon::DefaultDeploy(char *szViewModel, char *szWeaponModel, int iAnim, char *szAnimExt, int skiplocal)
{
	if (!CanDeploy())
		return FALSE;

	m_pPlayer->TabulateAmmo();
	m_pPlayer->pev->viewmodel = MAKE_STRING(szViewModel);
	m_pPlayer->pev->weaponmodel = MAKE_STRING(szWeaponModel);
	model_name = m_pPlayer->pev->viewmodel;
	Q_strcpy(m_pPlayer->m_szAnimExtention, szAnimExt);
	SendWeaponAnim(iAnim, skiplocal);

	m_pPlayer->m_flNextAttack = 0.75f;
	m_flTimeWeaponIdle = 1.5f;
	m_flLastFireTime = 0.0f;
	m_flDecreaseShotsFired = gpGlobals->time;

	m_pPlayer->m_iFOV = DEFAULT_FOV;
	m_pPlayer->pev->fov = DEFAULT_FOV;
	m_pPlayer->m_iLastZoom = DEFAULT_FOV;
	m_pPlayer->m_bResumeZoom = false;

	return TRUE;
}

/* <1d3df3> ../cstrike/dlls/weapons.cpp:1333 */
void CBasePlayerWeapon::ReloadSound(void)
{
	Vector newVector;
	Vector origin;
	CBaseEntity *pPlayer = NULL;
	float distance;

	m_pPlayer->pev->origin.CopyToArray(origin);

	while ((pPlayer = (CBasePlayer *)UTIL_FindEntityByClassname(pPlayer, "player")) != NULL)
	{
		if (pPlayer->IsDormant())
			break;

		if (pPlayer == m_pPlayer)
			continue;

		newVector = origin - pPlayer->pev->origin;
		distance = newVector.Length();

		if (distance <= 512.0f)
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgReloadSound, NULL, pPlayer->pev);
				WRITE_BYTE((int)((1.0f - (distance / 512.0f)) * 255.0f));
			if (!Q_strcmp(STRING(pev->classname), "weapon_m3") || !Q_strcmp(STRING(pev->classname), "weapon_xm1014"))
				WRITE_BYTE(0);
			else
				WRITE_BYTE(1);
			MESSAGE_END();
		}
	}
}

/* <1d3f0c> ../cstrike/dlls/weapons.cpp:1366 */
int CBasePlayerWeapon::DefaultReload(int iClipSize, int iAnim, float fDelay)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return FALSE;

	int j = Q_min(iClipSize - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);

	if (!j)
	{
		return FALSE;
	}

	m_pPlayer->m_flNextAttack = fDelay;

	ReloadSound();
	SendWeaponAnim(iAnim, UseDecrement() ? 1 : 0);

	m_fInReload = TRUE;
	m_flTimeWeaponIdle = fDelay + 0.5f;

	return TRUE;
}

/* <1d2607> ../cstrike/dlls/weapons.cpp:1389 */
BOOL CBasePlayerWeapon::__MAKE_VHOOK(PlayEmptySound)(void)
{
	if (m_iPlayEmptySound)
	{
		switch (m_iId)
		{
		case WEAPON_USP:
		case WEAPON_GLOCK18:
		case WEAPON_P228:
		case WEAPON_DEAGLE:
		case WEAPON_ELITE:
		case WEAPON_FIVESEVEN:
			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dryfire_pistol.wav", 0.8, ATTN_NORM);
			break;
		default:
			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/dryfire_rifle.wav", 0.8, ATTN_NORM);
			break;
		}
	}

	return FALSE;
}

/* <1d1a1c> ../cstrike/dlls/weapons.cpp:1414 */
void CBasePlayerWeapon::__MAKE_VHOOK(ResetEmptySound)(void)
{
	m_iPlayEmptySound = 1;
}

/* <1d1a44> ../cstrike/dlls/weapons.cpp:1421 */
int CBasePlayerWeapon::__MAKE_VHOOK(PrimaryAmmoIndex)(void)
{
	return m_iPrimaryAmmoType;
}

/* <1d1a6c> ../cstrike/dlls/weapons.cpp:1428 */
int CBasePlayerWeapon::__MAKE_VHOOK(SecondaryAmmoIndex)(void)
{
	return -1;
}

/* <1d1a94> ../cstrike/dlls/weapons.cpp:1433 */
void CBasePlayerWeapon::__MAKE_VHOOK(Holster)(int skiplocal)
{
	// cancel any reload in progress.
	m_fInReload = FALSE;
	m_pPlayer->pev->viewmodel = 0;
	m_pPlayer->pev->weaponmodel = 0;
}

/* <1d20fb> ../cstrike/dlls/weapons.cpp:1440 */
void CBasePlayerAmmo::__MAKE_VHOOK(Spawn)(void)
{
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;
	UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 16));
	UTIL_SetOrigin(pev, pev->origin);

	SetTouch(&CBasePlayerAmmo::DefaultTouch);

	if (g_pGameRules->IsMultiplayer())
	{
		SetThink(&CBaseEntity::SUB_Remove);
		pev->nextthink = gpGlobals->time + 2.0f;
	}
}

/* <1d1d10> ../cstrike/dlls/weapons.cpp:1458 */
CBaseEntity *CBasePlayerAmmo::__MAKE_VHOOK(Respawn)(void)
{
	pev->effects |= EF_NODRAW;
	SetTouch(NULL);

	// move to wherever I'm supposed to repawn.
	UTIL_SetOrigin(pev, g_pGameRules->VecAmmoRespawnSpot(this));

	SetThink(&CBasePlayerAmmo::Materialize);
	pev->nextthink = g_pGameRules->FlAmmoRespawnTime(this);

	return this;
}

/* <1d1de0> ../cstrike/dlls/weapons.cpp:1471 */
void CBasePlayerAmmo::Materialize(void)
{
	if (pev->effects & EF_NODRAW)
	{
		// changing from invisible state to visible.
		if (g_pGameRules->IsMultiplayer())
		{
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "items/suitchargeok1.wav", VOL_NORM, ATTN_NORM, 0, 150);
		}

		pev->effects &= ~EF_NODRAW;
		pev->effects |= EF_MUZZLEFLASH;
	}

	SetTouch(&CBasePlayerAmmo::DefaultTouch);
}

/* <1d1e94> ../cstrike/dlls/weapons.cpp:1488 */
void CBasePlayerAmmo::DefaultTouch(CBaseEntity *pOther)
{
	if (!pOther->IsPlayer())
		return;

	if (AddAmmo(pOther))
	{
		if (g_pGameRules->AmmoShouldRespawn(this) == GR_AMMO_RESPAWN_YES)
		{
			Respawn();
		}
		else
		{
			SetTouch(NULL);
			SetThink(&CBaseEntity::SUB_Remove);
			pev->nextthink = gpGlobals->time + 0.1f;
		}
	}
	else if (gEvilImpulse101)
	{
		// evil impulse 101 hack, kill always
		SetTouch(NULL);
		SetThink(&CBaseEntity::SUB_Remove);
		pev->nextthink = gpGlobals->time + 0.1f;
	}
}

// called by the new item with the existing item as parameter
//
// if we call ExtractAmmo(), it's because the player is picking up this type of weapon for
// the first time. If it is spawned by the world, m_iDefaultAmmo will have a default ammo amount in it.
// if  this is a weapon dropped by a dying player, has 0 m_iDefaultAmmo, which means only the ammo in
// the weapon clip comes along.

/* <1d3bc3> ../cstrike/dlls/weapons.cpp:1525 */
int CBasePlayerWeapon::__MAKE_VHOOK(ExtractAmmo)(CBasePlayerWeapon *pWeapon)
{
	int iReturn = 0;

	if (pszAmmo1() != NULL)
	{
		// blindly call with m_iDefaultAmmo. It's either going to be a value or zero. If it is zero,
		// we only get the ammo in the weapon's clip, which is what we want.
		iReturn = pWeapon->AddPrimaryAmmo(m_iDefaultAmmo, (char *)pszAmmo1(), iMaxClip(), iMaxAmmo1());
		m_iDefaultAmmo = 0;
	}

	if (pszAmmo2() != NULL)
	{
		iReturn = AddSecondaryAmmo(0, (char *)pszAmmo2(), iMaxAmmo2());
	}

	return iReturn;
}

// called by the new item's class with the existing item as parameter

/* <1d1acc> ../cstrike/dlls/weapons.cpp:1548 */
int CBasePlayerWeapon::__MAKE_VHOOK(ExtractClipAmmo)(CBasePlayerWeapon *pWeapon)
{
	int iAmmo;
	if (m_iClip == WEAPON_NOCLIP)
	{
		// guns with no clips always come empty if they are second-hand
		iAmmo = 0;
	}
	else
	{
		iAmmo = m_iClip;
	}

	return pWeapon->m_pPlayer->GiveAmmo(iAmmo, (char *)pszAmmo1(), iMaxAmmo1());
}

// RetireWeapon - no more ammo for this gun, put it away.

/* <1d1b2e> ../cstrike/dlls/weapons.cpp:1567 */
void CBasePlayerWeapon::__MAKE_VHOOK(RetireWeapon)(void)
{
	// first, no viewmodel at all.
	m_pPlayer->pev->viewmodel = iStringNull;
	m_pPlayer->pev->weaponmodel = iStringNull;

	g_pGameRules->GetNextBestWeapon(m_pPlayer, this);
}

// GetNextAttackDelay - An accurate way of calcualting the next attack time.

/* <1d3f76> ../cstrike/dlls/weapons.cpp:1580 */
float CBasePlayerWeapon::GetNextAttackDelay(float delay)
{
	if (m_flLastFireTime == 0.0f || m_flNextPrimaryAttack == -1.0f)
	{
		// At this point, we are assuming that the client has stopped firing
		// and we are going to reset our book keeping variables.
		m_flPrevPrimaryAttack = delay;
		m_flLastFireTime = gpGlobals->time;
	}

#ifdef REGAMEDLL_BUILD_6153

	// TODO: Build 6xxx
	// at build 6153 beta this removed
	// maybe it was initiated due to the delay of the shot

	// calculate the time between this shot and the previous
	float flTimeBetweenFires = gpGlobals->time - m_flLastFireTime;
	float flCreep = 0.0f;

	if (flTimeBetweenFires > 0)
	{
		flCreep = flTimeBetweenFires - m_flPrevPrimaryAttack;
	}

	float flNextAttack = UTIL_WeaponTimeBase() + delay - flCreep;
#else
	float flNextAttack = UTIL_WeaponTimeBase() + delay;
#endif // REGAMEDLL_BUILD_6153

	// save the last fire time
	m_flLastFireTime = gpGlobals->time;

	// we need to remember what the m_flNextPrimaryAttack time is set to for each shot,
	// store it as m_flPrevPrimaryAttack.
	m_flPrevPrimaryAttack = flNextAttack - UTIL_WeaponTimeBase();

	return flNextAttack;
}

/* <1d3fe8> ../cstrike/dlls/weapons.cpp:1614 */
LINK_ENTITY_TO_CLASS(weaponbox, CWeaponBox);

/* <1d2002> ../cstrike/dlls/weapons.cpp:1624 */
IMPLEMENT_SAVERESTORE(CWeaponBox, CBaseEntity);

/* <1d1b57> ../cstrike/dlls/weapons.cpp:1629 */
void CWeaponBox::__MAKE_VHOOK(Precache)(void)
{
	PRECACHE_MODEL("models/w_weaponbox.mdl");
}

/* <1d2978> ../cstrike/dlls/weapons.cpp:1636 */
void CWeaponBox::__MAKE_VHOOK(KeyValue)(KeyValueData *pkvd)
{
	if (m_cAmmoTypes >= MAX_AMMO_SLOTS)
	{
		ALERT(at_console, "WeaponBox too full! only %d ammotypes allowed\n", MAX_AMMO_SLOTS);
		return;
	}

	PackAmmo(ALLOC_STRING(pkvd->szKeyName), Q_atoi(pkvd->szValue));

	// count this new ammo type.
	m_cAmmoTypes++;
	pkvd->fHandled = TRUE;
}

/* <1d48ba> ../cstrike/dlls/weapons.cpp:1652 */
void CWeaponBox::BombThink(void)
{
	if (!m_bIsBomb)
		return;

	CBaseEntity *pEntity = NULL;
	while ((pEntity = UTIL_FindEntityByClassname(pEntity, "player")) != NULL)
	{
		if (FNullEnt(pEntity->edict()))
			break;

		if (!pEntity->IsPlayer() || pEntity->IsDormant())
			continue;

		CBasePlayer *pTempPlayer = GetClassPtr((CBasePlayer *)pEntity->pev);

		if (pTempPlayer->pev->deadflag == DEAD_NO && pTempPlayer->m_iTeam == TERRORIST)
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgBombDrop, NULL, pTempPlayer->edict());
				WRITE_COORD(pev->origin.x);
				WRITE_COORD(pev->origin.y);
				WRITE_COORD(pev->origin.z);
				WRITE_BYTE(0);
			MESSAGE_END();
		}
	}

	pev->nextthink = gpGlobals->time + 1;
}

/* <1d1ce7> ../cstrike/dlls/weapons.cpp:1687 */
void CWeaponBox::__MAKE_VHOOK(Spawn)(void)
{
	Precache();

	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;

	m_bIsBomb = false;

	UTIL_SetSize(pev, g_vecZero, g_vecZero);
	SET_MODEL(ENT(pev), "models/w_weaponbox.mdl");
}

// CWeaponBox - Kill - the think function that removes the
// box from the world.

/* <1d40c4> ../cstrike/dlls/weapons.cpp:1704 */
void CWeaponBox::Kill(void)
{
	CBasePlayerItem *pWeapon;
	int i;

	// destroy the weapons
	for (i = 0; i < MAX_ITEM_TYPES; i++)
	{
		pWeapon = m_rgpPlayerItems[i];

		while (pWeapon != NULL)
		{
			pWeapon->SetThink(&CBaseEntity::SUB_Remove);
			pWeapon->pev->nextthink = gpGlobals->time + 0.1f;
			pWeapon = pWeapon->m_pNext;
		}
	}

	// remove the box
	UTIL_Remove(this);
}

// CWeaponBox - Touch: try to add my contents to the toucher
// if the toucher is a player.

/* <1d0640> ../cstrike/dlls/weapons.cpp:1732 */
void CWeaponBox::__MAKE_VHOOK(Touch)(CBaseEntity *pOther)
{
	if (!(pev->flags & FL_ONGROUND))
	{
		return;
	}

	if (!pOther->IsPlayer())
	{
		// only players may touch a weaponbox.
		return;
	}

	if (!pOther->IsAlive())
	{
		// no dead guys.
		return;
	}

	CBasePlayer *pPlayer = reinterpret_cast<CBasePlayer *>(pOther);

	if (pPlayer->m_bIsVIP || pPlayer->m_bShieldDrawn)
		return;

	pPlayer->OnTouchingWeapon(this);

	bool bRemove = true;
	bool bEmitSound = false;

	// go through my weapons and try to give the usable ones to the player.
	// it's important the the player be given ammo first, so the weapons code doesn't refuse
	// to deploy a better weapon that the player may pick up because he has no ammo for it.
	for (int i = 0; i < MAX_ITEM_TYPES; i++)
	{
		if (!m_rgpPlayerItems[i])
		{
			continue;
		}

		CBasePlayerItem *pItem = m_rgpPlayerItems[i];

		// have at least one weapon in this slot
		while (pItem != NULL)
		{
			CCSBotManager *ctrl = TheCSBots();

			if ((pPlayer->HasShield() && pItem->m_iId == WEAPON_ELITE)
				|| (pPlayer->IsBot() && !ctrl->IsWeaponUseable(pItem)))
			{
				return;
			}

			if (FClassnameIs(pItem->pev, "weapon_c4"))
			{
				if (pPlayer->m_iTeam != TERRORIST || pPlayer->pev->deadflag != DEAD_NO)
					return;

				if (pPlayer->m_bShowHints && !(pPlayer->m_flDisplayHistory & DHF_BOMB_RETRIEVED))
				{
					pPlayer->m_flDisplayHistory |= DHF_BOMB_RETRIEVED;
					pPlayer->HintMessage("#Hint_you_have_the_bomb");
				}
				else
					ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "#Got_bomb");

				UTIL_LogPrintf("\"%s<%i><%s><TERRORIST>\" triggered \"Got_The_Bomb\"\n",
					STRING(pPlayer->pev->netname),
					GETPLAYERUSERID(pPlayer->edict()),
					GETPLAYERAUTHID(pPlayer->edict()));

				g_pGameRules->m_bBombDropped = FALSE;

				MESSAGE_BEGIN(MSG_SPEC, SVC_DIRECTOR);
					WRITE_BYTE(9);
					WRITE_BYTE(DRC_CMD_EVENT);
					WRITE_SHORT(ENTINDEX(pPlayer->edict()));
					WRITE_SHORT(ENTINDEX(edict()));
					WRITE_LONG(6);
				MESSAGE_END();

				pPlayer->m_bHasC4 = true;
				pPlayer->SetBombIcon(FALSE);
				pPlayer->pev->body = 1;

				CBaseEntity *pEntity = NULL;

				while (pEntity = UTIL_FindEntityByClassname(pEntity, "player"))
				{
					if (FNullEnt(pEntity->edict()))
						break;

					if (!pEntity->IsPlayer())
						continue;

					if (pEntity->pev->flags == FL_DORMANT)
						continue;

					CBasePlayer *pTempPlayer = GetClassPtr((CBasePlayer *)pEntity->pev);

					if (pTempPlayer->pev->deadflag == DEAD_NO && pTempPlayer->m_iTeam == TERRORIST)
					{
						if (pTempPlayer != pPlayer)
						{
							ClientPrint(pTempPlayer->pev, HUD_PRINTCENTER, "#Game_bomb_pickup", STRING(pPlayer->pev->netname));
						}

						MESSAGE_BEGIN(MSG_ONE, gmsgBombPickup, NULL, pTempPlayer->pev);
						MESSAGE_END();
					}
				}

				CCSBotManager *csBots = TheCSBots();

				csBots->SetLooseBomb(NULL);
				TheBots->OnEvent(EVENT_BOMB_PICKED_UP, pPlayer);
			}

			if (i >= PRIMARY_WEAPON_SLOT && i <= PISTOL_SLOT && pPlayer->m_rgpPlayerItems[i] != NULL)
			{
				// ...
			}
			else if (i == GRENADE_SLOT)
			{
				if (m_rgpPlayerItems[i]->IsWeapon() && m_rgpPlayerItems[i])
				{
					CBasePlayerWeapon *pGrenade = reinterpret_cast<CBasePlayerWeapon *>(m_rgpPlayerItems[i]);
					int playerGrenades = pPlayer->m_rgAmmo[pGrenade->m_iPrimaryAmmoType];
					int maxGrenades = 0;
					const char *grenadeName = NULL;

					switch (pGrenade->m_iId)
					{
					case WEAPON_HEGRENADE:
						grenadeName = "weapon_hegrenade";
						maxGrenades = 1;
						break;
					case WEAPON_SMOKEGRENADE:
						grenadeName = "weapon_smokegrenade";
						maxGrenades = 1;
						break;
					case WEAPON_FLASHBANG:
						grenadeName = "weapon_flashbang";
						maxGrenades = 2;
						break;
					}

					if (playerGrenades < maxGrenades && grenadeName != NULL)
					{
						bEmitSound = true;
						pPlayer->GiveNamedItem(grenadeName);

						// unlink this weapon from the box
						pItem = m_rgpPlayerItems[i]->m_pNext;
						m_rgpPlayerItems[i] = pItem;

						continue;
					}
				}
			}
			else if (pPlayer->HasShield() && i == PRIMARY_WEAPON_SLOT)
			{
				// ...
			}
			else
			{
				if (pPlayer->AddPlayerItem(pItem))
				{
					pItem->AttachToPlayer(pPlayer);
					bEmitSound = true;
				}

				// unlink this weapon from the box
				pItem = m_rgpPlayerItems[i]->m_pNext;
				m_rgpPlayerItems[i] = pItem;

				continue;
			}

			bRemove = false;
			pItem = m_rgpPlayerItems[i]->m_pNext;
		}
	}

	if (bRemove)
	{
		// dole out ammo
		for (int n = 0; n < MAX_AMMO_SLOTS; n++)
		{
			if (!FStringNull(m_rgiszAmmo[n]))
			{
				// there's some ammo of this type.
				pPlayer->GiveAmmo(m_rgAmmo[n], (char *)STRING(m_rgiszAmmo[n]), MaxAmmoCarry(m_rgiszAmmo[n]));

				// now empty the ammo from the weaponbox since we just gave it to the player
				m_rgiszAmmo[n] = iStringNull;
				m_rgAmmo[n] = 0;
			}
		}
	}

	if (bEmitSound)
	{
		EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, "items/gunpickup2.wav", VOL_NORM, ATTN_NORM);
	}

	if (bRemove)
	{
		SetTouch(NULL);
		UTIL_Remove(this);
	}
}

// CWeaponBox - PackWeapon: Add this weapon to the box

/* <1d4148> ../cstrike/dlls/weapons.cpp:1981 */
BOOL CWeaponBox::PackWeapon(CBasePlayerItem *pWeapon)
{
	// is one of these weapons already packed in this box?
	if (HasWeapon(pWeapon))
	{
		// box can only hold one of each weapon type
		return FALSE;
	}

	if (pWeapon->m_pPlayer)
	{
		if (pWeapon->m_pPlayer->m_pActiveItem == pWeapon)
		{
			pWeapon->Holster();
		}

		if (!pWeapon->m_pPlayer->RemovePlayerItem(pWeapon))
		{
			// failed to unhook the weapon from the player!
			return FALSE;
		}
	}

	int iWeaponSlot = pWeapon->iItemSlot();
	if (m_rgpPlayerItems[iWeaponSlot])
	{
		// there's already one weapon in this slot, so link this into the slot's column
		pWeapon->m_pNext = m_rgpPlayerItems[iWeaponSlot];
		m_rgpPlayerItems[iWeaponSlot] = pWeapon;
	}
	else
	{
		// first weapon we have for this slot
		m_rgpPlayerItems[iWeaponSlot] = pWeapon;
		pWeapon->m_pNext = NULL;
	}

	// never respawn
	pWeapon->pev->spawnflags |= SF_NORESPAWN;
	pWeapon->pev->movetype = MOVETYPE_NONE;
	pWeapon->pev->solid = SOLID_NOT;
	pWeapon->pev->effects = EF_NODRAW;
	pWeapon->pev->modelindex = 0;
	pWeapon->pev->model = NULL;
	pWeapon->pev->owner = ENT(pev);
	pWeapon->SetThink(NULL);
	pWeapon->SetTouch(NULL);
	pWeapon->m_pPlayer = NULL;

	return TRUE;
}

/* <1d4224> ../cstrike/dlls/weapons.cpp:2036 */
int CWeaponBox::PackAmmo(int iszName, int iCount)
{
	if (!iszName)
	{
		// error here
		ALERT(at_console, "NULL String in PackAmmo!\n");
		return FALSE;
	}

	int iMaxCarry = MaxAmmoCarry(iszName);

	if (iMaxCarry != -1 && iCount > 0)
	{
		GiveAmmo(iCount, (char *)STRING(iszName), iMaxCarry);
		return TRUE;
	}

	return FALSE;
}

/* <1d426b> ../cstrike/dlls/weapons.cpp:2061 */
int CWeaponBox::GiveAmmo(int iCount, char *szName, int iMax, int *pIndex)
{
	int i;

	for (i = 1; i < MAX_AMMO_SLOTS && !FStringNull(m_rgiszAmmo[i]); i++)
	{
		if (!Q_stricmp(szName, STRING(m_rgiszAmmo[i])))
		{
			if (pIndex)
				*pIndex = i;

			int iAdd = Q_min(iCount, iMax - m_rgAmmo[i]);
			if (iCount == 0 || iAdd > 0)
			{
				m_rgAmmo[i] += iAdd;

				return i;
			}
			return -1;
		}
	}

	if (i < MAX_AMMO_SLOTS)
	{
		if (pIndex)
			*pIndex = i;

		m_rgiszAmmo[i] = MAKE_STRING(szName);
		m_rgAmmo[i] = iCount;

		return i;
	}

	ALERT(at_console, "out of named ammo slots\n");
	return i;
}

// CWeaponBox::HasWeapon - is a weapon of this type already
// packed in this box?

/* <1d42d5> ../cstrike/dlls/weapons.cpp:2100 */
BOOL CWeaponBox::HasWeapon(CBasePlayerItem *pCheckItem)
{
	CBasePlayerItem *pItem = m_rgpPlayerItems[pCheckItem->iItemSlot()];

	while (pItem != NULL)
	{
		if (FClassnameIs(pItem->pev, STRING(pCheckItem->pev->classname)))
		{
			return TRUE;
		}

		pItem = pItem->m_pNext;
	}

	return FALSE;
}

// CWeaponBox::IsEmpty - is there anything in this box?

/* <1d4354> ../cstrike/dlls/weapons.cpp:2119 */
BOOL CWeaponBox::IsEmpty(void)
{
	int i;

	for (i = 0; i < MAX_ITEM_TYPES; i++)
	{
		if (m_rgpPlayerItems[i])
		{
			return FALSE;
		}
	}

	for (i = 0; i < MAX_AMMO_SLOTS; i++)
	{
		if (m_rgiszAmmo[i])
		{
			// still have a bit of this type of ammo
			return FALSE;
		}
	}

	return TRUE;
}

/* <1d1b7f> ../cstrike/dlls/weapons.cpp:2145 */
void CWeaponBox::__MAKE_VHOOK(SetObjectCollisionBox)(void)
{
	pev->absmin = pev->origin + Vector(-16, -16, 0);
	pev->absmax = pev->origin + Vector(16, 16, 16);
}

/* <1d1d39> ../cstrike/dlls/weapons.cpp:2167 */
void CArmoury::__MAKE_VHOOK(Spawn)(void)
{
	Precache();
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;

	UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 16));
	UTIL_SetOrigin(pev, pev->origin);
	SetTouch(&CArmoury::ArmouryTouch);

	switch (m_iItem)
	{
	case ARMOURY_MP5NAVY:		SET_MODEL(ENT(pev), "models/w_mp5.mdl"); break;
	case ARMOURY_TMP:		SET_MODEL(ENT(pev), "models/w_tmp.mdl"); break;
	case ARMOURY_P90:		SET_MODEL(ENT(pev), "models/w_p90.mdl"); break;
	case ARMOURY_MAC10:		SET_MODEL(ENT(pev), "models/w_mac10.mdl"); break;
	case ARMOURY_AK47:		SET_MODEL(ENT(pev), "models/w_ak47.mdl"); break;
	case ARMOURY_SG552:		SET_MODEL(ENT(pev), "models/w_sg552.mdl"); break;
	case ARMOURY_M4A1:		SET_MODEL(ENT(pev), "models/w_m4a1.mdl"); break;
	case ARMOURY_AUG:		SET_MODEL(ENT(pev), "models/w_aug.mdl"); break;
	case ARMOURY_SCOUT:		SET_MODEL(ENT(pev), "models/w_scout.mdl"); break;
	case ARMOURY_G3SG1:		SET_MODEL(ENT(pev), "models/w_g3sg1.mdl"); break;
	case ARMOURY_AWP:		SET_MODEL(ENT(pev), "models/w_awp.mdl"); break;
	case ARMOURY_M3:		SET_MODEL(ENT(pev), "models/w_m3.mdl"); break;
	case ARMOURY_XM1014:		SET_MODEL(ENT(pev), "models/w_xm1014.mdl"); break;
	case ARMOURY_M249:		SET_MODEL(ENT(pev), "models/w_m249.mdl"); break;
	case ARMOURY_FLASHBANG:		SET_MODEL(ENT(pev), "models/w_flashbang.mdl"); break;
	case ARMOURY_HEGRENADE:		SET_MODEL(ENT(pev), "models/w_hegrenade.mdl"); break;
	case ARMOURY_KEVLAR:		SET_MODEL(ENT(pev), "models/w_kevlar.mdl"); break;
	case ARMOURY_ASSAULT:		SET_MODEL(ENT(pev), "models/w_assault.mdl"); break;
	case ARMOURY_SMOKEGRENADE:	SET_MODEL(ENT(pev), "models/w_smokegrenade.mdl"); break;
	default:			SET_MODEL(ENT(pev), "models/w_kevlar.mdl"); break;
	}

	if (m_iCount <= 0)
	{
		m_iCount = 1;
	}

	m_bAlreadyCounted = false;
	m_iInitialCount = m_iCount;
}

/* <1d1bfb> ../cstrike/dlls/weapons.cpp:2207 */
void CArmoury::__MAKE_VHOOK(Restart)(void)
{
	CHalfLifeMultiplay *mp = g_pGameRules;

	if (m_iItem == ARMOURY_FLASHBANG || m_iItem == ARMOURY_HEGRENADE)
	{
		if (!m_bAlreadyCounted)
		{
			m_bAlreadyCounted = true;
			mp->m_iTotalGrenadeCount += m_iInitialCount;
			m_iCount = m_iInitialCount;
			pev->effects &= ~EF_NODRAW;
			return;
		}

		float flRatio = (float_precision)(m_iInitialCount / mp->m_iTotalGrenadeCount) * (float_precision)mp->m_iNumTerrorist * 1.75;
		m_iCount = (int)flRatio;
	}
	else if (m_iItem == ARMOURY_KEVLAR || m_iItem == ARMOURY_ASSAULT)
	{
		if (!m_bAlreadyCounted)
		{
			m_bAlreadyCounted = true;
			mp->m_iTotalArmourCount += m_iInitialCount;
			m_iCount = m_iInitialCount;
			pev->effects &= ~EF_NODRAW;
			return;
		}

		float flRatio = (float_precision)(m_iInitialCount / mp->m_iTotalArmourCount) * (float_precision)mp->m_iNumTerrorist;
		m_iCount = (int)flRatio;
	}
	else
	{
		if (!m_bAlreadyCounted)
		{
			m_bAlreadyCounted = true;
			mp->m_iTotalGunCount += m_iInitialCount;
			m_iCount = m_iInitialCount;
			pev->effects &= ~EF_NODRAW;
			return;
		}

		float flRatio = (float_precision)(m_iInitialCount / mp->m_iTotalGunCount) * (float_precision)mp->m_iNumTerrorist * 0.85;
		m_iCount = (int)flRatio;
	}

	if (m_iCount < 1)
	{
		m_iCount = 1;
	}

	pev->effects &= ~EF_NODRAW;
}

/* <1d1ee9> ../cstrike/dlls/weapons.cpp:2268 */
void CArmoury::__MAKE_VHOOK(Precache)(void)
{
	switch (m_iItem)
	{
	case ARMOURY_MP5NAVY:		PRECACHE_MODEL("models/w_mp5.mdl"); break;
	case ARMOURY_TMP:		PRECACHE_MODEL("models/w_tmp.mdl"); break;
	case ARMOURY_P90:		PRECACHE_MODEL("models/w_p90.mdl"); break;
	case ARMOURY_MAC10:		PRECACHE_MODEL("models/w_mac10.mdl"); break;
	case ARMOURY_AK47:		PRECACHE_MODEL("models/w_ak47.mdl"); break;
	case ARMOURY_SG552:		PRECACHE_MODEL("models/w_sg552.mdl"); break;
	case ARMOURY_M4A1:		PRECACHE_MODEL("models/w_m4a1.mdl"); break;
	case ARMOURY_AUG:		PRECACHE_MODEL("models/w_aug.mdl"); break;
	case ARMOURY_SCOUT:		PRECACHE_MODEL("models/w_scout.mdl"); break;
	case ARMOURY_G3SG1:		PRECACHE_MODEL("models/w_g3sg1.mdl"); break;
	case ARMOURY_AWP:		PRECACHE_MODEL("models/w_awp.mdl"); break;
	case ARMOURY_M3:		PRECACHE_MODEL("models/w_m3.mdl"); break;
	case ARMOURY_XM1014:		PRECACHE_MODEL("models/w_xm1014.mdl"); break;
	case ARMOURY_M249:		PRECACHE_MODEL("models/w_m249.mdl"); break;
	case ARMOURY_FLASHBANG:		PRECACHE_MODEL("models/w_flashbang.mdl"); break;
	case ARMOURY_HEGRENADE:		PRECACHE_MODEL("models/w_hegrenade.mdl"); break;
	case ARMOURY_KEVLAR:		PRECACHE_MODEL("models/w_kevlar.mdl"); break;
	case ARMOURY_ASSAULT:		PRECACHE_MODEL("models/w_assault.mdl"); break;
	case ARMOURY_SMOKEGRENADE:	PRECACHE_MODEL("models/w_smokegrenade.mdl"); break;
	default:			PRECACHE_MODEL("models/w_kevlar.mdl"); break;
	}
}

/* <1d1f2f> ../cstrike/dlls/weapons.cpp:2294 */
void CArmoury::ArmouryTouch(CBaseEntity *pOther)
{
	if (!pOther->IsPlayer())
		return;

	CBasePlayer *p = reinterpret_cast<CBasePlayer *>(pOther);

	if (p->m_bIsVIP)
		return;

	if (m_iCount > 0 && m_iItem <= ARMOURY_M249)
	{
		if (p->m_bHasPrimary)
			return;

		m_iCount--;

		switch (m_iItem)
		{
		case ARMOURY_MP5NAVY:
			p->GiveNamedItem("weapon_mp5navy");
			p->GiveAmmo(60, "9mm", MAX_AMMO_9MM);
			break;
		case ARMOURY_TMP:
			p->GiveNamedItem("weapon_tmp");
			p->GiveAmmo(60, "9mm", MAX_AMMO_9MM);
			break;
		case ARMOURY_P90:
			p->GiveNamedItem("weapon_p90");
			p->GiveAmmo(50, "57mm", MAX_AMMO_57MM);
			break;
		case ARMOURY_MAC10:
			p->GiveNamedItem("weapon_mac10");
			p->GiveAmmo(60, "45acp", MAX_AMMO_45ACP);
			break;
		case ARMOURY_AK47:
			p->GiveNamedItem("weapon_ak47");
			p->GiveAmmo(60, "762Nato", MAX_AMMO_762NATO);
			break;
		case ARMOURY_SG552:
			p->GiveNamedItem("weapon_sg552");
			p->GiveAmmo(60, "556Nato", MAX_AMMO_556NATO);
			break;
		case ARMOURY_M4A1:
			p->GiveNamedItem("weapon_m4a1");
			p->GiveAmmo(60, "556Nato", MAX_AMMO_556NATO);
			break;
		case ARMOURY_AUG:
			p->GiveNamedItem("weapon_aug");
			p->GiveAmmo(60, "556Nato", MAX_AMMO_556NATO);
			break;
		case ARMOURY_SCOUT:
			p->GiveNamedItem("weapon_scout");
			p->GiveAmmo(30, "762Nato", MAX_AMMO_762NATO);
			break;
		case ARMOURY_G3SG1:
			p->GiveNamedItem("weapon_g3sg1");
			p->GiveAmmo(30, "762Nato", MAX_AMMO_762NATO);
			break;
		case ARMOURY_AWP:
			p->GiveNamedItem("weapon_awp");
			p->GiveAmmo(20, "338Magnum", MAX_AMMO_338MAGNUM);
			break;
		case ARMOURY_M3:
			p->GiveNamedItem("weapon_m3");
			p->GiveAmmo(24, "buckshot", MAX_AMMO_BUCKSHOT);
			break;
		case ARMOURY_XM1014:
			p->GiveNamedItem("weapon_xm1014");
			p->GiveAmmo(24, "buckshot", MAX_AMMO_BUCKSHOT);
			break;
		case ARMOURY_M249:
			p->GiveNamedItem("weapon_m249");
			p->GiveAmmo(60, "556NatoBox", MAX_AMMO_556NATOBOX);
			break;
		}
	}
	else if (m_iCount > 0 && m_iItem >= ARMOURY_FLASHBANG)
	{
		switch (m_iItem)
		{
			case ARMOURY_FLASHBANG:
			{
				if (p->AmmoInventory(p->GetAmmoIndex("Flashbang")) >= 2)
					return;

				p->GiveNamedItem("weapon_flashbang");
				m_iCount--;
				break;
			}
			case ARMOURY_HEGRENADE:
			{
				if (p->AmmoInventory(p->GetAmmoIndex("HEGrenade")) >= 1)
					return;

				p->GiveNamedItem("weapon_hegrenade");
				m_iCount--;
				break;
			}
			case ARMOURY_KEVLAR:
			{
				if (p->m_iKevlar == ARMOR_TYPE_KEVLAR)
					return;

				p->GiveNamedItem("item_kevlar");
				m_iCount--;
				break;
			}
			case ARMOURY_ASSAULT:
			{
				if (p->m_iKevlar == ARMOR_TYPE_HELMET)
					return;

				p->GiveNamedItem("item_assaultsuit");
				m_iCount--;
				break;
			}
			case ARMOURY_SMOKEGRENADE:
			{
				if (p->AmmoInventory(p->GetAmmoIndex("SmokeGrenade")) >= 1)
					return;

				p->GiveNamedItem("weapon_smokegrenade");
				m_iCount--;
				break;
			}
		}
	}

	if (!m_iCount)
	{
		pev->effects |= EF_NODRAW;
	}
}

/* <1d21bd> ../cstrike/dlls/weapons.cpp:2352 */
void CArmoury::__MAKE_VHOOK(KeyValue)(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "item"))
	{
		m_iItem = Q_atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "count"))
	{
		m_iCount = Q_atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

/* <1d4392> ../cstrike/dlls/weapons.cpp:2368 */
LINK_ENTITY_TO_CLASS(armoury_entity, CArmoury);

#ifdef HOOK_GAMEDLL

void CArmoury::Spawn(void)
{
	Spawn_();
}

void CArmoury::Precache(void)
{
	Precache_();
}

void CArmoury::Restart(void)
{
	Restart_();
}

void CArmoury::KeyValue(KeyValueData *pkvd)
{
	KeyValue_(pkvd);
}

void CBasePlayerAmmo::Spawn(void)
{
	Spawn_();
}

CBaseEntity *CBasePlayerAmmo::Respawn(void)
{
	return Respawn_();
}

int CBasePlayerWeapon::Save(CSave &save)
{
	return Save_(save);
}

int CBasePlayerWeapon::Restore(CRestore &restore)
{
	return Restore_(restore);
}

int CBasePlayerWeapon::AddToPlayer(CBasePlayer *pPlayer)
{
	return AddToPlayer_(pPlayer);
}

int CBasePlayerWeapon::AddDuplicate(CBasePlayerItem *pItem)
{
	return AddDuplicate_(pItem);
}

BOOL CBasePlayerWeapon::CanDeploy(void)
{
	return CanDeploy_();
}

void CBasePlayerWeapon::Holster(int skiplocal)
{
	Holster_(skiplocal);
}

void CBasePlayerWeapon::ItemPostFrame(void)
{
	ItemPostFrame_();
}

int CBasePlayerWeapon::PrimaryAmmoIndex(void)
{
	return PrimaryAmmoIndex_();
}

int CBasePlayerWeapon::SecondaryAmmoIndex(void)
{
	return SecondaryAmmoIndex_();
}

int CBasePlayerWeapon::UpdateClientData(CBasePlayer *pPlayer)
{
	return UpdateClientData_(pPlayer);
}

int CBasePlayerWeapon::ExtractAmmo(CBasePlayerWeapon *pWeapon)
{
	return ExtractAmmo_(pWeapon);
}

int CBasePlayerWeapon::ExtractClipAmmo(CBasePlayerWeapon *pWeapon)
{
	return ExtractClipAmmo_(pWeapon);
}

BOOL CBasePlayerWeapon::PlayEmptySound(void)
{
	return PlayEmptySound_();
}

void CBasePlayerWeapon::ResetEmptySound(void)
{
	ResetEmptySound_();
}

void CBasePlayerWeapon::SendWeaponAnim(int iAnim,int skiplocal)
{
	SendWeaponAnim_(iAnim,skiplocal);
}

BOOL CBasePlayerWeapon::IsUseable(void)
{
	return IsUseable_();
}

void CBasePlayerWeapon::RetireWeapon(void)
{
	RetireWeapon_();
}

int CBasePlayerItem::Save(CSave &save)
{
	return Save_(save);
}

int CBasePlayerItem::Restore(CRestore &restore)
{
	return Restore_(restore);
}

void CBasePlayerItem::SetObjectCollisionBox(void)
{
	SetObjectCollisionBox_();
}

CBaseEntity *CBasePlayerItem::Respawn(void)
{
	return Respawn_();
}

int CBasePlayerItem::AddToPlayer(CBasePlayer *pPlayer)
{
	return AddToPlayer_(pPlayer);
}

void CBasePlayerItem::Holster(int skiplocal)
{
	Holster_(skiplocal);
}

void CBasePlayerItem::Drop(void)
{
	Drop_();
}

void CBasePlayerItem::Kill(void)
{
	Kill_();
}

void CBasePlayerItem::AttachToPlayer(CBasePlayer *pPlayer)
{
	AttachToPlayer_(pPlayer);
}

void CWeaponBox::Spawn(void)
{
	Spawn_();
}

void CWeaponBox::Precache(void)
{
	Precache_();
}

void CWeaponBox::KeyValue(KeyValueData *pkvd)
{
	KeyValue_(pkvd);
}

int CWeaponBox::Save(CSave &save)
{
	return Save_(save);
}

int CWeaponBox::Restore(CRestore &restore)
{
	return Restore_(restore);
}

void CWeaponBox::SetObjectCollisionBox(void)
{
	SetObjectCollisionBox_();
}

void CWeaponBox::Touch(CBaseEntity *pOther)
{
	Touch_(pOther);
}

#endif // HOOK_GAMEDLL
