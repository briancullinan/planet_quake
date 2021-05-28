/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "client.h"

#include "../botlib/botlib.h"

#ifdef USE_RMLUI
#include <RmlUi/Wrapper.h>
#endif

#ifdef USE_RMLUI_DLOPEN
static void	*rmlLib;
static char dllName[ MAX_OSPATH ];
Rml_ContextRender_t Rml_ContextRender;
Rml_ContextUpdate_t Rml_ContextUpdate;
#endif

extern	botlib_export_t	*botlib_export;

int  uivm = 0;
vm_t *uivms[MAX_NUM_VMS];

/*
====================
GetClientState
====================
*/
static void GetClientState( uiClientState_t *state ) {
	state->connectPacketCount = clc.connectPacketCount;
	state->connState = cls.state;
	Q_strncpyz( state->servername, cls.servername, sizeof( state->servername ) );
	Q_strncpyz( state->updateInfoString, cls.updateInfoString, sizeof( state->updateInfoString ) );
	Q_strncpyz( state->messageString, clc.serverMessage, sizeof( state->messageString ) );
#ifdef USE_MULTIVM_CLIENT
	int igs = clientGames[clc.currentView];
#else
	int igs = clientGames[0];
#endif
	state->clientNum = cl.snap[igs].ps.clientNum;
}


/*
====================
LAN_LoadCachedServers
====================
*/
void LAN_LoadCachedServers( void ) {
	fileHandle_t fileIn;
	int size, file_size;

	cls.numglobalservers = cls.numfavoriteservers = 0;
	cls.numGlobalServerAddresses = 0;

	file_size = FS_Home_FOpenFileRead( "servercache.dat", &fileIn );
	if ( file_size < (3*sizeof(int)) ) {
		if ( fileIn != FS_INVALID_HANDLE ) {
			FS_FCloseFile( fileIn );
		}
		return;
	} 

	FS_Read( &cls.numglobalservers, sizeof(int), fileIn );
	FS_Read( &cls.numfavoriteservers, sizeof(int), fileIn );
	FS_Read( &size, sizeof(int), fileIn );

	if ( size == sizeof(cls.globalServers) + sizeof(cls.favoriteServers) ) {
		FS_Read( &cls.globalServers, sizeof(cls.globalServers), fileIn );
		FS_Read( &cls.favoriteServers, sizeof(cls.favoriteServers), fileIn );
	} else {
		cls.numglobalservers = cls.numfavoriteservers = 0;
		cls.numGlobalServerAddresses = 0;
	}

	FS_FCloseFile( fileIn );
}


/*
====================
LAN_SaveServersToCache
====================
*/
void LAN_SaveServersToCache( void ) {
	fileHandle_t fileOut;
	int size;

	fileOut = FS_FOpenFileWrite( "servercache.dat" );
	if ( fileOut == FS_INVALID_HANDLE )
		return;

	FS_Write(&cls.numglobalservers, sizeof(int), fileOut);
	FS_Write(&cls.numfavoriteservers, sizeof(int), fileOut);
	size = sizeof(cls.globalServers) + sizeof(cls.favoriteServers);
	FS_Write(&size, sizeof(int), fileOut);
	FS_Write(&cls.globalServers, sizeof(cls.globalServers), fileOut);
	FS_Write(&cls.favoriteServers, sizeof(cls.favoriteServers), fileOut);

	FS_FCloseFile(fileOut);
}


/*
====================
LAN_ResetPings
====================
*/
static void LAN_ResetPings(int source) {
	int count,i;
	serverInfo_t *servers = NULL;
	count = 0;

	switch (source) {
		case AS_LOCAL :
			servers = &cls.localServers[0];
			count = MAX_OTHER_SERVERS;
			break;
		case AS_MPLAYER:
		case AS_GLOBAL :
			servers = &cls.globalServers[0];
			count = MAX_GLOBAL_SERVERS;
			break;
		case AS_FAVORITES :
			servers = &cls.favoriteServers[0];
			count = MAX_OTHER_SERVERS;
			break;
	}
	if (servers) {
		for (i = 0; i < count; i++) {
			servers[i].ping = -1;
		}
	}
}


/*
====================
LAN_AddServer
====================
*/
static int LAN_AddServer(int source, const char *name, const char *address) {
	int max, *count, i;
	netadr_t adr;
	serverInfo_t *servers = NULL;
	max = MAX_OTHER_SERVERS;
	count = NULL;

	switch (source) {
		case AS_LOCAL :
			count = &cls.numlocalservers;
			servers = &cls.localServers[0];
			break;
		case AS_MPLAYER:
		case AS_GLOBAL :
			max = MAX_GLOBAL_SERVERS;
			count = &cls.numglobalservers;
			servers = &cls.globalServers[0];
			break;
		case AS_FAVORITES :
			count = &cls.numfavoriteservers;
			servers = &cls.favoriteServers[0];
			break;
	}
	if (servers && *count < max) {
		NET_StringToAdr( address, &adr, NA_UNSPEC );
		for ( i = 0; i < *count; i++ ) {
			if (NET_CompareAdr(&servers[i].adr, &adr)) {
				break;
			}
		}
		if (i >= *count) {
			servers[*count].adr = adr;
			Q_strncpyz(servers[*count].hostName, name, sizeof(servers[*count].hostName));
			servers[*count].visible = qtrue;
			(*count)++;
			return 1;
		}
		return 0;
	}
	return -1;
}


/*
====================
LAN_RemoveServer
====================
*/
static void LAN_RemoveServer(int source, const char *addr) {
	int *count, i;
	serverInfo_t *servers = NULL;
	count = NULL;
	switch (source) {
		case AS_LOCAL :
			count = &cls.numlocalservers;
			servers = &cls.localServers[0];
			break;
		case AS_MPLAYER:
		case AS_GLOBAL :
			count = &cls.numglobalservers;
			servers = &cls.globalServers[0];
			break;
		case AS_FAVORITES :
			count = &cls.numfavoriteservers;
			servers = &cls.favoriteServers[0];
			break;
	}
	if (servers) {
		netadr_t comp;
		NET_StringToAdr( addr, &comp, NA_UNSPEC );
		for (i = 0; i < *count; i++) {
			if (NET_CompareAdr( &comp, &servers[i].adr)) {
				int j = i;
				while (j < *count - 1) {
					Com_Memcpy(&servers[j], &servers[j+1], sizeof(servers[j]));
					j++;
				}
				(*count)--;
				break;
			}
		}
	}
}


/*
====================
LAN_GetServerCount
====================
*/
static int LAN_GetServerCount( int source ) {
	switch (source) {
		case AS_LOCAL :
			return cls.numlocalservers;
			break;
		case AS_MPLAYER:
		case AS_GLOBAL :
			return cls.numglobalservers;
			break;
		case AS_FAVORITES :
			return cls.numfavoriteservers;
			break;
	}
	return 0;
}


