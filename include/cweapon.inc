#if defined _cweapon_included
	#endinput
#endif
#define _cweapon_included
#include <centity>
#if defined REQUIRE_EXTENSIONS
	#undef REQUIRE_EXTENSIONS
	#include <cstrike>
	#define REQUIRE_EXTENSIONS
#else
	#include <cstrike>
#endif

static StringMap g_hWeaponIndexes;
static StringMap g_hWeaponAmmo;
static bool g_bNeedsGameCheck = true;
//static bool g_bNeedsItemDef = false;


/*
 * Each symbol has their corresponding item definition index.
 * Because of this, weapon_healthshot and weapon_tagrenade are not included.
 */
enum WeaponDef
{
	Weapon_Invalid = -1,
	Weapon_Deagle = 1,
	Weapon_Elite = 2,
	Weapon_FiveSeven = 3,
	Weapon_Glock = 4,
	Weapon_Ak47 = 7,
	Weapon_Aug = 8,
	Weapon_Awp = 9,
	Weapon_Famas = 10,
	Weapon_G3 = 11,
	Weapon_Galil = 13,
	Weapon_M249 = 14,
	Weapon_M4A4 = 16,
	Weapon_Mac10 = 17,
	Weapon_P90 = 19,
	Weapon_MP5SD = 23,
	Weapon_Ump = 24,
	Weapon_Xm1014 = 25,
	Weapon_Bizon = 26,
	Weapon_Mag7 = 27,
	Weapon_Negev = 28,
	Weapon_SawedOff = 29,
	Weapon_Tec9 = 30,
	Weapon_Taser = 31,
	Weapon_P2000 = 32,
	Weapon_Mp7 = 33,
	Weapon_Mp9 = 34,
	Weapon_Nova = 35,
	Weapon_P250 = 36,
	Weapon_Scar20 = 38,
	Weapon_SG = 39,
	Weapon_Scout = 40,
	Weapon_Knife = 59,
	Weapon_Flashbang = 43,
	Weapon_HE = 44,
	Weapon_Smoke = 45,
	Weapon_Molotov = 46,
	Weapon_Decoy = 47,
	Weapon_Incin = 48,
	Weapon_C4 = 49,
	Weapon_M4A1S = 60,
	Weapon_USP = 61,
	Weapon_CZ = 63,
	Weapon_Revolver = 64,
	Weapon_KnifeCT = 42,
	Weapon_Bayonet = 500,
	Weapon_FlipKnife = 505,
	Weapon_GutKnife = 506,
	Weapon_Karambit = 507,
	Weapon_M9Bayonet = 508,
	Weapon_Huntsman = 509,
	Weapon_Butterfly = 515,
	Weapon_Falchion = 512,
	Weapon_Daggers = 516,
	Weapon_Bowie = 514,
	Weapon_Diversion = 82,
	Weapon_Frag_Grenade = 83,
	Weapon_Firebomb = 81,
	Weapon_BreachCharge = 70,
	Weapon_Spanner = 78,
	Weapon_Hammer = 76,
	Weapon_Axe = 75,
	Weapon_Tablet = 72
}

// We want to be able to differentiate WeaponGroups from WeaponDefs easily.
// To do this, we're going to pick an arbitrary high number for the first valid one.
enum WeaponGroup
{
	WeaponGroup_Invalid = -1,
	WeaponGroup_Pistol = 600,
	WeaponGroup_Rifle,
	WeaponGroup_Heavy,
	WeaponGroup_Knife,
	WeaponGroup_Grenade,
	WeaponGroup_SMG,
	WeaponGroup_C4,
	WeaponGroup_Taser,
	WeaponGroup_Healthshot,
	WeaponGroup_Unknown
}

enum CWeapon 
{
	NULL_CWEAPON = -1
}

