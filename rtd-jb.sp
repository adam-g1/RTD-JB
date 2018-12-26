#include <sourcemod>
#include <ccsplayer>
#include <AutoExecConfig>
#include <sdkhooks>
#include <jb_chat>

#pragma newdecls required
#pragma semicolon 1

#define PREFIX "[SM] "

public Plugin myinfo = 
{
	name = "Roll The Dice JB", 
	author = "AdamG", 
	description = "a RTD Plugin for JB servers.", 
	version = "1.0", 
	url = "https://github.com/adam-g1"
}

typedef OnRoll = function void (CCSPlayer pPlayer);

#define MAX_ROLL_LEN 32
/*
 * Block 0:				Unique ID
 * Block 1:				Name of the callback inside a DataPack
 * Block 2:				OnRoll callback inside a DataPack
 */
ArrayList g_hEffects;
#define BLOCK_ID 0
#define BLOCK_CALLBACK 1
#define BLOCK_NAME 2

bool g_bBetweenRounds = false;

// Ids for Rolls that need special resets
int g_iBlindId;
int g_iLowGravId;
int g_iInvisId;
int g_iFallId;
int g_iBurningBulletId;
int g_iRubberBulletId;

// Stores the Unique ID of the roll the player rolled.
int g_iRoll[MAXPLAYERS + 1];

bool g_bHasInvis[MAXPLAYERS + 1];
Handle g_hInvisTimer[MAXPLAYERS + 1];
Handle g_hPoisonTimer[MAXPLAYERS + 1];

// timestamp of round start
int g_iRoundStart;

int g_iPrePoisonColor[MAXPLAYERS + 1][4];
int g_iLastHealth[MAXPLAYERS + 1];
int g_iAlpha[MAXPLAYERS + 1];
// CSGO ConVars used
ConVar g_hMaxMoney;

// Custom ConVars
ConVar g_hInvisTime;
ConVar g_hIncreaseHpMin;
ConVar g_hIncreaseHpMax;
ConVar g_hDecreaseHpMin;
ConVar g_hDecreaseHpMax;
ConVar g_hSlowedMin;
ConVar g_hSlowedMax;
ConVar g_hMeepMin;
ConVar g_hMeepMax;
ConVar g_hMaxGlockReserve;
ConVar g_hMaxDeagReserve;
ConVar g_hIgniteTime;
ConVar g_hRobberyMin;
ConVar g_hRobberyMax;
ConVar g_hLotteryMin;
ConVar g_hLotteryMax;
ConVar g_hSnowballAmount;
ConVar g_hLowGrav;
ConVar g_hMaxFallNegate;
ConVar g_hFallReduce;
ConVar g_hBurnBulletTime;
ConVar g_hRubberBulletReduce;
ConVar g_hPoisonInterval;
ConVar g_hPoisonStopChance;
ConVar g_hPoisonDmg;
ConVar g_hMoneyPrice;
ConVar g_hLimitTeams;
ConVar g_hTimeLimit;