/*
====================
LAN_GetLocalServerAddressString
====================
*/
static void LAN_GetServerAddressString( int source, int n, char *buf, int buflen ) {
	switch (source) {
		case AS_LOCAL :
			if (n >= 0 && n < MAX_OTHER_SERVERS) {
				Q_strncpyz(buf, NET_AdrToStringwPortandProtocol( &cls.localServers[n].adr) , buflen );
				return;
			}
			break;
		case AS_MPLAYER:
		case AS_GLOBAL :
			if (n >= 0 && n < MAX_GLOBAL_SERVERS) {
				Q_strncpyz(buf, NET_AdrToStringwPortandProtocol( &cls.globalServers[n].adr) , buflen );
				return;
			}
			break;
		case AS_FAVORITES :
			if (n >= 0 && n < MAX_OTHER_SERVERS) {
				Q_strncpyz(buf, NET_AdrToStringwPortandProtocol( &cls.favoriteServers[n].adr) , buflen );
				return;
			}
			break;
	}
	buf[0] = '\0';
}


/*
====================
LAN_GetServerInfo
====================
*/
static void LAN_GetServerInfo( int source, int n, char *buf, int buflen ) {
	char info[MAX_STRING_CHARS];
	serverInfo_t *server = NULL;
	info[0] = '\0';
	switch (source) {
		case AS_LOCAL :
			if (n >= 0 && n < MAX_OTHER_SERVERS) {
				server = &cls.localServers[n];
			}
			break;
		case AS_MPLAYER:
		case AS_GLOBAL :
			if (n >= 0 && n < MAX_GLOBAL_SERVERS) {
				server = &cls.globalServers[n];
			}
			break;
		case AS_FAVORITES :
			if (n >= 0 && n < MAX_OTHER_SERVERS) {
				server = &cls.favoriteServers[n];
			}
			break;
	}
	if (server && buf) {
		buf[0] = '\0';
		Info_SetValueForKey( info, "hostname", server->hostName);
		Info_SetValueForKey( info, "mapname", server->mapName);
		Info_SetValueForKey( info, "clients", va("%i",server->clients));
		Info_SetValueForKey( info, "sv_maxclients", va("%i",server->maxClients));
		Info_SetValueForKey( info, "ping", va("%i",server->ping));
		Info_SetValueForKey( info, "minping", va("%i",server->minPing));
		Info_SetValueForKey( info, "maxping", va("%i",server->maxPing));
		Info_SetValueForKey( info, "game", server->game);
		Info_SetValueForKey( info, "gametype", va("%i",server->gameType));
		Info_SetValueForKey( info, "nettype", va("%i",server->netType));
		Info_SetValueForKey( info, "addr", NET_AdrToStringwPort(&server->adr));
		Info_SetValueForKey( info, "punkbuster", va("%i", server->punkbuster));
		Info_SetValueForKey( info, "g_needpass", va("%i", server->g_needpass));
		Info_SetValueForKey( info, "g_humanplayers", va("%i", server->g_humanplayers));
		Q_strncpyz(buf, info, buflen);
	} else {
		if (buf) {
			buf[0] = '\0';
		}
	}
}


/*
====================
LAN_GetServerPing
====================
*/
static int LAN_GetServerPing( int source, int n ) {
	serverInfo_t *server = NULL;
	switch (source) {
		case AS_LOCAL :
			if (n >= 0 && n < MAX_OTHER_SERVERS) {
				server = &cls.localServers[n];
			}
			break;
		case AS_MPLAYER:
		case AS_GLOBAL :
			if (n >= 0 && n < MAX_GLOBAL_SERVERS) {
				server = &cls.globalServers[n];
			}
			break;
		case AS_FAVORITES :
			if (n >= 0 && n < MAX_OTHER_SERVERS) {
				server = &cls.favoriteServers[n];
			}
			break;
	}
	if (server) {
		return server->ping;
	}
	return -1;
}

/*
====================
LAN_GetServerPtr
====================
*/
static serverInfo_t *LAN_GetServerPtr( int source, int n ) {
	switch (source) {
		case AS_LOCAL :
			if (n >= 0 && n < MAX_OTHER_SERVERS) {
				return &cls.localServers[n];
			}
			break;
		case AS_MPLAYER:
		case AS_GLOBAL :
			if (n >= 0 && n < MAX_GLOBAL_SERVERS) {
				return &cls.globalServers[n];
			}
			break;
		case AS_FAVORITES :
			if (n >= 0 && n < MAX_OTHER_SERVERS) {
				return &cls.favoriteServers[n];
			}
			break;
	}
	return NULL;
}


/*
====================
LAN_CompareServers
====================
*/
static int LAN_CompareServers( int source, int sortKey, int sortDir, int s1, int s2 ) {
	int res;
	serverInfo_t *server1, *server2;

	server1 = LAN_GetServerPtr(source, s1);
	server2 = LAN_GetServerPtr(source, s2);
	if (!server1 || !server2) {
		return 0;
	}

	res = 0;
	switch( sortKey ) {
		case SORT_HOST:
			res = Q_stricmp( server1->hostName, server2->hostName );
			break;

		case SORT_MAP:
			res = Q_stricmp( server1->mapName, server2->mapName );
			break;
		case SORT_CLIENTS:
			if (server1->clients < server2->clients) {
				res = -1;
			}
			else if (server1->clients > server2->clients) {
				res = 1;
			}
			else {
				res = 0;
			}
			break;
		case SORT_GAME:
			if (server1->gameType < server2->gameType) {
				res = -1;
			}
			else if (server1->gameType > server2->gameType) {
				res = 1;
			}
			else {
				res = 0;
			}
			break;
		case SORT_PING:
			if (server1->ping < server2->ping) {
				res = -1;
			}
			else if (server1->ping > server2->ping) {
				res = 1;
			}
			else {
				res = 0;
			}
			break;
	}

	if (sortDir) {
		if (res < 0)
			return 1;
		if (res > 0)
			return -1;
		return 0;
	}
	return res;
}


/*
====================
LAN_GetPingQueueCount
====================
*/
static int LAN_GetPingQueueCount( void ) {
	return (CL_GetPingQueueCount());
}


/*
====================
LAN_ClearPing
====================
*/
static void LAN_ClearPing( int n ) {
	CL_ClearPing( n );
}


/*
====================
LAN_GetPing
====================
*/
static void LAN_GetPing( int n, char *buf, int buflen, int *pingtime ) {
	CL_GetPing( n, buf, buflen, pingtime );
}


