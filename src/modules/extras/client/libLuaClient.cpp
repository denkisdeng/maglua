#include <iostream>
#ifndef WIN32
#include <strings.h>
#include <unistd.h>
#include <error.h>
#else
#define close(a) closesocket(a)
static int WSAStartupCalled = 0;
#endif
#include <errno.h>
#include <string.h>
#include "net_helpers.h"
#include <stdlib.h>

#include "libLuaClient.h"

#define SHUTDOWN        0

extern "C" {
	#include <lua.h>
	#include <lualib.h>
	#include <lauxlib.h>
}

using namespace std;

#define MAXHOSTCACHE 64
static long servers[MAXHOSTCACHE];
static char names[MAXHOSTCACHE][256];
static int num_name_cache = 0;

LuaClient::LuaClient()
	: LuaBaseObject(hash32(LuaClient::typeName()))
{
#ifdef WIN32
	if(!WSAStartupCalled)
	{
		WSAStartupCalled = 1;
		WORD wVersionRequested;
		WSADATA wsaData;
		wVersionRequested = MAKEWORD(1, 1);
		int err = WSAStartup(wVersionRequested, &wsaData); 
		if (err != 0)
	        printf("(%s:%i) WSAStartup error: %i\n", __FILE__, __LINE__, err);
	}
#endif
	_connected = false;
	refcount = 0;
}

LuaClient::~LuaClient()
{
	disconnect();
}

int LuaClient::luaInit(lua_State* L)
{
	if(lua_isstring(L, 1))
	{
		if(!connectTo(lua_tostring(L, 1)))
			return luaL_error(L, "Failed to connect to `%s'\n", lua_tostring(L, 1));
	}
	return 0;
}

bool LuaClient::connectTo(const char* host_port)
{
	if(_connected)
		return false;

	char* host = new char[strlen(host_port)+1];
	int port;
	
	strcpy(host, host_port);
	int spot = 0;
	
	for(int i=strlen(host)-1; i>=0; i--)
	{
		if(host[i] == ':')
		{
			host[i] = 0;
			spot = i;
			i = 0;
		}
	}
	
	if(!spot)
	{
		cerr << "Unable to parse host and port: " << host_port << endl;
		delete [] host;
		return false;
	}
	
	port = atoi(host_port + spot + 1);
	
// 	_client = SHM_Connect(host, port);
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		fprintf(stderr, "ERROR: failed to open socket: %s\n", strerror(errno));
		return false;
	}
	
	// gethostbyname seems to have anti-DOS gibbereish inside so we'll cache
	// any lookedup values. This means we'll first look at the cache for
	// server_addys
	long saddr = 0;
	for(int i=0; i<num_name_cache && !saddr; i++)
	{
		if(strcmp(names[i], host) == 0)
		{
			saddr = servers[i];
		}
	}
	
	if(!saddr) //then we need to look it up
	{
		struct hostent *server = gethostbyname(host);
		if(server == NULL) 
		{
			fprintf(stderr,"ERROR: No such host (%s)\n", host);
			return 0;
		}
		
		if(num_name_cache < MAXHOSTCACHE)
		{
			servers[num_name_cache] = *(long*) (server->h_addr_list[0]);
			saddr = servers[num_name_cache];
			strncpy(names[num_name_cache], host, 256);
			num_name_cache++;
		}
	}
	
	memset(&serv_addr, 0, sizeof(serv_addr));	// create and zero struct
	
	serv_addr.sin_family = AF_INET;    /* select internet protocol */
	serv_addr.sin_port = htons(port);         /* set the port # */
	serv_addr.sin_addr.s_addr = saddr; //*(long*) (server->h_addr_list[0]);  /* set the addr */
	
	if(connect(sockfd, (const sockaddr*)&(serv_addr), sizeof(serv_addr)))
	{
		fprintf(stderr, "ERROR: failed to connect to %s:%i\n", host, port);
		fprintf(stderr, "       %s\n", strerror(errno));
		return false;
	}
		
	sem_init(&rwSem, 0, 1);
	sem_init(&ioSem, 0, 1);

	_connected = true;
	sourcename = host_port;

	delete [] host;

	return true;
}