public void OnPluginStart() {
	g_hEffects = new ArrayList(3);
	
	HookEvent("player_death", Event_PlayerDeath);
	HookEvent("round_end", Event_RoundEnd);
	HookEvent("round_start", Event_RoundStart);
	
	// Used for invisibility
	AddCommandListener(Command_Inspect, "+lookatweapon");
	
	// CSGO ConVars
	g_hMaxMoney = FindConVar("mp_maxmoney");
	
	// Create Effects
	CreateEffect("No Effect", 3, Roll_DoNothing);
	CreateEffect("Increased HP", 2, Roll_IncreaseHp);
	CreateEffect("Decreased HP", 2, Roll_DecreaseHp);
	CreateEffect("Random HP", 2, Roll_RandomHp); 
	CreateEffect("Slowed Down", 1, Roll_Slowed);
	CreateEffect("Meep Meep", 1, Roll_Meep);
	CreateEffect("Random Speed", 1, Roll_RandSpeed);
	g_iBlindId = CreateEffect("Blindness", 2, Roll_Blindness);
	g_iInvisId = CreateEffect("Temporary Invisibility", 1, Roll_Invis);
	CreateEffect("Spontaneous Ignition", 2, Roll_Ignite);
	CreateEffect("Small Armor", 2, Roll_SmallArmor);
	CreateEffect("Heavy Armor", 1, Roll_HeavyArmor);
	CreateEffect("Grenades", 2, Roll_Grenades);
	CreateEffect("Glock", 2, Roll_Glock);
	CreateEffect("Deagle", 1, Roll_Deagle);
	CreateEffect("Wrench", 2, Roll_Wrench);
	CreateEffect("Axe", 2, Roll_Axe);
	CreateEffect("Hammer", 2, Roll_Hammer);
	CreateEffect("Knife", 2, Roll_Knife);
	CreateEffect("Snowballs", 2, Roll_Snowballs); // TODO: Find model that needs to be precached
	g_iRubberBulletId = CreateEffect("Rubber Bullets", 2, Roll_RubberBullets); 
	g_iBurningBulletId = CreateEffect("Burning Bullets", 2, Roll_BurningBullets);
	CreateEffect("Poisoned", 1, Roll_Poisoned);
	CreateEffect("Guard Model", 1, Roll_Model);
	g_iFallId = CreateEffect("Long Fall Boots", 1, Roll_FallDamage);
	g_iLowGravId = CreateEffect("Moon Boots", 1, Roll_LowGrav);
	CreateEffect("Lottery", 1, Roll_Lottery);
	CreateEffect("Robbery", 1, Roll_Robbery);
	CreateEffect("Medic", 2, Roll_Medic); 
	
	RegConsoleCmd("sm_rtdchances", Command_RtdChances);
	RegConsoleCmd("sm_rtd", Command_Rtd);
	
	#if defined RTD_DEBUG
	RegAdminCmd("sm_forceroll", Command_ForceRoll, ADMFLAG_ROOT);
	#endif
	
	// sv_disable_immunity_alpha must be set to 1 for invisibility to work
	ConVar hImmAlpha = FindConVar("sv_disable_immunity_alpha");
	hImmAlpha.IntValue = 1;
	hImmAlpha.AddChangeHook(OnAlphaChange);
	
	// ConVars
	
	AutoExecConfig_SetFile("plugin.rtd-jb");
	
	g_hInvisTime = AutoExecConfig_CreateConVar("sm_rtd_invis_time", 
							"7.0", 
							"How long full invisibility should last for Invis RTD");
							
	g_hIncreaseHpMin = AutoExecConfig_CreateConVar("sm_rtd_inc_hp_min", 
							"10", 
							"Minimum amount of random hp to give for increased hp",
							0,
							true,
							7.0);
							
	g_hIncreaseHpMax = AutoExecConfig_CreateConVar("sm_rtd_inc_hp_max", 
							"25", 
							"Maximum amount of random hp to give for increased hp",
							0,
							false,
							_,
							true,
							50.0);
							
	g_hDecreaseHpMin = AutoExecConfig_CreateConVar("sm_rtd_dec_hp_min", 
							"10", 
							"Minimum amount of random hp to remove for decreased hp",
							0,
							true,
							7.0);
	g_hDecreaseHpMax = AutoExecConfig_CreateConVar("sm_rtd_dec_hp_max", 
							"20", 
							"Minimum amount of random hp to remove for decreased hp",
							0,
							false,
							_,
							true,
							50.0);
							
	g_hSlowedMax = AutoExecConfig_CreateConVar("sm_rtd_slowed_max", 
							"0.84", 
							"Maximum speed for slowed. This should be lower than the minimum.\n1.0=normal speed, lower is slower.");
							
	g_hSlowedMin = AutoExecConfig_CreateConVar("sm_rtd_slowed_min", 
							"0.92", 
							"Maximum speed for slowed. This should be higher than the maximum\n1.0=normal speed");
							
	g_hMeepMax =  AutoExecConfig_CreateConVar("sm_rtd_meep_max", 
							"1.2", 
							"Maximum speed for meep.\n1.0=normal speed, higher is faster.");
							
	g_hMeepMin =  AutoExecConfig_CreateConVar("sm_rtd_meep_min", 
							"1.07", 
							"Minimum speed for meep.\n1.0=normal speed, higher is faster.");
							
	g_hMaxGlockReserve = AutoExecConfig_CreateConVar("sm_rtd_glock_max_mags",
							"2",
							"Maximum amount of extra mags to give for glocks");
							
	g_hMaxDeagReserve = AutoExecConfig_CreateConVar("sm_rtd_deag_max_mags",
							"2",
							"Maximum amount of extra mags to give for deags");
							
	g_hIgniteTime = AutoExecConfig_CreateConVar("sm_rtd_ignite_time",
							"7.5",
							"How long the ignite rtd should ignite the user for");
							
	g_hLotteryMin = AutoExecConfig_CreateConVar("sm_rtd_lottery_min", 
							"1000", 
							"Minimum lottery amount");
							
	g_hLotteryMax = AutoExecConfig_CreateConVar("sm_rtd_lottery_max", 
							"5000", 
							"Maximum lottery amount");
							
	g_hRobberyMin = AutoExecConfig_CreateConVar("sm_rtd_robbery_min", 
							"500", 
							"Maximum lottery amount");
							
	g_hRobberyMax = AutoExecConfig_CreateConVar("sm_rtd_robbery_max", 
							"3500", 
							"Maximum lottery amount");
							
	g_hSnowballAmount = AutoExecConfig_CreateConVar("sm_rtd_snowball_amount", 
							"3", 
							"How many snowballs to give");
							
	g_hLowGrav = AutoExecConfig_CreateConVar("sm_rtd_low_grav_amount", 
							"0.775", 
							"Gravity to use for low grav (1.0 = normal)");
							
	g_hMaxFallNegate = AutoExecConfig_CreateConVar("sm_rtd_fall_negate_dmg",
							"40",
							"Maximum amount of fall damage to completely negate");
	
	g_hFallReduce = AutoExecConfig_CreateConVar("sm_rtd_fall_reduce",
							"0.75",
							"Percentage of damage to take after reducing fall damage\n0.75 = take 75% of damage");
							
	g_hBurnBulletTime = AutoExecConfig_CreateConVar("sm_rtd_burning_bullet_time",
							"2.5",
							"How long should burning bullets burn the victim for");
							
	g_hRubberBulletReduce = AutoExecConfig_CreateConVar("sm_rtd_rubber_bullet_reduce",
							"0.70",
							"Percentage of damage to give after reducing damage\n0.70 = give 70% of damage");
							
	g_hPoisonInterval = AutoExecConfig_CreateConVar("sm_rtd_poison_interval",
							"1.0",
							"How often poison should damage in seconds");
	
	g_hPoisonStopChance = AutoExecConfig_CreateConVar("sm_rtd_poison_cure_chance",
							"40",
							"Chance for poison to be cured",
							_,
							true,
							0.0,
							true,
							100.0);
							
	g_hPoisonDmg = AutoExecConfig_CreateConVar("sm_rtd_poison_dmg",
							"5",
							"Amount of damage each tick of poison does");
							
	g_hMoneyPrice = AutoExecConfig_CreateConVar("sm_rtd_money_price", 
							"1000",
							"Price to RTD using in game money");
							
	g_hLimitTeams = AutoExecConfig_CreateConVar("sm_rtd_limitteams",
							"1",
							"If set to 1, only Ts can use sm_rtd");
						
	g_hTimeLimit = AutoExecConfig_CreateConVar("sm_rtd_time_limit",
							"30",
							"How long into the round users can RTD (In seconds)");
	
	AutoExecConfig_ExecuteFile();
	AutoExecConfig_CleanFile();
	
	// Late Load Support	
	for(int i = 1; i < MaxClients; i++) {
		if(IsClientInGame(i)) {
			OnClientPutInServer(i);
		}
	}
}