/*
====================
LAN_GetPingInfo
====================
*/
static void LAN_GetPingInfo( int n, char *buf, int buflen ) {
	CL_GetPingInfo( n, buf, buflen );
}


/*
====================
LAN_MarkServerVisible
====================
*/
static void LAN_MarkServerVisible(int source, int n, qboolean visible ) {
	if (n == -1) {
		int count = MAX_OTHER_SERVERS;
		serverInfo_t *server = NULL;
		switch (source) {
			case AS_LOCAL :
				server = &cls.localServers[0];
				break;
			case AS_MPLAYER:
			case AS_GLOBAL :
				server = &cls.globalServers[0];
				count = MAX_GLOBAL_SERVERS;
				break;
			case AS_FAVORITES :
				server = &cls.favoriteServers[0];
				break;
		}
		if (server) {
			for (n = 0; n < count; n++) {
				server[n].visible = visible;
			}
		}

	} else {
		switch (source) {
			case AS_LOCAL :
				if (n >= 0 && n < MAX_OTHER_SERVERS) {
					cls.localServers[n].visible = visible;
				}
				break;
			case AS_MPLAYER:
			case AS_GLOBAL :
				if (n >= 0 && n < MAX_GLOBAL_SERVERS) {
					cls.globalServers[n].visible = visible;
				}
				break;
			case AS_FAVORITES :
				if (n >= 0 && n < MAX_OTHER_SERVERS) {
					cls.favoriteServers[n].visible = visible;
				}
				break;
		}
	}
}


/*
=======================
LAN_ServerIsVisible
=======================
*/
static int LAN_ServerIsVisible(int source, int n ) {
	switch (source) {
		case AS_LOCAL :
			if (n >= 0 && n < MAX_OTHER_SERVERS) {
				return cls.localServers[n].visible;
			}
			break;
		case AS_MPLAYER:
		case AS_GLOBAL :
			if (n >= 0 && n < MAX_GLOBAL_SERVERS) {
				return cls.globalServers[n].visible;
			}
			break;
		case AS_FAVORITES :
			if (n >= 0 && n < MAX_OTHER_SERVERS) {
				return cls.favoriteServers[n].visible;
			}
			break;
	}
	return qfalse;
}


/*
=======================
LAN_UpdateVisiblePings
=======================
*/
qboolean LAN_UpdateVisiblePings(int source ) {
	return CL_UpdateVisiblePings_f(source);
}


/*
====================
LAN_GetServerStatus
====================
*/
static int LAN_GetServerStatus( const char *serverAddress, char *serverStatus, int maxLen ) {
	return CL_ServerStatus( serverAddress, serverStatus, maxLen );
}


/*
====================
CL_GetGlConfig
====================
*/
static void CL_GetGlconfig( glconfig_t *config ) {
	*config = *re.GetConfig();
}


/*
====================
CL_GetClipboardData
====================
*/
static void CL_GetClipboardData( char *buf, int buflen ) {
	char	*cbd;

	cbd = Sys_GetClipboardData();

	if ( !cbd ) {
		*buf = '\0';
		return;
	}

	Q_strncpyz( buf, cbd, buflen );

	Z_Free( cbd );
}


/*
====================
Key_KeynumToStringBuf
====================
*/
static void Key_KeynumToStringBuf( int keynum, char *buf, int buflen ) {
	Q_strncpyz( buf, Key_KeynumToString( keynum ), buflen );
}


/*
====================
Key_GetBindingBuf
====================
*/
static void Key_GetBindingBuf( int keynum, char *buf, int buflen ) {
	const char *value;

	value = Key_GetBinding( keynum );
	if ( value ) {
		Q_strncpyz( buf, value, buflen );
	}
	else {
		*buf = '\0';
	}
}


/*
====================
CLUI_GetCDKey
====================
*/
static void CLUI_GetCDKey( char *buf, int buflen ) {
#ifndef STANDALONE
	const char *gamedir;
	gamedir = Cvar_VariableString( "fs_game" );
	if ( UI_usesUniqueCDKey() && gamedir[0] != '\0' ) {
		Com_Memcpy( buf, &cl_cdkey[16], 16 );
		buf[16] = '\0';
	} else {
		Com_Memcpy( buf, cl_cdkey, 16 );
		buf[16] = '\0';
	}
#else
	*buf = '\0';
#endif
}


/*
====================
CLUI_SetCDKey
====================
*/
#ifndef STANDALONE
static void CLUI_SetCDKey( char *buf ) {
	const char *gamedir;
	gamedir = Cvar_VariableString( "fs_game" );
	if ( UI_usesUniqueCDKey() && gamedir[0] != '\0' ) {
		Com_Memcpy( &cl_cdkey[16], buf, 16 );
		cl_cdkey[32] = '\0';
		// set the flag so the fle will be written at the next opportunity
		cvar_modifiedFlags |= CVAR_ARCHIVE;
	} else {
		Com_Memcpy( cl_cdkey, buf, 16 );
		// set the flag so the fle will be written at the next opportunity
		cvar_modifiedFlags |= CVAR_ARCHIVE;
	}
}
#endif


/*
====================
GetConfigString
====================
*/
static int GetConfigString(int index, char *buf, int size)
{
	int		offset;

	if (index < 0 || index >= MAX_CONFIGSTRINGS)
		return qfalse;

	offset = cl.gameState[cgvm].stringOffsets[index];
	if (!offset) {
		if( size ) {
			buf[0] = 0;
		}
		return qfalse;
	}

	Q_strncpyz( buf, cl.gameState[cgvm].stringData+offset, size);
 
	return qtrue;
}


/*
====================
FloatAsInt
====================
*/
static int FloatAsInt( float f ) {
	floatint_t fi;
	fi.f = f;
	return fi.i;
}


/*
====================
VM_ArgPtr
====================
*/
static void *VM_ArgPtr( intptr_t intValue ) {

	if ( !intValue || uivms[uivm] == NULL )
	  return NULL;

	if ( uivms[uivm]->entryPoint )
		return (void *)(intValue);
	else
		return (void *)(uivms[uivm]->dataBase + (intValue & uivms[uivm]->dataMask));
}


static qboolean UI_GetValue( char* value, int valueSize, const char* key ) {

	if ( !Q_stricmp( key, "trap_R_AddRefEntityToScene2" ) ) {
		Com_sprintf( value, valueSize, "%i", UI_R_ADDREFENTITYTOSCENE2 );
		return qtrue;
	}

	if ( !Q_stricmp( key, "trap_R_AddLinearLightToScene_Q3E" ) && re.AddLinearLightToScene ) {
		Com_sprintf( value, valueSize, "%i", UI_R_ADDLINEARLIGHTTOSCENE );
		return qtrue;
	}

	return qfalse;
}


