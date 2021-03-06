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

#ifndef SPINOPERATIONDIPOLECUDA
#define SPINOPERATIONDIPOLECUDA

#include "spinoperation.h"
#include "../longrange_cuda/spinoperationlongrange.h"


#ifdef WIN32
 #define strcasecmp(A,B) _stricmp(A,B)
 #define strncasecmp(A,B,C) _strnicmp(A,B,C)
 #pragma warning(disable: 4251)

 #ifdef DIPOLECUDA_EXPORTS
  #define DIPOLECUDA_API __declspec(dllexport)
 #else
  #define DIPOLECUDA_API __declspec(dllimport)
 #endif
#else
 #define DIPOLECUDA_API 
#endif

using namespace std;

class DIPOLECUDA_API DipoleCuda : public LongRangeCuda
{
public:
	DipoleCuda(int nx=32, int ny=32, int nz=1);
	virtual ~DipoleCuda();
	
	virtual void encode(buffer* b);
	virtual int  decode(buffer* b);

	void loadMatrixFunction(double* XX, double* XY, double* XZ, double* YY, double* YZ, double* ZZ);

	LINEAGE3("Dipole", "LongRange", "SpinOperation")
	static const luaL_Reg* luaMethods();
	virtual int luaInit(lua_State* L);
	virtual void push(lua_State* L);
	static int help(lua_State* L);
};

#endif
