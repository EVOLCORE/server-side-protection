// main.h: include file for standart system include files or include files for specific project.

#pragma once

#include <extdll.h>
#include <meta_api.h>
#include <stdio.h>
#include <string.h>
#include <rehlds_api.h>
#include "ex_rehlds_api.h"
#include "cvars.h"
#include "entity_state.h"
#include "cbase.h"

#define TICK_INTERVAL			(gpGlobals->frametime)
#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )
#define TICKS_TO_TIME( t )		( TICK_INTERVAL *( t ) )
#define ROUND_TO_TICKS( t )		( TICK_INTERVAL * TIME_TO_TICKS( t ) )
#define TICK_NEVER_THINK		(-1)

class player_s
{
public:
	bool state[MAX_CLIENTS]; // if true -> block
};
extern player_s players[MAX_CLIENTS];

bool UTIL_TraceHull(Vector vecSrc, Vector vecEnd, Vector vecMins, Vector vecMaxs, edict_t* pSkip);
BOOL AddToFullPack_Pre(struct entity_state_s* state, int e, edict_t* ent, edict_t* host, int hostflags, BOOL player, unsigned char* pSet);
BOOL AddToFullPack_Post(struct entity_state_s* state, int e, edict_t* ent, edict_t* host, int hostflags, BOOL player, unsigned char* pSet);
void StartFrame_Post();