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

#include "luacommon.h"
#include "magnetostaticssupport.h"
#include "magnetostatics/gamma_ab_v.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

///periodic XY
static void getGAB(
	const double* ABC, 
	const double* prism, /* 3 vector */
	const int nA, const int nB, const int nC,  //width, depth, layers 
	const int ix, const int iy, const int iz, 
	const int gmax, 
	double* XX, double* XY, double* XZ,
	double* YY, double* YZ, double* ZZ)
{
	int smax;
	int i, j, c;
	double r2;
	double ir,ir3,ir5;
	double rx, ry, rz;
	int iL;

	if(nA < nB)
		iL = nA;
	else
		iL = nB;

	smax = gmax/iL;

	const double d1 = prism[0];
	const double l1 = prism[1];
	const double w1 = prism[2];
	
	const double d2 = prism[0];
	const double l2 = prism[1];
	const double w2 = prism[2];
	
	double* gXX = new double[smax*2+1];
	double* gXY = new double[smax*2+1];
	double* gXZ = new double[smax*2+1];
	double* gYY = new double[smax*2+1];
	double* gYZ = new double[smax*2+1];
	double* gZZ = new double[smax*2+1];

	for(c=0; c<smax*2+1; c++)
	{
		gXX[c] = 0;
		gXY[c] = 0;
		gXZ[c] = 0;
		gYY[c] = 0;
		gYZ[c] = 0;
		gZZ[c] = 0;
	}

	/* sum over periodic lattices */
	for(j=-smax; j<= smax; j++)
		for(i=-smax, c=0; i<= smax; i++, c++)
		{
			rx = ((double)i*nA+ix)*ABC[0] + ((double)j*nB+iy)*ABC[3] + ((double)iz)*ABC[6];
			ry = ((double)i*nA+ix)*ABC[1] + ((double)j*nB+iy)*ABC[4] + ((double)iz)*ABC[7];
			rz = ((double)i*nA+ix)*ABC[2] + ((double)j*nB+iy)*ABC[5] + ((double)iz)*ABC[8];
			r2 = rx*rx + ry*ry + rz*rz;
			//if(r2 != 0)
			{
				gXX[c] += gamma_xx_v(rx, ry, rz, d1, l1, w1, d2, l2, w2);
				gXY[c] += gamma_xy_v(rx, ry, rz, d1, l1, w1, d2, l2, w2);
				gXZ[c] += gamma_xz_v(rx, ry, rz, d1, l1, w1, d2, l2, w2);

				gYY[c] += gamma_yy_v(rx, ry, rz, d1, l1, w1, d2, l2, w2);
				gYZ[c] += gamma_yz_v(rx, ry, rz, d1, l1, w1, d2, l2, w2);

				gZZ[c] += gamma_zz_v(rx, ry, rz, d1, l1, w1, d2, l2, w2);
			}

#warning This is a hack to fix self terms. Eventually this will be in the numerical code.
			if(r2 < 1E-10)
			{
			  gXX[c] *= 0.5;
			  gXY[c] *= 0.5;
			  gXZ[c] *= 0.5;

			  gYY[c] *= 0.5;
			  gYZ[c] *= 0.5;

			  gZZ[c] *= 0.5;
			}
		}
	
	*XX = 0;
	for(c=0; c<smax*2+1; c++)
		*XX += gXX[c];

	*XY = 0;
	for(c=0; c<smax*2+1; c++)
		*XY += gXY[c];

	*XZ = 0;
	for(c=0; c<smax*2+1; c++)
		*XZ += gXZ[c];

	*YY = 0;
	for(c=0; c<smax*2+1; c++)
		*YY += gYY[c];

	*YZ = 0;
	for(c=0; c<smax*2+1; c++)
		*YZ += gYZ[c];

	*ZZ = 0;
	for(c=0; c<smax*2+1; c++)
		*ZZ += gZZ[c];

	
	delete [] gXX;
	delete [] gXY;
	delete [] gXZ;

	delete [] gYY;
	delete [] gYZ;

	delete [] gZZ;
}//end function WAB



