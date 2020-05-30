#include "framework.h"

player_s players[MAX_CLIENTS];

BOOL AddToFullPack_Pre(struct entity_state_s* state, int e, edict_t* ent, edict_t* host, int hostflags, BOOL player, unsigned char* pSet)
{
	if (player && ent != host)
	{
		auto attacker_index = ENTINDEX(host);
		auto enemy_index = e;

		if (players[attacker_index - 1].state[enemy_index - 1] && ((pcv_ssp_reversed_visibility->value && players[enemy_index - 1].state[attacker_index - 1]) || !pcv_ssp_reversed_visibility->value))
		{
			RETURN_META_VALUE(MRES_SUPERCEDE, FALSE);
		}
	}

	RETURN_META_VALUE(MRES_IGNORED, 0);
}

BOOL AddToFullPack_Post(struct entity_state_s* state, int e, edict_t* ent, edict_t* host, int hostflags, BOOL player, unsigned char* pSet)
{
	if (player && ent != host && pcv_ssp_remove_players_solid->value)
	{
		CBasePlayer* pAttackerPlayer = (CBasePlayer*)GET_PRIVATE(host);
		CBasePlayer* pEnemyPlayer = (CBasePlayer*)GET_PRIVATE(ent);

		if (pEnemyPlayer && pAttackerPlayer)
		{
			if ((pcv_ssp_teammates->value && pAttackerPlayer->m_iTeam == pEnemyPlayer->m_iTeam) || pAttackerPlayer->m_iTeam != pEnemyPlayer->m_iTeam)
			{
				float flDistance = Vector(host->v.origin - ent->v.origin).Length();

				if (flDistance > 128.f) // For stuck
				{
					state->solid = SOLID_NOT;
					//gpMetaUtilFuncs->pfnLogConsole(PLID, "SOLID_NOT for %i", ENTINDEX(ent));
				}
			}
		}
	}

	RETURN_META_VALUE(MRES_IGNORED, 0);
}

