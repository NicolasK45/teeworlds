// copyright (c) 2007 magnus auvinen, see licence.txt for more info
#include <engine/shared/config.h>
#include <game/mapitems.h>


#include <game/generated/protocol.h>


#include "gamecontroller.h"
#include "gamecontext.h"

#include "entities/pickup.h"
#include "entities/light.h"
#include "entities/dragger.h"
#include "entities/gun.h"
#include "entities/projectile.h"
#include "entities/plasma.h"
#include "entities/trigger.h"
#include "entities/door.h"

#include <game/layers.h>


IGameController::IGameController(class CGameContext *pGameServer)
{
	m_pGameServer = pGameServer;
	m_pServer = m_pGameServer->Server();
	m_pGameType = "unknown";

	//
	DoWarmup(g_Config.m_SvWarmup);
	m_GameOverTick = -1;
	m_SuddenDeath = 0;
	m_RoundStartTick = Server()->Tick();
	m_RoundCount = 0;
	m_GameFlags = 0;
	//m_aTeamscore[0] = 0;
	//m_aTeamscore[1] = 0;
	m_aMapWish[0] = 0;

	m_UnbalancedTick = -1;
	m_ForceBalanced = false;

	m_aNumSpawnPoints[0] = 0;
	m_aNumSpawnPoints[1] = 0;
	m_aNumSpawnPoints[2] = 0;
	
	m_CurrentRecord = 0;

}

IGameController::~IGameController()
{

}

float IGameController::EvaluateSpawnPos(CSpawnEval *pEval, vec2 Pos)
{
	float Score = 0.0f;
	CCharacter *pC = static_cast<CCharacter *>(GameServer()->m_World.FindFirst(NETOBJTYPE_CHARACTER));
	for(; pC; pC = (CCharacter *)pC->TypeNext())
	{
		// team mates are not as dangerous as enemies
		float Scoremod = 1.0f;
		if(pEval->m_FriendlyTeam != -1 && pC->GetPlayer()->GetTeam() == pEval->m_FriendlyTeam)
			Scoremod = 0.5f;

		float d = distance(Pos, pC->m_Pos);
		if(d == 0)
			Score += 1000000000.0f;
		else
			Score += 1.0f/d;
	}

	return Score;
}

void IGameController::EvaluateSpawnType(CSpawnEval *pEval, int T)
{
	// get spawn point
	for(int i  = 0; i < m_aNumSpawnPoints[T]; i++)
	{
		vec2 P = m_aaSpawnPoints[T][i];
		float S = EvaluateSpawnPos(pEval, P);
		if(!pEval->m_Got || pEval->m_Score > S)
		{
			pEval->m_Got = true;
			pEval->m_Score = S;
			pEval->m_Pos = P;
		}
	}
}

bool IGameController::CanSpawn(CPlayer *pPlayer, vec2 *pOutPos)
{
	CSpawnEval Eval;

	// spectators can't spawn
	if(pPlayer->GetTeam() == -1)
		return false;

	/*if(IsTeamplay())
	{
		Eval.m_FriendlyTeam = pPlayer->GetTeam();

		// try first try own team spawn, then normal spawn and then enemy
		EvaluateSpawnType(&Eval, 1+(pPlayer->GetTeam()&1));
		if(!Eval.m_Got)
		{
			EvaluateSpawnType(&Eval, 0);
			if(!Eval.m_Got)
				EvaluateSpawnType(&Eval, 1+((pPlayer->GetTeam()+1)&1));
		}
	}
	else
	{*/
		EvaluateSpawnType(&Eval, 0);
		EvaluateSpawnType(&Eval, 1);
		EvaluateSpawnType(&Eval, 2);
	//}

	*pOutPos = Eval.m_Pos;
	return Eval.m_Got;
}

