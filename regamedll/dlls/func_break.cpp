#include "precompiled.h"

/*
* Globals initialization
*/
#ifndef HOOK_GAMEDLL

// Just add more items to the bottom of this array and they will automagically be supported
// This is done instead of just a classname in the FGD so we can control which entities can
// be spawned, and still remain fairly flexible
const char *CBreakable::pSpawnObjects[] =
{
	NULL,
	"item_battery",
	"item_healthkit",
	"weapon_9mmhandgun",
	"ammo_9mmclip",
	"weapon_9mmAR",
	"ammo_9mmAR",
	"ammo_ARgrenades",
	"weapon_shotgun",
	"ammo_buckshot",
	"weapon_crossbow",
	"ammo_crossbow",
	"weapon_357",
	"ammo_357",
	"weapon_rpg",
	"ammo_rpgclip",
	"ammo_gaussclip",
	"weapon_handgrenade",
	"weapon_tripmine",
	"weapon_satchel",
	"weapon_snark",
	"weapon_hornetgun",
	"weapon_usp",
	"weapon_glock18",
	"weapon_awp",
	"weapon_mp5n",
	"weapon_m249",
	"weapon_m3",
	"weapon_m4a1",
	"weapon_tmp",
	"weapon_g3sg1",
	"weapon_flashbang"
};

const char *CBreakable::pSoundsWood[] =
{
	"debris/wood1.wav",
	"debris/wood2.wav",
	"debris/wood3.wav",
};

const char *CBreakable::pSoundsFlesh[] =
{
	"debris/flesh1.wav",
	"debris/flesh2.wav",
	"debris/flesh3.wav",
	"debris/flesh5.wav",
	"debris/flesh6.wav",
	"debris/flesh7.wav",
};

const char *CBreakable::pSoundsMetal[] =
{
	"debris/metal1.wav",
	"debris/metal2.wav",
	"debris/metal3.wav",
};

const char *CBreakable::pSoundsConcrete[] =
{
	"debris/concrete1.wav",
	"debris/concrete2.wav",
	"debris/concrete3.wav",
};

const char *CBreakable::pSoundsGlass[] =
{
	"debris/glass1.wav",
	"debris/glass2.wav",
	"debris/glass3.wav",
};

char *CPushable::m_soundNames[] =
{
	"debris/pushbox1.wav",
	"debris/pushbox2.wav",
	"debris/pushbox3.wav"
};

TYPEDESCRIPTION CBreakable::m_SaveData[] =
{
	DEFINE_FIELD(CBreakable, m_Material, FIELD_INTEGER),
	DEFINE_FIELD(CBreakable, m_Explosion, FIELD_INTEGER),
	DEFINE_FIELD(CBreakable, m_angle, FIELD_FLOAT),
	DEFINE_FIELD(CBreakable, m_iszGibModel, FIELD_STRING),
	DEFINE_FIELD(CBreakable, m_iszSpawnObject, FIELD_STRING),
};

TYPEDESCRIPTION CPushable::m_SaveData[] =
{
	DEFINE_FIELD(CPushable, m_maxSpeed, FIELD_FLOAT),
	DEFINE_FIELD(CPushable, m_soundTime, FIELD_TIME),
};

#else // HOOK_GAMEDLL

const char *(*CBreakable::ppSpawnObjects)[32];

const char *(*CBreakable::ppSoundsWood)[3];
const char *(*CBreakable::ppSoundsFlesh)[6];
const char *(*CBreakable::ppSoundsMetal)[3];
const char *(*CBreakable::ppSoundsConcrete)[3];
const char *(*CBreakable::ppSoundsGlass)[3];

char *(*CPushable::pm_soundNames)[3];

TYPEDESCRIPTION IMPLEMENT_ARRAY_CLASS(CBreakable, m_SaveData)[5];
TYPEDESCRIPTION IMPLEMENT_ARRAY_CLASS(CPushable, m_SaveData)[2];

#endif // HOOK_GAMEDLL