/*
====================
CL_UISystemCalls

The ui module is making a system call
====================
*/
static intptr_t CL_UISystemCalls( intptr_t *args ) {
	switch( args[0] ) {
	case UI_ERROR:
		Com_Error( ERR_DROP, "%s", (const char*)VMA(1) );
		return 0;

	case UI_PRINT:
		Com_Printf( "%s", (const char*)VMA(1) );
		return 0;

	case UI_MILLISECONDS:
		return Sys_Milliseconds();

	case UI_CVAR_REGISTER:
		Cvar_Register( VMA(1), VMA(2), VMA(3), args[4], uivms[uivm]->privateFlag ); 
		return 0;

	case UI_CVAR_UPDATE:
		Cvar_Update( VMA(1), uivms[uivm]->privateFlag );
		return 0;

	case UI_CVAR_SET:
		Cvar_SetSafe( VMA(1), VMA(2) );
		return 0;

	case UI_CVAR_VARIABLEVALUE:
		return FloatAsInt( Cvar_VariableValue( VMA(1) ) );

	case UI_CVAR_VARIABLESTRINGBUFFER:
		VM_CHECKBOUNDS( uivms[uivm], args[2], args[3] );
		Cvar_VariableStringBufferSafe( VMA(1), VMA(2), args[3], CVAR_PRIVATE );
		return 0;

	case UI_CVAR_SETVALUE:
		Cvar_SetValueSafe( VMA(1), VMF(2) );
		return 0;

	case UI_CVAR_RESET:
		Cvar_Reset( VMA(1) );
		return 0;

	case UI_CVAR_CREATE:
		Cvar_Register( NULL, VMA(1), VMA(2), args[3], uivms[uivm]->privateFlag );
		return 0;

	case UI_CVAR_INFOSTRINGBUFFER:
		VM_CHECKBOUNDS( uivms[uivm], args[2], args[3] );
		Cvar_InfoStringBuffer( args[1], VMA(2), args[3] );
		return 0;

	case UI_ARGC:
		return Cmd_Argc();

	case UI_ARGV:
		VM_CHECKBOUNDS( uivms[uivm], args[2], args[3] );
		Cmd_ArgvBuffer( args[1], VMA(2), args[3] );
		return 0;

	case UI_CMD_EXECUTETEXT:
		if(args[1] == EXEC_NOW
		&& (!strncmp(VMA(2), "snd_restart", 11)
		|| !strncmp(VMA(2), "vid_restart", 11)
		|| !strncmp(VMA(2), "disconnect", 10)
		|| !strncmp(VMA(2), "quit", 5)))
		{
			Com_Printf (S_COLOR_YELLOW "turning EXEC_NOW '%.11s' into EXEC_INSERT\n", (const char*)VMA(2));
			args[1] = EXEC_INSERT;
		}
		Cbuf_ExecuteTagged( args[1], VMA(2), uivm );
		return 0;

	case UI_FS_FOPENFILE:
		return FS_VM_OpenFile( VMA(1), VMA(2), args[3], H_Q3UI );

	case UI_FS_READ:
		VM_CHECKBOUNDS( uivms[uivm], args[1], args[2] );
		FS_VM_ReadFile( VMA(1), args[2], args[3], H_Q3UI );
		return 0;

	case UI_FS_WRITE:
		VM_CHECKBOUNDS( uivms[uivm], args[1], args[2] );
		FS_VM_WriteFile( VMA(1), args[2], args[3], H_Q3UI );
		return 0;

	case UI_FS_FCLOSEFILE:
		FS_VM_CloseFile( args[1], H_Q3UI );
		return 0;

	case UI_FS_SEEK:
		return FS_VM_SeekFile( args[1], args[2], args[3], H_Q3UI );

	case UI_FS_GETFILELIST:
		VM_CHECKBOUNDS( uivms[uivm], args[3], args[4] );
		return FS_GetFileList( VMA(1), VMA(2), VMA(3), args[4] );

	case UI_R_REGISTERMODEL:
		return re.RegisterModel( VMA(1) );

	case UI_R_REGISTERSKIN:
		return re.RegisterSkin( VMA(1) );

	case UI_R_REGISTERSHADERNOMIP:
		return re.RegisterShaderNoMip( VMA(1) );

	case UI_R_CLEARSCENE:
		re.ClearScene();
		return 0;

	case UI_R_ADDREFENTITYTOSCENE:
		re.AddRefEntityToScene( VMA(1), qfalse );
		return 0;

	case UI_R_ADDPOLYTOSCENE:
		re.AddPolyToScene( args[1], args[2], VMA(3), 1 );
		return 0;

	case UI_R_ADDLIGHTTOSCENE:
		re.AddLightToScene( VMA(1), VMF(2), VMF(3), VMF(4), VMF(5) );
		return 0;

	case UI_R_RENDERSCENE:
		re.RenderScene( VMA(1) );
		return 0;

	case UI_R_SETCOLOR:
		re.SetColor( VMA(1) );
		return 0;

	case UI_R_DRAWSTRETCHPIC:
		//re.DrawStretchPic( VMF(1), VMF(2), VMF(3), VMF(4), VMF(5), VMF(6), VMF(7), VMF(8), args[9] );
		return 0;

	case UI_R_MODELBOUNDS:
		re.ModelBounds( args[1], VMA(2), VMA(3) );
		return 0;

	case UI_UPDATESCREEN:
		if(uivm == 0)
			SCR_UpdateScreen(qtrue);
		return 0;

	case UI_CM_LERPTAG:
		re.LerpTag( VMA(1), args[2], args[3], args[4], VMF(5), VMA(6) );
		return 0;

	case UI_S_REGISTERSOUND:
		return S_RegisterSound( VMA(1), args[2] );

	case UI_S_STARTLOCALSOUND:
		S_StartLocalSound( args[1], args[2] );
		return 0;

	case UI_KEY_KEYNUMTOSTRINGBUF:
		VM_CHECKBOUNDS( uivms[uivm], args[2], args[3] );
		Key_KeynumToStringBuf( args[1], VMA(2), args[3] );
		return 0;

	case UI_KEY_GETBINDINGBUF:
		VM_CHECKBOUNDS( uivms[uivm], args[2], args[3] );
		Key_GetBindingBuf( args[1], VMA(2), args[3] );
		return 0;

	case UI_KEY_SETBINDING:
		Key_SetBinding( args[1], VMA(2) );
		return 0;

	case UI_KEY_ISDOWN:
		return Key_IsDown( args[1] );

	case UI_KEY_GETOVERSTRIKEMODE:
		return Key_GetOverstrikeMode();

	case UI_KEY_SETOVERSTRIKEMODE:
		Key_SetOverstrikeMode( args[1] );
		return 0;

	case UI_KEY_CLEARSTATES:
		Key_ClearStates();
		return 0;

	case UI_KEY_GETCATCHER:
		return Key_GetCatcher() & ~KEYCATCH_CONSOLE;

	case UI_KEY_SETCATCHER:
		// Don't allow the ui module to close the console
		if(uivm == 0)
			Key_SetCatcher( args[1] | ( Key_GetCatcher( ) & KEYCATCH_CONSOLE ) );
		return 0;

	case UI_GETCLIPBOARDDATA:
		VM_CHECKBOUNDS( uivms[uivm], args[1], args[2] );
		CL_GetClipboardData( VMA(1), args[2] );
		return 0;

	case UI_GETCLIENTSTATE:
		VM_CHECKBOUNDS( uivms[uivm], args[1], sizeof( uiClientState_t ) );
		GetClientState( VMA(1) );
		return 0;		

	case UI_GETGLCONFIG:
		VM_CHECKBOUNDS( uivms[uivm], args[1], sizeof( glconfig_t ) );
#ifdef USE_VID_FAST
		cls.uiGlConfig = VMA(1);
#endif
		CL_GetGlconfig( VMA(1) );
		return 0;

	case UI_GETCONFIGSTRING:
		VM_CHECKBOUNDS( uivms[uivm], args[2], args[3] );
		return GetConfigString( args[1], VMA(2), args[3] );

	case UI_LAN_LOADCACHEDSERVERS:
		LAN_LoadCachedServers();
		return 0;

	case UI_LAN_SAVECACHEDSERVERS:
		LAN_SaveServersToCache();
		return 0;

	case UI_LAN_ADDSERVER:
		return LAN_AddServer(args[1], VMA(2), VMA(3));

	case UI_LAN_REMOVESERVER:
		LAN_RemoveServer(args[1], VMA(2));
		return 0;

	case UI_LAN_GETPINGQUEUECOUNT:
		return LAN_GetPingQueueCount();

	case UI_LAN_CLEARPING:
		LAN_ClearPing( args[1] );
		return 0;

	case UI_LAN_GETPING:
		VM_CHECKBOUNDS( uivms[uivm], args[2], args[3] );
		LAN_GetPing( args[1], VMA(2), args[3], VMA(4) );
		return 0;

	case UI_LAN_GETPINGINFO:
		VM_CHECKBOUNDS( uivms[uivm], args[2], args[3] );
		LAN_GetPingInfo( args[1], VMA(2), args[3] );
		return 0;

	case UI_LAN_GETSERVERCOUNT:
		return LAN_GetServerCount(args[1]);

	case UI_LAN_GETSERVERADDRESSSTRING:
		VM_CHECKBOUNDS( uivms[uivm], args[3], args[4] );
		LAN_GetServerAddressString( args[1], args[2], VMA(3), args[4] );
		return 0;

	case UI_LAN_GETSERVERINFO:
		VM_CHECKBOUNDS( uivms[uivm], args[3], args[4] );
		LAN_GetServerInfo( args[1], args[2], VMA(3), args[4] );
		return 0;

	case UI_LAN_GETSERVERPING:
		return LAN_GetServerPing( args[1], args[2] );

	case UI_LAN_MARKSERVERVISIBLE:
		LAN_MarkServerVisible( args[1], args[2], args[3] );
		return 0;

	case UI_LAN_SERVERISVISIBLE:
		return LAN_ServerIsVisible( args[1], args[2] );

	case UI_LAN_UPDATEVISIBLEPINGS:
		return LAN_UpdateVisiblePings( args[1] );

	case UI_LAN_RESETPINGS:
		LAN_ResetPings( args[1] );
		return 0;

	case UI_LAN_SERVERSTATUS:
		VM_CHECKBOUNDS( uivms[uivm], args[2], args[3] );
		return LAN_GetServerStatus( VMA(1), VMA(2), args[3] );

	case UI_LAN_COMPARESERVERS:
		return LAN_CompareServers( args[1], args[2], args[3], args[4], args[5] );

	case UI_MEMORY_REMAINING:
		return Hunk_MemoryRemaining();

	case UI_GET_CDKEY:
		VM_CHECKBOUNDS( uivms[uivm], args[1], args[2] );
		CLUI_GetCDKey( VMA(1), args[2] );
		return 0;

	case UI_SET_CDKEY:
#ifndef STANDALONE
		CLUI_SetCDKey( VMA(1) );
#endif
		return 0;
	
	case UI_SET_PBCLSTATUS:
		return 0;

	case UI_R_REGISTERFONT:
		re.RegisterFont( VMA(1), args[2], VMA(3));
		return 0;

	// shared syscalls

	case TRAP_MEMSET:
		VM_CHECKBOUNDS( uivms[uivm], args[1], args[3] );
		Com_Memset( VMA(1), args[2], args[3] );
		return args[1];

	case TRAP_MEMCPY:
		VM_CHECKBOUNDS2( uivms[uivm], args[1], args[2], args[3] );
		Com_Memcpy( VMA(1), VMA(2), args[3] );
		return args[1];

	case TRAP_STRNCPY:
		VM_CHECKBOUNDS( uivms[uivm], args[1], args[3] );
		strncpy( VMA(1), VMA(2), args[3] );
		return args[1];

	case TRAP_SIN:
		return FloatAsInt( sin( VMF(1) ) );

	case TRAP_COS:
		return FloatAsInt( cos( VMF(1) ) );

	case TRAP_ATAN2:
		return FloatAsInt( atan2( VMF(1), VMF(2) ) );

	case TRAP_SQRT:
		return FloatAsInt( sqrt( VMF(1) ) );

	case UI_FLOOR:
		return FloatAsInt( floor( VMF(1) ) );

	case UI_CEIL:
		return FloatAsInt( ceil( VMF(1) ) );

	case UI_PC_ADD_GLOBAL_DEFINE:
		return botlib_export->PC_AddGlobalDefine( VMA(1) );
	case UI_PC_LOAD_SOURCE:
		return botlib_export->PC_LoadSourceHandle( VMA(1) );
	case UI_PC_FREE_SOURCE:
		return botlib_export->PC_FreeSourceHandle( args[1] );
	case UI_PC_READ_TOKEN:
		return botlib_export->PC_ReadTokenHandle( args[1], VMA(2) );
	case UI_PC_SOURCE_FILE_AND_LINE:
		return botlib_export->PC_SourceFileAndLine( args[1], VMA(2), VMA(3) );

	case UI_S_STOPBACKGROUNDTRACK:
		S_StopBackgroundTrack();
		return 0;
	case UI_S_STARTBACKGROUNDTRACK:
		S_StartBackgroundTrack( VMA(1), VMA(2));
		return 0;

	case UI_REAL_TIME:
		return Com_RealTime( VMA(1) );

	case UI_CIN_PLAYCINEMATIC:
		Com_DPrintf("UI_CIN_PlayCinematic\n");
		return CIN_PlayCinematic(VMA(1), args[2], args[3], args[4], args[5], args[6]);

	case UI_CIN_STOPCINEMATIC:
		return CIN_StopCinematic(args[1]);

	case UI_CIN_RUNCINEMATIC:
		return CIN_RunCinematic(args[1]);

	case UI_CIN_DRAWCINEMATIC:
		CIN_DrawCinematic(args[1]);
		return 0;

	case UI_CIN_SETEXTENTS:
		CIN_SetExtents(args[1], args[2], args[3], args[4], args[5]);
		return 0;

	case UI_R_REMAP_SHADER:
		re.RemapShader( VMA(1), VMA(2), VMA(3) );
		return 0;

	case UI_VERIFY_CDKEY:
		return Com_CDKeyValidate(VMA(1), VMA(2));

	// engine extensions
	case UI_R_ADDREFENTITYTOSCENE2:
		re.AddRefEntityToScene( VMA(1), qtrue );
		return 0;

	// engine extensions
	case UI_R_ADDLINEARLIGHTTOSCENE:
		re.AddLinearLightToScene( VMA(1), VMA(2), VMF(3), VMF(4), VMF(5), VMF(6) );
		return 0;

	case UI_TRAP_GETVALUE:
		VM_CHECKBOUNDS( uivms[uivm], args[1], args[2] );
		return UI_GetValue( VMA(1), args[2], VMA(3) );
		
	default:
		Com_Error( ERR_DROP, "Bad UI system trap: %ld", (long int) args[0] );

	}

	return 0;
}