void StartFrame_Post()
{
	for (int attacker_index = 1; attacker_index <= gpGlobals->maxClients; attacker_index++)
	{
		edict_t* pAttackerEdict = INDEXENT(attacker_index);

		if (!pAttackerEdict)
			continue;

		CBasePlayer* pAttackerPlayer = (CBasePlayer*)GET_PRIVATE(pAttackerEdict);

		if (!pAttackerPlayer)
			continue;

		if (!pAttackerPlayer->pev)
			continue;

		if (!pAttackerPlayer->IsAlive())
			continue;

		int ping, packetloss;

		g_engfuncs.pfnGetPlayerStats(pAttackerEdict, &ping, &packetloss);

		//gpMetaUtilFuncs->pfnLogConsole(PLID, "%i %f %f %i", ping, ping / 1000.f, gpGlobals->frametime, TIME_TO_TICKS(ping / 1000.f));
		
		for (int enemy_index = 1; enemy_index <= gpGlobals->maxClients; enemy_index++)
		{
			players[attacker_index - 1].state[enemy_index - 1] = false; // disable block for player

			if (enemy_index == attacker_index)
				continue;

			edict_t* pEnemyEdict = INDEXENT(enemy_index);

			if (!pEnemyEdict)
				continue;

			CBasePlayer* pEnemyPlayer = (CBasePlayer*)GET_PRIVATE(pEnemyEdict);

			if (!pEnemyPlayer)
				continue;

			if (!pEnemyPlayer->pev)
				continue;

			if (!pEnemyPlayer->IsAlive())
				continue;

			if (pEnemyPlayer->pev->effects & EF_NODRAW) // optimization?
				continue;

			//gpMetaUtilFuncs->pfnLogConsole(PLID, "%i %i %f", pEnemyPlayer->m_iTeam, pAttackerPlayer->m_iTeam, pcv_ssp_teammates->value);

			if (!pcv_ssp_teammates->value && pEnemyPlayer->m_iTeam == pAttackerPlayer->m_iTeam)
				continue;

			players[attacker_index - 1].state[enemy_index - 1] = true; // enable block for player

			if (UTIL_TraceHull(pAttackerPlayer->EyePosition(), pEnemyPlayer->pev->origin, pEnemyPlayer->pev->mins, pEnemyPlayer->pev->maxs, pAttackerEdict)) // attacker see enemy
			{
				players[attacker_index - 1].state[enemy_index - 1] = false;
				continue;
			}

			if (pEnemyPlayer->m_pActiveItem && pEnemyPlayer->m_pActiveItem->m_iId != WEAPON_NONE)
			{
				Vector vecAngles(pEnemyPlayer->pev->angles), vecForward;

				g_engfuncs.pfnAngleVectors(vecAngles, vecForward, NULL, NULL);

				TraceResult trace;

				TRACE_LINE(pEnemyPlayer->EyePosition(), pEnemyPlayer->EyePosition() + vecForward * 64.f, ignore_monsters, pAttackerEdict, &trace); // mb cuustom for weapons

				TRACE_LINE(pAttackerPlayer->EyePosition(), trace.vecEndPos, ignore_monsters, pAttackerEdict, &trace); // maybe for no bugs => - endpos - vecdir * 4.f

				if (trace.flFraction == 1.f) // attacker see weapon enemy
				{
					//gpMetaUtilFuncs->pfnLogConsole(PLID, "weapon => attacker_index: %i, enemy_index: %i", attacker_index, enemy_index);
					players[attacker_index - 1].state[enemy_index - 1] = false;
					continue;
				}
			}

			if (pcv_ssp_ping_correction->value && !pAttackerPlayer->pev->velocity.IsZero() && ping)
			{
				float flPing = ping / 1000.f;

				int nFuture = TIME_TO_TICKS(flPing);

				Vector vecPrediction = pAttackerPlayer->pev->velocity * gpGlobals->frametime * nFuture;

				if (UTIL_TraceHull(pAttackerPlayer->EyePosition() + vecPrediction, pEnemyPlayer->pev->origin, pEnemyPlayer->pev->mins, pEnemyPlayer->pev->maxs, pAttackerEdict))
				{
					//gpMetaUtilFuncs->pfnLogConsole(PLID, "velocity %i => attacker_index: %i, enemy_index: %i | %f %f %f", nFuture, attacker_index, enemy_index, vecPrediction.x, vecPrediction.y, vecPrediction.z);
					players[attacker_index - 1].state[enemy_index - 1] = false;
					continue;
				}
			}
		}
	}

	RETURN_META(MRES_IGNORED);
}

bool UTIL_TraceHull(Vector vecSrc, Vector vecEnd, Vector vecMins, Vector vecMaxs, edict_t* pSkip)
{
	TraceResult trace;
	float* minmaxs[2] = { vecMins, vecMaxs };
	Vector vecTemp;
	int i, j, k;

	for (i = 0; i < 2; i++)
	{
		for (j = 0; j < 2; j++)
		{
			for (k = 0; k < 2; k++)
			{
				vecTemp.x = vecEnd.x + minmaxs[i][0];
				vecTemp.y = vecEnd.y + minmaxs[j][1];
				vecTemp.z = vecEnd.z + minmaxs[k][2];

				TRACE_LINE(vecSrc, vecTemp, ignore_monsters | 0x100, pSkip, &trace); // ignore glass

				if (trace.flFraction == 1.0f)
					return true;

				if (trace.pHit)
				{
					auto pTextureName = TRACE_TEXTURE(trace.pHit, vecSrc, vecTemp);

					if (pTextureName)
					{
						//gpMetaUtilFuncs->pfnLogConsole(PLID, "classname: %s | %s %i %i %i %i", STRING(VARS(trace.pHit)->classname), pTextureName, trace.pHit->v.solid, trace.pHit->v.renderamt, trace.pHit->v.renderfx, trace.pHit->v.rendermode);

						if (pTextureName[0] == '{' || strstr(pTextureName, "glass"))
							return true;
					}
				}
			}
		}
	}

	return false;
}