/* <85bf3> ../cstrike/dlls/func_break.cpp:76 */
void CBreakable::__MAKE_VHOOK(KeyValue)(KeyValueData *pkvd)
{
	// UNDONE_WC: explicitly ignoring these fields, but they shouldn't be in the map file!
	if (FStrEq(pkvd->szKeyName, "explosion"))
	{
		if (!Q_stricmp(pkvd->szValue, "directed"))
			m_Explosion = expDirected;

		else if (!Q_stricmp(pkvd->szValue, "random"))
			m_Explosion = expRandom;
		else
			m_Explosion = expRandom;

		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "material"))
	{
		int i = Q_atoi(pkvd->szValue);

		// 0:glass, 1:metal, 2:flesh, 3:wood

		if (i < 0 || i >= matLastMaterial)
			m_Material = matWood;
		else
			m_Material = (Materials)i;

		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "deadmodel"))
	{
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "shards"))
	{
		//m_iShards = Q_atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "gibmodel"))
	{
		m_iszGibModel = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "spawnobject"))
	{
		int object = Q_atoi(pkvd->szValue);
		if (object > 0 && object < ARRAYSIZE(pSpawnObjects))
		{
			m_iszSpawnObject = MAKE_STRING(pSpawnObjects[object]);
		}

		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "explodemagnitude"))
	{
		ExplosionSetMagnitude(Q_atoi(pkvd->szValue));
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "lip"))
		pkvd->fHandled = TRUE;
	else
		CBaseDelay::KeyValue(pkvd);
}

/* <86426> ../cstrike/dlls/func_break.cpp:139 */
LINK_ENTITY_TO_CLASS(func_breakable, CBreakable);

/* <85b30> ../cstrike/dlls/func_break.cpp:155 */
IMPLEMENT_SAVERESTORE(CBreakable, CBaseEntity);

/* <85663> ../cstrike/dlls/func_break.cpp:157 */
void CBreakable::__MAKE_VHOOK(Spawn)(void)
{
	Precache();

	if (pev->spawnflags & SF_BREAK_TRIGGER_ONLY)
		pev->takedamage	= DAMAGE_NO;
	else
		pev->takedamage	= DAMAGE_YES;

	m_flHealth = pev->health;
	pev->solid = SOLID_BSP;
	pev->movetype = MOVETYPE_PUSH;
	m_angle = pev->angles.y;
	pev->angles.y = 0;

	// HACK:  matGlass can receive decals, we need the client to know about this
	//  so use class to store the material flag
	if (m_Material == matGlass)
	{
		pev->playerclass = 1;
	}

	//set size and link into world.
	SET_MODEL(ENT(pev), STRING(pev->model));

	SetTouch(&CBreakable::BreakTouch);

	// Only break on trigger
	if (pev->spawnflags & SF_BREAK_TRIGGER_ONLY)
	{
		SetTouch(NULL);
	}

	// Flag unbreakable glass as "worldbrush" so it will block ALL tracelines
	if (!IsBreakable() && pev->rendermode != kRenderNormal)
	{
		pev->flags |= FL_WORLDBRUSH;
	}
}

/* <8568a> ../cstrike/dlls/func_break.cpp:191 */
void CBreakable::__MAKE_VHOOK(Restart)(void)
{
	pev->solid = SOLID_BSP;
	pev->movetype = MOVETYPE_PUSH;
	pev->deadflag = DEAD_NO;

	if (pev->spawnflags & SF_BREAK_TRIGGER_ONLY)
		pev->takedamage = DAMAGE_NO;
	else
		pev->takedamage = DAMAGE_YES;

	pev->deadflag = DEAD_NO;
	pev->health = m_flHealth;
	pev->effects &= ~EF_NODRAW;
	m_angle = pev->angles.y;
	pev->angles.y = 0;

	SET_MODEL(ENT(pev), STRING(pev->model));
	SetTouch(&CBreakable::BreakTouch);

	if (pev->spawnflags & SF_BREAK_TRIGGER_ONLY)
	{
		SetTouch(NULL);
	}

	if (!IsBreakable() && pev->rendermode != kRenderNormal)
	{
		pev->flags |= FL_WORLDBRUSH;
	}
}

