/******************************************************************************
* Copyright (C) 2008-2012 Jason Mercer.  All rights reserved.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/



#ifdef WIN32
 #ifdef SCRIPTS_EXPORTS
  #define SCRIPTS_API __declspec(dllexport)
 #else
  #define SCRIPTS_API __declspec(dllimport)
 #endif
#else
 #define SCRIPTS_API 
#endif


extern "C" {
        #include <lua.h>
        #include <lualib.h>
        #include <lauxlib.h>

typedef struct ss
{
	const char* a;
	const char* b;
} ss;

#include "CGS.h"
#include "RungeKutta.h"
#include "POVRay.h"
#include "PredictorCorrector.h"
#include "ColorMap.h"
#include "AdaptiveTimeStep.h"
#include "MakeMacro.h"
#include "PathDynamics.h"

SCRIPTS_API int lib_register(lua_State* L)
{
	static const ss data[] = {
		{__CGS_filename(),  __CGS()},
		{__POVRay_filename(),  __POVRay()},
		{__RungeKutta_filename(),  __RungeKutta()},
		{__PredictorCorrector_filename(), __PredictorCorrector()},
		{__AdaptiveTimeStep_filename(),  __AdaptiveTimeStep()},
		{__ColorMap_filename(),  __ColorMap()},
		{__MakeMacro_filename(),  __MakeMacro()},
		{__PathDynamics_filename(), __PathDynamics()},
		{0,0}
	};
	
	for(int i=0; data[i].a; i++)
	{
		lua_getglobal(L, "dofile_add");
		lua_pushstring(L, data[i].a);
		lua_pushstring(L, data[i].b);
		lua_call(L, 2, 0);
	}
	
	return 0;
}

#include "info.h"
SCRIPTS_API int lib_version(lua_State* L)
{
	return __revi;
}


SCRIPTS_API const char* lib_name(lua_State* L)
{
#if defined NDEBUG || defined __OPTIMIZE__
	return "Scripts";
#else
	return "Scripts-Debug";
#endif
}

SCRIPTS_API int lib_main(lua_State* L)
{
	return 0;
}

}