bool IGameController::OnEntity(int Index, vec2 Pos, bool Front)
{
	if (Index<0)
		return false;
		int temp1=Pos.x;
		int	temp2=Pos.y;
	
	dbg_msg("OnEntity","Index=%d  Front=%d",Index,Front);//Remove*/
	int Type = -1;
	int SubType = 0;
	int x,y;
	x=(Pos.x-16.0f)/32.0f;
	y=(Pos.y-16.0f)/32.0f;
	dbg_msg("OnEntity","Pos %d %d, x%d  y%d",temp1,temp2,x,y);//Remove*/
	/*dbg_msg("OnEntity","Pos(%d,%d)",x,y);//Remove*/
	int sides[8];
	sides[0]=GameServer()->Collision()->Entity(x,y+1, Front);
	sides[1]=GameServer()->Collision()->Entity(x+1,y+1, Front);
	sides[2]=GameServer()->Collision()->Entity(x+1,y, Front);
	sides[3]=GameServer()->Collision()->Entity(x+1,y-1, Front);
	sides[4]=GameServer()->Collision()->Entity(x,y-1, Front);
	sides[5]=GameServer()->Collision()->Entity(x-1,y-1, Front);
	sides[6]=GameServer()->Collision()->Entity(x-1,y, Front);
	sides[7]=GameServer()->Collision()->Entity(x-1,y+1, Front);


	if(Index == ENTITY_SPAWN)
		m_aaSpawnPoints[0][m_aNumSpawnPoints[0]++] = Pos;
	else if(Index == ENTITY_SPAWN_RED)
		m_aaSpawnPoints[1][m_aNumSpawnPoints[1]++] = Pos;
	else if(Index == ENTITY_SPAWN_BLUE)
		m_aaSpawnPoints[2][m_aNumSpawnPoints[2]++] = Pos;

	else if(Index >= ENTITY_CRAZY_SHOTGUN_U_EX && Index <= ENTITY_CRAZY_SHOTGUN_L_EX)
	{
		for (int i = 0; i < 4; i++)
		{
			if (Index - ENTITY_CRAZY_SHOTGUN_U_EX == i)
			{
				float Deg = i*(pi/2);
				CProjectile *bullet = new CProjectile
					(
					&GameServer()->m_World,
					WEAPON_SHOTGUN, //Type
					-1, //Owner
					Pos, //Pos
					vec2(sin(Deg),cos(Deg)), //Dir
					-2, //Span
					true, //Freeze
					true, //Explosive
					0,//Force
					(g_Config.m_SvShotgunBulletSound)?SOUND_GRENADE_EXPLODE:-1,//SoundImpact
					WEAPON_SHOTGUN//Weapon
					);
				bullet->SetBouncing(2 - (i % 2));

			}
		}
	}
	else if(Index >= ENTITY_CRAZY_SHOTGUN_U && Index <= ENTITY_CRAZY_SHOTGUN_L)
	{
		for (int i = 0; i < 4; i++)
		{
			if (Index - ENTITY_CRAZY_SHOTGUN_U == i)
			{
				float Deg = i*(pi/2);
				CProjectile *bullet = new CProjectile(&GameServer()->m_World,
					WEAPON_SHOTGUN, //Type
					-1, //Owner
					Pos, //Pos
					vec2(sin(Deg),cos(Deg)), //Dir
					-2, //Span
					true, //Freeze
					false, //Explosive
					0,
					SOUND_GRENADE_EXPLODE,
					WEAPON_SHOTGUN);
				bullet->SetBouncing(2 - (i % 2));
			}
		}
	}

	if(Index == ENTITY_ARMOR_1)
		Type = POWERUP_ARMOR;
	else if(Index == ENTITY_HEALTH_1)
		Type = POWERUP_HEALTH;
	else if(Index == ENTITY_WEAPON_SHOTGUN)
	{
		Type = POWERUP_WEAPON;
		SubType = WEAPON_SHOTGUN;
	}
	else if(Index == ENTITY_WEAPON_GRENADE)
	{
		Type = POWERUP_WEAPON;
		SubType = WEAPON_GRENADE;
	}
	else if(Index == ENTITY_WEAPON_RIFLE)
	{
		Type = POWERUP_WEAPON;
		SubType = WEAPON_RIFLE;
	}
	else if(Index == ENTITY_POWERUP_NINJA)
	{
		Type = POWERUP_NINJA;
		SubType = WEAPON_NINJA;
	}
	else if(Index >= ENTITY_LASER_FAST_CW && Index <= ENTITY_LASER_FAST_CCW)
   {
		int sides2[8];
		sides2[0]=GameServer()->Collision()->Entity(x,y+2, Front);
		sides2[1]=GameServer()->Collision()->Entity(x+2,y+2, Front);
		sides2[2]=GameServer()->Collision()->Entity(x+2,y, Front);
		sides2[3]=GameServer()->Collision()->Entity(x+2,y-2, Front);
		sides2[4]=GameServer()->Collision()->Entity(x,y-2, Front);
		sides2[5]=GameServer()->Collision()->Entity(x-2,y-2, Front);
		sides2[6]=GameServer()->Collision()->Entity(x-2,y, Front);
		sides2[7]=GameServer()->Collision()->Entity(x-2,y+2, Front);

		float AngularSpeed;
		int Ind=Index-ENTITY_LASER_STOP;
		int M;
		if (Ind<0)
		{
		   Ind=-Ind;
		   M=1;
		}
		else if(Ind==0)
		   M=0;
		else
		   M=-1;


		if (Ind==0)
		   AngularSpeed=0.0f;
		else if (Ind==1)
		   AngularSpeed=pi/360;
		else if (Ind==2)
		   AngularSpeed=pi/180;
		else if (Ind==3)
		   AngularSpeed=pi/90;
		AngularSpeed*=M;

		for(int i=0; i<8;i++)
		{
		   if (sides[i] >= ENTITY_LASER_SHORT && sides[i] <= ENTITY_LASER_LONG)
		   {
			   CLight *Lgt = new CLight(&GameServer()->m_World, Pos, pi/4*i,32*3 + 32*(sides[i] - ENTITY_LASER_SHORT)*3);
			   Lgt->m_AngularSpeed=AngularSpeed;
			   if (sides2[i]>=ENTITY_LASER_C_SLOW && sides2[i]<=ENTITY_LASER_C_FAST)
			   {
				   Lgt->m_Speed=1+(sides2[i]-ENTITY_LASER_C_SLOW)*2;
				   Lgt->m_CurveLength=Lgt->m_Length;
			   }
			   else if(sides2[i]>=ENTITY_LASER_O_SLOW && sides2[i]<=ENTITY_LASER_O_FAST)
			   {
				   Lgt->m_Speed=1+(sides2[i]-ENTITY_LASER_O_SLOW)*2;
				   Lgt->m_CurveLength=0;
			   }
			   else
				   Lgt->m_CurveLength=Lgt->m_Length;
		   }
		}

	}
   else if(Index>=ENTITY_DRAGGER_WEAK && Index <=ENTITY_DRAGGER_STRONG)
   {
       new CDraggerTeam(&GameServer()->m_World,Pos,Index-ENTITY_DRAGGER_WEAK+1, false);
   }
   else if(Index>=ENTITY_DRAGGER_WEAK_NW && Index <=ENTITY_DRAGGER_STRONG_NW)
   {
       new CDraggerTeam(&GameServer()->m_World, Pos,Index-ENTITY_DRAGGER_WEAK_NW+1,true);
   }
   else if(Index==ENTITY_PLASMAE)
   {
       new CGun(&GameServer()->m_World, Pos, 0, true);
   }
   else if(Index==ENTITY_PLASMAF)
   {
       new CGun(&GameServer()->m_World, Pos, 1, false);
   }
   else if(Index==ENTITY_PLASMA)
   {
       new CGun(&GameServer()->m_World, Pos, 1, true);
   }
   else if(Index==ENTITY_PLASMAU)
   {
       new CGun(&GameServer()->m_World, Pos, -1, false);
   }
	if(Type != -1)
	{
		CPickup *pPickup = new CPickup(&GameServer()->m_World, Type, SubType);
		pPickup->m_Pos = Pos;
		return true;
	}

	return false;
}


