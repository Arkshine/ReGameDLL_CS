#include "precompiled.h"

enginefuncs_t g_engfuncs;
globalvars_t *gpGlobals;

/* <d0737> ../cstrike/dlls/h_export.cpp:58 */
C_DLLEXPORT void WINAPI GiveFnptrsToDll(enginefuncs_t *pEnginefuncsTable,globalvars_t *pGlobals)
{
	Q_memcpy(&g_engfuncs, pEnginefuncsTable, sizeof(enginefuncs_t));
	gpGlobals = pGlobals;

#if defined(HOOK_GAMEDLL) && !defined(REGAMEDLL_UNIT_TESTS)
	Regamedll_Game_Init();
#endif // HOOK_GAMEDLL

}
