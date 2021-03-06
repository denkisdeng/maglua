
#include "luabaseobject.h"

#ifdef WIN32
 #define strcasecmp(A,B) _stricmp(A,B)
 #define strncasecmp(A,B,C) _strnicmp(A,B,C)
 #pragma warning(disable: 4251)

 #ifdef ARRAY_EXPORTS
  #define ARRAY_API __declspec(dllexport)
 #else
  #define ARRAY_API __declspec(dllimport)
 #endif
#else
 #define ARRAY_API 
#endif

#include <complex>
#ifndef COMPLEX_TYPES
#define COMPLEX_TYPES
using namespace std;
typedef std::complex<double> doubleComplex; //cpu version
typedef std::complex<float>   floatComplex;
#endif



#ifndef LUA_TYPE_TRANSLATE
#define LUA_TYPE_TRANSLATE

// These static methods define Lua and buffer interaction for different data types

template<typename T>
class ARRAY_API luaT
{
public:
	static int elements() {return 0;}
	static int push(lua_State* L, const T& v){return elements();}
	static T to(lua_State* L, int idx){return 0;}
	static int is(lua_State* L, int idx){return 0;}
	static	void encode(const T& v, buffer* b){}
	static	T decode(buffer* b){return 0;}
	static	T zero() {return 0;}
	static	T one(double scale=1) {return scale*1;}
	static	T neg_one() {one(-1);}
	static bool lt(const T& a, const T& b) {return false;}
};

template<>
class ARRAY_API luaT<double>{
public:
	static int elements() {return 1;}
	static int push(lua_State* L, const double& v){lua_pushnumber(L, v);return elements();}
	static double to(lua_State* L, int idx){return lua_tonumber(L, idx);}
	static int is(lua_State* L, int idx){return lua_isnumber(L, idx);}
	static	void encode(const double& v, buffer* b){	encodeDouble(v, b);	}
	static	double decode(buffer* b){return decodeDouble(b);}
	static	double zero() {return 0;}
	static	double one(double scale=1) {return scale*1;}
	static	double neg_one() {return one(-1);}
	static bool lt(const double& a, const double& b) {return a<b;}
};

template<>
class ARRAY_API luaT<float>{
public:
	static int elements() {return 1;}
	static int push(lua_State* L, const float& v){lua_pushnumber(L, v);return elements();}
	static float to(lua_State* L, int idx){return (float)lua_tonumber(L, idx);}
	static int is(lua_State* L, int idx){return lua_isnumber(L, idx);}
	static	void encode(const float& v, buffer* b){	encodeDouble(v, b);	}
	static	float decode(buffer* b){return (float)decodeDouble(b);}
	static	float zero() {return 0;}
	static	float one(double scale=1) {return scale*1;}
	static	float neg_one() {return one(-1);}
	static bool lt(const float& a, const float& b) {return a<b;}
};

template<>
class ARRAY_API luaT<int>{
public:
	static int elements() {return 1;}
	static int push(lua_State* L, const int& v){lua_pushnumber(L, v);return elements();}
	static int to(lua_State* L, int idx){return lua_tointeger(L, idx);}
	static int is(lua_State* L, int idx){return lua_isnumber(L, idx);}
	static	void encode(const int& v, buffer* b){	encodeInteger(v, b);	}
	static	int decode(buffer* b){return decodeInteger(b);}
	static	int zero() {return 0;}
	static	int one(double scale=1) {return 1*scale;}
	static	int neg_one() {return one(-1);}
	static bool lt(const int& a, const int& b) {return a<b;}
};

template<>
class ARRAY_API luaT<doubleComplex>{
public:
	static int elements() {return 2;}
	static int push(lua_State* L, const doubleComplex& v){lua_pushnumber(L, v.real());lua_pushnumber(L, v.imag());return elements();}
	static doubleComplex to(lua_State* L, int idx){double a = lua_tonumber(L, idx); double b = lua_tonumber(L, idx+1); return doubleComplex(a,b);}
	static int is(lua_State* L, int idx){return lua_isnumber(L, idx) && lua_isnumber(L, idx+1);}
	static	void encode(const doubleComplex& v, buffer* b){	encodeDouble(v.real(), b);encodeDouble(v.imag(), b);}
	static	doubleComplex decode(buffer* b){return doubleComplex(decodeDouble(b), decodeDouble(b));}
	static	doubleComplex zero() {return doubleComplex(0,0);}
	static	doubleComplex one(double scale=1) {return doubleComplex(scale,0);}
	static	doubleComplex neg_one() {return one(-1);}
	static bool lt(const doubleComplex& a, const doubleComplex& b) {if(a.real() < b.real()) return true; if(a.real() == b.real() && a.imag() < b.imag()) return true; return false;}
};

template<>
class ARRAY_API luaT<floatComplex>{
public:
	static int elements() {return 2;}
	static int push(lua_State* L, const floatComplex& v){lua_pushnumber(L, v.real());lua_pushnumber(L, v.imag());return elements();}
	static floatComplex to(lua_State* L, int idx){float a = (float)lua_tonumber(L, idx); float b = (float)lua_tonumber(L, idx+1); return floatComplex(a,b);}
	static int is(lua_State* L, int idx){return lua_isnumber(L, idx) && lua_isnumber(L, idx+1);}
	static	void encode(const floatComplex& v, buffer* b){	encodeDouble(v.real(), b);encodeDouble(v.imag(), b);}
	static	floatComplex decode(buffer* b){return floatComplex((float)decodeDouble(b), (float)decodeDouble(b));}
	static	floatComplex zero() {return floatComplex(0,0);}
	static	floatComplex one(double scale=1) {return floatComplex(scale,0);}
	static	floatComplex neg_one() {return one(-1);}
	static bool lt(const floatComplex& a, const floatComplex& b) {if(a.real() < b.real()) return true; if(a.real() == b.real() && a.imag() < b.imag()) return true; return false;}
};

#endif

