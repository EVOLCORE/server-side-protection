#include "framework.h"

cvar_t cv_ssp_version = { "ssp_version", (char*)Plugin_info.version, FCVAR_SERVER | FCVAR_EXTDLL, 0, nullptr };
cvar_t cv_ssp_ping_correction = { "cv_ssp_ping_correction", "1.0", FCVAR_SERVER | FCVAR_EXTDLL, 1.f, nullptr };
cvar_t cv_ssp_reversed_visibility = { "ssp_reversed_visibility", "1.0", FCVAR_SERVER | FCVAR_EXTDLL, 1.f, nullptr };
cvar_t cv_ssp_remove_players_solid = { "ssp_remove_players_solid", "1.0", FCVAR_SERVER | FCVAR_EXTDLL, 1.f, nullptr };
cvar_t cv_ssp_teammates = { "ssp_teammates", "1.0", FCVAR_SERVER | FCVAR_EXTDLL, 1.f, nullptr };

cvar_t* pcv_ssp_ping_correction = nullptr;
cvar_t* pcv_ssp_reversed_visibility = nullptr;
cvar_t* pcv_ssp_remove_players_solid = nullptr;
cvar_t* pcv_ssp_teammates = nullptr;

void InitCvars()
{
	g_engfuncs.pfnCvar_RegisterVariable(&cv_ssp_version);
	g_engfuncs.pfnCvar_RegisterVariable(&cv_ssp_ping_correction);
	g_engfuncs.pfnCvar_RegisterVariable(&cv_ssp_reversed_visibility);
	g_engfuncs.pfnCvar_RegisterVariable(&cv_ssp_remove_players_solid);
	g_engfuncs.pfnCvar_RegisterVariable(&cv_ssp_teammates);

	pcv_ssp_ping_correction = g_engfuncs.pfnCVarGetPointer(cv_ssp_ping_correction.name);
	pcv_ssp_reversed_visibility = g_engfuncs.pfnCVarGetPointer(cv_ssp_reversed_visibility.name);
	pcv_ssp_remove_players_solid = g_engfuncs.pfnCVarGetPointer(cv_ssp_remove_players_solid.name);
	pcv_ssp_teammates = g_engfuncs.pfnCVarGetPointer(cv_ssp_teammates.name);
}