/*
====================
UI_DllSyscall
====================
*/
static intptr_t QDECL UI_DllSyscall( intptr_t arg, ... ) {
#if !id386 || defined __clang__
	intptr_t	args[10]; // max.count for UI
	va_list	ap;
	int i;

	args[0] = arg;
	va_start( ap, arg );
	for (i = 1; i < ARRAY_LEN( args ); i++ )
		args[ i ] = va_arg( ap, intptr_t );
	va_end( ap );

	return CL_UISystemCalls( args );
#else
	return CL_UISystemCalls( &arg );
#endif
}


/*
====================
CL_ShutdownUI
====================
*/
void CL_ShutdownUI( void ) {
	Key_SetCatcher( Key_GetCatcher() & ~KEYCATCH_UI );
	cls.uiStarted = qfalse;
	for(int i = 0; i < MAX_NUM_VMS; i++) {
		uivm = i;
		if ( !uivms[uivm] ) {
			continue;
		}
		VM_Call( uivms[uivm], 0, UI_SHUTDOWN );
		VM_Free( uivms[uivm] );
		uivms[uivm] = NULL;
	}
	uivm = 0;
	FS_VM_CloseFiles( H_Q3UI );

#ifdef USE_RMLUI
#ifdef USE_RMLUI_DLOPEN
  Rml_Shutdown_t Rml_Shutdown = Sys_LoadFunction( rmlLib, "Rml_Shutdown" );
#endif
	if(cls.rmlStarted)
		Rml_Shutdown();
#endif
#ifdef USE_ABS_MOUSE
	cls.cursorx = 0;
	cls.cursory = 0;
	cls.uiGlConfig = NULL;
	cls.numUiPatches = 0;
#endif
}