/* <864f1> ../cstrike/dlls/func_break.cpp:260 */
const char **CBreakable::MaterialSoundList(Materials precacheMaterial, int &soundCount)
{
	const char **pSoundList = NULL;

	switch (precacheMaterial)
	{
		case matWood:
		{
			pSoundList = pSoundsWood;
			soundCount = ARRAYSIZE(pSoundsWood);
			break;
		}
		case matFlesh:
		{
			pSoundList = pSoundsFlesh;
			soundCount = ARRAYSIZE(pSoundsFlesh);
			break;
		}
		case matGlass:
		case matComputer:
		case matUnbreakableGlass:
		{
			pSoundList = pSoundsGlass;
			soundCount = ARRAYSIZE(pSoundsGlass);
			break;
		}
		case matMetal:
		{
			pSoundList = pSoundsMetal;
			soundCount = ARRAYSIZE(pSoundsMetal);
			break;
		}
		case matCinderBlock:
		case matRocks:
		{
			pSoundList = pSoundsConcrete;
			soundCount = ARRAYSIZE(pSoundsConcrete);
			break;
		}
		case matCeilingTile:
		case matNone:
		default:
			soundCount = 0;
			break;
	}

	return pSoundList;
}

/* <86526> ../cstrike/dlls/func_break.cpp:303 */
void CBreakable::MaterialSoundPrecache(Materials precacheMaterial)
{
	const char **pSoundList;
	int i, soundCount = 0;

	pSoundList = MaterialSoundList(precacheMaterial, soundCount);

	for (i = 0; i < soundCount; i++)
	{
		PRECACHE_SOUND((char *)pSoundList[i]);
	}
}

/* <86598> ../cstrike/dlls/func_break.cpp:316 */
void CBreakable::MaterialSoundRandom(edict_t *pEdict, Materials soundMaterial, float volume)
{
	int soundCount = 0;
	const char **pSoundList = MaterialSoundList(soundMaterial, soundCount);

	if (soundCount)
	{
		EMIT_SOUND(pEdict, CHAN_BODY, pSoundList[ RANDOM_LONG(0, soundCount - 1) ], volume, 1.0);
	}
}

/* <8634b> ../cstrike/dlls/func_break.cpp:328 */
void CBreakable::__MAKE_VHOOK(Precache)(void)
{
	const char *pGibName = NULL;

	switch (m_Material)
	{
	case matWood:
		pGibName = "models/woodgibs.mdl";

		PRECACHE_SOUND("debris/bustcrate1.wav");
		PRECACHE_SOUND("debris/bustcrate2.wav");
		break;
	case matFlesh:
		pGibName = "models/fleshgibs.mdl";

		PRECACHE_SOUND("debris/bustflesh1.wav");
		PRECACHE_SOUND("debris/bustflesh2.wav");
		break;
	case matComputer:
		PRECACHE_SOUND("buttons/spark5.wav");
		PRECACHE_SOUND("buttons/spark6.wav");
		pGibName = "models/computergibs.mdl";

		PRECACHE_SOUND("debris/bustmetal1.wav");
		PRECACHE_SOUND("debris/bustmetal2.wav");
		break;
	case matGlass:
	case matUnbreakableGlass:
		pGibName = "models/glassgibs.mdl";

		PRECACHE_SOUND("debris/bustglass1.wav");
		PRECACHE_SOUND("debris/bustglass2.wav");
		break;
	case matMetal:
		pGibName = "models/metalplategibs.mdl";

		PRECACHE_SOUND("debris/bustmetal1.wav");
		PRECACHE_SOUND("debris/bustmetal2.wav");
		break;
	case matCinderBlock:
		pGibName = "models/cindergibs.mdl";

		PRECACHE_SOUND("debris/bustconcrete1.wav");
		PRECACHE_SOUND("debris/bustconcrete2.wav");
		break;
	case matRocks:
		pGibName = "models/rockgibs.mdl";

		PRECACHE_SOUND("debris/bustconcrete1.wav");
		PRECACHE_SOUND("debris/bustconcrete2.wav");
		break;
	case matCeilingTile:
		pGibName = "models/ceilinggibs.mdl";

		PRECACHE_SOUND("debris/bustceiling.wav");
		break;
	}

	MaterialSoundPrecache(m_Material);

	if (m_iszGibModel)
	{
		pGibName = STRING(m_iszGibModel);
	}

	if (pGibName != NULL)
	{
		m_idShard = PRECACHE_MODEL((char *)pGibName);
	}

	// Precache the spawn item's data
	if (m_iszSpawnObject)
	{
		UTIL_PrecacheOther((char *)STRING(m_iszSpawnObject));
	}
}

