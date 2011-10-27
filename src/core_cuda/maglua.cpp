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

#include "luacommon.h"

extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
        
CORECUDA_API int lib_register(lua_State* L);
CORECUDA_API int lib_deps(lua_State* L);
CORECUDA_API int lib_version(lua_State* L);
CORECUDA_API const char* lib_name(lua_State* L);
CORECUDA_API void lib_main(lua_State* L, int argc, char** argv);
}

#include "info.h"
#include "spinsystem.h"
#include "spinoperation.h"
	
CORECUDA_API int lib_register(lua_State* L)
{
	registerSpinSystem(L);

	return 0;
}

CORECUDA_API int lib_version(lua_State* L)
{
	return __revi;
}

CORECUDA_API const char* lib_name(lua_State* L)
{
	return "Core-Cuda";
}

CORECUDA_API void lib_main(lua_State* L, int argc, char** argv)
{
}
