static char g_sColorTags[][] = {"{DEFAULT}", "{WHITE}", "{RED}",  "{TEAM}", "{GREEN}", "{YELLOW}", "{LIGHTGREEN}", "{LIGHTRED}", "{GRAY}", "{ORANGE}", "{LIGHTBLUE}", "{DARKBLUE}", "{PURPLE}"};
static char g_sColorCodes[][] = {"\x01",     "\x01",    "\x02",   "\x03",    "\x04",   "\x05",      "\x06",        "\x07",       "\x08",     "\x09",   "\x0B",        "\x0C",       "\x0E"};

stock void JB_PrintToChat(int iClient, const char[] sMsg, any ...)
{
	char sBuffer[255];
	VFormat(sBuffer, sizeof(sBuffer), sMsg, 3);
	
	JB_ReplaceColorCodes(sBuffer, sizeof(sBuffer));
	
	JB_SayText2(iClient, sBuffer);
}

stock void JB_PrintToChatAll(const char[] sMsg, any ...)
{
	char sBuffer[255];
	VFormat(sBuffer, sizeof(sBuffer), sMsg, 2);
	
	JB_ReplaceColorCodes(sBuffer, sizeof(sBuffer));
	
	for(int i = 1; i <= MaxClients;i++)
	{
		if(IsClientInGame(i))
		{
			JB_SayText2(i, sBuffer);
		}
	}
}

stock void JB_PrintToAdmins(const char[] sMsg, any ...)
{
	char sBuffer[255];
	VFormat(sBuffer, sizeof(sBuffer), sMsg, 2);
	
	JB_ReplaceColorCodes(sBuffer, sizeof(sBuffer));
	
	for(int i = 1; i <= MaxClients;i++)
	{
		if(IsClientInGame(i) && CheckCommandAccess(i, "sm_chat", ADMFLAG_CHAT))
		{
			JB_SayText2(i, sBuffer);
		}
	}
}

stock void JB_ReplyToCommand(int iClient, char[] sMsg, any ...)
{
	char sBuffer[255];
	VFormat(sBuffer, sizeof(sBuffer), sMsg, 3);

	if(GetCmdReplySource() == SM_REPLY_TO_CHAT)
	{
		JB_ReplaceColorCodes(sBuffer, sizeof(sBuffer));
		JB_SayText2(iClient, sBuffer);
	}
	else
	{
		JB_RemoveColorCodes(sBuffer, sizeof(sBuffer));
		PrintToConsole(iClient, sBuffer);
	}
}

stock void JB_PrintToAdminsConsole(char[] sMsg, any ...)
{
	char sBuffer[255];
	VFormat(sBuffer, sizeof(sBuffer), sMsg, 2);
	Format(sBuffer, sizeof(sBuffer), "%s%s", JB_PREFIX, sBuffer);
	
	JB_RemoveColorCodes(sBuffer, sizeof(sBuffer));
	
	for(int i = 1; i <= MaxClients;i++)
	{
		if(IsClientInGame(i) && CheckCommandAccess(i, "sm_chat", ADMFLAG_CHAT))
		{
			PrintToConsole(i, sBuffer);
		}
	}
}

stock int JB_ReplaceString(char[] sBuffer, int iMax, const char[] sToReplace, const char[] sReplacement, bool bCase = true)
{
	JB_ReplaceColorCodes(sBuffer, iMax);
	return ReplaceString(sBuffer, iMax, sToReplace, sReplacement, bCase);
}

stock void JB_ShowActivity2(int iClient, char[] sMsg, any ...)
{
	static ConVar hCvar;
	if(hCvar == null)
	{
		hCvar = FindConVar("sm_show_activity");
	}
	char sBuffer[255];
	VFormat(sBuffer, sizeof(sBuffer), sMsg, 3);
	JB_ReplaceColorCodes(sBuffer, sizeof(sBuffer), false);
	
	char sName[MAX_NAME_LENGTH];
	if(iClient != 0)
	{
		GetClientName(iClient, sName, sizeof(sName));
	}
	else
	{
		strcopy(sName, sizeof(sName), "CONSOLE");
	}
	
	char sHide[255], sShow[255];
	Format(sHide, sizeof(sHide), " " ... JB_PREFIX ... "ADMIN: %s", sBuffer);
	Format(sShow, sizeof(sShow), " " ... JB_PREFIX ... "%s: %s", sName, sBuffer);
	
	int iFlags = hCvar.IntValue;
	
	for(int i = 1; i <= MaxClients;i++)
	{
		if(IsClientInGame(i) && i != iClient)
		{
			AdminId admin = GetUserAdmin(i);
			if(admin == INVALID_ADMIN_ID || !GetAdminFlag(admin, Admin_Generic, Access_Effective))
			{
				if(iFlags & 2)
				{
					JB_SayText2(i, sShow);
				}
				else
				{
					JB_SayText2(i, sHide);
				}
			}
			else if(iFlags & 8 || (iFlags & 16 && GetAdminFlag(admin, Admin_Root, Access_Effective)))
			{
				JB_SayText2(i, sShow);
			}
			else
			{
				JB_SayText2(i, sHide);
			}
		}
	}
	
	if(GetCmdReplySource() == SM_REPLY_TO_CHAT)
	{
		char sTemp[255];
		Format(sTemp, sizeof(sTemp), " %s%s", JB_PREFIX, sBuffer);
		JB_SayText2(iClient, sTemp);
	}
	else
	{
		char sTemp[255];
		Format(sTemp, sizeof(sTemp), "%s%s", JB_PREFIX, sBuffer);
		JB_RemoveColorCodes(sTemp, sizeof(sTemp));
		PrintToConsole(iClient, sTemp);
	}
}

static stock void JB_ReplaceColorCodes(char[] sBuffer, int iMax, bool bAddSpace=true)
{
	for(int i = 0; i < sizeof(g_sColorTags);i++)
	{
		if(bAddSpace && (StrContains(sBuffer, g_sColorTags[i]) == 0 || StrContains(sBuffer, g_sColorCodes[i]) == 0))
		{
			Format(sBuffer, iMax, " %s", sBuffer);
		}
		ReplaceString(sBuffer, iMax, g_sColorTags[i], g_sColorCodes[i]);
	}
}

static stock void JB_RemoveColorCodes(char[] sBuffer, int iMax)
{
	for(int i = 0; i < sizeof(g_sColorTags);i++)
	{
		ReplaceString(sBuffer, iMax, g_sColorTags[i], "");
	}
}

static stock void JB_SayText2(int iClient, const char[] sMsg)
{
	Handle hMsg = StartMessageOne("SayText2", iClient, USERMSG_RELIABLE | USERMSG_BLOCKHOOKS);
	PbSetInt(hMsg, "ent_idx", iClient);
	PbSetBool(hMsg, "chat", true);
	PbSetString(hMsg, "msg_name", sMsg);
	PbAddString(hMsg, "params", "");
	PbAddString(hMsg, "params", "");
	PbAddString(hMsg, "params", "");
	PbAddString(hMsg, "params", "");
	EndMessage();
}