/* <86676> ../cstrike/dlls/func_break.cpp:401 */
void CBreakable::DamageSound(void)
{
	int pitch;
	float fvol;
	char *rgpsz[6];
	int i;
	int material = m_Material;

	if (RANDOM_LONG(0, 2))
		pitch = PITCH_NORM;
	else
		pitch = 95 + RANDOM_LONG(0, 34);

	fvol = RANDOM_FLOAT(0.75, 1.0);

	if (material == matComputer && RANDOM_LONG(0, 1))
		material = matMetal;

	switch (material)
	{
	case matGlass:
	case matComputer:
	case matUnbreakableGlass:
		rgpsz[0] = "debris/glass1.wav";
		rgpsz[1] = "debris/glass2.wav";
		rgpsz[2] = "debris/glass3.wav";
		i = 3;
		break;

	case matWood:
		rgpsz[0] = "debris/wood1.wav";
		rgpsz[1] = "debris/wood2.wav";
		rgpsz[2] = "debris/wood3.wav";
		i = 3;
		break;

	case matMetal:
		rgpsz[0] = "debris/metal1.wav";
		rgpsz[1] = "debris/metal3.wav";
		rgpsz[2] = "debris/metal2.wav";
		i = 2;
		break;

	case matFlesh:
		rgpsz[0] = "debris/flesh1.wav";
		rgpsz[1] = "debris/flesh2.wav";
		rgpsz[2] = "debris/flesh3.wav";
		rgpsz[3] = "debris/flesh5.wav";
		rgpsz[4] = "debris/flesh6.wav";
		rgpsz[5] = "debris/flesh7.wav";
		i = 6;
		break;

	case matRocks:
	case matCinderBlock:
		rgpsz[0] = "debris/concrete1.wav";
		rgpsz[1] = "debris/concrete2.wav";
		rgpsz[2] = "debris/concrete3.wav";
		i = 3;
		break;

	case matCeilingTile:
		// UNDONE: no ceiling tile shard sound yet
		i = 0;
		break;
	}

	if (i)
	{
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, rgpsz[RANDOM_LONG(0, i - 1)], fvol, ATTN_NORM, 0, pitch);
	}
}

/* <8691c> ../cstrike/dlls/func_break.cpp:475 */
void CBreakable::BreakTouch(CBaseEntity *pOther)
{
	float flDamage;
	entvars_t *pevToucher = pOther->pev;

	// only players can break these right now
	if (!pOther->IsPlayer() || !IsBreakable())
	{
		if (pev->rendermode == kRenderNormal || !FClassnameIs(pOther->pev, "grenade"))
			return;

		pev->angles.y = m_angle;
		UTIL_MakeVectors(pev->angles);

		g_vecAttackDir = gpGlobals->v_forward;

		pev->takedamage = DAMAGE_NO;
		pev->deadflag = DEAD_DEAD;
		pev->effects = EF_NODRAW;

		Die();
	}

	// can be broken when run into
	if (pev->spawnflags & SF_BREAK_TOUCH)
	{
		flDamage = pevToucher->velocity.Length() * 0.01;

		if (flDamage >= pev->health)
		{
			SetTouch(NULL);
			TakeDamage(pevToucher, pevToucher, flDamage, DMG_CRUSH);

			// do a little damage to player if we broke glass or computer
			pOther->TakeDamage(pev, pev, flDamage / 4, DMG_SLASH);
		}
	}

	// can be broken when stood upon
	if ((pev->spawnflags & SF_BREAK_PRESSURE) && pevToucher->absmin.z >= pev->maxs.z - 2)
	{
		// play creaking sound here.
		DamageSound();

		SetThink(&CBreakable::Die);
		SetTouch(NULL);

		// !!!BUGBUG - why doesn't zero delay work?
		if (m_flDelay == 0)
		{
			m_flDelay = 0.1;
		}

		pev->nextthink = pev->ltime + m_flDelay;
	}
}

// Smash the our breakable object
// Break when triggered

/* <85f2d> ../cstrike/dlls/func_break.cpp:538 */
void CBreakable::__MAKE_VHOOK(Use)(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (IsBreakable())
	{
		pev->angles.y = m_angle;
		UTIL_MakeVectors(pev->angles);
		g_vecAttackDir = gpGlobals->v_forward;

		pev->takedamage = DAMAGE_NO;
		pev->deadflag = DEAD_DEAD;
		pev->effects = EF_NODRAW;

		Die();
	}
}