public void OnClientPutInServer(int iClient) {
	SDKHook(iClient, SDKHook_OnTakeDamageAlive, OnTakeDamageAlive);
}

public void OnClientDisconnect(int iClient) {
	ResetPlayer(CCSPlayer(iClient));
}

public Action OnTakeDamageAlive(int iVictim, int &iAttacker, int &iInflictor, float &fDmg,
		int &iDmgType, int &iWeapon, float fDmgForce[3], float fDmgPos[3], int iDmgCustom) {
	// If fall damage and user has Long Fall Boots
	
	if(iDmgType & DMG_FALL && g_iRoll[iVictim] == g_iFallId) {
		
		// Damage should be negated
		if(fDmg < g_hMaxFallNegate.FloatValue) {
			fDmg = 0.0;
			return Plugin_Changed;
		}
		else { // Reduce damage
			fDmg *= g_hFallReduce.FloatValue;
			return Plugin_Changed;
		}
	}

	// Valid attacker
	if(!CCSPlayer(iAttacker).IsNull) {
		
		// User has burning bullets and damage was from a bullet
		if(g_iRoll[iAttacker] == g_iBurningBulletId && iDmgType & DMG_BULLET) {
			// Extinguish to prevent multiple fires
			ExtinguishEntity(iVictim);
			IgniteEntity(iVictim, g_hBurnBulletTime.FloatValue);
		}
		// User has rubber bullets and damage was from a bullet
		else if(g_iRoll[iAttacker] == g_iRubberBulletId && iDmgType & DMG_BULLET) {
			fDmg *= g_hRubberBulletReduce.FloatValue;
			return Plugin_Changed;
		}
	}
	
	return Plugin_Continue;
}

