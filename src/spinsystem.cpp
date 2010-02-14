#include "spinsystem.h"
#include "spinoperation.h"
#include <iostream>
#include <math.h>
#include <strings.h>

using namespace std;
#define CLAMP(x, m) ((x<0)?0:(x>m?m:x))

SpinSystem::SpinSystem(int NX, int NY, int NZ)
	: nx(NX), ny(NY), nz(NZ), refcount(0), nslots(NSLOTS), time(0),
	  x(0), y(0), z(0), ms(0), Encodable(ENCODE_SPINSYSTEM)
{
	init();
}

SpinSystem::~SpinSystem()
{
	deinit();
}

void SpinSystem::deinit()
{
	if(x)
	{
		delete [] x;
		delete [] y;
		delete [] z;
		delete [] ms;

		
		for(int i=0; i<nslots; i++)
		{
			delete [] hx[i];
			delete [] hy[i];
			delete [] hz[i];
		}

		delete [] hx;
		delete [] hy;
		delete [] hz;

		delete [] rx;
		delete [] ry;
		delete [] rz;

		delete [] qx;
		delete [] qy;
		delete [] qz;
		
		
		fftw_destroy_plan(r2q);
	}
}


void SpinSystem::init()
{
	nxyz = nx * ny * nz;

	x = new double[nxyz];
	y = new double[nxyz];
	z = new double[nxyz];
	ms= new double[nxyz];
	
	for(int i=0; i<nxyz; i++)
		set(i, 0, 0, 0);

	hx = new double* [nslots];
	hy = new double* [nslots];
	hz = new double* [nslots];
	
	for(int i=0; i<nslots; i++)
	{
		hx[i] = new double[nxyz];
		hy[i] = new double[nxyz];
		hz[i] = new double[nxyz];
		
		for(int j=0; j<nxyz; j++)
		{
			hx[i][j] = 0;
			hy[i][j] = 0;
			hz[i][j] = 0;
		}
	}

	rx = new complex<double>[nxyz];
	ry = new complex<double>[nxyz];
	rz = new complex<double>[nxyz];

	qx = new complex<double>[nxyz];
	qy = new complex<double>[nxyz];
	qz = new complex<double>[nxyz];
	
	fftw_iodim dims[2];
	dims[0].n = nx;
	dims[0].is= 1;
	dims[0].os= 1;
	dims[1].n = ny;
	dims[1].is= nx;
	dims[1].os= nx;

	r2q = fftw_plan_guru_dft(2, dims, 0, dims,
				reinterpret_cast<fftw_complex*>(rx),
				reinterpret_cast<fftw_complex*>(qx),
				FFTW_FORWARD, FFTW_PATIENT);
}

//   void encodeBuffer(const void* s, int len, buffer* b);
//   void encodeDouble(const double d, buffer* b);
//   void encodeInteger(const int i, buffer* b);
//    int decodeInteger(const char* buf, int* pos);
// double decodeDouble(const char* buf, int* pos);


void SpinSystem::encode(buffer* b) const
{
	encodeInteger(nx, b);
	encodeInteger(ny, b);
	encodeInteger(nz, b);

	for(int i=0; i<nxyz; i++)
	{
		encodeDouble(x[i], b);
		encodeDouble(y[i], b);
		encodeDouble(z[i], b);
		
		encodeDouble(qx[i].real(), b);
		encodeDouble(qx[i].imag(), b);

		encodeDouble(qy[i].real(), b);
		encodeDouble(qy[i].imag(), b);

		encodeDouble(qz[i].real(), b);
		encodeDouble(qz[i].imag(), b);


		for(int j=0; j<nslots; j++)
		{
			encodeDouble(hx[j][i], b);
			encodeDouble(hy[j][i], b);
			encodeDouble(hz[j][i], b);
		}
	}
}

