#if defined _cbaseplayer_included
	#endinput
#endif
#define _cbaseplayer_included
#include <cweapon>

//#define FORP(%1) for(CBasePlayer p = CBasePlayer(1); p.Index <= MaxClients; p = p.NextPlayer())

enum CBasePlayer 
{
	NULL_CBASEPLAYER = -1
}

methodmap CBasePlayer < CEntity
{
	/*
	 * Creates new CBasePlayer of the client index 
	 * returns NULL_CBASEPLAYER if index is invalid 
	 */
	public CBasePlayer(int iIndex)
	{
		if (iIndex > MaxClients || iIndex <= 0)
		{
			return NULL_CBASEPLAYER;
		}
		return view_as<CBasePlayer>(iIndex);
	}
	// Creates a new CBasePlayer from a userid
	public static CBasePlayer FromUserId(int iUserid)
	{
		return CBasePlayer(GetClientOfUserId(iUserid));
	}
	// creates a new CBasePlayer from an event
	public static CBasePlayer FromEvent(Event hEvent, const char[] sField)
	{
		return CBasePlayer.FromUserId(hEvent.GetInt(sField));
	}
	/*
	 * Use in a while loop to loop through all players. 
	 * CBasePlayer p;
	 * while(CBasePlayer.Next(p))...
	 */
	public static bool Next(CBasePlayer &p)
	{
		if(p.IsNull || p.Index == 0)
		{
			p = CBasePlayer(1);
			return true;
		}
		else if(p.Index == MaxClients)
		{
			return false;
		}
		p = CBasePlayer(p.Index + 1);
		return true;
	}
	// casts this as an int
	property int Index
	{
		public get()
		{
			return view_as<int>(this);
		}
	}
	// Gets if a player is in game
	property bool InGame
	{
		public get()
		{
			return IsClientInGame(this.Index);
		}
	}
	// Gets if a player is alive
	property bool Alive
	{
		public get()
		{
			return IsPlayerAlive(this.Index);
		}
	}
	// Gets if a player is connected
	property bool Connected
	{
		public get()
		{
			return IsClientConnected(this.Index);
		}
	}
	// Gets if a player is authorized.
	property bool Authorized
	{
		public get()
		{
			return IsClientAuthorized(this.Index);
		}
	}
	// Gets if a player is an obvserver
	property bool Observer
	{
		public get()
		{
			return IsClientObserver(this.Index);
		}
	}
	// Gets if a player is a fake client
	property bool FakeClient
	{
		public get()
		{
			return IsFakeClient(this.Index);
		}
	}
	// Gets if a player is a SourceTV Bot
	property bool SourceTV
	{
		public get()
		{
			return IsClientSourceTV(this.Index);
		}
	}
	// Gets/Sets a player's deaths
	property int Deaths
	{
		public get()
		{
			return GetClientDeaths(this.Index);
		}
		public set(int iDeaths)
		{
			SetEntProp(this.Index, Prop_Data, "m_iDeaths", iDeaths);
		}
	}
	// Gets/Sets a player's health
	property int Health
	{
		public get()
		{
			return GetClientHealth(this.Index);
		}
		public set(int iHealth)
		{
			SetEntityHealth(this.Index, iHealth);
		}
	}
	// Gets/Sets a player's frags
	property int Frags
	{
		public get()
		{
			return GetClientFrags(this.Index);
		}
		public set(int iFrags)
		{
			SetEntProp(this.Index, Prop_Data, "m_iFrags", iFrags);
		}
	}
	// Gets/sets a player's team
	property int Team
	{
		public get()
		{
			return GetClientTeam(this.Index);
		}
		public set(int iTeam)
		{
			ChangeClientTeam(this.Index, iTeam);
		}
	}
	// Gets/sets a player's armor
	property int Armor
	{
		public get()
		{
			return GetClientArmor(this.Index);
		}
		public set(int iArmor)
		{
			SetEntProp(this.Index, Prop_Data, "m_ArmorValue", iArmor);
		}
	}
	// returns a player's userid
	property int UserID
	{
		public get()
		{
			return GetClientUserId(this.Index);
		}
	}
	// returns a player's steam account id
	property int AccountID
	{
		public get()
		{
			return GetSteamAccountID(this.Index, true);
		}
	}
	// Gets/sets a player's speed
	property float Speed
	{
		public get()
		{
			return GetEntPropFloat(this.Index, Prop_Send, "m_flLaggedMovementValue");
		}
		public set(float fSpeed)
		{
			SetEntPropFloat(this.Index, Prop_Send, "m_flLaggedMovementValue", fSpeed);
		}
	}
	// returns the active weapon
	property CWeapon ActiveWeapon
	{
		public get()
		{
			return CWeapon.FromIndex(GetEntPropEnt(this.Index, Prop_Data, "m_hActiveWeapon"));
		}
	}
	public void Steam64(char[] sBuffer, int iMax)
	{
		GetClientAuthId(this.Index, AuthId_SteamID64, sBuffer, iMax);
	}
	// Gets a weapon from a slot
	public CWeapon GetWeapon(int iSlot)
	{
		return CWeapon.FromIndex(GetPlayerWeaponSlot(this.Index, iSlot));
	}
	// Forces a player to equip a weapon
	public void EquipItem(CWeapon cWep)
	{
		EquipPlayerWeapon(this.Index, cWep.Index);
	}
	// Removes a weapon from a player
	public void RemoveItem(CWeapon cWep)
	{
		RemovePlayerItem(this.Index, cWep.Index);
	}
	// Gets a player's name
	public void GetName(char[] sBuffer, int iMaxLen)
	{
		GetClientName(this.Index, sBuffer, iMaxLen);
	}
	// Sets a player's name
	public void SetName(char[] sName, any ...)
	{
		char sBuffer[MAX_NAME_LENGTH];
		VFormat(sBuffer, sizeof(sBuffer), sName, 3);
		SetClientName(this.Index, sBuffer);
	}
	// Gets the player's model
	public void GetModel(char[] sModel, int iSize)
	{
		GetClientModel(this.Index, sModel, iSize);
	}
	// Gets a player's abs origin
	public void AbsOrigin(float fLoc[3])
	{
		GetClientAbsOrigin(this.Index, fLoc);
	}
	// Gets a player's eye location
	public void EyePosition(float fLoc[3])
	{
		GetClientEyePosition(this.Index, fLoc);
	}
	// Gets a player's eye angle
	public void EyeAngle(float fAngles[3])
	{
		GetClientEyeAngles(this.Index, fAngles);
	}
	// gets a players vecVelocity
	public void VecVelocity(float fVel[3])
	{
		GetEntPropVector(this.Index, Prop_Data, "m_vecVelocity", fVel);
	}
	// Gets if a player can target another player
	public bool CanTarget(CBasePlayer p)
	{
		return CanUserTarget(this.Index, p.Index);
	}
	// Kicks a player
	public void Kick(char[] sReason, any...)
	{
		char sFormat[128];
		VFormat(sFormat, sizeof(sFormat), sReason, 3);
		KickClient(this.Index, sFormat);
	}
	// Bans a player
	public void Ban(int iTime, int iFlags, char[] sReason, char[] sKickMsg, char[] sCommand, any source)
	{
		BanClient(this.Index, iTime, iFlags, sReason, sKickMsg, sCommand, source);
	}
	// Forces a suicide
	public void Kill()
	{
		ForcePlayerSuicide(this.Index);
	}
	// Forces a client command
	public void Command(char[] sCommand, any ...)
	{
		char sBuffer[128];
		VFormat(sBuffer, sizeof(sBuffer), sCommand, 3);
		ClientCommand(this.Index, sBuffer);
	}
	// Fakes a client command
	public void FakeCommand(char[] sCommand, any ...)
	{
		char sBuffer[128];
		VFormat(sBuffer, sizeof(sBuffer), sCommand, 3);
		FakeClientCommand(this.Index, sBuffer);
	}
	// Gets entity that the player is looking at.
	public CEntity GetAimTarget(bool bPlayersOnly = true)
	{
		return CEntity.FromIndex(GetClientAimTarget(this.Index, bPlayersOnly));
	}
	public void GetSteamId(char[] sBuffer, int iMax)
	{
		GetClientAuthId(this.Index, AuthId_Steam2, sBuffer, iMax);
	}
	// Gets a client's IP
	public void GetIP(char[] sBuffer, int iMaxLen, bool bRemovePort = true)
	{
		GetClientIP(this.Index, sBuffer, iMaxLen, bRemovePort);
	}
}

/* Use in a while loop to loop through all clients
 * This does not guarantee that the CBasePlayer is in game.
 */
 #pragma deprecated Use CBasePlayer.Next().
stock bool GetNextPlayer(CBasePlayer &p)
{
	if(p == NULL_CBASEPLAYER || p.Index == 0)
	{
		p = CBasePlayer(1);
		return true;
	}
	else if(p.Index == MaxClients)
	{
		return false;
	}
	p = CBasePlayer(p.Index + 1);
	return true;
}

// Gets a CBasePlayer from an event
#pragma deprecated Use CBasePlayer.FromEvent().
stock CBasePlayer GetEventCBasePlayer(Handle hEvent, const char[] sField)
{
	return UserIdToCBasePlayer(GetEventInt(hEvent, sField));
}

// Converts a User ID to a CBasePlayer
#pragma deprecated Use CBasePlayer.FromUserId().
stock CBasePlayer UserIdToCBasePlayer(int iUserId)
{
	return CBasePlayer(GetClientOfUserId(iUserId));
}