// Forces sv_disable_immunity_alpha to be enabled when changed.
public void OnAlphaChange(ConVar hCvar, const char[] sOld, const char[] sNew) {
	if(StringToInt(sNew) != 1) {
		hCvar.IntValue = 1;
	}
}

public Action Command_Inspect(int iClient, const char[] sCmd, int iArgs) {
	
	// User has invis and hasn't used it yet.
	if(g_bHasInvis[iClient]) {
		
		PrintToChat(iClient, "You start to fade....");
		
		g_bHasInvis[iClient] = false;
		
		DataPack hPack = new DataPack();
		
		// Alpha only works when using RENDER_TRANSALPHA
		CCSPlayer p = CCSPlayer(iClient);
		p.Render = RENDER_TRANSALPHA;
		g_iAlpha[iClient] = 255;
		
		g_hInvisTimer[iClient] = CreateDataTimer(0.5, Timer_IncreaseInvis, hPack, TIMER_REPEAT);
		hPack.WriteCell(GetClientUserId(iClient));
		hPack.WriteCell(iClient);
		TriggerTimer(g_hInvisTimer[iClient]);
	}
}

/*
 * CCSPlayer.GetRenderColor gives somewhat garbage values.
 * They will give extremely large numbers, even directly after
 * setting using CCSPlayer.SetRenderColor.
 * We're just going to use a global to figure out the alpha value.
 */

public Action Timer_IncreaseInvis(Handle hTimer, DataPack hPack) {
	hPack.Reset();
	CCSPlayer p = CCSPlayer.FromUserId(hPack.ReadCell());
	
	// Player disconnected, cleanup
	if(p.IsNull) {
		g_hInvisTimer[hPack.ReadCell()] = null;
		return Plugin_Stop;
	}
	else {
		// Get current render color
		int iColor[4];
		p.GetRenderColor(iColor);
		
		// Lower alpha value to make more invisible
		iColor[3] -= 20;
		g_iAlpha[p.Index] -= 20;
		
		// If player is fully invisible
		if(g_iAlpha[p.Index] <= 0) {
			// Clamp to 0 before setting color
			iColor[3] = 0;
			p.SetRenderColor(iColor);
			
			// Create new timer to remove invisibility
			DataPack hCopy = new DataPack();
			g_hInvisTimer[p.Index] = CreateDataTimer(g_hInvisTime.FloatValue, Timer_RemoveInvis, hCopy);
			hCopy.WriteCell(p.UserID);
			hCopy.WriteCell(p);
			
			return Plugin_Stop;
		}
		// Player not yet fully invisible
		else {
			p.SetRenderColor(iColor);
			return Plugin_Continue;
		}
	}
}

public Action Timer_RemoveInvis(Handle hTimer, DataPack hPack) {
	hPack.Reset();
	CCSPlayer p = CCSPlayer.FromUserId(hPack.ReadCell());
	
	// Player disconnected, cleanup
	if(p.IsNull) {
		g_hInvisTimer[hPack.ReadCell()] = null;
		return Plugin_Stop;
	}
	else {
		// Get color first, then change alpha.
		// This makes it so if another plugin changed color, we don't override it.
		int iColor[4];
		p.GetRenderColor(iColor);
		iColor[3] = 255;
		p.SetRenderColor(iColor);
	}
	g_hInvisTimer[p.Index] = null;
	return Plugin_Stop;
}

public Action Event_PlayerDeath(Event hEvent, const char[] sName, bool bDontBroadcast) {
	CCSPlayer p = CCSPlayer.FromEvent(hEvent, "userid");
	
	if(p.InGame) {
		ResetPlayer(p);
	}
}