void LuaClient::disconnect()
{
	if(_connected)
	{
		bool ok;
		int cmd = SHUTDOWN;
		sem_wait(&rwSem);
		sure_write(sockfd, &cmd, sizeof(int), &ok);
		//printf("ok: %i\n", ok);
		close(sockfd);
		sockfd = 0;
		sem_post(&rwSem);
		_connected = false;
		//printf("Sent shutdown signal\n");
	}
}

#if 0
void LuaClient::uploadLua(lua_State* L)
{
	int ok = 1;
	int n = lua_gettop(L);

	if(!n) return;

 	sem_wait(&ioSem);
	lua_Variable* vars = (lua_Variable*)malloc(sizeof(lua_Variable)*n);

	for(int i=0; i<n; i++)
	{
		initLuaVariable(&vars[i]);
		exportLuaVariable(L, i+1, &vars[i]);
	}

	int b[2];
	b[0] = 10; //10 == UPLOADLUA
	b[1] = n;

	sure_write(sockfd, b, sizeof(int)*2);

	for(int i=0; i<n; i++)
		rawVarUpload(&vars[i]);
	
	for(int i=0; i<n; i++)
		freeLuaVariable(&vars[i]);

	free(vars);
 	sem_post(&ioSem);
}
#endif

int LuaClient::remoteExecuteLua(lua_State* L)
{
	bool ok = 1;
 	sem_wait(&ioSem);
	int n = lua_gettop(L);
	
	LuaVariableGroup input;
	LuaVariableGroup output;

	if(!n)
	{
		sem_post(&ioSem);
		return 0;
	}

	input.readState(L);

	int b = 11; //11 == REMOTELUA

	sure_write(sockfd, &b, sizeof(int), &ok);

	input.write(sockfd, ok);
	input.clear();

	if(!ok)
	{
		sem_post(&ioSem); //we can let other communication happen now. 
		return luaL_error(L, "network fail in :remote");
	}

	//lua function is now being run on the server. 
 	sem_post(&ioSem); //we can let other communication happen now. 

	//the server has "consumed" the variables. we should pop them from the stack.
	lua_pop(L, n);

 	sem_wait(&ioSem);
	output.read(sockfd, ok);
	if(!ok)
	{
		sem_post(&ioSem);
		return luaL_error(L, "network fail in :remote");
	}
	sem_post(&ioSem);

	output.writeState(L);
	
	return output.sizes.size();
}













static int l_remote(lua_State* L)
{
	LUA_PREAMBLE(LuaClient, lc, 1);
	lua_remove(L, 1); //get object off the stack
	return lc->remoteExecuteLua(L);
}


static int l_connect(lua_State* L)
{
	LUA_PREAMBLE(LuaClient, lc, 1);
	lua_pushboolean(L, lc->connectTo(lua_tostring(L, 2)));
	return 1;
}

static int l_connected(lua_State* L)
{
	LUA_PREAMBLE(LuaClient, lc, 1);
	lua_pushboolean(L, lc->connected());
	return 1;
}

static int l_disconnect(lua_State* L)
{
	LUA_PREAMBLE(LuaClient, lc, 1);
	lc->disconnect();
	return 0;
}

static luaL_Reg m[128] = {_NULLPAIR128};
const luaL_Reg* LuaClient::luaMethods()
{
	if(m[127].name)return m;

	static const luaL_Reg _m[] =
	{
		{"remote",       l_remote},
		{"connect",      l_connect},
		{"connected",    l_connected},
		{"disconnect",   l_disconnect},
		{NULL, NULL}
	};
	merge_luaL_Reg(m, _m);
	m[127].name = (char*)1;
	return m;
}


#include "info.h"
extern "C"
{
CLIENT_API int lib_register(lua_State* L);
CLIENT_API int lib_version(lua_State* L);
CLIENT_API const char* lib_name(lua_State* L);
CLIENT_API int lib_main(lua_State* L);
}

CLIENT_API int lib_register(lua_State* L)
{
	luaT_register<LuaClient>(L);
	return 0;
}

CLIENT_API int lib_version(lua_State* L)
{
	return __revi;
}

CLIENT_API const char* lib_name(lua_State* L)
{
	return "Client";
}

CLIENT_API int lib_main(lua_State* L)
{
	return 0;
}

