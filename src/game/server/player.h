#ifndef GAME_SERVER_PLAYER_H
#define GAME_SERVER_PLAYER_H

// this include should perhaps be removed
#include "entities/character.h"
#include "gamecontext.h"

// player object
class CPlayer
{
	MACRO_ALLOC_POOL_ID()
	
public:
	CPlayer(CGameContext *pGameServer, int CID, int Team);
	~CPlayer();

	//void Init(int CID); idk what this does or where it is so i commented it. GreYFoXWas Here

	void TryRespawn();
	void Respawn();
	void SetTeam(int Team);
	int GetTeam() const { return m_Team; };
	int GetCID() const { return m_ClientID; };
	
	
	void Tick();
	void Snap(int SnappingClient);

	void OnDirectInput(CNetObj_PlayerInput *NewInput);
	void OnPredictedInput(CNetObj_PlayerInput *NewInput);
	void OnDisconnect();
	
	void KillCharacter(int Weapon = WEAPON_GAME);
	CCharacter *GetCharacter();
	
	struct PauseInfo {
		CCharacterCore m_Core;
		int m_StartTime;
		int m_RaceState;
		//int m_RefreshTime;
		int m_FreezeTime;
		bool m_Doored;
		vec2 m_OldPos;
		vec2 m_OlderPos;
		int m_LastAction;
		int m_Jumped;
		int m_Health;
		int m_Armor;
		int m_PlayerState;
		int m_LastMove;
		vec2 m_PrevPos;
		int m_ActiveWeapon;
		int m_LastWeapon;
		bool m_Respawn;
		bool m_aHasWeapon[NUM_WEAPONS];
		int m_HammerType;
		bool m_Super;
		int m_PauseTime;
	} m_PauseInfo;
	bool m_InfoSaved;
	void LoadCharacter();
	void SaveCharacter();
	
	//---------------------------------------------------------
	// this is used for snapping so we know how we can clip the view for the player
	vec2 m_ViewPos;
	
	//
	int m_Vote;
	int m_VotePos;
	//
	int64 m_Last_KickVote;
	
	int64 m_Last_VoteCall;
	int64 m_Last_VoteTry;
	int64 m_Last_Chat;
	int64 m_Last_SetTeam;
	int64 m_Last_ChangeInfo;
	int64 m_Last_Emote;
	int64 m_Last_Kill;

	int m_Last_ShowOthers;

	bool m_ShowOthers;
	bool m_Invisible;
	
	//DDRace  		 
   int m_Muted;  		 
   //int hammer_ type;  		 
 		 
   // TODO: clean this up
   int m_Authed;  		 
   int m_Resistent;  
   
   bool m_ColorSet; // Set if player changed color at least once 
   
   //DDRace var  		 
   int m_Starttime;  		 
   int m_Refreshtime;  		 
   int m_RaceState;  		 
   int m_Besttick;  		 
   int m_Lasttick;  		 
   float m_BestLap;  
	
	// TODO: clean this up
	struct 
	{
		char m_SkinName[64];
		int m_UseCustomColor;
		int m_ColorBody;
		int m_ColorFeet;
	} m_TeeInfos;
	
	int m_RespawnTick;
	int m_DieTick;
	int m_Score;
	int m_ScoreStartTick;
	bool m_ForceBalanced;
	
	float m_BestTime;
	float m_aBestCpTime[25];
	
	bool m_ResetPickups;
	
	bool m_IsUsingRaceClient;
	float m_LastSentTime;
	
	// afk timer
	void AfkTimer(int new_target_x, int new_target_y);
	int64 m_LastPlaytime;
	int m_LastTarget_x;
	int m_LastTarget_y;
	int m_SentAfkWarning; // afk timer's 1st warning after 50% of sv_max_afk_time
	int m_SentAfkWarning2; // afk timer's 2nd warning after 90% of sv_max_afk_time
	char m_pAfkMsg[160];
	bool m_RconFreeze;
private:
	CCharacter *Character;
	CGameContext *m_pGameServer;
	
	CGameContext *GameServer() const { return m_pGameServer; }
	IServer *Server() const;
	
	
	
	//
	bool m_Spawning;
	int m_ClientID;
	int m_Team;

	// network latency calculations	
	struct
	{
		int m_Accum;
		int m_AccumMin;
		int m_AccumMax;
		int m_Avg;
		int m_Min;
		int m_Max;	
	} m_Latency;
};

#endif