#ifdef USE_RMLUI
static fileHandle_t CL_RmlOpen(const char * filename) {
	fileHandle_t h;
	/*int size = */ FS_FOpenFileRead(filename, &h, qfalse);
	return h;
}

static void CL_RmlClose(fileHandle_t h) {
	FS_FCloseFile( h );
}

static size_t CL_RmlRead(void* buffer, size_t size, fileHandle_t file) {
	return FS_Read(buffer, size, file);
}

static qboolean CL_RmlSeek(fileHandle_t file, long offset, int origin) {
	return FS_Seek(file, offset, origin);
}

static int CL_RmlTell(fileHandle_t file) {
	return FS_FTell(file);
}

static int CL_RmlLength(fileHandle_t h) {
	int pos = FS_FTell( h );
	FS_Seek( h, 0, FS_SEEK_END );
	int end = FS_FTell( h );
	FS_Seek( h, pos, FS_SEEK_SET );
	return end;
}

int CL_RmlLoadFile( const char *qpath, char **buffer )
{
	Com_Printf("Load file: %s\n", qpath);
	return FS_ReadFile(qpath, (void **)buffer);
}

typedef enum 
{
  LT_ALWAYS = 0,
  LT_ERROR,
  LT_ASSERT,
  LT_WARNING,
  LT_INFO,
  LT_DEBUG,
  LT_MAX
} rmlLog_t;

static qboolean CL_RmlLogMessage(int type, const char *message) {
	switch(type) {
		case LT_ALWAYS:
		Com_Printf("RMLUI: %s\n", message);
		break;
	  case LT_ERROR:
		Com_Error(ERR_FATAL, "RMLUI: %s\n", message);
		break;
	  case LT_ASSERT:
		Com_Error(ERR_FATAL, "RMLUI: %s\n", message);
		break;
	  case LT_WARNING:
		Com_Printf(S_COLOR_YELLOW "RMLUI: %s\n", message);
		break;
	  case LT_INFO:
		Com_Printf(S_COLOR_WHITE "RMLUI: %s\n", message);
		break;
	  case LT_DEBUG:
		Com_DPrintf("RMLUI: %s\n", message);
		break;
	  case LT_MAX:
		Com_Error(ERR_FATAL, "RMLUI: %s\n", message);
	}
	return qtrue;
}

static double CL_RmlGetElapsedTime( void ) {
	return Sys_Milliseconds() / 1000;
}

static qhandle_t CL_RmlLoadTexture(int *dimensions, const char *source) {
  qhandle_t result = re.RegisterImage(dimensions, source);
  return result;
}

static int imgCount = 0;
static qhandle_t CL_RmlGenerateTexture(const byte* source, const int *source_dimensions) {
	return re.CreateShaderFromImageBytes(va("rml_%i", ++imgCount), source, source_dimensions[0], source_dimensions[1]);
}

