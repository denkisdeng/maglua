/******************************************************************************
* Copyright (C) 2008-2010 Jason Mercer.  All rights reserved.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/

#include "spinoperationappliedfield.h"
#include "spinsystem.h"

#include <stdlib.h>

AppliedField::AppliedField(int nx, int ny, int nz)
	: SpinOperation("AppliedField", APPLIEDFIELD_SLOT, nx, ny, nz, ENCODE_APPLIEDFIELD)
{
	B[0] = 0;
	B[1] = 0;
	B[2] = 0;
}

void AppliedField::encode(buffer* b) const
{
	encodeDouble(B[0], b);
	encodeDouble(B[1], b);
	encodeDouble(B[2], b);
}

int  AppliedField::decode(buffer* b)
{
	B[0] = decodeDouble(b);
	B[1] = decodeDouble(b);
	B[2] = decodeDouble(b);
	return 0;
}

AppliedField::~AppliedField()
{
}

bool AppliedField::apply(SpinSystem* ss)
{
	double* hx = ss->hx[slot];
	double* hy = ss->hy[slot];
	double* hz = ss->hz[slot];

	for(int i=0; i<nxyz; i++)
	{
		hx[i] = B[0];
		hy[i] = B[1];
		hz[i] = B[2];
		
		//printf("%g %g %g\n", hx[i], hy[i], hz[i]);
	}
	return true;
}







AppliedField* checkAppliedField(lua_State* L, int idx)
{
	AppliedField** pp = (AppliedField**)luaL_checkudata(L, idx, "MERCER.appliedfield");
    luaL_argcheck(L, pp != NULL, 1, "`AppliedField' expected");
    return *pp;
}

void lua_pushAppliedField(lua_State* L, AppliedField* ap)
{
	ap->refcount++;
	
	AppliedField** pp = (AppliedField**)lua_newuserdata(L, sizeof(AppliedField**));
	
	*pp = ap;
	luaL_getmetatable(L, "MERCER.appliedfield");
	lua_setmetatable(L, -2);
}

int l_ap_new(lua_State* L)
{
	int n[3];
	lua_getnewargs(L, n, 1);

	lua_pushAppliedField(L, new AppliedField(n[0], n[1], n[2]));
	
	return 1;
}

int l_ap_gc(lua_State* L)
{
	AppliedField* ap = checkAppliedField(L, 1);
	if(!ap) return 0;
	
	ap->refcount--;
	if(ap->refcount == 0)
		delete ap;
	
	return 0;
}

int l_ap_apply(lua_State* L)
{
	AppliedField* ap = checkAppliedField(L, 1);
	SpinSystem* ss = checkSpinSystem(L, 2);
	
	if(!ap->apply(ss))
		return luaL_error(L, ap->errormsg.c_str());
	
	return 0;
}

int l_ap_member(lua_State* L)
{
	AppliedField* ap = checkAppliedField(L, 1);
	if(!ap) return 0;

	int px = lua_tointeger(L, 2) - 1;
	int py = lua_tointeger(L, 3) - 1;
	int pz = lua_tointeger(L, 4) - 1;
	
	if(ap->member(px, py, pz))
		lua_pushboolean(L, 1);
	else
		lua_pushboolean(L, 0);

	return 1;
}

int l_ap_set(lua_State* L)
{
	AppliedField* ap = checkAppliedField(L, 1);
	if(!ap) return 0;
	
	double a[3];
	int r = lua_getNdouble(L, 3, a, 2, 0);
	if(r<0)
		return luaL_error(L, "invalid field");
	
	ap->B[0] = a[0];
	ap->B[1] = a[1];
	ap->B[2] = a[2];
	
	return 0;
}


int l_ap_tostring(lua_State* L)
{
	AppliedField* ap = checkAppliedField(L, 1);
	if(!ap) return 0;
	
	lua_pushfstring(L, "AppliedField (%dx%dx%d)", ap->nx, ap->ny, ap->nz);
	
	return 1;
}

int l_ap_get(lua_State* L)
{
	AppliedField* ap = checkAppliedField(L, 1);
	if(!ap) return 0;
	
	lua_newtable(L);
	for(int i=0; i<3; i++)
	{
		lua_pushinteger(L, i+1);
		lua_pushnumber(L, ap->B[i]);
		lua_settable(L, -3);
	}
	
	return 1;
}

static int l_ap_mt(lua_State* L)
{
	luaL_getmetatable(L, "MERCER.appliedfield");
	return 1;
}

static int l_ap_help(lua_State* L)
{
	if(lua_gettop(L) == 0)
	{
		lua_pushstring(L, "Applies an external, global field to a *SpinSystem*");
		lua_pushstring(L, ""); //input, empty
		lua_pushstring(L, ""); //output, empty
		return 3;
	}
	
	if(lua_istable(L, 1))
	{
		return 0;
	}
	
	if(!lua_iscfunction(L, 1))
	{
		return luaL_error(L, "help expect zero arguments or 1 function.");
	}
	
	lua_CFunction func = lua_tocfunction(L, 1);
	
	if(func == l_ap_new)
	{
		lua_pushstring(L, "Create a new Applied Field Operator.");
		lua_pushstring(L, "1 *3Vector* system size"); 
		lua_pushstring(L, "1 Applied Field object");
		return 3;
	}
	
	
	if(func == l_ap_apply)
	{
		lua_pushstring(L, "Applies a field on to a *SpinSystem*");
		lua_pushstring(L, "1 *SpinSystem*: This spin system will receive the field");
		lua_pushstring(L, "");
		return 3;
	}
	
	if(func == l_ap_set)
	{
		lua_pushstring(L, "Set the direction and strength of the Applied Field");
		lua_pushstring(L, "1 *3Vector*: The *3Vector* defines the strength and direction of the applied field");
		lua_pushstring(L, "");
		return 3;
	}
	
	
	if(func == l_ap_get)
	{
		lua_pushstring(L, "Get the direction and strength of the Applied Field");
		lua_pushstring(L, "");
		lua_pushstring(L, "1 Table: The table has the x, y and z of the field stored at indices 1, 2 and 3.");
		return 3;
	}
	
	if(func == l_ap_member)
	{
		lua_pushstring(L, "Determine if a lattice site is a member of the Operation.");
		lua_pushstring(L, "3 Integers: lattics site x, y, z.");
		lua_pushstring(L, "1 Boolean: True if x, y, z is part of the Operation, False otherwise.");
		return 3;
	}
	
	return 0;
}


void registerAppliedField(lua_State* L)
{
	static const struct luaL_reg methods [] = { //methods
		{"__gc",         l_ap_gc},
		{"__tostring",   l_ap_tostring},
		{"apply",        l_ap_apply},
		{"set",          l_ap_set},
		{"get",          l_ap_get},
		{"member",       l_ap_member},
		{NULL, NULL}
	};
		
	luaL_newmetatable(L, "MERCER.appliedfield");
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);  /* pushes the metatable */
	lua_settable(L, -3);  /* metatable.__index = metatable */
	luaL_register(L, NULL, methods);
	lua_pop(L,1); //metatable is registered
		
	static const struct luaL_reg functions [] = {
		{"new",                 l_ap_new},
		{"help",                l_ap_help},
		{"metatable",           l_ap_mt},
		{NULL, NULL}
	};
		
	luaL_register(L, "AppliedField", functions);
	lua_pop(L,1);	
}