static void _writemat(FILE* f, const char* name, int zoffset, const double* M, int nx, int ny)
{
	fprintf(f, "\n");
	fprintf(f, "%s[%i] = {\n", name, zoffset);

	for(int j=0; j<ny; j++)
	{
		fprintf(f, "    {");
		for(int i=0; i<nx; i++)
		{
			fprintf(f, "%-12g%s", M[i+j*nx], i==(nx-1)?"}":", ");
		}
		fprintf(f, "%c\n", j==(ny-1)?' ':',');
	}
	fprintf(f, "}\n");
	fprintf(f, "\n");
}

static bool magnetostatics_write_matrix(const char* filename,
	const double* ABC,
	const double* prism,
	const int nx, const int ny, const int nz,  //width, depth, layers 
	const int gmax, 
	const double* XX, const double* XY, const double* XZ,
	const double* YY, const double* YZ, const double* ZZ)
{
	FILE* f = fopen(filename, "w");
	if(!f)
		return false;
	
	fprintf(f, "-- This file contains magnetostatic interaction matrices\n");
	fprintf(f, "\n");
	fprintf(f, "gmax = %i\n", gmax);
	fprintf(f, "nx, ny, nz = %i, %i, %i\n", nx, ny, nz);
	fprintf(f, "cellDimensions = {%g, %g, %g}\n", prism[0], prism[1], prism[2]);
	fprintf(f, "ABC = {{%g, %g, %g}, --unit cell\n       {%g, %g, %g},\n       {%g, %g, %g}}\n\n", 
		ABC[0], ABC[1], ABC[2],
		ABC[3], ABC[4], ABC[5],
		ABC[6], ABC[7], ABC[8]);
	fprintf(f, "XX={} XY={} XZ={} YY={} YZ={} ZZ={}\n");
	
	int c = 0;
	for(int zoffset=-nz+1; zoffset<nz; zoffset++)
	{
		_writemat(f, "XX", zoffset, &XX[c*nx*ny], nx, ny);
		_writemat(f, "XY", zoffset, &XY[c*nx*ny], nx, ny);
		_writemat(f, "XZ", zoffset, &XZ[c*nx*ny], nx, ny);
		
		_writemat(f, "YY", zoffset, &YY[c*nx*ny], nx, ny);
		_writemat(f, "YZ", zoffset, &YZ[c*nx*ny], nx, ny);
		
		_writemat(f, "ZZ", zoffset, &ZZ[c*nx*ny], nx, ny);
		
		c++;
	}
	
	fclose(f);
	return true;
}

static void next_magnetostaticsfilename(const char* current, char* next, int len, const int nx, const int ny)
{
	if(current && current[0])
	{
		int x, y, v;
		sscanf(current, "GAB_%ix%i.%i.lua", &x, &y, &v);
		snprintf(next, len, "GAB_%ix%i.%i.lua", x, y, v+1);
	}
	else
	{
		snprintf(next, len, "GAB_%ix%i.%i.lua", nx, ny, 1);
	}
}

static int file_exists(const char* filename)
{
	FILE* f = fopen(filename, "r");
	if(f)
	{
		fclose(f);
		return 1;
	}
	return 0;
}


static bool valueMatch(lua_State* L, const char* name, int value)
{
	lua_getglobal(L, name);
	if(!lua_isnumber(L, -1))
	{
		lua_pop(L, 1);
		return false;
	}
	
	int v = lua_tointeger(L, -1);
	lua_pop(L, 1);
	return v == value;
}

static bool checkTable(lua_State* L, const double* v3)
{
	if(!lua_istable(L, -1))
		return false;
	for(int i=0; i<3; i++)
	{
		lua_pushinteger(L, i+1);
		lua_gettable(L, -2);
		if(!lua_isnumber(L, -1) || (lua_tonumber(L, -1) != v3[i]))
		{
			lua_pop(L, 1);
			return false;
		}
		lua_pop(L, 1);
	}
	return true;
}