/* <85964> ../cstrike/dlls/func_break.cpp:554 */
void CBreakable::__MAKE_VHOOK(TraceAttack)(entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	// random spark if this is a 'computer' object
	if (RANDOM_LONG(0, 1))
	{
		switch (m_Material)
		{
			case matComputer:
			{
				UTIL_Sparks(ptr->vecEndPos);

				//random volume range
				float flVolume = RANDOM_FLOAT(0.7 , 1.0);
				switch (RANDOM_LONG(0, 1))
				{
				case 0: EMIT_SOUND(ENT(pev), CHAN_VOICE, "buttons/spark5.wav", flVolume, ATTN_NORM); break;
				case 1: EMIT_SOUND(ENT(pev), CHAN_VOICE, "buttons/spark6.wav", flVolume, ATTN_NORM); break;
				}

				break;
			}

			case matUnbreakableGlass:
			{
				UTIL_Ricochet(ptr->vecEndPos, RANDOM_FLOAT(0.5, 1.5));
				break;
			}
		}
	}

	CBaseDelay::TraceAttack(pevAttacker, flDamage, vecDir, ptr, bitsDamageType);
}

// Special takedamage for func_breakable. Allows us to make
// exceptions that are breakable-specific
// bitsDamageType indicates the type of damage sustained ie: DMG_CRUSH

/* <86719> ../cstrike/dlls/func_break.cpp:588 */
int CBreakable::__MAKE_VHOOK(TakeDamage)(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType)
{
	Vector vecTemp;

	// if Attacker == Inflictor, the attack was a melee or other instant-hit attack.
	// (that is, no actual entity projectile was involved in the attack so use the shooter's origin).
	if (pevAttacker == pevInflictor)
	{
		vecTemp = pevInflictor->origin - (pev->absmin + (pev->size * 0.5));

		// if a client hit the breakable with a crowbar, and breakable is crowbar-sensitive, break it now.
		if ((pevAttacker->flags & FL_CLIENT) && (pev->spawnflags & SF_BREAK_CROWBAR) && (bitsDamageType & DMG_CLUB))
		{
			flDamage = pev->health;
		}
	}
	else
	{
		// an actual missile was involved.
		vecTemp = pevInflictor->origin - (pev->absmin + (pev->size * 0.5));
	}

	if (!IsBreakable())
		return 0;

	// Breakables take double damage from the crowbar
	if (bitsDamageType & DMG_CLUB)
	{
		flDamage *= 2;
	}

	// Boxes / glass / etc. don't take much poison damage, just the impact of the dart - consider that 10%
	if (bitsDamageType & DMG_POISON)
	{
		flDamage *= 0.1;
	}

	// this global is still used for glass and other non-monster killables, along with decals.
	g_vecAttackDir = vecTemp.Normalize();

	// do the damage
	pev->health -= flDamage;

	if (pev->health <= 0)
	{
		pev->takedamage = 0;
		pev->deadflag = DEAD_DEAD;
		pev->effects = EF_NODRAW;
		Die();

		if (m_flDelay == 0)
		{
			m_flDelay = 0.1;
		}

		pev->nextthink = pev->ltime + m_flDelay;
		return 0;
	}

	// Make a shard noise each time func breakable is hit.
	// Don't play shard noise if cbreakable actually died.
	DamageSound();

	return 1;
}