vec2 GetSidePos(int side) {
	switch(side)
	{
	case 0: return vec2(0, 1);
	case 1: return vec2(1, 1);
	case 2: return vec2(1, 0);
	case 3: return vec2(1, -1);
	case 4: return vec2(0, -1);
	case 5: return vec2(-1, -1);
	case 6: return vec2(-1, 0);
	case 7: return vec2(-1, 1);
	}
	return vec2(0, 0);
}

void IGameController::EndRound()
{
	if(m_Warmup) // game can't end when we are running warmup
		return;

	GameServer()->m_World.m_Paused = true;
	m_GameOverTick = Server()->Tick();
	m_SuddenDeath = 0;
}

void IGameController::ResetGame()
{
	GameServer()->m_World.m_ResetRequested = true;
}

const char *IGameController::GetTeamName(int Team)
{
		if(Team == 0)
			return "game";
	return "spectators";
}


bool IsSeparator(char c) { return c == ';' || c == ' ' || c == ',' || c == '\t'; }

void IGameController::StartRound()
{
	ResetGame();

	m_RoundStartTick = Server()->Tick();
	m_SuddenDeath = 0;
	m_GameOverTick = -1;
	GameServer()->m_World.m_Paused = false;
	m_ForceBalanced = false;
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "start round type='%s' teamplay='%d'", m_pGameType, m_GameFlags&GAMEFLAG_TEAMS);
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
}

