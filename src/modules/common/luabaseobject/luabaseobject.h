/******************************************************************************
* Copyright (C) 2008-2011 Jason Mercer.  All rights reserved.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/

#ifndef LUABASEOBJECT_H
#define LUABASEOBJECT_H

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "factory.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
typedef struct buffer
{
	char* buf;
	int pos;
	int size;
	FILE* debug;
	std::vector<void*> encoded;
	std::vector<int> encoded_table_refs;
	std::vector<const void*> encoded_table_pointers;
	buffer() { debug = 0; }
}buffer;

inline void buffer_init(buffer* b)
{
	b->pos = 0;
	b->size = 32;
	b->buf = (char*)malloc(32);
}

inline void buffer_unref(lua_State* L, buffer* b)
{
    // importing can create table references, we need to unref them
	for(unsigned int i=0; i<b->encoded_table_refs.size(); i++)
	{
		int r = b->encoded_table_refs[i];
		luaL_unref(L, LUA_REGISTRYINDEX, r);
	}
}

inline int luaL_dostringn(lua_State* L, const char* code, const char* name)
{
    return luaL_loadbuffer(L, code, strlen(code), name) || lua_pcall(L, 0, LUA_MULTRET, 0);
}


#ifndef ENCODE_PREAMBLE
#define ENCODE_MAGIC_NEW ((char)111)
#define ENCODE_MAGIC_OLD ((char)222)
#define ENCODE_PREAMBLE \
if(encodeContains(this, b)) \
{ \
	encodeOldThis(this, b); \
	return; \
} \
encodeNewThis(this, b);
#endif



extern "C"
{
   void encodeBuffer(const void* s, const int len, buffer* b);
   void encodeDouble(const double d, buffer* b);
   void encodeInteger(const int i, buffer* b);
   void encodeChar(const char c, buffer* b);
//   void encode

   int  encodeContains(LuaBaseObject* o, buffer* b);
   void encodeOldThis (LuaBaseObject* o, buffer* b);
   void encodeNewThis (LuaBaseObject* o, buffer* b);


    char decodeChar(buffer* b);
	int decodeInteger(buffer* b);
 double decodeDouble(buffer* b);
   void decodeBuffer(void* dest, const int len, buffer* b);
   void merge_luaL_Reg(luaL_Reg* old_vals, const luaL_Reg* new_vals);
   void merge_luaL_pair(luaL_Reg* old_vals, const char* name, lua_CFunction func);


   LuaBaseObject* decodeLuaBaseObject(lua_State* L, buffer* b);
}

template<class T>
void encodeT(T* lbo, buffer* b)
{
    encodeInteger(LUA_TUSERDATA, b);
    encodeInteger(lbo->type, b);
    lbo->encode(b);
}



template<class T>
T* decodeT(lua_State* L, buffer* b)
{
	return dynamic_cast<T*>(decodeLuaBaseObject(L, b));
}



#include <string.h>
#include <string>
#include <vector>
using namespace std;



class LuaBaseObject;
template<class T>
void luaT_push(lua_State* L, LuaBaseObject* tt);

#if defined (__GNUC__) || defined (__ICC)
#define TYPEOF(x) typeof(x)
#else
#define TYPEOF(x) typeof(x)
#endif

#define LINEAGE5(v1,v2,v3,v4,v5) \
	virtual const char* lineage(int i) { switch(i) { \
	case 0: return v1; case 1: return v2; \
	case 2: return v3; case 3: return v4; \
	case 4: return v5; } return 0; } \
	static const char* slineage(int i) { switch(i) { \
	case 0: return v1; case 1: return v2; \
	case 2: return v3; case 3: return v4; \
	case 4: return v5; } return 0; } \
	static const char* typeName() {return v1;} \
	virtual void push(lua_State * L){ luaT_push< TYPEOF(*this) >(L, this); }

#define LINEAGE4(v1,v2,v3,v4) LINEAGE5(v1,v2,v3,v4, 0)
#define LINEAGE3(v1,v2,v3)    LINEAGE4(v1,v2,v3, 0)
#define LINEAGE2(v1,v2)       LINEAGE3(v1,v2, 0)
#define LINEAGE1(v1)          LINEAGE2(v1, 0)
#define LINEAGE0()            LINEAGE1(0)

class  LuaBaseObject
{
public:
	LuaBaseObject(int type = 0);

	LINEAGE1("LuaBaseObject")
	static const luaL_Reg* luaMethods();
	// return value is number of arguments consumed from the bottom of the stack:
	virtual int luaInit(lua_State* _L, const int base=1) {const int i = base; L=_L; return i-i;}

	static int help(lua_State* /*L*/) {return 0;}
	static int regExtra(lua_State* /*L*/) {return 0;}

	virtual void encode(buffer* b);
	virtual int  decode(buffer* b);

	//string name;
	int type; //type is used internally for encoding/decoding
	int refcount;
	lua_State* L;
};


