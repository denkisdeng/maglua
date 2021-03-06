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

#ifndef SPINOPERATIONAPPLIEDFIELD
#define SPINOPERATIONAPPLIEDFIELD

#include "spinoperation.h"

#ifdef WIN32
 #define strcasecmp(A,B) _stricmp(A,B)
 #define strncasecmp(A,B,C) _strnicmp(A,B,C)
 #pragma warning(disable: 4251)

 #ifdef APPLIEDFIELD_EXPORTS
  #define APPLIEDFIELD_API __declspec(dllexport)
 #else
  #define APPLIEDFIELD_API __declspec(dllimport)
 #endif
#else
 #define APPLIEDFIELD_API 
#endif


class APPLIEDFIELD_API AppliedField : public SpinOperation
{
public:
	AppliedField(int nx=32, int ny=32, int nz=1);
	virtual ~AppliedField();
	
	LINEAGE2("AppliedField", "SpinOperation")
	static const luaL_Reg* luaMethods();
	virtual int luaInit(lua_State* L);
	static int help(lua_State* L);
	
	bool apply(SpinSystem* ss);
	bool apply(SpinSystem** ss, int n);

	double B[3];

	virtual void encode(buffer* b);
	virtual int  decode(buffer* b);
};

#endif