static void CL_RmlRenderGeometry(void *vertices, int num_vertices, int* indices, 
  int num_indices, qhandle_t texture, const vec2_t translation)
{
  int *sourceVerts = (int *)vertices;
  polyVert_t verts[num_vertices];
  for(int  i = 0; i < num_vertices; i++) {
    vec2_t pos;
    memcpy(&pos, &sourceVerts[i*5+0], sizeof(vec2_t));
    vec2_t size;
    memcpy(&size, &sourceVerts[i*5+3], sizeof(vec2_t));
    verts[i].xyz[0] = pos[0] + translation[0];
    verts[i].xyz[2] = 1;
    verts[i].xyz[1] = pos[1] + translation[1];
    verts[i].st[0] = size[0] * 512;
    verts[i].st[1] = size[1] * 512;
    //Com_Printf("%f x %f <-> %f x %f\n", verts[i].xyz[0],
    //  verts[i].xyz[1], verts[i].st[0], verts[i].st[1]);
    verts[i].modulate[0] = //sourceVerts[i*5+2] >> 24 & 0xFF;
    verts[i].modulate[1] = //sourceVerts[i*5+2] >> 16 & 0xFF;
    verts[i].modulate[2] = //sourceVerts[i*5+2] >> 8 & 0xFF;
    verts[i].modulate[3] = 255; //sourceVerts[i*5+2] & 0xFF;
  }
  
  for(int  i = 0; i < num_vertices / 4; i++) {
    vec2_t pos;
    memcpy(&pos, &sourceVerts[(i*4)*5+0], sizeof(vec2_t));
    vec2_t size;
    memcpy(&size, &sourceVerts[(i*4)*5+3], sizeof(vec2_t));
    vec2_t pos2;
    memcpy(&pos2, &sourceVerts[(i*4+2)*5+0], sizeof(vec2_t));
    vec2_t size2;
    memcpy(&size2, &sourceVerts[(i*4+2)*5+3], sizeof(vec2_t));
    re.DrawStretchPic( pos[0] + translation[0], pos[1] + translation[1], pos2[0] + translation[0], pos2[1] + translation[1], size[0], size[1], size2[0], size2[1], texture );
  }
  

  //re.AddPolyToScene(texture, num_vertices, verts, 1);
  //re.DrawElements(num_indices, indices);
}

static qhandle_t CL_RmlCompileGeometry(void *vertices, int num_vertices, int* indices, 
  int num_indices, qhandle_t texture)
{
  return 0;
}

void CL_UIContextRender(void) {
  Rml_ContextRender(0);
}

#endif

/*
====================
CL_InitUI
====================
*/
#define UI_OLD_API_VERSION	4

void CL_InitUI( qboolean loadNew ) {
	int		v;
	vmInterpret_t		interpret;

	// disallow vl.collapse for UI elements
	re.VertexLighting( qfalse );

	// load the dll or bytecode
	interpret = Cvar_VariableIntegerValue( "vm_ui" );
	if ( cl_connectedToPureServer )
	{
		// if sv_pure is set we only allow qvms to be loaded
		if ( interpret != VMI_COMPILED && interpret != VMI_BYTECODE )
			interpret = VMI_COMPILED;
	}
	
	if(loadNew && uivms[uivm] != NULL) {
		uivm++;
	}
	uivms[uivm] = VM_Create( VM_UI, CL_UISystemCalls, UI_DllSyscall, interpret );
	if ( !uivms[uivm] ) {
		if ( cl_connectedToPureServer && CL_GameSwitch() ) {
			// server-side modificaton may require and reference only single custom ui.qvm
			// so allow referencing everything until we download all files
			// new gamestate will be requested after downloads complete
			// which will correct filesystem permissions
			fs_reordered = qfalse;
			FS_PureServerSetLoadedPaks( "", "" );
			uivms[uivm] = VM_Create( VM_UI, CL_UISystemCalls, UI_DllSyscall, interpret );
			if ( !uivms[uivm] ) {
				Com_Error( ERR_DROP, "VM_Create on UI failed" );
			}
		} else {
			Com_Error( ERR_DROP, "VM_Create on UI failed" );
		}
	}

	// sanity check
	v = VM_Call( uivms[uivm], 0, UI_GETAPIVERSION );
	if (v == UI_OLD_API_VERSION) {
//		Com_Printf(S_COLOR_YELLOW "WARNING: loading old Quake III Arena User Interface version %d\n", v );
		// init for this gamestate
		VM_Call( uivms[uivm], 1, UI_INIT, (cls.state >= CA_AUTHORIZING && cls.state < CA_ACTIVE) );
	}
	else if (v != UI_API_VERSION) {
		// Free uivms[uivm] now, so UI_SHUTDOWN doesn't get called later.
		VM_Free( uivms[uivm] );
		uivms[uivm] = NULL;

		Com_Error( ERR_DROP, "User Interface is version %d, expected %d", v, UI_API_VERSION );
		cls.uiStarted = qfalse;
	}
	else {
		// init for this gamestate
		VM_Call( uivms[uivm], 1, UI_INIT, (cls.state >= CA_AUTHORIZING && cls.state < CA_ACTIVE) );
	}

#ifdef USE_RMLUI
#ifdef USE_RMLUI_DLOPEN

#ifdef EMSCRIPTEN
#define REND_ARCH_STRING "js"
#else
#if defined (__linux__) && defined(__i386__)
#define REND_ARCH_STRING "x86"
#else
#define REND_ARCH_STRING ARCH_STRING
#endif // __linux__
#endif // EMSCRIPTEN

	Com_sprintf( dllName, sizeof( dllName ), "libRmlCore_" REND_ARCH_STRING DLL_EXT );
	rmlLib = FS_LoadLibrary( dllName );
#ifdef EMSCRIPTEN
	Com_Frame_RentryHandle(CL_InitUI_After_Load);
}