// this is the common preamble for most
// lua methods: Check to see if the element
// at the index is of the correct type
// and return 0 if it's not.
// T Class, t variable name, i stack index
#define LUA_PREAMBLE(T,t,i) \
	T* t = luaT_to<T>(L, i); \
	if(!t) return 0;



// decrement refcounter. delete if needed.
// always return resulting pointer
// does not rely on a lua_State
template<class T>
T* luaT_dec(T* t)
{
	if(!t)
		return 0;
	t->refcount--;
	if(t->refcount == 0)
	{
		delete t;
		return 0;
	}

	return t;
}

// increment refcounter. This could easily
// be a method of the baseClass but decrement
// cannot without some sort of a delayed delete
// mechanism. Keeping increment as an external
// function for symmetry
template<class T>
T* luaT_inc(T* t)
{
	if(!t)
		return 0;
	t->refcount++;
	return t;
}

// this is a common pattern
template<class T>
void luaT_set(T** pdest, T* src)
{
	luaT_inc<T>(src);
	luaT_dec<T>(*pdest);
	*pdest = src;
}

// Cast from LuaBaseObject to T
// useful when getting a LuaBaseObject from a buffer stream
template<class T>
T* luaT_cast(LuaBaseObject* t)
{
	return dynamic_cast<T*>(t);
}

// Convenience
template<class T>
T* luaT_inc_cast(LuaBaseObject* t)
{
	return luaT_inc<T>(luaT_cast<T>(t));
}


// test type
template<class T>
int luaT_is(lua_State* L, int idx)
{
	if(!lua_isuserdata(L, idx))
	{
		return 0;
	}

	LuaBaseObject** pp = (LuaBaseObject**)lua_touserdata(L, idx);

	if(!pp)
		return 0;

	int eq = 0;
	int i=0;
	while((*pp)->lineage(i) && !eq)
	{
		if(strcmp(T::typeName(), (*pp)->lineage(i)) == 0)
			eq = 1;
		i++;
	}

	return eq;
}

// convert an object on a stack to a C pointer
template<class T>
T* luaT_to(lua_State* L, int idx)
{
	if(luaT_is<T>(L, idx))
	{
		T** pp = (T**)lua_touserdata(L, idx);
		if(pp)
			return *pp;
		printf("null!\n");
	}

	char msg[128];
	snprintf(msg, 128, "%s expected, got %s", T::typeName(), lua_typename(L, lua_type(L, idx)));
	luaL_argerror(L, idx, msg);
	return 0;
}

// push an object on a Lua stack
template<class T>
void luaT_push(lua_State* L, LuaBaseObject* tt)
{
	T* t = dynamic_cast<T*>(tt);
	if(!t)
	{
		lua_pushnil(L);
	}
	else
	{
		T** pp = (T**)lua_newuserdata(L, sizeof(T**));
		*pp = luaT_inc<T>(t);

		luaL_getmetatable(L, T::typeName());
		lua_setmetatable(L, -2);
	}
}