public Action Event_RoundStart(Event hEvent, const char[] sName, bool bDontBroadcast) {
	g_bBetweenRounds = false;
	g_iRoundStart = GetTime();
}

public Action Event_RoundEnd(Event hEvent, const char[] sName, bool bDontBroadcast) {
	g_bBetweenRounds = true;
	// Reset players for 
	for(CCSPlayer p = CCSPlayer(0); CCSPlayer.Next(p);) {
		if(p.InGame) {
			ResetPlayer(p);
		}
	}
}

public Action Command_Rtd(int iClient, int iArgs) {
	
	// Already Rolled
	if(g_iRoll[iClient] != -1) {
		JB_ReplyToCommand(iClient, PREFIX ... "{RED}You have already rolled!");
		return Plugin_Handled;
	}
	
	// Don't allow rtd between rounds
	if(g_bBetweenRounds) {
		JB_ReplyToCommand(iClient, PREFIX ... "{RED}You cannot rtd between rounds!");
		return Plugin_Handled;
	}
	
	// Past time to rtd
	if(GetTime() > g_iRoundStart + g_hTimeLimit.IntValue) {
		JB_ReplyToCommand(iClient, PREFIX ... "{RED}Its too late to rtd!");
		return Plugin_Handled;
	}
	
	// If teams are limited and user isn't a T (Or is spectatr)
	CCSPlayer p = CCSPlayer(iClient);
	if(p.Team == CS_TEAM_SPECTATOR || (g_hLimitTeams.BoolValue && p.Team != CS_TEAM_T)) {
		JB_ReplyToCommand(iClient, PREFIX ... "{RED}You must be a T to rtd!");
		return Plugin_Handled;
	}
	
	int iMoney = p.Money;
	
	// User doesn't have enough money
	if(g_hMoneyPrice.IntValue > iMoney) {
		JB_ReplyToCommand(iClient, "{RED}You must have at least {GREEN}$%d {RED}to rtd!", g_hMoneyPrice.IntValue);
		return Plugin_Handled;
	}
	
	p.Money = iMoney - g_hMoneyPrice.IntValue;
	
	// Gets a random effect
	int iIndex = GetRandomInt(0, g_hEffects.Length - 1);
	
	// Store Unique ID of the roll
	g_iRoll[iClient] = g_hEffects.Get(iIndex, BLOCK_ID);
	
	// Print name of the roll
	char sName[MAX_ROLL_LEN];
	DataPack hPack = g_hEffects.Get(iIndex, BLOCK_NAME);
	hPack.Reset();
	hPack.ReadString(sName, sizeof(sName));
	JB_PrintToChat(iClient, PREFIX ... "{LIGHTBLUE}You rolled a {GREEN}%d{LIGHTBLUE} and got {ORANGE}%s{LIGHTBLUE}!", g_iRoll[iClient], sName);
	
	// Get callback datapack
	hPack = g_hEffects.Get(iIndex, BLOCK_CALLBACK);
	hPack.Reset();
	
	// Call the function for the roll rolled
	Call_StartFunction(null, hPack.ReadFunction());
	Call_PushCell(iClient);
	Call_Finish();
	
	return Plugin_Handled;
}

public Action Command_RtdChances(int iClient, int iArgs) {
	ArrayList hNames = new ArrayList(ByteCountToCells(MAX_ROLL_LEN));
	ArrayList hTickets = new ArrayList();
	
	int iLastId = -1;
	for(int i = 0; i < g_hEffects.Length; i++) {
		// Current roll's id, as well as the index for hNames and hTickets.
		int iId = g_hEffects.Get(i, BLOCK_ID);
		
		// On a new roll
		if(iId != iLastId) {
			// Push single ticket & name to arrays.
			char sName[MAX_ROLL_LEN];
			DataPack hPack = g_hEffects.Get(i, BLOCK_NAME);
			hPack.Reset();
			hPack.ReadString(sName, sizeof(sName));
			hNames.PushString(sName);
			hTickets.Push(1);
			iLastId = iId;
		}
		// On the same roll as last iteration
		else { 
			// iId represents the current's roll's index.
			// Increment tickets for current roll.
			hTickets.Set(iId, hTickets.Get(iId) + 1);
		}
	}
	
	// Print chances to console
	for(int i = 0; i < hNames.Length; i++) {
		char sName[MAX_ROLL_LEN];
		hNames.GetString(i, sName, sizeof(sName));
		PrintToConsole(iClient, "%d %s %.2f%%", i, sName, float(hTickets.Get(i)) / float(hNames.Length) * 100.0);
	}
	
	// Notify check for console if command was from chat
	if(GetCmdReplySource() == SM_REPLY_TO_CHAT) {
		JB_PrintToChat(iClient, PREFIX ... "{ORANGE}Check console for details.");
	}
	
	// Cleanup
	delete hNames;
	delete hTickets;
	
	return Plugin_Handled;
}