/* <856fe> ../cstrike/dlls/func_break.cpp:653 */
void CBreakable::Die(void)
{
	Vector vecSpot;	// shard origin
	Vector vecVelocity;	// shard velocity
	CBaseEntity *pEntity = NULL;
	char cFlag = 0;
	int pitch;
	float fvol;

	pitch = 95 + RANDOM_LONG(0, 29);

	if (pitch > 97 && pitch < 103)
		pitch = 100;

	// The more negative pev->health, the louder
	// the sound should be.
	fvol = RANDOM_FLOAT(0.85, 1.0) + (abs((int)pev->health) / 100.0);

	if (fvol > 1.0)
		fvol = 1.0;

	switch (m_Material)
	{
	case matGlass:
		switch (RANDOM_LONG(0, 1))
		{
		case 0:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustglass1.wav", fvol, ATTN_NORM, 0, pitch);
			break;
		case 1:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustglass2.wav", fvol, ATTN_NORM, 0, pitch);
			break;
		}
		cFlag = BREAK_GLASS;
		TheBots->OnEvent(EVENT_BREAK_GLASS, this);
		break;

	case matWood:
		switch (RANDOM_LONG(0, 1))
		{
		case 0:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustcrate1.wav", fvol, ATTN_NORM, 0, pitch);
			break;
		case 1:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustcrate2.wav", fvol, ATTN_NORM, 0, pitch);
			break;
		}
		cFlag = BREAK_WOOD;
		TheBots->OnEvent(EVENT_BREAK_WOOD, this);
		break;

	case matMetal:
	case matComputer:
		switch (RANDOM_LONG(0, 1))
		{
		case 0:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustmetal1.wav", fvol, ATTN_NORM, 0, pitch);
			break;
		case 1:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustmetal2.wav", fvol, ATTN_NORM, 0, pitch);
			break;
		}
		cFlag = BREAK_METAL;
		TheBots->OnEvent(EVENT_BREAK_METAL, this);
		break;

	case matFlesh:
		switch (RANDOM_LONG(0, 1))
		{
		case 0:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustflesh1.wav", fvol, ATTN_NORM, 0, pitch);
			break;
		case 1:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustflesh2.wav", fvol, ATTN_NORM, 0, pitch);
			break;
		}
		cFlag = BREAK_FLESH;
		TheBots->OnEvent(EVENT_BREAK_FLESH, this);
		break;

	case matCinderBlock:
	case matRocks:
		switch (RANDOM_LONG(0, 1))
		{
		case 0:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustconcrete1.wav", fvol, ATTN_NORM, 0, pitch);
			break;
		case 1:	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustconcrete2.wav", fvol, ATTN_NORM, 0, pitch);
			break;
		}
		cFlag = BREAK_CONCRETE;
		TheBots->OnEvent(EVENT_BREAK_CONCRETE, this);
		break;

	case matCeilingTile:
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustceiling.wav", fvol, ATTN_NORM, 0, pitch);
		break;
	}

	if (m_Explosion == expDirected)
	{
		vecVelocity = g_vecAttackDir * 200;
	}
	else
	{
		vecVelocity.x = 0;
		vecVelocity.y = 0;
		vecVelocity.z = 0;
	}

	vecSpot = pev->origin + (pev->mins + pev->maxs) * 0.5;

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecSpot);
		WRITE_BYTE(TE_BREAKMODEL);
		WRITE_COORD(vecSpot.x);		// position
		WRITE_COORD(vecSpot.y);
		WRITE_COORD(vecSpot.z);
		WRITE_COORD(pev->size.x);	// size
		WRITE_COORD(pev->size.y);
		WRITE_COORD(pev->size.z);
		WRITE_COORD(vecVelocity.x);	// velocity
		WRITE_COORD(vecVelocity.y);
		WRITE_COORD(vecVelocity.z);
		WRITE_BYTE(10);			// randomization
		WRITE_SHORT(m_idShard);		// model id#
		WRITE_BYTE(0);			// # of shards, let client decide
		WRITE_BYTE(25);			// duration, 2.5 seconds
		WRITE_BYTE(cFlag);		// flags
	MESSAGE_END();

	float size = pev->size.x;

	if (size < pev->size.y)
		size = pev->size.y;

	if (size < pev->size.z)
		size = pev->size.z;

	Vector mins = pev->absmin;
	Vector maxs = pev->absmax;
	mins.z = pev->absmax.z;
	maxs.z += 8;

	CBaseEntity *pList[256];
	int count = UTIL_EntitiesInBox(pList, ARRAYSIZE(pList), mins, maxs, FL_ONGROUND);

	if (count)
	{
		for (int i = 0; i < count; i++)
		{
			pList[i]->pev->flags &= ~FL_ONGROUND;
			pList[i]->pev->groundentity = NULL;
		}
	}

	pev->solid = SOLID_NOT;
	SUB_UseTargets(NULL, USE_TOGGLE, 0);
	SetThink(NULL);

	pev->nextthink = pev->ltime + 0.1;

	if (m_iszSpawnObject)
	{
		CBaseEntity::Create((char *)STRING(m_iszSpawnObject), VecBModelOrigin(pev), pev->angles, edict());
	}

	if (Explodable())
	{
		ExplosionCreate(Center(), pev->angles, edict(), ExplosionMagnitude(), TRUE);
	}
}