// the following to and push functions are used in the wrappers
template<typename T>
inline T luaTT_to(lua_State* L, int idx)
{
	return luaT_to<T>(L, idx);
}
// specializations for to
template<>
inline int luaTT_to<int>(lua_State* L, int idx)
{
	return lua_tointeger(L, idx);
}
template<>
inline double luaTT_to<double>(lua_State* L, int idx)
{
	return lua_tonumber(L, idx);
}
template<>
inline const char* luaTT_to<const char*>(lua_State* L, int idx)
{
	return lua_tostring(L, idx);
}

// The following luaT_wrapper teplated functions can be used
// as C++/lua interface functions
//
// example:
// {"setValue", luaT_wrapper<ClassName, &ClassName::setValue>},

// caller - no args, no return
template<typename T, void (T::*method)()>
int luaT_wrapper(lua_State* L)
{
	LUA_PREAMBLE(T, t, 1);
	((*t).*method)();
	return 0;
}


// get double from class using getter
template<typename T, double (T::*getValue)() const>
int luaT_wrapper(lua_State* L)
{
	LUA_PREAMBLE(T, t, 1);
	double d = ((*t).*getValue)();
	lua_pushnumber(L, d);
	return 1;
}
template<typename T, double (T::*getValue)()>
int luaT_wrapper(lua_State* L)
{
	LUA_PREAMBLE(T, t, 1);
	double d = ((*t).*getValue)();
	lua_pushnumber(L, d);
	return 1;
}
// int getter
template<typename T, int (T::*getValue)()>
int luaT_wrapper(lua_State* L)
{
	LUA_PREAMBLE(T, t, 1);
	int d = ((*t).*getValue)();
	lua_pushinteger(L, d);
	return 1;
}
template<typename T, bool (T::*getValue)()>
int luaT_wrapper(lua_State* L)
{
	LUA_PREAMBLE(T, t, 1);
	bool d = ((*t).*getValue)();
	lua_pushboolean(L, d);
	return 1;
}
template<typename T, long (T::*getValue)()>
int luaT_wrapper(lua_State* L)
{
	LUA_PREAMBLE(T, t, 1);
	long d = ((*t).*getValue)();
	lua_pushinteger(L, d);
	return 1;
}
template<typename T, double (T::*getValue)(int,int)>
int luaT_wrapper(lua_State* L)
{
	LUA_PREAMBLE(T, t, 1);
	int v2 = lua_tointeger(L, 2);
	int v3 = lua_tointeger(L, 3);
	double d = ((*t).*getValue)(v2,v3);
	lua_pushinteger(L, d);
	return 1;
}

template<typename T, void (T::*setValue)(double)>
int luaT_wrapper(lua_State* L)
{
	LUA_PREAMBLE(T, t, 1);
	double d = lua_tonumber(L, 2);
	((*t).*setValue)(d);
	return 0;
}
template<typename T, void (T::*setValue)(float)>
int luaT_wrapper(lua_State* L)
{
	LUA_PREAMBLE(T, t, 1);
	double d = lua_tonumber(L, 2);
	((*t).*setValue)(d);
	return 0;
}
template<typename T, void (T::*setValue)(int)>
int luaT_wrapper(lua_State* L)
{
	LUA_PREAMBLE(T, t, 1);
	int d = lua_tointeger(L, 2);
	((*t).*setValue)(d);
	return 0;
}
template<typename T, void (T::*setValue)(long)>
int luaT_wrapper(lua_State* L)
{
	LUA_PREAMBLE(T, t, 1);
	long d = lua_tointeger(L, 2);
	((*t).*setValue)(d);
	return 0;
}
template<typename T, void (T::*setValue)(int,int,double)>
int luaT_wrapper(lua_State* L)
{
	LUA_PREAMBLE(T, t, 1);
	int v2 = lua_tointeger(L, 2);
	int v3 = lua_tointeger(L, 3);
	double v4 = lua_tonumber(L, 4);
	((*t).*setValue)(v2,v3,v4);
	return 0;
}