static bool magnetostaticsParamsMatch(
	const char* filename,
	const int nx, const int ny, const int nz,
	const int gmax, const double* ABC, const double* prism)
{
	lua_State *L = lua_open();
	luaL_openlibs(L);
	
	if(luaL_dofile(L, filename))
	{
		fprintf(stderr, "%s\n", lua_tostring(L, -1));
		lua_close(L);
		return false;
	}
	
	const char* nn[4] = {"nx", "ny", "nz", "gmax"};
	int  nv[4]; 
	nv[0] = nx; nv[1] = ny; 
	nv[2] = nz; nv[3] = gmax;

	for(int i=0; i<4; i++)
	{
		if(!valueMatch(L, nn[i], nv[i]))
		{
			lua_close(L);
			return false;
		}
	}
	
	lua_getglobal(L, "cellDimensions");
	if(!checkTable(L, prism))
	{
		lua_close(L);
		return false;
	}
	
	//see if unit cell matches
	lua_getglobal(L, "ABC");
	if(!lua_istable(L, -1))
	{
		lua_close(L);
		return false;
	}
	
	for(int i=0; i<3; i++)
	{
		lua_pushinteger(L, i+1);
		lua_gettable(L, -2); // get A/B/C
		if(!checkTable(L, ABC+3*i))
		{
			lua_close(L);
			return false;
		}
		lua_pop(L, 1);
	}
	lua_close(L);
	return true;
}

static void loadXYZ(
	const char* filename,
	const int nx, const int ny, const int nz,
	double* XX, double* XY, double* XZ,
	double* YY, double* YZ, double* ZZ)
{
	lua_State* L = lua_open();
	luaL_openlibs(L);
	
	const char* vars[6] = {"XX", "XY", "XZ", "YY", "YZ", "ZZ"};
	double* arrs[6];
	arrs[0] = XX;
	arrs[1] = XY;
	arrs[2] = XZ;
	arrs[3] = YY;
	arrs[4] = YZ;
	arrs[5] = ZZ;
	
	if(luaL_dofile(L, filename))
	{
		fprintf(stderr, "%s\n", lua_tostring(L, -1));
		lua_close(L);
		return;
	}
	
	const int nxyz = nx*ny*nz;
	for(int a=0; a<6; a++)
	{
		int c = 0;
		//int p = 0;
		lua_getglobal(L, vars[a]); //XX
		for(int k=-nz+1; k<nz; k++, c++)
		{
			lua_pushinteger(L, k); //XX 0
			lua_gettable(L, -2);   //XX XX[0]
			for(int j=0; j<ny; j++)
			{
				lua_pushinteger(L, j+1); // XX XX[0] 1
				lua_gettable(L, -2);     // XX XX[0] XX[0,1]
				for(int i=0; i<nx; i++)
				{
					lua_pushinteger(L, i+1); // XX XX[0] XX[0,1] 2
					lua_gettable(L, -2);     // XX XX[0] XX[0,1] XX[0,1,2]
					arrs[a][c*nxyz + i + j*nx] = lua_tonumber(L, -1);
					lua_pop(L, 1); // XX XX[0] XX[0,1]
				}
				lua_pop(L, 1); // XX XX[0]
			}
			lua_pop(L, 1); // XX
			c++;
		}
		lua_pop(L, 1); //
	}
	
	lua_close(L);
}

void magnetostaticsLoad(
	const int nx, const int ny, const int nz,
	const int gmax, const double* ABC,
	const double* prism, /* 3 vector */
	double* XX, double* XY, double* XZ,
	double* YY, double* YZ, double* ZZ)
{
	char fn[64] = "";
	
	while(true)
	{
		next_magnetostaticsfilename(fn, fn, 64, nx, ny);
		if(file_exists(fn))
		{
			if(magnetostaticsParamsMatch(fn, nx, ny, nz, gmax, ABC, prism))
			{
				loadXYZ(fn, 
						nx, ny, nz,
						XX, XY, XZ,
						YY, YZ, ZZ);
				return;
			}
		}
		else
		{
			int c = 0;
			for(int k=-nz+1; k<nz; k++)
			{
				for(int j=0; j<ny; j++)
				{
					for(int i=0; i<nx; i++)
					{
						getGAB(ABC,
							prism,
							nx, ny, nz,
							i, j, k,
							gmax,
							XX+c, XY+c, XZ+c,
							YY+c, YZ+c, ZZ+c);
						c++;
					}
				}
			}

			magnetostatics_write_matrix(fn,
				ABC, prism,
				nx, ny, nz,
				gmax,
				XX, XY, XZ,
				YY, YZ, ZZ);
			return;
		}
	}
}