methodmap CWeapon < CEntity
{
	/*
	 * This constructor is primarily used to spawn weapons into the map.
	 * If you want to directly give a player a weapon, use GivePlayerWeapon
	 * As of 1/1/16, this constructor does not work with silenced weapons.
	 */
	public CWeapon(char[] sWeapon)
	{
		if(g_hWeaponIndexes == null)
		{
			CWeaponCreateWeapons();
		}
		if(g_bNeedsGameCheck)
		{
			CWeapon_CheckGame();
		}
		CEntity eEnt = CEntity(sWeapon);
		eEnt.Spawn();
		return view_as<CWeapon>(eEnt);
	}
	// Gets a CWeapon from an entity index
	public static CWeapon FromIndex(int iIndex)
	{
		return view_as<CWeapon>(CEntity.FromIndex(iIndex));
	}
	public static CWeapon FromRef(int iRef)
	{
		return view_as<CWeapon>(CEntity.FromRef(iRef));
	}
	public static WeaponGroup GroupFromDef(WeaponDef def)
	{
		switch(view_as<int>(def))
		{
			case 1, 2, 3, 4, 12, 36, 61, 63, 32, 30, 64:
			{
				return view_as<WeaponGroup>(WeaponGroup_Pistol);
			}
			case 7, 8, 9, 10, 11, 13, 16, 38, 39, 40, 60:
			{
				return WeaponGroup_Rifle;
			}
			case 14, 25, 27, 28, 29, 35:
			{
				return WeaponGroup_Heavy;
			}
			case 500, 506, 507, 508, 509, 515, 42, 59, 512, 516, 514, 505:
			{
				return WeaponGroup_Knife;
			}
			case 43, 44, 45, 46, 47, 48:
			{
				return WeaponGroup_Grenade;
			}
			case 17, 19, 24, 26, 33, 34:
			{
				return WeaponGroup_SMG;
			}
			case 31:
			{
				return WeaponGroup_Taser;
			}
			case 49:
			{
				return WeaponGroup_C4;
			}
			default:
			{
				// Healthshot and TA grenade don't have item definition indexes.
				char sClass[32];
				this.GetClassname(sClass, sizeof(sClass));
				if(StrEqual(sClass, "weapon_healthshot"))
				{
					return WeaponGroup_Healthshot;
				}
				else if(StrEqual(sClass, "weapon_tagrenade"))
				{
					return WeaponGroup_Grenade;
				}
				return WeaponGroup_Invalid;
			}
		}
	}
	public static WeaponDef DefFromName(const char[] sName)
	{
		if(g_hWeaponIndexes == null)
		{
			CWeaponCreateWeapons();
		}
		int iDef;
		if(g_hWeaponIndexes.GetValue(sName, iDef))
		{
			return view_as<WeaponDef>(iDef);
		}
		return Weapon_Invalid;
		
	}
	public static WeaponGroup GroupFromName(const char[] sName)
	{
		return GetWeaponTypeByName(sName);
	}
	property CEntity PrevOwner
	{
		public get()
		{
			return CEntity.FromIndex(this.GetProp(Prop_Send, "m_hPrevOwner"));
		}
	}
	property WeaponGroup Group
	{
		public get()
		{
			switch(this.GetProp(Prop_Send, "m_iItemDefinitionIndex"))
			{
				case 1, 2, 3, 4, 12, 36, 61, 63, 32, 30, 64:
				{
					return WeaponGroup_Pistol;
				}
				case 7, 8, 9, 10, 11, 13, 16, 38, 39, 40, 60:
				{
					return WeaponGroup_Rifle;
				}
				case 14, 25, 27, 28, 29, 35:
				{
					return WeaponGroup_Heavy;
				}
				case 500, 506, 507, 508, 509, 515, 42, 59, 512, 516, 514, 505, 75, 76, 78:
				{
					return WeaponGroup_Knife;
				}
				case 43, 44, 45, 46, 47, 48, 81, 82, 83:
				{
					return WeaponGroup_Grenade;
				}
				case 17, 19, 24, 26, 33, 34, 23:
				{
					return WeaponGroup_SMG;
				}
				case 49:
				{
					return WeaponGroup_C4;
				}
				case 31:
				{
					return WeaponGroup_Taser;
				}
				default:
				{
					// Healthshot and TA grenade don't have item definition indexes.
					char sClass[32];
					this.GetClassname(sClass, sizeof(sClass));
					if(StrEqual(sClass, "weapon_healthshot"))
					{
						return WeaponGroup_Healthshot;
					}
					else if(StrEqual(sClass, "weapon_tagrenade"))
					{
						return WeaponGroup_Grenade;
					}
					return WeaponGroup_Invalid;
				}
			}
			return WeaponGroup_Invalid; // Fixes warning 209: should return a value, even though it should never get here
		}
	}
	#pragma deprecated Use CWeapon.Group
	property WeaponGroup Type 
	{
		public get()
		{
			return this.Group;
		}
	}
	// Gets/Sets the primary ammo in a gun
	property int Ammo
	{
		public get()
		{
			return this.GetProp(Prop_Send, "m_iClip1");
		}
		public set(int iAmmo)
		{
			this.SetProp(Prop_Send, "m_iClip1", iAmmo);
		}
	}
	// Set reserve (aka secondary) ammo
	property int ReserveAmmo
	{
		public get()
		{
			return this.GetProp(Prop_Send, "m_iPrimaryReserveAmmoCount");
		}
		public set(int iAmmo)
		{
			this.SetProp(Prop_Send, "m_iPrimaryReserveAmmoCount", iAmmo);
		}
	}
	// Gets the state of the weapon. This is usually 0 = no owner, 1 = player currently carries (Might not be active), 2 = currently out.
	property int State
	{
		public get()
		{
			return this.GetProp(Prop_Send, "m_iState");
		}
		public set(int iState)
		{
			this.SetProp(Prop_Send, "m_iState", iState);
		}
	}
	property WeaponDef Def 
	{
		public get()
		{
			return view_as<WeaponDef>(this.GetProp(Prop_Send, "m_iItemDefinitionIndex"));
		}
	}
}

