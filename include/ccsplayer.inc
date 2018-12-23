#if defined _ccsplayer_included
	#endinput
#endif
#define _ccsplayer_included
#include <cbaseplayer>
#if defined REQUIRE_EXTENSIONS
	#undef REQUIRE_EXTENSIONS
	#include <cstrike>
	#define REQUIRE_EXTENSIONS
#else
	#include <cstrike>
#endif

enum CCSPlayer 
{
	NULL_CCSPLAYER = -1
};	

methodmap CCSPlayer < CBasePlayer
{
	public CCSPlayer(int iIndex)
	{
		if (iIndex > MaxClients || iIndex <= 0)
		{
			return view_as<CCSPlayer>(NULL_CCSPLAYER);
		}
		return view_as<CCSPlayer>(iIndex);
	}
	// Creates a new CCSPlayer from a userid
	public static CCSPlayer FromUserId(int iUserid)
	{
		return CCSPlayer(GetClientOfUserId(iUserid));
	}
	// Creates a new CCSPlayer from an event
	public static CCSPlayer FromEvent(Event hEvent, const char[] sField)
	{
		return CCSPlayer.FromUserId(hEvent.GetInt(sField));
	}
	/*
	 * Use in a while loop to loop through all players. 
	 * CCSPlayer p;
	 * while(CCSPlayer.Next(p))...
	 */
	public static bool Next(CCSPlayer &p)
	{
		if(p.IsNull || p.Index == 0)
		{
			p = CCSPlayer(1);
			return true;
		}
		else if(p.Index == MaxClients)
		{
			return false;
		}
		p = CCSPlayer(p.Index + 1);
		return true;
	}
	// Gets/sets a players assists
	property int Assists
	{
		public get()
		{
			return CS_GetClientAssists(this.Index);
		}
		public set(int iAssists)
		{
			CS_SetClientAssists(this.Index, iAssists);
		}
	}
	// Gets/sets a player's contribution score (CSGO only)
	property int ContributionScore
	{
		public get()
		{
			return CS_GetClientContributionScore(this.Index);
		}
		public set(int iScore)
		{
			CS_SetClientContributionScore(this.Index, iScore);
		}
	}
	// Gets/sets a player's helmet
	property bool Helmet
	{
		public get()
		{
			return view_as<bool>(GetEntProp(this.Index, Prop_Send, "m_bHasHelmet"));
		}
		public set(bool bEquip)
		{
			SetEntProp(this.Index, Prop_Send, "m_bHasHelmet", bEquip);
		}
	}
	// Gets/sets a player's defuser
	property bool Defuser
	{
		public get()
		{
			return view_as<bool>(GetEntProp(this.Index, Prop_Send, "m_bHasDefuser"));
		}
		public set(bool bEquip)
		{
			SetEntProp(this.Index, Prop_Send, "m_bHasDefuser", bEquip);
		}
	}
	// Gets/sets a player's MVP
	property int MVP
	{
		public get()
		{
			return CS_GetMVPCount(this.Index);
		}
		public set(int iMVP)
		{
			CS_SetMVPCount(this.Index, iMVP);
		}
	}
	// Gets/Sets a players money. If you're using the event player_spawn you must use RequestFrame() if you're lowering money. Increasing can be used inside it.
	property int Money
	{
		public get()
		{
			return this.GetProp(Prop_Send, "m_iAccount");
		}
		public set(int iMoney)
		{
			// Prevent money from going over the maximum value
			if(iMoney > 64000)
			{
				this.SetProp(Prop_Send, "m_iAccount", 64000);
			}
			else
			{
				this.SetProp(Prop_Send, "m_iAccount", iMoney);
			}
		}
	}
	// gets/sets if a player has heavy armor
	property bool HeavyArmor
	{
		public get()
		{
			return view_as<bool>(this.GetProp(Prop_Send, "m_bHasHeavyArmor"));
		}
		public set(bool bHas)
		{
			this.SetProp(Prop_Send, "m_bHasHeavyArmor", bHas);
		}
	}
	// Drops a players weapon
	public void DropItem(CWeapon cWep, bool bToss, bool bBlockHook = false)
	{
		CS_DropWeapon(this.Index, cWep.Index, bToss, bBlockHook);
	}
	// Gets a clan tag
	public void GetClanTag(char[] sBuffer, int iMaxlen)
	{
		CS_GetClientClanTag(this.Index, sBuffer, iMaxlen);
	}
	// Sets a clan tag
	public void SetClanTag(char[] sTag, any ...)
	{
		char sBuffer[32];
		VFormat(sBuffer, sizeof(sBuffer), sTag, 3);
		CS_SetClientClanTag(this.Index, sBuffer);
	}
	// Respawns a player
	public void Respawn()
	{
		CS_RespawnPlayer(this.Index);
	}
	// Switches team using CS_SwitchTeam
	public void SwitchTeam(int iTeam)
	{
		CS_SwitchTeam(this.Index, iTeam);
	}
}

// Use in a while loop to loop through all clients
#pragma deprecated Use CCSPlayer.Next().
stock bool GetNextCCSPlayer(CCSPlayer &p)
{
	if(p.IsNull || p.Index == 0)
	{
		p = CCSPlayer(1);
		return true;
	}
	else if(p.Index == MaxClients)
	{
		return false;
	}
	p = CCSPlayer(p.Index + 1);
	return true;
}

#pragma deprecated Use CCSPlayer.FromEvent().
stock CCSPlayer GetEventCCSPlayer(Handle hEvent, const char[] sField)
{
	return UserIdToCCSPlayer(GetEventInt(hEvent, sField));
}

// converts UserID to CCSPlayer
#pragma deprecated Use CCSPlayer.FromUserId().
stock CCSPlayer UserIdToCCSPlayer(int iUserID)
{
	return CCSPlayer(GetClientOfUserId(iUserID));
}

// returns if this game allows for CCSPlayer
stock bool CCSPlayerAvailable()
{
	static int iChecked = -1;
	if(iChecked == -1)
	{
		char sGame[16];
		GetGameFolderName(sGame, sizeof(sGame))
		if(StrEqual(sGame, "csgo") || StrEqual(sGame, "cstrike"))
		{
			iChecked = 1;
		}
		else
		{
			iChecked = 0;
		}
	}
	return view_as<bool>(iChecked);
}