int  SpinSystem::decode(buffer* b)
{
	deinit();
	nx = decodeInteger(b);
	ny = decodeInteger(b);
	nz = decodeInteger(b);
	init();

	for(int i=0; i<nxyz; i++)
	{
		x[i] = decodeDouble(b);
		y[i] = decodeDouble(b);
		z[i] = decodeDouble(b);
		
		qx[i] = complex<double>(decodeDouble(b), decodeDouble(b));
		qy[i] = complex<double>(decodeDouble(b), decodeDouble(b));
		qz[i] = complex<double>(decodeDouble(b), decodeDouble(b));
				
		for(int j=0; j<nslots; j++)
		{
			hx[j][i] = decodeDouble(b);
			hy[j][i] = decodeDouble(b);
			hz[j][i] = decodeDouble(b);
		}
	}
}


void SpinSystem::sumFields()
{
	for(int j=0; j<nxyz; j++)
	{
		hx[SUM_SLOT][j] = hx[1][j];
		hy[SUM_SLOT][j] = hy[1][j];
		hz[SUM_SLOT][j] = hz[1][j];
	}

	for(int i=2; i<NSLOTS; i++)
	{
		for(int j=0; j<nxyz; j++)
		{
			hx[SUM_SLOT][j] += hx[i][j];
			hy[SUM_SLOT][j] += hy[i][j];
			hz[SUM_SLOT][j] += hz[i][j];
		}
	}
}

int SpinSystem::getSlot(const char* name)
{
	if(strcasecmp(name, "exchange") == 0)
		return EXCHANGE_SLOT;
	if(strcasecmp(name, "anisotropy") == 0)
		return ANISOTROPY_SLOT;
	if(strcasecmp(name, "thermal") == 0 || strcasecmp(name, "stochastic") == 0 || strcasecmp(name, "temperature") == 0)
		return THERMAL_SLOT;
	if(strcasecmp(name, "dipole") == 0)
		return DIPOLE_SLOT;
	if(strcasecmp(name, "applied") == 0 || strcasecmp(name, "zeeman") == 0)
		return APPLIEDFIELD_SLOT;
	if(strcasecmp(name, "total") == 0 || strcasecmp(name, "sum") == 0)
		return SUM_SLOT;
	return -1;
}

void SpinSystem::fft()
{
	printf("FFT\n");
	for(int i=0; i<nxyz; i++) rx[i] = x[i];
	for(int i=0; i<nxyz; i++) ry[i] = y[i];
	for(int i=0; i<nxyz; i++) rz[i] = z[i];
	
	for(int k=0; k<nz; k++)
	{
		fftw_execute_dft(r2q, 
			reinterpret_cast<fftw_complex*>(&rx[k*nx*ny]),
			reinterpret_cast<fftw_complex*>(&qx[k*nx*ny]));
		fftw_execute_dft(r2q, 
			reinterpret_cast<fftw_complex*>(&ry[k*nx*ny]),
			reinterpret_cast<fftw_complex*>(&qy[k*nx*ny]));
		fftw_execute_dft(r2q, 
			reinterpret_cast<fftw_complex*>(&rz[k*nx*ny]),
			reinterpret_cast<fftw_complex*>(&qz[k*nx*ny]));
	}

}


void SpinSystem::zeroFields()
{
	for(int i=0; i<NSLOTS; i++)
	{
		for(int j=0; j<nxyz; j++)
		{
			hx[i][j] = 0;
			hy[i][j] = 0;
			hz[i][j] = 0;
		}
	}
}

bool SpinSystem::member(int px, int py, int pz)
{
	if(px < 0 || py < 0 || pz < 0)
		return false;

	if(px >= nx || py >= ny || pz >= nz)
		return false;
	
	return true;
}

void  SpinSystem::set(int i, double sx, double sy, double sz)
{
	x[i] = sx;
	y[i] = sy;
	z[i] = sz;

	ms[i]= sqrt(sx*sx+sy*sy+sz*sz);
}


