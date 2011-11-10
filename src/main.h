#ifndef WIN32
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <dirent.h>
#define HOME "HOME"
#define SO_EXT "so"
#define MAGLUA_SETUP_PATH "/.maglua.d/module_path.lua"
#define PATH_SEP "/"
#else
 #include <windows.h>
 #define strncasecmp(A,B,C) _strnicmp(A,B,C)
 #pragma warning(disable: 4251)
 #pragma warning(disable: 4996)
 #define snprintf _snprintf
 #define HOME "APPDATA"
 #define SO_EXT "dll"
 #define MAGLUA_SETUP_PATH "\\maglua\\module_path.lua"
 #define PATH_SEP "\\"
#endif

#ifndef SETUP_CODE
#define SETUP_CODE

static const char* setup_lua_code = 
#ifndef WIN32
"function setup(mod_path)\n"\
"	print(\"Creating default startup files in $(HOME)/.maglua.d\")\n"\
"	print(\"adding path `\" .. mod_path .. \"'\")\n"\
"	local home = os.getenv(\"HOME\")\n"\
"	os.execute(\"mkdir -p \" .. home .. \"/.maglua.d/\")\n"\
"	f = io.open(home .. \"/.maglua.d/module_path.lua\", \"w\")\n"\
"	f:write(\"-- Modules in the following directories will be loaded\\n\")\n"\
"	f:write(\"module_path = {\\\"\" .. mod_path .. \"\\\"}\\n\")\n"\
"end\n";
#else
"function setup(mod_path)\n"\
"   mod_path = string.gsub(mod_path, \"\\\\\", \"\\\\\\\\\")\n"\
"	print(\"Creating default startup files in $(APPDATA)\\\\maglua\")\n"\
"	print(\"adding path `\" .. mod_path .. \"'\")\n"\
"	local home = os.getenv(\"APPDATA\")\n"\
"	os.execute(\"mkdir \\\"\" .. home .. \"\\\\maglua\")\n"\
"	f = io.open(home .. \"\\\\maglua\\\\module_path.lua\", \"w\")\n"\
"	f:write(\"-- Modules in the following directories will be loaded\\n\")\n"\
"	f:write(\"module_path = {\\\"\" .. mod_path .. \"\\\"}\\n\")\n"\
"end\n";
#endif
#endif //SETUP_CODE

#ifdef WIN32
 #define strcasecmp(A,B) _stricmp(A,B)
 #define strncasecmp(A,B,C) _strnicmp(A,B,C)
 #pragma warning(disable: 4251)

 #ifdef MAGLUA_EXPORTS
  #define MAGLUA_API __declspec(dllexport)
 #else
  #define MAGLUA_API __declspec(dllimport)
 #endif
#else
 #define MAGLUA_API 
#endif

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

MAGLUA_API int registerMain(lua_State* L);
}



