// #include <sqlite3.h>

extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

int registerSQLite(lua_State* L);
int  lib_register(lua_State* L);
int  lib_deps(lua_State* L);
}