/* <86992> ../cstrike/dlls/func_break.cpp:833 */
BOOL CBreakable::IsBreakable(void)
{
	return m_Material != matUnbreakableGlass;
}

/* <85a99> ../cstrike/dlls/func_break.cpp:839 */
int CBreakable::__MAKE_VHOOK(DamageDecal)(int bitsDamageType)
{
	if (m_Material == matGlass)
		return DECAL_GLASSBREAK1 + RANDOM_LONG(0, 2);

	if (m_Material == matUnbreakableGlass)
		return DECAL_BPROOF1;

	return CBaseEntity::DamageDecal(bitsDamageType);
}

/* <869b4> ../cstrike/dlls/func_break.cpp:888 */
LINK_ENTITY_TO_CLASS(func_pushable, CPushable);

/* <85e0b> ../cstrike/dlls/func_break.cpp:886 */
IMPLEMENT_SAVERESTORE(CPushable, CBreakable);

/* <856d7> ../cstrike/dlls/func_break.cpp:893 */
void CPushable::__MAKE_VHOOK(Spawn)(void)
{
	if (pev->spawnflags & SF_PUSH_BREAKABLE)
		CBreakable::Spawn();
	else
		Precache();

	pev->movetype = MOVETYPE_PUSHSTEP;
	pev->solid = SOLID_BBOX;

	SET_MODEL(ENT(pev), STRING(pev->model));

	if (pev->friction > 399)
	{
		pev->friction = 399;
	}

	m_maxSpeed = 400 - pev->friction;

	pev->flags |= FL_FLOAT;
	pev->friction = 0;

	// Pick up off of the floor
	pev->origin.z += 1;
	UTIL_SetOrigin(pev, pev->origin);

	// Multiply by area of the box's cross-section (assume 1000 units^3 standard volume)
	pev->skin = (int)((pev->skin * (pev->maxs.x - pev->mins.x) * (pev->maxs.y - pev->mins.y)) * 0.0005);
	m_soundTime = 0;
}

/* <863eb> ../cstrike/dlls/func_break.cpp:920 */
void CPushable::__MAKE_VHOOK(Precache)(void)
{
	for (int i = 0; i < 3; i++)
	{
		PRECACHE_SOUND(m_soundNames[i]);
	}

	if (pev->spawnflags & SF_PUSH_BREAKABLE)
	{
		CBreakable::Precache();
	}
}

/* <85fa3> ../cstrike/dlls/func_break.cpp:930 */
void CPushable::__MAKE_VHOOK(KeyValue)(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "size"))
	{
		int bbox = Q_atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;

		switch (bbox)
		{
		case 0: // Point
			UTIL_SetSize(pev, Vector(-8, -8, -8), Vector(8, 8, 8));
			break;

		case 2: // Big Hull!?!?	!!!BUGBUG Figure out what this hull really is
			UTIL_SetSize(pev, VEC_DUCK_HULL_MIN * 2, VEC_DUCK_HULL_MAX * 2);
			break;

		case 3: // Player duck
			UTIL_SetSize(pev, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX);
			break;

		default:
		case 1: // Player
			UTIL_SetSize(pev, VEC_HULL_MIN, VEC_HULL_MAX);
			break;
		}

	}
	else if (FStrEq(pkvd->szKeyName, "buoyancy"))
	{
		pev->skin = (int)Q_atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBreakable::KeyValue(pkvd);
}

// Pull the func_pushable

/* <86c0d> ../cstrike/dlls/func_break.cpp:969 */
void CPushable::__MAKE_VHOOK(Use)(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (!pActivator || !pActivator->IsPlayer())
	{
		if (pev->spawnflags & SF_PUSH_BREAKABLE)
		{
			this->CBreakable::Use(pActivator, pCaller, useType, value);
		}

		return;
	}

	if (pActivator->pev->velocity != g_vecZero)
	{
		Move(pActivator, 0);
	}
}

/* <86b99> ../cstrike/dlls/func_break.cpp:983 */
void CPushable::__MAKE_VHOOK(Touch)(CBaseEntity *pOther)
{
	if (FClassnameIs(pOther->pev, "worldspawn"))
		return;

	Move(pOther, 1);
}