int CreateEffect(const char[] sName, int iTickets, OnRoll callback) {
	// Used as a unique id for each effect
	static int iId = 0;
	
	// Push effect iTicket times into the array
	for(int i = 0; i < iTickets;i++) {
		DataPack hNamePack = new DataPack();
		hNamePack.WriteString(sName);
		int iIndex = g_hEffects.Push(iId);
		DataPack hCbPack = new DataPack();
		hCbPack.WriteFunction(callback);
		g_hEffects.Set(iIndex, hCbPack, BLOCK_CALLBACK);
		g_hEffects.Set(iIndex, hNamePack, BLOCK_NAME);
	}
	
	// Return iId and increment for next effect
	return iId++;
}

void ResetPlayer(CCSPlayer p) {
	// Player is currently blinded
	if(p.InGame && g_iRoll[p.Index] == g_iBlindId) {
		// Removes current Fade usermessage
		Handle hMsg = StartMessageOne("Fade", p.Index, USERMSG_RELIABLE | USERMSG_BLOCKHOOKS);
		PbSetInt(hMsg, "duration", 5000);
		PbSetInt(hMsg, "hold_time", 1500);
		PbSetInt(hMsg, "flags", 0x0010); // PURGE
		PbSetColor(hMsg, "clr", {255, 255, 255, 0}); // Full alpha to be invisible
		EndMessage();
	}
	else if(p.InGame && g_iRoll[p.Index] == g_iLowGravId) { // User has low grav
		p.Gravity = 1.0;
	}
	else if(g_iRoll[p.Index] == g_iInvisId) { // User has invis
		delete g_hInvisTimer[p.Index];
		// TODO: Verify that player render color stays through round change,
		// This part may be unnecessary.
		if(p.InGame) {
			int iColor[4];
			p.GetRenderColor(iColor);
			iColor[3] = 255;
			p.SetRenderColor(iColor);
		}
		g_bHasInvis[p.Index] = false;
	}
	
	delete g_hPoisonTimer[p.Index];
	
	g_iRoll[p.Index] = -1;
}

// Debug command used to test rolls.
// Less sanity checking because we're going to assume whoever manually enables
// this knows what they're doing
public Action Command_ForceRoll(int iClient, int iArgs) {
	
	// Ensure arguments exist
	if(iArgs == 0) {
		ReplyToCommand(iClient, PREFIX ... "Usage: sm_forceroll <unique id>");
		return Plugin_Handled;
	}
	
	// Index to the roll to give the player
	char sIndex[8];
	GetCmdArg(1, sIndex, sizeof(sIndex));
	int iIndex = StringToInt(sIndex);
	
	// Search for effect with the unique id given
	int iFind = g_hEffects.FindValue(iIndex, BLOCK_ID);
	
	// If not found, tell them index must be >= 0 and <= largest unique id
	if(iFind == -1) {
		ReplyToCommand(iClient, "Unique ID must be between 0 and %d inclusively.", g_hEffects.Get(g_hEffects.Length - 1, BLOCK_ID));
		return Plugin_Handled;
	}
	
	g_iRoll[iClient] = iIndex;
	
	// Print roll given
	char sName[MAX_ROLL_LEN];
	DataPack hPack = g_hEffects.Get(iFind, BLOCK_NAME);
	hPack.Reset();
	hPack.ReadString(sName, sizeof(sName));
	ReplyToCommand(iClient, "Forcing roll to %s", sName);
	
	// Get callback to call
	hPack = g_hEffects.Get(iFind, BLOCK_CALLBACK);
	hPack.Reset();
	
	// Give them the roll
	Call_StartFunction(null, hPack.ReadFunction());
	Call_PushCell(iClient);
	Call_Finish();
	
	return Plugin_Handled;
}

public void Roll_DoNothing(CCSPlayer p) {
	
}