template<typename T, void (T::*setValue)(const char*)>
int luaT_wrapper(lua_State* L)
{
	LUA_PREAMBLE(T, t, 1);
	const char* c = lua_tostring(L, 2);
	((*t).*setValue)(c);
	return 0;
}



template<typename T, typename R, void (T::*method)(const R&)>
int luaT_wrapper(lua_State* L)
{
	LUA_PREAMBLE(T, t, 1);
	R v2;
	v2.luaInit(L, 2);
	((*t).*method)(v2);
	return 0;
}
template<typename T, typename R, void (T::*method)(R*)>
int luaT_wrapper(lua_State* L)
{
	LUA_PREAMBLE(T, t, 1);
	R* v2 = luaT_to<R>(L, 2);
	((*t).*method)(v2);
	return 0;
}
template<typename T, typename R, void (T::*method)(const R*)>
int luaT_wrapper(lua_State* L)
{
	LUA_PREAMBLE(T, t, 1);
	R* v2 = luaT_to<R>(L, 2);
	((*t).*method)(v2);
	return 0;
}

template<typename T, typename R, typename S, void (T::*method)(R*, S)>
int luaT_wrapper(lua_State* L)
{
	LUA_PREAMBLE(T, t, 1);
	R* v2 = luaT_to<R>(L, 2);
	S v3 = luaTT_to<S>(L, 3);
	((*t).*method)(v2,v3);
	return 0;
}
template<typename T, typename R, bool (T::*method)(const R&)>
int luaT_wrapper(lua_State* L)
{
	LUA_PREAMBLE(T, t, 1);
	R v2;
	v2.luaInit(L, 2);
	bool b = ((*t).*method)(v2);
	lua_pushboolean(L, b);
	return 1;
}
template<typename T, typename R, double (T::*method)(const R&)>
int luaT_wrapper(lua_State* L)
{
	LUA_PREAMBLE(T, t, 1);
	R v2;
	v2.luaInit(L, 2);
	double retval = ((*t).*method)(v2);
	lua_pushnumber(L, retval);
	return 1;
}
template<typename T, typename R, R* (T::*method)()>
int luaT_wrapper(lua_State* L)
{
	LUA_PREAMBLE(T, t, 1);
	luaT_push<R>(L, ((*t).*method)() );
	return 1;
}













// garbage collection metamethod.
// decrement refcount, delete if needed
template<class T>
int luaT_gc(lua_State* L)
{
	luaT_dec<T>(luaT_to<T>(L, 1));

	return 0;
}

// push metatable on the stack
template<class T>
int luaT_mt(lua_State* L)
{
	luaL_getmetatable(L, T::typeName());
	return 1;
}

// basic tostring metamathod acting like type()
template<class T>
int luaT_tostring(lua_State* L)
{
	T* t = luaT_to<T>(L, 1);
	if(!t) return 0;

	lua_pushfstring(L, "%s %p", T::typeName(), t);
	//lua_pushfstring(L, "%s", T::typeName());
	return 1;
}

// method to get a pointer as a string
template<class T>
int luaT_stringpointer(lua_State* L)
{
	T* t = luaT_to<T>(L, 1);
	if(!t) return 0;

	lua_pushfstring(L, "0x%p", t);
	return 1;
}

template<class T>
int luaT_setname(lua_State* L)
{
	T* t = luaT_to<T>(L, 1);
	if(!t) return 0;

	t->name = lua_tostring(L, 2);
	return 0;
}

//template<class T>
//int luaT_getname(lua_State* L)
//{
//	T* t = luaT_to<T>(L, 1);
//	if(!t) return 0;

//	lua_pushstring(L, t->name.toStdString().c_str());
//	return 1;
//}

// create a new object and push it on the stack
template<class T>
int luaT_new(lua_State* L)
{
	T* t = new T;
	t->luaInit(L);
	luaT_push<T>(L, t);
	return 1;
}