void IGameController::ChangeMap(const char *pToMap)
{
	str_copy(m_aMapWish, pToMap, sizeof(m_aMapWish));
	EndRound();
}
/*
void IGameController::CycleMap()
{
	if(m_aMapWish[0] != 0)
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "rotating map to %s", m_aMapWish);
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
		str_copy(g_Config.m_SvMap, m_aMapWish, sizeof(g_Config.m_SvMap));
		m_aMapWish[0] = 0;
		m_RoundCount = 0;
		return;
	}
	if(!str_length(g_Config.m_SvMaprotation))
		return;

	if(m_RoundCount < g_Config.m_SvRoundsPerMap-1)
		return;

	// handle maprotation
	const char *pMapRotation = g_Config.m_SvMaprotation;
	const char *pCurrentMap = g_Config.m_SvMap;

	int CurrentMapLen = str_length(pCurrentMap);
	const char *pNextMap = pMapRotation;
	while(*pNextMap)
	{
		int WordLen = 0;
		while(pNextMap[WordLen] && !IsSeparator(pNextMap[WordLen]))
			WordLen++;

		if(WordLen == CurrentMapLen && str_comp_num(pNextMap, pCurrentMap, CurrentMapLen) == 0)
		{
			// map found
			pNextMap += CurrentMapLen;
			while(*pNextMap && IsSeparator(*pNextMap))
				pNextMap++;

			break;
		}

		pNextMap++;
	}

	// restart rotation
	if(pNextMap[0] == 0)
		pNextMap = pMapRotation;

	// cut out the next map
	char aBuf[512];
	for(int i = 0; i < 512; i++)
	{
		aBuf[i] = pNextMap[i];
		if(IsSeparator(pNextMap[i]) || pNextMap[i] == 0)
		{
			aBuf[i] = 0;
			break;
		}
	}

	// skip spaces
	int i = 0;
	while(IsSeparator(aBuf[i]))
		i++;

	m_RoundCount = 0;

	char aBufMsg[256];
	str_format(aBufMsg, sizeof(aBufMsg), "rotating map to %s", &aBuf[i]);
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
	str_copy(g_Config.m_SvMap, &aBuf[i], sizeof(g_Config.m_SvMap));
}
*/
void IGameController::PostReset()
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(GameServer()->m_apPlayers[i])
		{
			GameServer()->m_apPlayers[i]->Respawn();
			//GameServer()->m_apPlayers[i]->m_Score = 0;
			//GameServer()->m_apPlayers[i]->m_ScoreStartTick = Server()->Tick();
			//GameServer()->m_apPlayers[i]->m_RespawnTick = Server()->Tick()+Server()->TickSpeed()/2;
		}
	}
}
/*
void IGameController::OnPlayerInfoChange(class CPlayer *pP)
{
	const int aTeamColors[2] = {65387, 10223467};
	if(IsTeamplay())
	{
		if(pP->GetTeam() >= 0 || pP->GetTeam() <= 1)
		{
			pP->m_TeeInfos.m_UseCustomColor = 1;
			pP->m_TeeInfos.m_ColorBody = aTeamColors[pP->GetTeam()];
			pP->m_TeeInfos.m_ColorFeet = aTeamColors[pP->GetTeam()];
		}
	}
}
*/

