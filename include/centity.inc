#if defined _centity_included
	#endinput
#endif
#define _centity_included
#include <sdktools>

enum CEntity
{
	NULL_CENTITY = -1
}

methodmap CEntity
{
	// Creates a new entity by classname. Returns null if not created.
	public CEntity(char[] sClass)
	{
		int iEnt = CreateEntityByName(sClass);
		if (iEnt == -1)
		{
			return NULL_CENTITY;
		}
		else
		{
			return view_as<CEntity>(iEnt);
		}
	}
	// Returns null if index is <= 0.
	public static CEntity FromIndex(int iIndex)
	{
		return iIndex <= 0 ? NULL_CENTITY : view_as<CEntity>(iIndex);
	}
	public static CEntity FromRef(int iRef)
	{
		return CEntity.FromIndex(EntRefToEntIndex(iRef));
	}
	// Returns the entity index.
	property int Index
	{
		public get()
		{
			if(this == NULL_CENTITY)
			{
				return -1;
			}
			return view_as<int>(this);
		}
	}
	// returns if this entity is null (-1). Usually indicates if the entity is 
	property bool IsNull
	{
		public get()
		{
			return this.Index == -1;
		}
	}
	// returns the entity reference. returns INVALID_ENT_REFERENCE if null
	property int Ref
	{
		public get()
		{
			return EntIndexToEntRef(view_as<int>(this));
		}
 	}
 	// set/get entity render mode
	property RenderMode Render
	{
		public get()
		{
			return GetEntityRenderMode(this.Index);
		}
		public set(RenderMode rMode)
		{
			SetEntityRenderMode(this.Index, rMode);
		}
	}
	// returns the entity that "owns" this entity
	property CEntity Owner
	{
		public get()
		{
			return CEntity.FromIndex(GetEntPropEnt(this.Index, Prop_Send, "m_hOwnerEntity"));
		}
		public set(CEntity eOwner)
		{
			SetEntPropEnt(this.Index, Prop_Send, "m_hOwnerEntity", eOwner.Index);
		}
	}
	// returns if an entity index is valid & existing
	property bool IsValid
	{
		public get()
		{
			return IsValidEntity(this.Index);
		}
	}
	// returns a player's velocity
	#pragma deprecated Use CEntity.Speed
	property float Velocity
	{
		public get()
		{
			float fVel[3];
			GetEntPropVector(this.Index, Prop_Data, "m_vecVelocity", fVel);
			return SquareRoot(Pow(fVel[0], 2.0) + Pow(fVel[1], 2.0) + Pow(fVel[2], 2.0));
		}
	}
	property float Speed
	{
		public get()
		{
			float fVel[3];
			GetEntPropVector(this.Index, Prop_Data, "m_vecVelocity", fVel);
			return SquareRoot(Pow(fVel[0], 2.0) + Pow(fVel[1], 2.0) + Pow(fVel[2], 2.0));
		}
	}
	// Gets an entity's model scale
	property float Scale 
	{
		public get()
		{
			return GetEntPropFloat(this.Index, Prop_Send, "m_flModelScale");
		}
		public set(float fVal)
		{
			SetEntPropFloat(this.Index, Prop_Send, "m_flModelScale", fVal);
		}
	}
	// Should an entity glow. Set glow with CEntity.SetGlowColor().
	property bool Glow
	{
		public get()
		{
			return view_as<bool>(GetEntProp(this.Index, Prop_Send, "m_bShouldGlow"));
		}
		public set(bool bGlow)
		{
			SetEntProp(this.Index, Prop_Send, "m_bShouldGlow", bGlow);
			SetEntPropFloat(this.Index, Prop_Send, "m_flGlowMaxDist", 9999999.9);
		}
	}
	property float Gravity
	{
		public get()
		{
			return GetEntityGravity(this.Index);
		}
		public set(float fVal)
		{
			SetEntityGravity(this.Index, fVal);
		}
	}
	// set/get entity movetype
	property MoveType Move 
	{
		public get()
		{
			return GetEntityMoveType(this.Index);
		}
		public set(MoveType type)
		{
			SetEntityMoveType(this.Index, type);
		}
	}
	// Dispatches a key value
	public void KeyValue(char[] sKeyName, char[] sValue)
	{
		DispatchKeyValue(this.Index, sKeyName, sValue);
	}
	// Dispatches a key value float
	public void KeyValueFloat(char[] sKeyName, float fValue)
	{
		DispatchKeyValueFloat(this.Index, sKeyName, fValue);
	}
	// Dispatches a key value vector
	public void KeyValueVector(char[] sKeyName, float fVec[3])
	{
		DispatchKeyValueVector(this.Index, sKeyName, fVec);
	}
	// Dispatches spawn
	public void Spawn()
	{
		DispatchSpawn(this.Index);
	}
	// Teleports an entity
	public void Teleport(float fOrigin[3], float fAngles[3], float fVel[3])
	{
		TeleportEntity(this.Index, fOrigin, fAngles, fVel);
	}
	// Gets the location
	public void AbsOrigin(float fLoc[3])
	{
		GetEntPropVector(this.Index, Prop_Send, "m_vecOrigin", fLoc);
	}
	// gets an ent prop
	public int GetProp(PropType pType, char[] sProp, int iSize = 4, int iElement = 0)
	{
		return GetEntProp(this.Index, pType, sProp, iSize, iElement);
	}
	// Sets an ent prop
	public void SetProp(PropType pType, char[] sProp, any aValue, int iSize = 4, int iElement = 0)
	{
		SetEntProp(this.Index, pType, sProp, aValue, iSize, iElement);
	}
	// Gets an ent prop of type ent
	public CEntity GetPropEnt(PropType pType, char[] sProp, int iElement = 0)
	{
		return CEntity.FromIndex(GetEntPropEnt(this.Index, pType, sProp, iElement));
	}
	// Sets an ent prop of type ent
	public void SetPropEnt(PropType pType, char[] sProp, CEntity eEnt, int iElement = 0)
	{
		SetEntPropEnt(this.Index, pType, sProp, eEnt.Index, iElement);
	}
	// Gets an ent prop of type float
	public float GetPropFloat(PropType pType, char[] sProp, int iElement = 0)
	{
		return GetEntPropFloat(this.Index, pType, sProp, iElement);
	}
	// Sets an ent prop of type float
	public void SetPropFloat(PropType pType, char[] sProp, float fValue, int iElement = 0)
	{
		SetEntPropFloat(this.Index, pType, sProp, fValue);
	}
	// Gets an ent prop of type string
	public void GetPropString(PropType pType, char[] sProp, char[] sBuffer, int iMaxLen, int iElement = 0)
	{
		GetEntPropString(this.Index, pType, sProp, sBuffer, iMaxLen, iElement);
	}
	// Sets an ent prop of type string
	public void SetPropString(PropType pType, char[] sProp, char[] sBuffer)
	{
		SetEntPropString(this.Index, pType, sProp, sBuffer);
	}
	// Gets an ent prop of type vector
	public void GetPropVector(PropType pType, char[] sProp, float fVector[3], int iElement = 0)
	{
		GetEntPropVector(this.Index, pType, sProp, fVector, iElement);
	}
	// Gets offset for an entity prop
	public int GetSendPropOffs(char[] sProp, bool bActual = false)
	{
		return GetEntSendPropOffs(this.Index, sProp, bActual);
	}
	// Sets ent data
	public void SetData(int iOffset, any value, int size = 4, bool bChangeState=false)
	{
		SetEntData(this.Index, iOffset, value, size, bChangeState);
	}
	// Gets ent data
	public int GetData(int iOffset, int iSize = 4)
	{
		return GetEntData(this.Index, iOffset, iSize);
	}
	// Sets render color
	public void SetRenderColor(int iColor[4])
	{
		SetEntityRenderColor(this.Index, iColor[0], iColor[1], iColor[2], iColor[3]);
	}
	// Gets render color
	public void GetRenderColor(int iColor[4])
	{
		// TODO: Use sourcemod's GetEntityRenderColor when sourcemod 1.8 is released
		int iOffset = GetEntSendPropOffs(this.Index, "m_clrRender");
		for (int i = 0; i < 4; i++)
		{
			iColor[i] = GetEntData(this.Index, iOffset + i);
		}
	}
	// Sets a glow color.
	public void SetGlowColor(int iColor[4])
	{
		int iOffset = this.GetSendPropOffs("m_clrGlow");
		if(iOffset == -1)
		{
			ThrowError("Cannot find m_clrGlow offset for this entity; likely not supported");
		}
		for(int i = 0; i < 4;i++)
		{		
			this.SetData(iOffset + i, iColor[i], _, true);
		}
		this.Glow = true;
	}
	// Gets glow color. Returns if the entity is currently glowing.
	public bool GetGlowColor(int iColor[4])
	{
		int iOffset = this.GetSendPropOffs("m_clrGlow");
		if(iOffset == -1)
		{
			ThrowError("Cannot find m_clrGlow offset for this entity; likely not supported");
		}
		for(int i = 0; i < 4;i++)
		{		
			iColor[i] = this.GetData(iOffset + i);
		}
		return this.Glow;
	}
	// Gets entity classname
	public void GetClassname(char[] sBuffer, int iMaxLen)
	{
		GetEntityClassname(this.Index, sBuffer, iMaxLen);
	}
	// Give input to an entity
	public void AcceptInput(char[] sInput, int iActivator = -1, int iCaller = -1, int iOutputID = 0)
	{
		AcceptEntityInput(this.Index, sInput, iActivator, iCaller, iOutputID);
	}
	// Gets an entity's model
	public void GetModel(char[] sBuffer, int iMaxSize)
	{
		GetEntPropString(this.Index, Prop_Data, "m_ModelName", sBuffer, iMaxSize);
	}
	// Sets an entity's model
	public void SetModel(char[] sModel)
	{
		SetEntityModel(this.Index, sModel);
	}
	// Accepts Input "Kill"
	public void Kill()
	{
		if(this.Index == -1)
		{
			ThrowError("Attempted to kill a null CEntity");
		}
		else
		{
			this.AcceptInput("Kill");
		}
	}
	// Sets this on fire
	public void Ignite(float fTime, bool bNPC = false, float fSize = 0.0, bool bLevel = false)
	{
		IgniteEntity(this.Index, fTime, bNPC, fSize, bLevel);
	}
	// Hooks output
	public void HookOutput(const char[] sOutput, EntityOutput outputCallback, bool bOnce = false)
	{
		HookSingleEntityOutput(this.Index, sOutput, outputCallback, bOnce);
	}
}

// Finds an entity by name
stock CEntity FindEntityByName(CEntity cEnt, char[] sName)
{
	int iNext = cEnt.Index;
	if((iNext = FindEntityByClassname(iNext, sName)) != -1)
	{
		return CEntity.FromIndex(iNext);
	}
	return NULL_CENTITY;
}

// takes an entity index and turns it into a CEntity. Returns NULL_CENTITY if index is invalid
#pragma deprecated Use CEntity.FromIndex().
stock CEntity IndexToCEntity(int iIndex)
{
	if(iIndex < 0 || iIndex > 2048)
	{
		return NULL_CENTITY;
	}
	return view_as<CEntity>(iIndex);
}

// returns a CEntity from an entity reference.
#pragma deprecated Use CEntity.FromRef().
stock CEntity RefToCEntity(int iRef)
{
	return IndexToCEntity(EntRefToEntIndex(iRef));
}