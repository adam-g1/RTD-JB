#include <sourcemod>
#include <ccsplayer>
#include <AutoExecConfig>

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
#define MAX_CB_LEN 64
/*
 * Block 0:				Unique ID
 * Block 1:				Name of the callback inside a DataPack
 * Block 2:				Name inside a DataPack
 */
ArrayList g_hEffects;
#define BLOCK_ID 0
#define BLOCK_CALLBACK 1
#define BLOCK_NAME 2

bool g_bBetweenRounds = false;

int g_iBlindId;

// Stores the Unique ID of the roll the player rolled.
int g_iRoll[MAXPLAYERS + 1];
bool g_bHasInvis[MAXPLAYERS + 1];
Handle g_hInvisTimer[MAXPLAYERS + 1];

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

public void OnPluginStart() {
	g_hEffects = new ArrayList(3);
	
	HookEvent("player_death", Event_PlayerDeath);
	HookEvent("round_end", Event_RoundEnd);
	HookEvent("round_start", Event_RoundStart);
	
	// Used for invisibility
	AddCommandListener(Command_Inspect, "+lookatweapon");
	
	// Create Effects
	CreateEffect("No Effect", 3, "Roll_DoNothing");
	CreateEffect("Increased HP", 2, "Roll_IncreaseHp");
	CreateEffect("Decreased HP", 2, "Roll_DecreaseHp");
	CreateEffect("Random HP", 2, "Roll_RandomHp"); // Unimplemented
	CreateEffect("Slowed Down", 1, "Roll_Slowed");
	CreateEffect("Meep Meep", 1, "Roll_Meep");
	CreateEffect("Random Speed", 1, "Roll_RandSpeed"); // Unimplemented
	g_iBlindId = CreateEffect("Blindness", 2, "Roll_Blindness");
	CreateEffect("Temporary Invisibility", 1, "Roll_Invis");
	CreateEffect("Spontaneous Ignition", 2, "Roll_Ignite");
	CreateEffect("Small Armor", 2, "Roll_SmallArmor");
	CreateEffect("Heavy Armor", 1, "Roll_HeavyArmor");
	CreateEffect("Grenades", 2, "Roll_Grenades");
	CreateEffect("Glock", 2, "Roll_Glock");
	CreateEffect("Deagle", 1, "Roll_Deagle");
	CreateEffect("Wrench", 2, "Roll_Wrench");
	CreateEffect("Axe", 2, "Roll_Axe");
	CreateEffect("Hammer", 2, "Roll_Hammer");
	CreateEffect("Knife", 2, "Roll_Knife");
	CreateEffect("Snowballs", 2, "Roll_Snowballs"); // Unimplemented TODO: Find model that needs to be precached
	CreateEffect("Rubber Bullets", 2, "Roll_RubberBullets"); // Unimplemented
	CreateEffect("Burning Bullets", 2, "Roll_BurningBullets"); // Unimplemented
	CreateEffect("Poisoned", 1, "Roll_Poisoned"); // Unimplemented
	CreateEffect("Guard Model", 1, "Roll_Model"); // Unimplemented
	CreateEffect("Long Fall Boots", 1, "Roll_FallDamage"); // Unimplemented
	CreateEffect("Moon Boots", 1, "Roll_LowGrav"); // Unimplemented
	CreateEffect("Lottery", 1, "Roll_Lottery"); // Unimplemented
	CreateEffect("Robbery", 1, "Roll_Robbery"); // Unimplemented
	CreateEffect("Medic", 2, "Roll_Medic"); // Unimplemented
	
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
		DataPack hPack = new DataPack();
		hPack.WriteCell(GetClientUserId(iClient));
		hPack.WriteCell(iClient);
		
		// Alpha only works when using RENDER_TRANSALPHA
		CCSPlayer(iClient).Render = RENDER_TRANSALPHA;
		g_hInvisTimer[iClient] = CreateDataTimer(0.1, Timer_IncreaseInvis, hPack);
		TriggerTimer(g_hInvisTimer[iClient]);
	}
}

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
		
		// If player is fully invisible
		if(iColor[3] <= 0) {
			// Clamp to 0 before setting color
			iColor[3] = 0;
			p.SetRenderColor(iColor);
			
			// Create new timer to remove invisibility
			DataPack hCopy = new DataPack();
			hCopy.WriteCell(p.UserID);
			hCopy.WriteCell(p);
			g_hInvisTimer[p.Index] = CreateDataTimer(g_hInvisTime.FloatValue, Timer_RemoveInvis, hCopy);
			
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
}

public Action Event_RoundEnd(Event hEvent, const char[] sName, bool bDontBroadcast) {
	g_bBetweenRounds = true;
	// Reset players for 
	for(CCSPlayer p = CCSPlayer(1); !p.IsNull; CCSPlayer.Next(p)) {
		if(p.InGame) {
			ResetPlayer(p);
		}
	}
}