int IGameController::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon)
{
	/*
	// do scoreing
	if(!pKiller || Weapon == WEAPON_GAME)
		return 0;
	if(pKiller == pVictim->GetPlayer())
		pVictim->GetPlayer()->m_Score--; // suicide
	else
	{
		if(IsTeamplay() && pVictim->GetPlayer()->GetTeam() == pKiller->GetTeam())
			pKiller->m_Score--; // teamkill
		else
			pKiller->m_Score++; // normal kill
	}*/
	return 0;
}

void IGameController::OnCharacterSpawn(class CCharacter *pChr)
{
	// default health
	pChr->IncreaseHealth(10);

	// give default weapons
	//pChr->GiveAllWeapons();
	pChr->GiveWeapon(WEAPON_HAMMER, -1);
	pChr->GiveWeapon(WEAPON_GUN, -1);
}

void IGameController::DoWarmup(int Seconds)
{
	if(Seconds < 0)
		m_Warmup = 0;
	else
		m_Warmup = Seconds*Server()->TickSpeed();
}
/*
bool IGameController::IsFriendlyFire(int Cid1, int Cid2)
{
	if(Cid1 == Cid2)
		return false;

	if(IsTeamplay())
	{
		if(!GameServer()->m_apPlayers[Cid1] || !GameServer()->m_apPlayers[Cid2])
			return false;

		if(GameServer()->m_apPlayers[Cid1]->GetTeam() == GameServer()->m_apPlayers[Cid2]->GetTeam())
			return true;
	}

	return false;
}
*/
bool IGameController::IsForceBalanced()
{
	/*if(m_ForceBalanced)
	{
		m_ForceBalanced = false;
		return true;
	}
	else*/
		return false;
}

bool IGameController::CanBeMovedOnBalance(int Cid)
{
	return true;
}

void IGameController::Tick()
{
	// do warmup
	if(m_Warmup)
	{
		m_Warmup--;
		if(!m_Warmup)
			StartRound();
	}

	if(m_GameOverTick != -1)
	{
		// game over.. wait for restart
		if(Server()->Tick() > m_GameOverTick+Server()->TickSpeed()*10)
		{
			//CycleMap();
			StartRound();
			m_RoundCount++;
		}
	}
	/*
	// do team-balancing
	if (IsTeamplay() && m_UnbalancedTick != -1 && Server()->Tick() > m_UnbalancedTick+g_Config.m_SvTeambalanceTime*Server()->TickSpeed()*60)
	{
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", "Balancing teams");

		int aT[2] = {0,0};
		float aTScore[2] = {0,0};
		float aPScore[MAX_CLIENTS] = {0.0f};
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->GetTeam() != -1)
			{
				aT[GameServer()->m_apPlayers[i]->GetTeam()]++;
				aPScore[i] = GameServer()->m_apPlayers[i]->m_Score*Server()->TickSpeed()*60.0f/
					(Server()->Tick()-GameServer()->m_apPlayers[i]->m_ScoreStartTick);
				aTScore[GameServer()->m_apPlayers[i]->GetTeam()] += aPScore[i];
			}
		}

		// are teams unbalanced?
		if(absolute(aT[0]-aT[1]) >= 2)
		{
			int M = (aT[0] > aT[1]) ? 0 : 1;
			int NumBalance = absolute(aT[0]-aT[1]) / 2;

			do
			{
				CPlayer *pP = 0;
				float PD = aTScore[M];
				for(int i = 0; i < MAX_CLIENTS; i++)
				{
					if(!GameServer()->m_apPlayers[i] || !CanBeMovedOnBalance(i))
						continue;
					// remember the player who would cause lowest score-difference
					if(GameServer()->m_apPlayers[i]->GetTeam() == M && (!pP || absolute((aTScore[M^1]+aPScore[i]) - (aTScore[M]-aPScore[i])) < PD))
					{
						pP = GameServer()->m_apPlayers[i];
						PD = absolute((aTScore[M^1]+aPScore[i]) - (aTScore[M]-aPScore[i]));
					}
				}

				// move the player to the other team
				pP->SetTeam(M^1);

				pP->Respawn();
				pP->m_ForceBalanced = true;
			} while (--NumBalance);

			m_ForceBalanced = true;
		}
		m_UnbalancedTick = -1;
	}
	*/
	// update browse info
	int Prog = -1;
	/*
	if(g_Config.m_SvTimelimit > 0)
		Prog = max(Prog, (Server()->Tick()-m_RoundStartTick) * 100 / (g_Config.m_SvTimelimit*Server()->TickSpeed()*60));

	if(g_Config.m_SvScorelimit)
	{
		if(IsTeamplay())
		{
			Prog = max(Prog, (m_aTeamscore[0]*100)/g_Config.m_SvScorelimit);
			Prog = max(Prog, (m_aTeamscore[1]*100)/g_Config.m_SvScorelimit);
		}
		else
		{
			for(int i = 0; i < MAX_CLIENTS; i++)
			{
				if(GameServer()->m_apPlayers[i])
					Prog = max(Prog, (GameServer()->m_apPlayers[i]->m_Score*100)/g_Config.m_SvScorelimit);
			}
		}
	}

	if(m_Warmup)
		Prog = -1;
		*/
	Server()->SetBrowseInfo(m_pGameType, Prog);
}