/* <86a82> ../cstrike/dlls/func_break.cpp:992 */
void CPushable::Move(CBaseEntity *pOther, int push)
{
	entvars_t *pevToucher = pOther->pev;
	int playerTouch = 0;

	// Is entity standing on this pushable ?
	if ((pevToucher->flags & FL_ONGROUND) && pevToucher->groundentity && VARS(pevToucher->groundentity) == pev)
	{
		// Only push if floating
		if (pev->waterlevel > 0)
		{
			pev->velocity.z += pevToucher->velocity.z * 0.1;
		}

		return;
	}

	if (pOther->IsPlayer())
	{
		// Don't push unless the player is pushing forward and NOT use (pull)
		if (push && !(pevToucher->button & (IN_FORWARD | IN_USE)))
		{
			return;
		}

		playerTouch = 1;
	}

	float_precision factor;

	if (playerTouch)
	{
		// Don't push away from jumping/falling players unless in water
		if (!(pevToucher->flags & FL_ONGROUND))
		{
			if (pev->waterlevel < 1)
				return;
			else
				factor = 0.1;
		}
		else
			factor = 1;
	}
	else
		factor = 0.25;

	pev->velocity.x += pevToucher->velocity.x * factor;
	pev->velocity.y += pevToucher->velocity.y * factor;

	float_precision length = sqrt(pev->velocity.x * pev->velocity.x + pev->velocity.y * pev->velocity.y);

	if (push && (length > MaxSpeed()))
	{
		pev->velocity.x = (pev->velocity.x * MaxSpeed() / length);
		pev->velocity.y = (pev->velocity.y * MaxSpeed() / length);
	}

	if (playerTouch)
	{
		pevToucher->velocity.x = pev->velocity.x;
		pevToucher->velocity.y = pev->velocity.y;

		if ((gpGlobals->time - m_soundTime) > 0.7)
		{
			m_soundTime = gpGlobals->time;

			if (length > 0 && (pev->flags & FL_ONGROUND))
			{
				m_lastSound = RANDOM_LONG(0, 2);
				EMIT_SOUND(ENT(pev), CHAN_WEAPON, m_soundNames[m_lastSound], 0.5, ATTN_NORM);

				//SetThink(StopSound);
				//pev->nextthink = pev->ltime + 0.1;
			}
			else
				STOP_SOUND(ENT(pev), CHAN_WEAPON, m_soundNames[m_lastSound]);
		}
	}
}

/* <868b0> ../cstrike/dlls/func_break.cpp:1061 */
int CPushable::__MAKE_VHOOK(TakeDamage)(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType)
{
	if (pev->spawnflags & SF_PUSH_BREAKABLE)
	{
		return CBreakable::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
	}

	return 1;
}

#ifdef HOOK_GAMEDLL

void CBreakable::Spawn(void)
{
	Spawn_();
}

void CBreakable::Precache(void)
{
	Precache_();
}

void CBreakable::Restart(void)
{
	Restart_();
}

void CBreakable::KeyValue(KeyValueData *pkvd)
{
	KeyValue_(pkvd);
}

int CBreakable::Save(CSave &save)
{
	return Save_(save);
}

int CBreakable::Restore(CRestore &restore)
{
	return Restore_(restore);
}

void CBreakable::TraceAttack(entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	TraceAttack_(pevAttacker, flDamage, vecDir, ptr, bitsDamageType);
}

int CBreakable::TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType)
{
	return TakeDamage_(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

int CBreakable::DamageDecal(int bitsDamageType)
{
	return DamageDecal_(bitsDamageType);
}

void CBreakable::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	Use_(pActivator, pCaller, useType, value);
}

void CPushable::Spawn(void)
{
	Spawn_();
}

void CPushable::Precache(void)
{
	Precache_();
}

void CPushable::KeyValue(KeyValueData *pkvd)
{
	KeyValue_(pkvd);
}

int CPushable::Save(CSave &save)
{
	return Save_(save);
}

int CPushable::Restore(CRestore &restore)
{
	return Restore_(restore);
}

int CPushable::TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType)
{
	return TakeDamage_(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

void CPushable::Touch(CBaseEntity *pOther)
{
	Touch_(pOther);
}

void CPushable::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	Use_(pActivator, pCaller, useType, value);
}

#endif // HOOK_GAMEDLL