public void Roll_IncreaseHp(CCSPlayer p) {
	p.Health = p.Health + GetRandomInt(g_hIncreaseHpMin.IntValue, g_hIncreaseHpMax.IntValue);
}

public void Roll_DecreaseHp(CCSPlayer p) {
	p.Health = p.Health - GetRandomInt(g_hDecreaseHpMin.IntValue, g_hDecreaseHpMax.IntValue);
}

public void Roll_RandomHp(CCSPlayer p) {
	if(GetRandomInt(0,1) % 2) {
		Roll_IncreaseHp(p);
	}
	else {
		Roll_DecreaseHp(p);
	}
}

public void Roll_Slowed(CCSPlayer p) {
	// This isn't flipped. Max = slower, min = less slow.
	p.Speed = GetRandomFloat(g_hSlowedMax.FloatValue, g_hSlowedMin.FloatValue);
}

public void Roll_Meep(CCSPlayer p) {
	p.Speed = GetRandomFloat(g_hMeepMin.FloatValue, g_hMeepMax.FloatValue);
}

public void Roll_RandSpeed(CCSPlayer p) {
	if(GetRandomInt(0,1) % 2) {
		Roll_Slowed(p);
	}
	else {
		Roll_Meep(p);
	}
}

public void Roll_Blindness(CCSPlayer p) {
	Handle hMsg = StartMessageOne("Fade", p.Index, USERMSG_RELIABLE | USERMSG_BLOCKHOOKS);
	PbSetInt(hMsg, "duration", 5000); // TODO: find correct duration
	PbSetInt(hMsg, "hold_time", 1500); // TODO: find correct hold time
	PbSetInt(hMsg, "flags", 0x0008 | 0x0010); // STAYOUT | PURGE
	PbSetColor(hMsg, "clr", {0, 0, 0, 100}); // TODO: Double check color. Make blindness amount random?
	EndMessage();
}

public void Roll_Invis(CCSPlayer p) {
	g_bHasInvis[p.Index] = true;
	JB_PrintToChat(p.Index, PREFIX ... "{ORANGE}Press Inspect to use your invisiblity!");
}

public void Roll_Ignite(CCSPlayer p) {
	p.Ignite(g_hIgniteTime.FloatValue);
}

public void Roll_SmallArmor(CCSPlayer p) {
	p.Armor = 100;
	p.Helmet = true;
}

public void Roll_HeavyArmor(CCSPlayer p) {
	p.Armor = 10; // Not a typo, low armor so after some damage the armor will go away
	p.Helmet = true;
	p.HeavyArmor = true;
}

public void Roll_Grenades(CCSPlayer p) {
	switch(GetRandomInt(0, 7)) {
		case 0: {
			GivePlayerWeapon(p, "weapon_flashbang");
			GivePlayerWeapon(p, "weapon_hegrenade");
			GivePlayerWeapon(p, "weapon_smokegrenade");
		}
		case 1,2: {
			GivePlayerWeapon(p, "weapon_flashbang");
		}
		case 3: {
			GivePlayerWeapon(p, "weapon_hegrenade");
		}
		case 4: {
			GivePlayerWeapon(p, "weapon_smokegrenade");
		}
		case 5: {
			GivePlayerWeapon(p, "weapon_molotov");
		}
		case 6: {
			GivePlayerWeapon(p, "weapon_tagrenade");
		}
	}
}

public void Roll_Glock(CCSPlayer p) {
	CWeapon wep = GivePlayerWeapon(p, "weapon_glock");
	wep.ReserveAmmo = wep.Ammo * GetRandomInt(0, g_hMaxGlockReserve.IntValue);
}

public void Roll_Deagle(CCSPlayer p) {
	CWeapon wep = GivePlayerWeapon(p, "weapon_deagle");
	wep.ReserveAmmo = wep.Ammo * GetRandomInt(0, g_hMaxDeagReserve.IntValue);
}

public void Roll_Wrench(CCSPlayer p) {
	CWeapon wep = GivePlayerWeapon(p, "weapon_spanner");
	p.EquipItem(wep);
}

public void Roll_Axe(CCSPlayer p) {
	CWeapon wep = GivePlayerWeapon(p, "weapon_axe");
	p.EquipItem(wep);
}

public void Roll_Hammer(CCSPlayer p) {
	CWeapon wep = GivePlayerWeapon(p, "weapon_hammer");
	p.EquipItem(wep);
}