/*
bool IGameController::IsTeamplay() const
{
	return m_GameFlags&GAMEFLAG_TEAMS;
}
*/
void IGameController::Snap(int SnappingClient)
{
	CNetObj_Game *pGameObj = (CNetObj_Game *)Server()->SnapNewItem(NETOBJTYPE_GAME, 0, sizeof(CNetObj_Game));
	pGameObj->m_Paused = GameServer()->m_World.m_Paused;
	pGameObj->m_GameOver = m_GameOverTick==-1?0:1;
	pGameObj->m_SuddenDeath = m_SuddenDeath;

	//pGameObj->m_ScoreLimit = g_Config.m_SvScorelimit;
	//pGameObj->m_TimeLimit = g_Config.m_SvTimelimit;
	pGameObj->m_RoundStartTick = m_RoundStartTick;
	pGameObj->m_Flags = m_GameFlags;

	pGameObj->m_Warmup = m_Warmup;

	pGameObj->m_RoundNum = /*(str_length(g_Config.m_SvMaprotation) && g_Config.m_SvRoundsPerMap) ? g_Config.m_SvRoundsPerMap :*/ 0;
	pGameObj->m_RoundCurrent = m_RoundCount+1;


	if(SnappingClient == -1)
	{
		// we are recording a demo, just set the scores
		pGameObj->m_TeamscoreRed = 0;//m_aTeamscore[0];
		pGameObj->m_TeamscoreBlue = 0;//m_aTeamscore[1];
	}
	else
	{
		// TODO: this little hack should be removed
		pGameObj->m_TeamscoreRed = /*IsTeamplay() ? m_aTeamscore[0] : */GameServer()->m_apPlayers[SnappingClient]->m_Score;
		pGameObj->m_TeamscoreBlue = 0;//m_aTeamscore[1];
	}
}

int IGameController::GetAutoTeam(int Notthisid)
{
	// this will force the auto balancer to work overtime aswell
	if(g_Config.m_DbgStress)
		return 0;

	int aNumplayers[2] = {0,0};
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(GameServer()->m_apPlayers[i] && i != Notthisid)
		{
			if(GameServer()->m_apPlayers[i]->GetTeam() == 0 || GameServer()->m_apPlayers[i]->GetTeam() == 1)
				aNumplayers[GameServer()->m_apPlayers[i]->GetTeam()]++;
		}
	}

	int Team = 0;
	//if(IsTeamplay())
	//	Team = aNumplayers[0] > aNumplayers[1] ? 1 : 0;

	if(CanJoinTeam(Team, Notthisid))
		return Team;
	return -1;
}