// Since CCSPlayer and CBasePlayer inherit from CEntity, you can pass those into player without explicitly casting via view_as<CEntity>
stock CWeapon GivePlayerWeapon(CEntity player, char[] sWep)
{
	return view_as<CWeapon>(GivePlayerItem(player.Index, sWep));
}

// Gets a weapon type from weapon name. Returns Weapon_Unknown if not found.
stock WeaponGroup GetWeaponTypeByName(const char[] sWeapon)
{
	if(g_hWeaponIndexes == null)
	{
		CWeaponCreateWeapons()
	}
	int iDef;
	if(g_hWeaponIndexes.GetValue(sWeapon, iDef))
	{
		switch(iDef)
		{
			case 1, 2, 3, 4, 12, 36, 61, 63, 32, 30, 64:
			{
				return WeaponGroup_Pistol;
			}
			case 7, 8, 9, 10, 11, 13, 16, 38, 39, 40, 60:
			{
				return WeaponGroup_Rifle;
			}
			case 14, 25, 27, 28, 29, 35:
			{
				return WeaponGroup_Heavy;
			}
			case 500, 506, 507, 508, 509, 515, 42, 59, 512, 516, 514, 505, 75, 76, 78:
			{
				return WeaponGroup_Knife;
			}
			case 43, 44, 45, 46, 47, 48, 81, 82, 83:
			{
				return WeaponGroup_Grenade;
			}
			case 17, 19, 24, 26, 33, 34, 23:
			{
				return WeaponGroup_SMG;
			}
			case 49:
			{
				return WeaponGroup_C4;
			}
			case 31:
			{
				return WeaponGroup_Taser;
			}
			default:
			{
				// Healthshot and TA grenade don't have item definition indexes.
				if(StrEqual(sWeapon, "weapon_healthshot"))
				{
					return WeaponGroup_Healthshot;
				}
				else if(StrEqual(sWeapon, "weapon_tagrenade"))
				{
					return WeaponGroup_Grenade;
				}
				return WeaponGroup_Invalid;
			}
		}
	}
	return WeaponGroup_Invalid;
}

// returns -1 if name not found
stock int GetMaxReserveAmmo(char[] sWeapon)
{
	int iReserve;
	if(g_hWeaponAmmo.GetValue(sWeapon, iReserve))
	{
		return iReserve;
	}
	return -1;
}

