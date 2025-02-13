/*
*
*   This program is free software; you can redistribute it and/or modify it
*   under the terms of the GNU General Public License as published by the
*   Free Software Foundation; either version 2 of the License, or (at
*   your option) any later version.
*
*   This program is distributed in the hope that it will be useful, but
*   WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*   General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software Foundation,
*   Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*   In addition, as a special exception, the author gives permission to
*   link the code of this program with the Half-Life Game Engine ("HL
*   Engine") and Modified Game Libraries ("MODs") developed by Valve,
*   L.L.C ("Valve").  You must obey the GNU General Public License in all
*   respects for all of the code used other than the HL Engine and MODs
*   from Valve.  If you modify this file, you may extend this exception
*   to your version of the file, but you are not obligated to do so.  If
*   you do not wish to do so, delete this exception statement from your
*   version.
*
*/

#ifndef HOSTAGE_LOCALNAV_H
#define HOSTAGE_LOCALNAV_H
#ifdef _WIN32
#pragma once
#endif

#define NODE_INVALID_EMPTY		-1

#define PATH_TRAVERSABLE_EMPTY		0
#define PATH_TRAVERSABLE_SLOPE		1
#define PATH_TRAVERSABLE_STEP		2
#define PATH_TRAVERSABLE_STEPJUMPABLE	3

typedef int node_index_t;

/* <48522d> ../cstrike/dlls/hostage/hostage_localnav.h:43 */
typedef struct localnode_s
{
	Vector vecLoc;
	int offsetX;
	int offsetY;
	byte bDepth;
	BOOL fSearched;
	node_index_t nindexParent;

} localnode_t;
/* size: 32, cachelines: 1, members: 6 */

#ifdef HOOK_GAMEDLL

#define flNextCvarCheck (*pflNextCvarCheck)
#define s_flStepSize (*ps_flStepSize)
#define flLastThinkTime (*pflLastThinkTime)
#define nodeval (*pnodeval)
#define tot_hostages (*ptot_hostages)
#define tot_inqueue (*ptot_inqueue)
#define qptr (*pqptr)
#define _queue (*pqueue)
#define hostages (*phostages)

#endif // HOOK_GAMEDLL

/* <45aaa2> ../cstrike/dlls/hostage/hostage_localnav.h:58 */
class CLocalNav
{
public:
	CLocalNav(CHostage *pOwner);
	virtual ~CLocalNav(void);

	void SetTargetEnt(CBaseEntity *pTarget)
	{
		if (pTarget)
			m_pTargetEnt = pTarget->edict();
		else
			m_pTargetEnt = NULL;
	}

	node_index_t FindPath(Vector &vecStart, Vector &vecDest, float flTargetRadius, int fNoMonsters);
	int SetupPathNodes(node_index_t nindex, Vector *vecNodes, int fNoMonsters);
	NOBODY int GetFurthestTraversableNode(Vector &vecStartingLoc, Vector *vecNodes, int nTotalNodes, int fNoMonsters);
	int PathTraversable(Vector &vecSource, Vector &vecDest, int fNoMonsters);
	BOOL PathClear(Vector &vecOrigin, Vector &vecDest, int fNoMonsters, TraceResult &tr);
	BOOL PathClear(Vector &vecSource, Vector &vecDest, BOOL fNoMonsters)
	{
		TraceResult tr;
		return PathClear(vecSource, vecDest, fNoMonsters, tr);
	}
	node_index_t AddNode(node_index_t nindexParent, Vector &vecLoc, int offsetX = 0, int offsetY = 0, byte bDepth = 0);
	localnode_t *GetNode(node_index_t nindex);
	node_index_t NodeExists(int offsetX, int offsetY);
	void AddPathNodes(node_index_t nindexSource, int fNoMonsters);
	void AddPathNode(node_index_t nindexSource, int offsetX, int offsetY, int fNoMonsters);
	node_index_t GetBestNode(Vector &vecOrigin, Vector &vecDest);
	BOOL SlopeTraversable(Vector &vecSource, Vector &vecDest, int fNoMonsters, TraceResult &tr);
	NOBODY BOOL LadderTraversable(Vector &vecSource, Vector &vecDest, int fNoMonsters, TraceResult &tr);
	BOOL StepTraversable(Vector &vecSource, Vector &vecDest, int fNoMonsters, TraceResult &tr);
	BOOL StepJumpable(Vector &vecSource, Vector &vecDest, int fNoMonsters, TraceResult &tr);
	node_index_t FindDirectPath(Vector &vecStart, Vector &vecDest, float flTargetRadius, int fNoMonsters);
	NOBODY BOOL LadderHit(Vector &vecSource, Vector &vecDest, TraceResult &tr);

	NOBODY static void Think(void);
	NOBODY static void RequestNav(CHostage *pCaller);
	static void Reset(void);
	static void HostagePrethink(void);

#ifndef HOOK_GAMEDLL
private:
#endif // HOOK_GAMEDLL

	static float s_flStepSize;
	static EHANDLE _queue[ MAX_HOSTAGES_NAV ];
	static int qptr;
	static int tot_inqueue;
	static float nodeval;
	static float flNextCvarCheck;
	static float flLastThinkTime;
	static EHANDLE hostages[ MAX_HOSTAGES_NAV ];
	static int tot_hostages;

#ifdef HOOK_GAMEDLL
private:
#endif // HOOK_GAMEDLL

	CHostage *m_pOwner;
	edict_t *m_pTargetEnt;
	BOOL m_fTargetEntHit;
	localnode_t *m_nodeArr;
	node_index_t m_nindexAvailableNode;
	Vector m_vecStartingLoc;

};/* size: 36, cachelines: 1, members: 16 */

#ifdef HOOK_GAMEDLL

typedef BOOL (CLocalNav::*PATH_CLEAR_TRACE_RESULT)(Vector &, Vector &, int, TraceResult &);
typedef BOOL (CLocalNav::*PATH_CLEAR_DEFAULT)(Vector &, Vector &, int);

#endif // HOOK_GAMEDLL

#endif // HOSTAGE_LOCALNAV_H