void SpinSystem::set(int px, int py, int pz, double sx, double sy, double sz)
{
	int i = getidx(px, py, pz);
	set(i, sx, sy, sz);
}

int  SpinSystem::getidx(int px, int py, int pz)
{
	px = CLAMP(px, nx);
	py = CLAMP(py, ny);
	pz = CLAMP(pz, nz);
	
	return px + py*nx + pz*nx*ny;
	//return px + nx * (py + ny * pz);
}

void SpinSystem::getNetMag(double* v4)
{
	v4[0] = 0; v4[1] = 0; v4[2] = 0; v4[3] = 0; 
	
	for(int i=0; i<nxyz; i++)
		v4[0] += x[i];
	for(int i=0; i<nxyz; i++)
		v4[1] += y[i];
	for(int i=0; i<nxyz; i++)
		v4[2] += z[i];

	v4[3] = sqrt(v4[0]*v4[0] + v4[1]*v4[1] + v4[2]*v4[2]);
}
















SpinSystem* checkSpinSystem(lua_State* L, int idx)
{
	SpinSystem** pp = (SpinSystem**)luaL_checkudata(L, idx, "MERCER.spinsystem");
    luaL_argcheck(L, pp != NULL, 1, "`SpinSystem' expected");
    return *pp;
}

void lua_pushSpinSystem(lua_State* L, SpinSystem* ss)
{
	ss->refcount++;
	
	SpinSystem** pp = (SpinSystem**)lua_newuserdata(L, sizeof(SpinSystem**));
	
	*pp = ss;
	luaL_getmetatable(L, "MERCER.spinsystem");
	lua_setmetatable(L, -2);
}


int l_ss_new(lua_State* L)
{
	SpinSystem* ss;
	int nx, ny, nz;
	
	nx = lua_tointeger(L, 1);
	ny = lua_tointeger(L, 2);
	nz = lua_tointeger(L, 3);

	if(lua_gettop(L) < 3 || nx < 1 || ny < 1 || nz < 1)
	{
		return luaL_error(L, "SpinSystem.new requires positive nx, ny and nz");
	}
	
	lua_pushSpinSystem(L, new SpinSystem(nx, ny, nz));
	
	return 1;
}

int l_ss_gc(lua_State* L)
{
	SpinSystem* ss = checkSpinSystem(L, 1);
	if(!ss) return 0;
	
	ss->refcount--;
	if(ss->refcount == 0)
		delete ss;
	
	return 0;
}

int l_ss_netmag(lua_State* L)
{
	SpinSystem* ss = checkSpinSystem(L, 1);
	if(!ss) return 0;
	
	double m = 1;
	if(lua_isnumber(L, 2))
		m = lua_tonumber(L, 2);

	double v4[3];
	
	ss->getNetMag(v4);
	
	lua_pushnumber(L, v4[0]*m);
	lua_pushnumber(L, v4[1]*m);
	lua_pushnumber(L, v4[2]*m);
	lua_pushnumber(L, v4[3]*m);
	
	return 4;
}

int l_ss_setspin(lua_State* L)
{
	SpinSystem* ss = checkSpinSystem(L, 1);
	if(!ss) return 0;
	
	int px = lua_tointeger(L, 2) - 1;
	int py = lua_tointeger(L, 3) - 1;
	int pz = lua_tointeger(L, 4) - 1;
	
	double sx = lua_tonumber(L, 5);
	double sy = lua_tonumber(L, 6);
	double sz = lua_tonumber(L, 7);
	
	ss->set(px, py, pz, sx, sy, sz);
	
	return 0;
}

int l_ss_getspin(lua_State* L)
{
	SpinSystem* ss = checkSpinSystem(L, 1);
	if(!ss) return 0;
	
	int px = lua_tointeger(L, 2) - 1;
	int py = lua_tointeger(L, 3) - 1;
	int pz = lua_tointeger(L, 4) - 1;
	
	if(!ss->member(px, py, pz))
		return 0;
	
	int idx = ss->getidx(px, py, pz);
	
	lua_pushnumber(L, ss->x[idx]);
	lua_pushnumber(L, ss->y[idx]);
	lua_pushnumber(L, ss->z[idx]);
	
	return 3;
}

