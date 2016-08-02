void cuda_llg_quat_apply32(const int nx, const int ny, const int nz,
	float* dsx, float* dsy, float* dsz, float* dms, //dest (spinto)
	float* ssx, float* ssy, float* ssz, float* sms, // src (spinfrom)
	float* ddx, float* ddy, float* ddz, float* dds, // dm/dt spins
    float* htx, float* hty, float* htz,              // dm/dt thermal fields
	float* dhx, float* dhy, float* dhz,              // dm/dt fields
	float* ws1, float* ws2, float* ws3, float* ws4,
	const float alpha, const float dt, const float gamma);