template<class T>
int luaT_help(lua_State* L)
{
	return T::help(L);
}

#define LUA_MAKEFUNCTIONHELP(cond, desc, input, output) \
    if(cond) {lua_pushstring(L, desc); lua_pushstring(L, input); lua_pushstring(L, output); return 3; }

template<class T>
int luaT_lineage(lua_State* L)
{
	lua_newtable(L);
	for(int i=1; i<=5; i++)
	{
		const char* l = T::slineage(i-1);
		if(l)
		{
			lua_pushinteger(L, i);
			lua_pushstring(L, l);
			lua_settable(L, -3);
		}
	}
	return 1;	
}

// add methods to the metamethod table of an object type
template<class T>
void luaT_addMethods(lua_State* L, const luaL_Reg* methods)
{
	if(!methods)
		return;
	luaL_getmetatable(L, T::typeName());
	luaL_register(L, NULL, methods);
	lua_pop(L,1);
}

template<class T>
LuaBaseObject* new_luabaseobject()
{
	return new T;
}

template<class T>
inline void luaT_register(lua_State* L)
{
	const int top = lua_gettop(L);
	if(!luaL_newmetatable(L, T::typeName()))
		return;

	T::regExtra(L);

	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);
	lua_pushstring(L, "__gc");
	lua_pushcfunction(L, luaT_gc<T>);
	lua_settable(L, -3);
	lua_pushstring(L, "__tostring");
	lua_pushcfunction(L, luaT_tostring<T>);
	lua_settable(L, -3);
        lua_pushstring(L, "topointer");
        lua_pushcfunction(L, luaT_stringpointer<T>);
	lua_settable(L, -3);


	const char* get_name =
	"return function(name)\n"
	" local t = {}\n"
	" for v in string.gmatch(name, \"%w+\") do\n"
	"  table.insert(t, v)\n"
	" end\n"
	" if t[1] == nil then return {} end\n"
	" _G[t[1]] = _G[t[1]] or {}\n"
	" local v = _G[t[1]]"
	" for i=2,table.maxn(t) do\n"
	"   v[t[i]] = v[t[i]] or {}\n"
	"   v = v[t[i]]\n"
	" end\n"
	" return v\n"
	"end\n"
	;
	luaL_dostringn(L, get_name, "@luabaseobject.h(get_name)");
	lua_pushstring(L, T::typeName());
	lua_call(L, 1,1);
	lua_pushstring(L, "new");
	lua_pushcfunction(L, luaT_new<T>);
	lua_settable(L, -3);
	lua_pushstring(L, "metatable");
	lua_pushcfunction(L, luaT_mt<T>);
	lua_settable(L, -3);
	lua_pushstring(L, "help");
	lua_pushcfunction(L, luaT_help<T>);
	lua_settable(L, -3);
	lua_pushstring(L, "lineage");
	lua_pushcfunction(L, luaT_lineage<T>);
	lua_settable(L, -3);

	if(T::luaMethods())
		luaT_addMethods<T>(L, T::luaMethods());

	Factory_registerItem(hash32(T::typeName()), new_luabaseobject<T>, luaT_push<T>, T::typeName());

	while(lua_gettop(L) > top)
		lua_pop(L, 1);
}

#define _NULLPAIR1    {NULL, NULL}
#define _NULLPAIR2   _NULLPAIR1, _NULLPAIR1
#define _NULLPAIR4   _NULLPAIR2, _NULLPAIR2
#define _NULLPAIR8   _NULLPAIR4, _NULLPAIR4
#define _NULLPAIR16  _NULLPAIR8, _NULLPAIR8
#define _NULLPAIR32  _NULLPAIR16,_NULLPAIR16
#define _NULLPAIR64  _NULLPAIR32,_NULLPAIR32
#define _NULLPAIR128 _NULLPAIR64,_NULLPAIR64


#endif // LUABASEOBJECT_H
