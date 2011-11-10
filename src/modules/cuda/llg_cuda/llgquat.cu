#include <cuda.h>
#include <cuda_runtime.h>
#include <math_functions.h>
#include <stdio.h>

#define CROSS(v, a, b) \
	v.x = a.y * b.z - a.z * b.y; \
	v.y = a.z * b.x - a.x * b.z; \
	v.z = a.x * b.y - a.y * b.x;
	
#if __CUDA_ARCH__ >= 200
#define FAST_DIV(x,y) __ddiv_rn(x,y)
#else
#define FAST_DIV(x,y) ((x)/(y))
#endif

#define CHECK \
{ \
	const cudaError_t i = cudaGetLastError();\
	if(i) \
		printf("(%s:%i) %s\n",  __FILE__, __LINE__-1, cudaGetErrorString(i));\
}


__device__ double4 qconjugate(const double4 q)
{
	double4 r;
	r.x = -1.0 * q.x;
	r.y = -1.0 * q.y;
	r.z = -1.0 * q.z;
	r.w =        q.w;
	return r;
}

__device__ double4 qmult(const double4 a, const double4 b)
{
	double4 ab;

	ab.w = a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z;
	ab.x = a.w*b.x + b.w*a.x  + a.y*b.z - a.z*b.y;
	ab.y = a.w*b.y + b.w*a.y  + a.z*b.x - a.x*b.z;
	ab.z = a.w*b.z + b.w*a.z  + a.x*b.y - a.y*b.x;

	return ab;
}

// quat mult without calculating the W component
__device__ double4 qmultXYZ(const double4 a, const double4 b)
{
	double4 ab;

	ab.x = a.w*b.x + b.w*a.x  + a.y*b.z - a.z*b.y;
	ab.y = a.w*b.y + b.w*a.y  + a.z*b.x - a.x*b.z;
	ab.z = a.w*b.z + b.w*a.z  + a.x*b.y - a.y*b.x;
	ab.w = 0;

	return ab;
}

// _1 will compute rhs and store in ws
// dS    -g           a
// -- = ---- S X (H +---S X H)
// dt   1+aa         |S|
__global__ void llg_quat_apply_1(
//	const int nx, const int ny, const int offset,
	const int nxyz,
	double alpha,
	double* sx, double* sy, double* sz, double* sms,
	double* hx, double* hy, double* hz,
	// ws1, ws2, ws3,     ws4
	double* wx, double* wy, double* wz, double* ww) 
{
	const int i = blockDim.x * blockIdx.x + threadIdx.x;
	
	if(i >= nxyz)
		return;
	
	wx[i] = sy[i]*hz[i] - sz[i]*hy[i];
	wy[i] = sz[i]*hx[i] - sx[i]*hz[i];
	wz[i] = sx[i]*hy[i] - sy[i]*hx[i];
	
	ww[i] = sms[i];
	if(ww[i] != 0)
	{
		ww[i] = FAST_DIV(alpha, ww[i]);
	}
	wx[i] *= ww[i];
	wy[i] *= ww[i];
	wz[i] *= ww[i];
	
	wx[i] += hx[i];
	wy[i] += hy[i];
	wz[i] += hz[i];

	ww[i] = sqrt(wx[i]*wx[i] + wy[i]*wy[i] + wz[i]*wz[i]);
	
	//(wx, wy, wz) = (a / |S|) S x H
	// ww = | (wx, wy, wz) |
}