static void CL_InitUI_After_Load( void *handle )
{
  rmlLib = handle;
#endif
;
	if ( !rmlLib )
	{
		Com_Error( ERR_FATAL, "Failed to load rmlui %s", dllName );
	}

	Rml_SetFileInterface_t Rml_SetFileInterface = Sys_LoadFunction( rmlLib, "Rml_SetFileInterface" );
	if( !Rml_SetFileInterface )
	{
		Com_Error( ERR_FATAL, "Can't load symbol Rml_SetFileInterface" );
		return;
	}
  Rml_SetSystemInterface_t Rml_SetSystemInterface = Sys_LoadFunction( rmlLib, "Rml_SetSystemInterface" );
  Rml_Initialize_t Rml_Initialize = Sys_LoadFunction( rmlLib, "Rml_Initialize" );
  Rml_SetRenderInterface_t Rml_SetRenderInterface = Sys_LoadFunction( rmlLib, "Rml_SetRenderInterface" );
  Rml_CreateContext_t Rml_CreateContext = Sys_LoadFunction( rmlLib, "Rml_CreateContext" );
  Rml_LoadDocument_t Rml_LoadDocument = Sys_LoadFunction( rmlLib, "Rml_LoadDocument" );
  Rml_ShowDocument_t Rml_ShowDocument = Sys_LoadFunction( rmlLib, "Rml_ShowDocument" );
  Rml_Shutdown_t Rml_Shutdown = Sys_LoadFunction( rmlLib, "Rml_Shutdown" );
  Rml_ContextRender = Sys_LoadFunction( rmlLib, "Rml_ContextRender" );
  Rml_ContextUpdate = Sys_LoadFunction( rmlLib, "Rml_ContextUpdate" );
#endif // USE_BOTLIB_DLOPEN
;
	static RmlFileInterface files;
	files.Open = CL_RmlOpen;
	files.Close = CL_RmlClose;
	files.Read = CL_RmlRead;
	files.Seek = CL_RmlSeek;
	files.Tell = CL_RmlTell;
	files.Length = CL_RmlLength;
	files.LoadFile = CL_RmlLoadFile;
	static RmlRenderInterface renderer;
	renderer.LoadTexture = CL_RmlLoadTexture;
  renderer.GenerateTexture = CL_RmlGenerateTexture;
  renderer.RenderGeometry = CL_RmlRenderGeometry;
  renderer.CompileGeometry = CL_RmlCompileGeometry;
	static RmlSystemInterface system;
	system.LogMessage = CL_RmlLogMessage;
	system.GetElapsedTime = CL_RmlGetElapsedTime;
	Rml_SetFileInterface(&files);
	Rml_SetRenderInterface(&renderer);
	Rml_SetSystemInterface(&system);
	if(!Rml_Initialize()) {
		Com_Printf("RMLUI: Error initializing.");
	}
	cls.rmlStarted = qtrue;
  
  
	struct FontFace {
		char *filename;
		qboolean fallback_face;
	};
	struct FontFace font_faces[] = {
		{ "LatoLatin-Regular.ttf",    qfalse },
		{ "LatoLatin-Italic.ttf",     qfalse },
		{ "LatoLatin-Bold.ttf",       qfalse },
		{ "LatoLatin-BoldItalic.ttf", qfalse },
		{ "NotoEmoji-Regular.ttf",    qtrue  },
	};

	for (int i = 0; i < ARRAY_LEN(font_faces); i++)
	{
		Rml_LoadFontFace(va("assets/%s", font_faces[i].filename), font_faces[i].fallback_face);
	}

  
	qhandle_t ctx = Rml_CreateContext("default", cls.glconfig.vidWidth, cls.glconfig.vidWidth);
	
	qhandle_t doc = Rml_LoadDocument(ctx, "assets/demo.rml");

	if (doc)
	{
		Rml_ShowDocument(doc);
		Com_Printf("RMLUI: Document loaded\n");
		
		Rml_ContextRender(ctx);

		Rml_ContextUpdate(ctx);
	}
	else
	{
		Com_Printf("RMLUI: Document failed\n");
		Rml_Shutdown();
		cls.rmlStarted = qfalse;
	}
#endif

	/*
	
	RmlUiSDL2Renderer Renderer(renderer, screen);

	RmlUiSDL2SystemInterface SystemInterface;

	// Rml::String root = Shell::FindSamplesRoot();
	// ShellFileInterface FileInterface(root);

	Rml::SetFileInterface(&FileInterface);
	Rml::SetRenderInterface(&Renderer);
	Rml::SetSystemInterface(&SystemInterface);

	if (!Rml::Initialise())
		return 1;
		struct FontFace {
		Rml::String filename;
		bool fallback_face;
	};
	
	FontFace font_faces[] = {
		{ "LatoLatin-Regular.ttf",    false },
		{ "LatoLatin-Italic.ttf",     false },
		{ "LatoLatin-Bold.ttf",       false },
		{ "LatoLatin-BoldItalic.ttf", false },
		{ "NotoEmoji-Regular.ttf",    true  },
	};

	for (const FontFace& face : font_faces)
	{
		Rml::LoadFontFace("assets/" + face.filename, face.fallback_face);
	}

		Rml::Context* Context = Rml::CreateContext(,
			Rml::Vector2i(window_width, window_height));
	
		Rml::ElementDocument* Document = Context->LoadDocument("assets/demo.rml");
				
		if (Document)
		{
			Document->Show();
			fprintf(stdout, "\nDocument loaded");
		}
		else
		{
			fprintf(stdout, "\nDocument is nullptr");
		}

		bool done = false;

		while (!done)
		{
			SDL_Event event;

			SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
			SDL_RenderClear(renderer);

			Context->Render();
			SDL_RenderPresent(renderer);

			while (SDL_PollEvent(&event))
			{
				switch (event.type)
				{
				case SDL_QUIT:
					done = true;
					break;

				case SDL_MOUSEMOTION:
					Context->ProcessMouseMove(event.motion.x, event.motion.y, SystemInterface.GetKeyModifiers());
					break;
				case SDL_MOUSEBUTTONDOWN:
					Context->ProcessMouseButtonDown(SystemInterface.TranslateMouseButton(event.button.button), SystemInterface.GetKeyModifiers());
					break;

				case SDL_MOUSEBUTTONUP:
					Context->ProcessMouseButtonUp(SystemInterface.TranslateMouseButton(event.button.button), SystemInterface.GetKeyModifiers());
					break;

				case SDL_MOUSEWHEEL:
					Context->ProcessMouseWheel(float(event.wheel.y), SystemInterface.GetKeyModifiers());
					break;

				case SDL_KEYDOWN:
				{
					// Intercept F8 key stroke to toggle RmlUi's visual debugger tool
					if (event.key.keysym.sym == SDLK_F8)
					{
						Rml::Debugger::SetVisible(!Rml::Debugger::IsVisible());
						break;
					}

					Context->ProcessKeyDown(SystemInterface.TranslateKey(event.key.keysym.sym), SystemInterface.GetKeyModifiers());
					break;
				}

				default:
					break;
				}
			}
			Context->Update();
		}

		Rml::Shutdown();

	*/
	
}


qboolean UI_usesUniqueCDKey( void ) {
#ifndef STANDALONE
	if (uivms[uivm]) {
		return (VM_Call( uivms[uivm], 0, UI_HASUNIQUECDKEY ) != 0);
	} else
#endif
	{
		return qfalse;
	}
}


/*
====================
UI_GameCommand

See if the current console command is claimed by the ui
====================
*/
qboolean UI_GameCommand( int igvm ) {
	qboolean result;
#ifdef USE_MULTIVM_CLIENT
	int prevGvm = uivm;
	uivm = igvm;
	CM_SwitchMap(clientMaps[uivm]);
#endif
	if ( !uivms[uivm] ) {
		return qfalse;
	}

	result = VM_Call( uivms[uivm], 1, UI_CONSOLE_COMMAND, cls.realtime );
#ifdef USE_MULTIVM_CLIENT
	uivm = prevGvm;
#endif
	return result;
}
