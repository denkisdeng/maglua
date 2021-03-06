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

#ifndef SPINOPERATIONANISOTROPY
#define SPINOPERATIONANISOTROPY

#include "spinoperation.h"

#ifdef WIN32
 #define strcasecmp(A,B) _stricmp(A,B)
 #define strncasecmp(A,B,C) _strnicmp(A,B,C)
 #pragma warning(disable: 4251)

 #ifdef ANISOTROPYCUDA_EXPORTS
  #define ANISOTROPYCUDA_API __declspec(dllexport)
 #else
  #define ANISOTROPYCUDA_API __declspec(dllimport)
 #endif
#else
 #define ANISOTROPYCUDA_API 
#endif

class ANISOTROPYCUDA_API Anisotropy : public SpinOperation
{
public:
	Anisotropy(int nx=32, int ny=32, int nz=1);
	virtual ~Anisotropy();
	
	LINEAGE2("Anisotropy", "SpinOperation")
	static const luaL_Reg* luaMethods();
	virtual int luaInit(lua_State* L);
	virtual void push(lua_State* L);
	static int help(lua_State* L);
	
	bool apply(SpinSystem* ss);
	bool applyToSum(SpinSystem* ss);
	void addAnisotropy(int site, double nx, double ny, double nz, double K);
    bool getAnisotropy(int site, double& nx, double& ny, double& nz, double& K);
	
	float* d_nx;
	float* d_ny;
	float* d_nz;
	float* d_k;
	
	float* h_nx;
	float* h_ny;
	float* h_nz;
	float* h_k;
	
	float* d_LUT;
	char*   d_idx;
	
	virtual void encode(buffer* b);
	virtual int  decode(buffer* b);
	
	bool new_host; // if host data is most recent

	void init();
	void deinit();
	
	bool make_uncompressed();
	bool make_compressed();
	bool make_host();
	
	void delete_uncompressed();
	void delete_compressed();
	void delete_host();
private:
	int unique; //number of LUT entries
};


#endif