public void Roll_Knife(CCSPlayer p) {
	CWeapon wep = GivePlayerWeapon(p, "weapon_knife");
	p.EquipItem(wep);
}

int Min(int x, int y) {
	return x < y ? x : y;
}

int Max(int x, int y) {
	return x > y ? x : y;
}

public void Roll_Lottery(CCSPlayer p) {
	int iMoney = p.Money;
	
	// divided by 100 for pretty numbers only
	int iAmount = GetRandomInt(g_hLotteryMin.IntValue, g_hLotteryMax.IntValue) / 100 * 100;
	
	// Ensure player doesn't exceed mp_maxmoney value
	iMoney = Min(iMoney + iAmount, g_hMaxMoney.IntValue);
	
	p.Money = iMoney;
	
	JB_PrintToChat(p.Index, PREFIX ... "{LIGHTGREEN}You won {GREEN}$%d{LIGHTGREEN} from the lottery!", iAmount);
}

public void Roll_Robbery(CCSPlayer p) {
	int iMoney = p.Money;
	
	// divided by 100 for pretty numbers only
	int iAmount = GetRandomInt(g_hRobberyMin.IntValue, g_hRobberyMax.IntValue) / 100 * 100;
	
	// Ensure player doesn't underflow
	iMoney = Max(iMoney - iAmount, 0);
	
	p.Money = iMoney;
	
	JB_PrintToChat(p.Index, PREFIX ... "{ORANGE}You lost {RED}$%d {ORANGE}from a robbery.", iAmount);
}

public void Roll_Snowballs(CCSPlayer p) {
	for(int i = 0; i < g_hSnowballAmount.IntValue;i++) {
		GivePlayerWeapon(p, "weapon_snowball");
	}
}

public void Roll_Medic(CCSPlayer p) {
	GivePlayerWeapon(p, "weapon_healthshot");
}

public void Roll_LowGrav(CCSPlayer p) {
	p.Gravity = g_hLowGrav.FloatValue;
}

public void Roll_RubberBullets(CCSPlayer p) {
	// TODO: Tell them how much damage is reduced by?
}

public void Roll_BurningBullets(CCSPlayer p) {
	// Do nothing
}

public void Roll_FallDamage(CCSPlayer p) {
	// Do Nothing
}

public void Roll_Model(CCSPlayer pRoller) {
	// Loop until we find a valid CT to steal their model
	for(CCSPlayer p = CCSPlayer(0); CCSPlayer.Next(p);) {
		
		// Player in Game & CT
		if(p.InGame && p.Team == CS_TEAM_CT) {
			
			// Get model of CT
			char sModel[PLATFORM_MAX_PATH];
			p.GetModel(sModel, sizeof(sModel));
			
			// Apply it to the roller
			pRoller.SetModel(sModel);
			
			break;
		}
	}
}

public void Roll_Poisoned(CCSPlayer p) {
	
	// Store render color in case another plugin changed it before us
	p.GetRenderColor(g_iPrePoisonColor[p.Index]);
	// Used to check for healing later
	g_iLastHealth[p.Index] = p.Health;
	
	DataPack hPack = new DataPack();
	g_hPoisonTimer[p.Index] = CreateDataTimer(g_hPoisonInterval.FloatValue, Timer_Poison, hPack, TIMER_REPEAT);
	hPack.WriteCell(p.UserID);
	hPack.WriteCell(p);
	
}

public Action Timer_Poison(Handle hTimer, DataPack hPack) {
	hPack.Reset();
	CCSPlayer p = CCSPlayer.FromUserId(hPack.ReadCell());
	
	// User disconnected
	if(p.IsNull) {
		g_hPoisonTimer[hPack.ReadCell()] = null;
		return Plugin_Stop;
	}
	else {
		// If the random number is less than the % chance of stopping,
		// OR
		// User has healed through other means,
		// Poison should stop.
		if(GetRandomInt(0, 100) < g_hPoisonStopChance.IntValue || g_iLastHealth[p.Index] < p.Health) {
			PrintToChat(p.Index, PREFIX ... "Your poison has been cured.");
			p.SetRenderColor(g_iPrePoisonColor[p.Index]);
			g_hPoisonTimer[p.Index] = null;
			return Plugin_Stop;
		}
		p.Health = p.Health - g_hPoisonDmg.IntValue;
		g_iLastHealth[p.Index] = p.Health;
		return Plugin_Continue;
	}
}