// converts weapon type to weapon slot
stock int WeaponTypeToSlot(WeaponGroup group)
{
	switch(group)
	{
		case WeaponGroup_C4, WeaponGroup_Healthshot:
		{
			return CS_SLOT_C4;
		}
		case WeaponGroup_Pistol:
		{
			return CS_SLOT_SECONDARY;
		}
		case WeaponGroup_Rifle, WeaponGroup_Heavy, WeaponGroup_SMG:
		{
			return CS_SLOT_PRIMARY;
		}
		case WeaponGroup_Knife, WeaponGroup_Taser:
		{
			return CS_SLOT_KNIFE;
		}
		case WeaponGroup_Grenade:
		{
			return CS_SLOT_GRENADE;
		}
		case WeaponGroup_Unknown:
		{
			return -1;
		}
	}
	return -1;
}

static stock void CWeapon_CheckGame()
{
	if(GetEngineVersion() == Engine_CSGO)
	{
		g_bNeedsItemDef = true;
	}
	else
	{
		g_bNeedsItemDef = false;
	}
	g_bNeedsGameCheck = false;
}

static stock bool CWeapon_IsCustomKnife(const char[] sWep)
{
	int iDefIndex = -1;
	g_hWeaponIndexes.GetValue(sWep, iDefIndex);
	switch(iDefIndex)
	{
		case 500, 505, 506, 507, 508, 509, 515, 512, 516, 514:
		{
			return true;
		}
	}
	return false;
}