int l_ss_nx(lua_State* L)
{
	SpinSystem* ss = checkSpinSystem(L, 1);
	if(!ss) return 0;
	lua_pushnumber(L, ss->nx);
	return 1;
}
int l_ss_ny(lua_State* L)
{
	SpinSystem* ss = checkSpinSystem(L, 1);
	if(!ss) return 0;
	lua_pushnumber(L, ss->ny);
	return 1;
}
int l_ss_nz(lua_State* L)
{
	SpinSystem* ss = checkSpinSystem(L, 1);
	if(!ss) return 0;
	lua_pushnumber(L, ss->nz);
	return 1;
}

int l_ss_sumfields(lua_State* L)
{
	SpinSystem* ss = checkSpinSystem(L, 1);
	if(!ss) return 0;
	
	ss->sumFields();
	
	return 0;
}

int l_ss_zerofields(lua_State* L)
{
	SpinSystem* ss = checkSpinSystem(L, 1);
	if(!ss) return 0;

	ss->zeroFields();

	return 0;
}

int l_ss_settime(lua_State* L)
{
	SpinSystem* ss = checkSpinSystem(L, 1);
	if(!ss) return 0;

	ss->time = lua_tonumber(L, 2);

	return 0;
}
int l_ss_gettime(lua_State* L)
{
	SpinSystem* ss = checkSpinSystem(L, 1);
	if(!ss) return 0;

	lua_pushnumber(L, ss->time);

	return 1;
}

static int l_ss_tostring(lua_State* L)
{
	SpinSystem* ss = checkSpinSystem(L, 1);
	if(!ss) return 0;
	
	lua_pushfstring(L, "SpinSystem (%dx%dx%d)", ss->nx, ss->ny, ss->nz);
	return 1;
}

int l_ss_getfield(lua_State* L)
{
	SpinSystem* ss = checkSpinSystem(L, 1);
	if(!ss) return 0;

	const char* name = lua_tostring(L, 2);
	const int x = lua_tointeger(L, 3) - 1;
	const int y = lua_tointeger(L, 4) - 1;
	const int z = lua_tointeger(L, 5) - 1;

	int idx = ss->getidx(x, y, z);
	int slot = ss->getSlot(name);

	if(slot < 0)
		return luaL_error(L, "Unknown field type`%s'", name);


	lua_pushnumber(L, ss->hx[slot][idx]);
	lua_pushnumber(L, ss->hy[slot][idx]);
	lua_pushnumber(L, ss->hz[slot][idx]);

	return 3;
}




void registerSpinSystem(lua_State* L)
{
	static const struct luaL_reg methods [] = { //methods
		{"__gc",         l_ss_gc},
		{"__tostring",   l_ss_tostring},
		{"netMag",       l_ss_netmag},
		{"setSpin",      l_ss_setspin},
		{"spin"   ,      l_ss_getspin},
		{"nx",           l_ss_nx},
		{"ny",           l_ss_ny},
		{"nz",           l_ss_nz},
		{"sumFields",    l_ss_sumfields},
		{"zeroFields",   l_ss_zerofields},
		{"setTime",      l_ss_settime},
		{"time",         l_ss_gettime},
		{"getField",     l_ss_getfield},
		{NULL, NULL}
	};
		
	luaL_newmetatable(L, "MERCER.spinsystem");
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);  /* pushes the metatable */
	lua_settable(L, -3);  /* metatable.__index = metatable */
	luaL_register(L, NULL, methods);
	lua_pop(L,1); //metatable is registered
		
	static const struct luaL_reg functions [] = {
		{"new",                 l_ss_new},
		{NULL, NULL}
	};
		
	luaL_register(L, "SpinSystem", functions);
	lua_pop(L,1);
}