// _2 will compute the rest
// dS    -g           a
// -- = ---- S X (H +---S X H)
// dt   1+aa         |S|
// the rhs vec and len is in ws
__global__ void llg_quat_apply_2(
//	const int nx, const int ny, const int offset,
	const int nxyz,
	double* ssx, double* ssy, double* ssz, // src
	double* wx, double* wy, double* wz, double* ww,
	double alpha, double gadt)
{
	const int i = blockDim.x * blockIdx.x + threadIdx.x;
	
	if(i >= nxyz)
		return;

	if(ww[i] == 0) // dst = src  in _3
	{
		return;
	}

// dS    -g           a
// -- = ---- S X (H +---S X H)
// dt   1+aa         |S|
	double4 qVec;
	qVec.x = ssx[i]; 
	qVec.y = ssy[i];
	qVec.z = ssz[i];
	qVec.w = 0;
			
	const double theta = ww[i] * gadt;

	double cost, sint;
	sincos(theta, &sint, &cost);
	const double ihlen = FAST_DIV(1, ww[i]);

	double4 qRot;
	qRot.x = sint * wx[i] * ihlen; 
	qRot.y = sint * wy[i] * ihlen;
	qRot.z = sint * wz[i] * ihlen;
	qRot.w = cost;

	//this is the rotation: qRes = qRot qVec qRot*
	double4 qRes = qmultXYZ(qmult(qRot, qVec), qconjugate(qRot));

	wx[i] = qRes.x;
	wy[i] = qRes.y;
	wz[i] = qRes.z;
	
	// (wx, wy, wz) = (qRot qVec) conj(qRot)
	// ww = unimportant
}

// _3 normalize
__global__ void llg_quat_apply_3(
//	const int nx, const int ny, const int offset,
	const int nxyz,
	double*  wx, double*  wy, double*  wz, double* ww,
	double* ssx, double* ssy, double* ssz, double* sms,
 	double* dsx, double* dsy, double* dsz, double* dms)
{
	const int i = blockDim.x * blockIdx.x + threadIdx.x;
	
	if(i >= nxyz)
		return;

	dms[i] = sms[i];
	
	if(ww[i] == 0)
	{
		dsx[i] = ssx[i];
		dsy[i] = ssy[i];
		dsz[i] = ssz[i];
		return;
	}
	
	//using ww as temp var. saves a reg?
	ww[i] = sqrt(wx[i]*wx[i] + wy[i]*wy[i] + wz[i]*wz[i]);
	
	if(ww[i] == 0)
	{
		dsx[i] = ssx[i];
		dsy[i] = ssy[i];
		dsz[i] = ssz[i];
		return;
	}
	
	
	ww[i] = sms[i] / ww[i];

	// (wx, wy, wz) = (qRot qVec) conj(qRot)
	// ww = | S | / |(wx, wy, wz)|
}

// _4
__global__ void llg_quat_apply_4(
//	const int nx, const int ny, const int offset,
	const int nxyz,
	double*  wx, double*  wy, double*  wz, double* ww,
 	double* dsx, double* dsy, double* dsz)
{
	const int i = blockDim.x * blockIdx.x + threadIdx.x;
	
	if(i >= nxyz)
		return;

	if(ww[i] == 0)
	{
		return;
	}
	
	dsx[i] = wx[i] * ww[i];
	dsy[i] = wy[i] * ww[i];
	dsz[i] = wz[i] * ww[i];
}



void cuda_llg_quat_apply(const int nx, const int ny, const int nz,
	double* dsx, double* dsy, double* dsz, double* dms, //dest
	double* ssx, double* ssy, double* ssz, double* sms, // src
	double* hx, double* hy, double* hz,
	double* ws1, double* ws2, double* ws3, double* ws4,
	const double alpha, const double dt, const double gamma)
{
	// the 0.5 is for the quaternions
	double gadt = (0.5 * gamma * dt) / (1.0 + alpha * alpha);

	const int nxyz = nx*ny*nz;
	const int threads = 512;
	const int blocks = nxyz / threads + 1;

	llg_quat_apply_1<<<blocks, threads>>>(nxyz,
					alpha,
					ssx, ssy, ssz, sms,
						hx,  hy,  hz,
					ws1, ws2, ws3, ws4);
	CHECK
	
	llg_quat_apply_2<<<blocks, threads>>>(nxyz,
					ssx, ssy, ssz,
					ws1, ws2, ws3, ws4,
					alpha, gadt);
	CHECK

	llg_quat_apply_3<<<blocks, threads>>>(nxyz,
					ws1, ws2, ws3, ws4,
					ssx, ssy, ssz, sms,
					dsx, dsy, dsz, dms);
	CHECK
	
	llg_quat_apply_4<<<blocks, threads>>>(nxyz,
					ws1, ws2, ws3, ws4,
					dsx, dsy, dsz);
	CHECK
}