public Action Command_Rtd(int iClient, int iArgs) {
	
	// Already Rolled
	if(g_iRoll[iClient] != -1) {
		ReplyToCommand(iClient, PREFIX ... "You have already rolled!");
		return Plugin_Handled;
	}
	
	// Don't allow rtd between rounds
	if(g_bBetweenRounds) {
		ReplyToCommand(iClient, PREFIX ... "You cannot rtd between rounds!");
		return Plugin_Handled;
	}
	
	// Gets a random effect
	int iIndex = GetRandomInt(0, g_hEffects.Length - 1);
	
	// Store Unique ID of the roll
	g_iRoll[iClient] = g_hEffects.Get(iIndex, BLOCK_ID);
	
	// Print name of the roll
	char sName[MAX_ROLL_LEN];
	DataPack hPack = g_hEffects.Get(iIndex, BLOCK_NAME);
	hPack.Reset();
	hPack.ReadString(sName, sizeof(sName));
	PrintToChat(iClient, PREFIX ... "You rolled a %d and got %s!", g_iRoll[iClient], sName);
	
	// Find callback
	hPack = g_hEffects.Get(iIndex, BLOCK_CALLBACK);
	char sCbName[MAX_CB_LEN];
	hPack.Reset();
	hPack.ReadString(sCbName, sizeof(sCbName));
	Function callback = GetFunctionByName(null, sCbName);
	
	// Ensure callback is valid
	if(callback == INVALID_FUNCTION) {
		ReplyToCommand(iClient, PREFIX ... "An internal error occured. Please try again.");
		LogError("Failed to find callback \"%s\", name %s", sCbName, sName);
		g_iRoll[iClient] = -1;
		return Plugin_Handled;
	}
	
	// Call the function for the roll rolled
	Call_StartFunction(null, callback);
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
		PrintToConsole(iClient, "%d %s %.2f%%", i, sName, hTickets.Get(i) / float(hNames.Length) * 100.0);
	}
	
	// Notify check for console if command was from chat
	if(GetCmdReplySource() == SM_REPLY_TO_CHAT) {
		PrintToChat(iClient, PREFIX ... "Check console for details.");
	}
	
	// Cleanup
	delete hNames;
	delete hTickets;
	
	return Plugin_Handled;
}

int CreateEffect(const char[] sName, int iTickets, const char[] sCallbackName) {
	// Used as a unique id for each effect
	static int iId = 0;
	
	// Push effect iTicket times into the array
	for(int i = 0; i < iTickets;i++) {
		DataPack hNamePack = new DataPack();
		hNamePack.WriteString(sName);
		int iIndex = g_hEffects.Push(iId);
		DataPack hCbPack = new DataPack();
		hCbPack.WriteString(sCallbackName);
		g_hEffects.Set(iIndex, hCbPack, BLOCK_CALLBACK);
		g_hEffects.Set(iIndex, hNamePack, BLOCK_NAME);
	}
	
	// Increment for next effect
	iId++;
	
	return iId;
}

void ResetPlayer(CCSPlayer p) {
	// Player is currently blinded
	if(g_iRoll[p.Index] == g_iBlindId) {
		// Removes current Fade usermessage
		// We dont care about any values other than 
		Handle hMsg = StartMessageOne("Fade", p.Index, USERMSG_RELIABLE | USERMSG_BLOCKHOOKS);
		PbSetInt(hMsg, "duration", 5000);
		PbSetInt(hMsg, "hold_time", 1500);
		PbSetInt(hMsg, "flags", 0x0010); // PURGE
		PbSetColor(hMsg, "clr", {255, 255, 255, 0}); // Full alpha to be invisible
		EndMessage();
	}
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
	
	// Print roll given
	char sName[MAX_ROLL_LEN];
	DataPack hPack = g_hEffects.Get(iIndex, BLOCK_NAME);
	hPack.Reset();
	hPack.ReadString(sName, sizeof(sName));
	ReplyToCommand(iClient, "Forcing roll to %s", sName);
	
	// Get callback to call
	hPack = g_hEffects.Get(iIndex, BLOCK_CALLBACK);
	char sCallback[MAX_CB_LEN];
	hPack.Reset();
	hPack.ReadString(sCallback, sizeof(sCallback));
	Function callback = GetFunctionByName(null, sCallback);
	
	if(callback == INVALID_FUNCTION) {
		LogError("Cannot find callback \"%s\" for roll %s. Check callback spelling.");
		ReplyToCommand(iClient, "Internal error, callback not found.");
		return Plugin_Handled;
	}
	
	// Give them the roll
	Call_StartFunction(null, callback);
	Call_PushCell(iClient);
	Call_Finish();
	
	return Plugin_Handled;
}

public void Roll_DoNothing(CCSPlayer p) {
	
}

public void Roll_IncreaseHp(CCSPlayer p) {
	p.Health = GetRandomInt(g_hIncreaseHpMin.IntValue, g_hIncreaseHpMax.IntValue);
}

public void Roll_DecreaseHp(CCSPlayer p) {
	p.Health = GetRandomInt(g_hDecreaseHpMin.IntValue, g_hDecreaseHpMax.IntValue);
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

public void Roll_Blindness(CCSPlayer p) {
	Handle hMsg = StartMessageOne("Fade", p.Index, USERMSG_RELIABLE | USERMSG_BLOCKHOOKS);
	PbSetInt(hMsg, "duration", 5000); // TODO: find correct duration
	PbSetInt(hMsg, "hold_time", 1500); // TODO: find correct hold time
	PbSetInt(hMsg, "flags", 0x0008 | 0x0010); // STAYOUT | PURGE
	PbSetColor(hMsg, "clr", {255, 255, 255, 100}); // TODO: Double check color. Make blindness amount random?
	EndMessage();
}

public void Roll_Invis(CCSPlayer p) {
	g_bHasInvis[p.Index] = true;
	PrintToChat(p.Index, PREFIX ... "Press Inspect to use your invisiblity!");
}

public void Roll_Ignite(CCSPlayer p) {
	p.Ignite(7.5);
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