static stock void CWeaponCreateWeapons()
{
	g_hWeaponIndexes = new StringMap();
	g_hWeaponIndexes.SetValue("weapon_deagle", 1, false);
	g_hWeaponIndexes.SetValue("weapon_elite", 2, false);
	g_hWeaponIndexes.SetValue("weapon_fiveseven", 3, false);
	g_hWeaponIndexes.SetValue("weapon_glock", 4, false);
	g_hWeaponIndexes.SetValue("weapon_ak47", 7, false);
	g_hWeaponIndexes.SetValue("weapon_aug", 8, false);
	g_hWeaponIndexes.SetValue("weapon_awp", 9, false);
	g_hWeaponIndexes.SetValue("weapon_famas", 10, false);
	g_hWeaponIndexes.SetValue("weapon_g3sg1", 11, false);
	g_hWeaponIndexes.SetValue("weapon_galilar", 13, false);
	g_hWeaponIndexes.SetValue("weapon_m249", 14, false);
	g_hWeaponIndexes.SetValue("weapon_m4a1", 16, false);
	g_hWeaponIndexes.SetValue("weapon_mac10", 17, false);
	g_hWeaponIndexes.SetValue("weapon_p90", 19, false);
	g_hWeaponIndexes.SetValue("weapon_mp5sd", 23, false);
	g_hWeaponIndexes.SetValue("weapon_ump45", 24, false);
	g_hWeaponIndexes.SetValue("weapon_xm1014", 25, false);
	g_hWeaponIndexes.SetValue("weapon_bizon", 26, false);
	g_hWeaponIndexes.SetValue("weapon_mag7", 27, false);
	g_hWeaponIndexes.SetValue("weapon_negev", 28, false);
	g_hWeaponIndexes.SetValue("weapon_sawedoff", 29, false);
	g_hWeaponIndexes.SetValue("weapon_tec9", 30, false);
	g_hWeaponIndexes.SetValue("weapon_taser", 31, false);
	g_hWeaponIndexes.SetValue("weapon_hkp2000", 32, false);
	g_hWeaponIndexes.SetValue("weapon_mp7", 33, false);
	g_hWeaponIndexes.SetValue("weapon_mp9", 34, false);
	g_hWeaponIndexes.SetValue("weapon_nova", 35, false);
	g_hWeaponIndexes.SetValue("weapon_p250", 36, false);
	g_hWeaponIndexes.SetValue("weapon_scar20", 38, false);
	g_hWeaponIndexes.SetValue("weapon_sg556", 39, false);
	g_hWeaponIndexes.SetValue("weapon_ssg08", 40, false);
	g_hWeaponIndexes.SetValue("weapon_knife", 59, false);
	g_hWeaponIndexes.SetValue("weapon_flashbang", 43, false);
	g_hWeaponIndexes.SetValue("weapon_hegrenade", 44, false);
	g_hWeaponIndexes.SetValue("weapon_smokegrenade", 45, false);
	g_hWeaponIndexes.SetValue("weapon_molotov", 46, false);
	g_hWeaponIndexes.SetValue("weapon_decoy", 47, false);
	g_hWeaponIndexes.SetValue("weapon_incgrenade", 48, false);
	g_hWeaponIndexes.SetValue("weapon_c4", 49, false);
	g_hWeaponIndexes.SetValue("weapon_m4a1_silencer", 60, false);
	g_hWeaponIndexes.SetValue("weapon_usp_silencer", 61, false);
	g_hWeaponIndexes.SetValue("weapon_cz75a", 63, false);
	g_hWeaponIndexes.SetValue("weapon_knife_ct", 42, false);
	g_hWeaponIndexes.SetValue("weapon_bayonet", 500, false);
	g_hWeaponIndexes.SetValue("weapon_knife_flip", 505, false);
	g_hWeaponIndexes.SetValue("weapon_knife_gut", 506, false);
	g_hWeaponIndexes.SetValue("weapon_knife_karambit", 507, false);
	g_hWeaponIndexes.SetValue("weapon_knife_m9_bayonet", 508, false);
	g_hWeaponIndexes.SetValue("weapon_knife_tactical", 509, false);
	g_hWeaponIndexes.SetValue("weapon_knife_butterfly", 515, false);
	g_hWeaponIndexes.SetValue("weapon_knife_falchion", 512, false);
	g_hWeaponIndexes.SetValue("weapon_knife_push", 516, false);
	g_hWeaponIndexes.SetValue("weapon_knife_survival_bowie", 514, false);
	g_hWeaponIndexes.SetValue("weapon_revolver", 64, false);
	g_hWeaponIndexes.SetValue("weapon_healthshot", 0, false);
	g_hWeaponIndexes.SetValue("weapon_tagrenade", 0, false);
	g_hWeaponIndexes.SetValue("weapon_diversion", 82, false);
	g_hWeaponIndexes.SetValue("weapon_frag_grenade", 83, false);
	g_hWeaponIndexes.SetValue("weapon_firebomb", 81, false);
	g_hWeaponIndexes.SetValue("weapon_breachcharge", 70, false);
	g_hWeaponIndexes.SetValue("weapon_spanner", 78, false);
	g_hWeaponIndexes.SetValue("weapon_hammer", 76, false);
	g_hWeaponIndexes.SetValue("weapon_axe", 75, false);
	g_hWeaponIndexes.SetValue("weapon_tablet", 72, false);
	
	g_hWeaponAmmo = new StringMap();
	g_hWeaponAmmo.SetValue("weapon_deagle", 35, false);
	g_hWeaponAmmo.SetValue("weapon_elite", 120, false);
	g_hWeaponAmmo.SetValue("weapon_fiveseven", 100, false);
	g_hWeaponAmmo.SetValue("weapon_glock", 120, false);
	g_hWeaponAmmo.SetValue("weapon_ak47", 90, false);
	g_hWeaponAmmo.SetValue("weapon_aug", 90, false);
	g_hWeaponAmmo.SetValue("weapon_awp", 30, false);
	g_hWeaponAmmo.SetValue("weapon_famas", 90, false);
	g_hWeaponAmmo.SetValue("weapon_g3sg1", 90, false);
	g_hWeaponAmmo.SetValue("weapon_galilar", 90, false);
	g_hWeaponAmmo.SetValue("weapon_m249", 200, false);
	g_hWeaponAmmo.SetValue("weapon_m4a1", 90, false);
	g_hWeaponAmmo.SetValue("weapon_mac10", 100, false);
	g_hWeaponAmmo.SetValue("weapon_p90", 100, false);
	g_hWeaponAmmo.SetValue("weapon_mp5sd", 120, false);
	g_hWeaponAmmo.SetValue("weapon_ump45", 100, false);
	g_hWeaponAmmo.SetValue("weapon_xm1014", 32, false);
	g_hWeaponAmmo.SetValue("weapon_bizon", 120, false);
	g_hWeaponAmmo.SetValue("weapon_mag7", 32, false);
	g_hWeaponAmmo.SetValue("weapon_negev", 200, false);
	g_hWeaponAmmo.SetValue("weapon_sawedoff", 32, false);
	g_hWeaponAmmo.SetValue("weapon_tec9", 120, false);
	g_hWeaponAmmo.SetValue("weapon_taser", 0, false);
	g_hWeaponAmmo.SetValue("weapon_hkp2000", 52, false);
	g_hWeaponAmmo.SetValue("weapon_mp7", 120, false);
	g_hWeaponAmmo.SetValue("weapon_mp9", 120, false);
	g_hWeaponAmmo.SetValue("weapon_nova", 32, false);
	g_hWeaponAmmo.SetValue("weapon_p250", 26, false);
	g_hWeaponAmmo.SetValue("weapon_scar20", 90, false);
	g_hWeaponAmmo.SetValue("weapon_sg556", 90, false);
	g_hWeaponAmmo.SetValue("weapon_ssg08", 90, false);
	g_hWeaponAmmo.SetValue("weapon_knife", 0, false);
	g_hWeaponAmmo.SetValue("weapon_flashbang", 0, false);
	g_hWeaponAmmo.SetValue("weapon_hegrenade", 0, false);
	g_hWeaponAmmo.SetValue("weapon_smokegrenade", 0, false);
	g_hWeaponAmmo.SetValue("weapon_molotov", 0, false);
	g_hWeaponAmmo.SetValue("weapon_decoy", 0, false);
	g_hWeaponAmmo.SetValue("weapon_incgrenade", 0, false);
	g_hWeaponAmmo.SetValue("weapon_c4", 0, false);
	g_hWeaponAmmo.SetValue("weapon_m4a1_silencer", 40, false);
	g_hWeaponAmmo.SetValue("weapon_usp_silencer", 24, false);
	g_hWeaponAmmo.SetValue("weapon_cz75a", 12, false);
	g_hWeaponAmmo.SetValue("weapon_knife_ct", 0, false);
	g_hWeaponAmmo.SetValue("weapon_bayonet", 0, false);
	g_hWeaponAmmo.SetValue("weapon_knife_flip", 0, false);
	g_hWeaponAmmo.SetValue("weapon_knife_gut", 0, false);
	g_hWeaponAmmo.SetValue("weapon_knife_karambit", 0, false);
	g_hWeaponAmmo.SetValue("weapon_knife_m9_bayonet", 0, false);
	g_hWeaponAmmo.SetValue("weapon_knife_tactical", 0, false);
	g_hWeaponAmmo.SetValue("weapon_knife_butterfly", 0, false);
	g_hWeaponAmmo.SetValue("weapon_knife_falchion", 0, false);
	g_hWeaponAmmo.SetValue("weapon_knife_push", 0, false);
	g_hWeaponAmmo.SetValue("weapon_knife_survival_bowie", 0, false);
	g_hWeaponAmmo.SetValue("weapon_revolver", 8, false);
	g_hWeaponAmmo.SetValue("weapon_healthshot", 0, false);
	g_hWeaponAmmo.SetValue("weapon_tagrenade", 0, false);
	g_hWeaponAmmo.SetValue("weapon_diversion", 0, false);
	g_hWeaponAmmo.SetValue("weapon_frag_grenade", 0, false);
	g_hWeaponAmmo.SetValue("weapon_firebomb", 0, false);
	g_hWeaponAmmo.SetValue("weapon_breachcharge", 0, false);
	g_hWeaponAmmo.SetValue("weapon_spanner", 0, false);
	g_hWeaponAmmo.SetValue("weapon_hammer", 0, false);
	g_hWeaponAmmo.SetValue("weapon_axe", 0, false);
	g_hWeaponAmmo.SetValue("weapon_tablet", 0, false);
}