bool IGameController::CanJoinTeam(int Team, int Notthisid)
{
	if(Team == -1 || (GameServer()->m_apPlayers[Notthisid] && GameServer()->m_apPlayers[Notthisid]->GetTeam() != -1))
		return true;

	int aNumplayers[2] = {0,0};
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(GameServer()->m_apPlayers[i] && i != Notthisid)
		{
			if(GameServer()->m_apPlayers[i]->GetTeam() >= 0 || GameServer()->m_apPlayers[i]->GetTeam() == 1)
				aNumplayers[GameServer()->m_apPlayers[i]->GetTeam()]++;
		}
	}

	return (aNumplayers[0] + aNumplayers[1]) < g_Config.m_SvMaxClients-g_Config.m_SvSpectatorSlots;
}
/*
bool IGameController::CheckTeamBalance()
{
	if(!IsTeamplay() || !g_Config.m_SvTeambalanceTime)
		return true;

	int aT[2] = {0, 0};
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pP = GameServer()->m_apPlayers[i];
		if(pP && pP->GetTeam() != -1)
			aT[pP->GetTeam()]++;
	}

	char aBuf[256];
	if(absolute(aT[0]-aT[1]) >= 2)
	{
		str_format(aBuf, sizeof(aBuf), "Team is NOT balanced (red=%d blue=%d)", aT[0], aT[1]);
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
		if(GameServer()->m_pController->m_UnbalancedTick == -1)
			GameServer()->m_pController->m_UnbalancedTick = Server()->Tick();
		return false;
	}
	else
	{
		str_format(aBuf, sizeof(aBuf), "Team is balanced (red=%d blue=%d)", aT[0], aT[1]);
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
		GameServer()->m_pController->m_UnbalancedTick = -1;
		return true;
	}
}
*//*
bool IGameController::CanChangeTeam(CPlayer *pPlayer, int JoinTeam)
{
	int aT[2] = {0, 0};

	if (!IsTeamplay() || JoinTeam == -1 || !g_Config.m_SvTeambalanceTime)
		return true;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pP = GameServer()->m_apPlayers[i];
		if(pP && pP->GetTeam() != -1)
			aT[pP->GetTeam()]++;
	}

	// simulate what would happen if changed team
	aT[JoinTeam]++;
	if (pPlayer->GetTeam() != -1)
		aT[JoinTeam^1]--;

	// there is a player-difference of at least 2
	if(absolute(aT[0]-aT[1]) >= 2)
	{
		// player wants to join team with less players
		if ((aT[0] < aT[1] && JoinTeam == 0) || (aT[0] > aT[1] && JoinTeam == 1))
			return true;
		else
			return false;
	}
	else
		return true;
}
*//*
void IGameController::DoPlayerScoreWincheck()
{
	if(m_GameOverTick == -1  && !m_Warmup)
	{
		// gather some stats
		int Topscore = 0;
		int TopscoreCount = 0;
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(GameServer()->m_apPlayers[i])
			{
				if(GameServer()->m_apPlayers[i]->m_Score > Topscore)
				{
					Topscore = GameServer()->m_apPlayers[i]->m_Score;
					TopscoreCount = 1;
				}
				else if(GameServer()->m_apPlayers[i]->m_Score == Topscore)
					TopscoreCount++;
			}
		}

		// check score win condition
		if((g_Config.m_SvScorelimit > 0 && Topscore >= g_Config.m_SvScorelimit) ||
			(g_Config.m_SvTimelimit > 0 && (Server()->Tick()-m_RoundStartTick) >= g_Config.m_SvTimelimit*Server()->TickSpeed()*60))
		{
			if(TopscoreCount == 1)
				EndRound();
			else
				m_SuddenDeath = 1;
		}
	}
}

void IGameController::DoTeamScoreWincheck()
{
	if(m_GameOverTick == -1 && !m_Warmup)
	{
		// check score win condition
		if((g_Config.m_SvScorelimit > 0 && (m_aTeamscore[0] >= g_Config.m_SvScorelimit || m_aTeamscore[1] >= g_Config.m_SvScorelimit)) ||
			(g_Config.m_SvTimelimit > 0 && (Server()->Tick()-m_RoundStartTick) >= g_Config.m_SvTimelimit*Server()->TickSpeed()*60))
		{
			if(m_aTeamscore[0] != m_aTeamscore[1])
				EndRound();
			else
				m_SuddenDeath = 1;
		}
	}
}
*/



int IGameController::ClampTeam(int Team)
{
	if(Team < 0) // spectator
		return -1;
	//if(IsTeamplay())
	//	return Team&1;
	return  0;
}
