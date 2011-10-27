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

#ifndef SPINOPERATIONEXCHANGE
#define SPINOPERATIONEXCHANGE

#include "spinoperation.h"

class Exchange : public SpinOperation
{
public:
	Exchange(int nx, int ny, int nz);
	virtual ~Exchange();
	
	bool apply(SpinSystem* ss);

	void addPath(int site1, int site2, double strength);

	virtual void encode(buffer* b);
	virtual int  decode(buffer* b);
	void opt();

	typedef struct sss
	{
		int fromsite;
		int tosite;
		double strength;
	} sss;

	
private:
	void deinit();
	void init();
	void sync();
	bool new_host;
	
	int size;
	int num;
	sss* pathways;
	
	double* d_strength;
	double* h_strength;
	int* d_fromsite;
	int* h_fromsite;
	int maxFromSites;
	
// 	int* fromsite;
// 	int* tosite;
// 	double* strength;
};

Exchange* checkExchange(lua_State* L, int idx);
void registerExchange(lua_State* L);
void lua_pushExchange(lua_State* L, Exchange* ex);

#endif
