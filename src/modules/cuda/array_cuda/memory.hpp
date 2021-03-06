#include <cuda.h>
#include <cuda_runtime.h>

#ifdef WIN32
 #define strcasecmp(A,B) _stricmp(A,B)
 #define strncasecmp(A,B,C) _strnicmp(A,B,C)
 #pragma warning(disable: 4251)

 #ifdef ARRAYCUDA_EXPORTS
  #define ARRAYCUDA_API __declspec(dllexport)
 #else
  #define ARRAYCUDA_API __declspec(dllimport)
 #endif
#else
 #define ARRAYCUDA_API 
#endif

/* v is a address of the pointer to the array */
cudaError_t ARRAYCUDA_API malloc_device_(void** d_v, size_t n, const char* file, const unsigned int line);
cudaError_t ARRAYCUDA_API malloc_host_(void** d_v, size_t n, const char* file, const unsigned int line);
#define malloc_device(v,n) malloc_device_((void**)v,n,__FILE__, __LINE__)
#define malloc_host(v,n) malloc_host_((void**)v,n,__FILE__, __LINE__)


cudaError_t ARRAYCUDA_API malloc_dh_(void** d, void** h, size_t n, const char* file, const unsigned int line);
#define malloc_dh(d,h,n) malloc_dh_((void**)d,(void**)h,n,__FILE__, __LINE__)


void ARRAYCUDA_API free_device_(void* d_v, const char* file, unsigned int line);
void ARRAYCUDA_API free_host_(void* d_v, const char* file, unsigned int line);
#define free_device(v) free_device_((void*)v,__FILE__,__LINE__)
#define free_host(v) free_host_((void*)v,__FILE__,__LINE__)


void ARRAYCUDA_API free_dh_(void* d, void* h, const char* file, unsigned int line);
#define free_dh(d,h) free_dh_((void*)d,(void*)h,__FILE__,__LINE__)



void ARRAYCUDA_API memcpy_d2d_(void* d_dest, void* d_src, size_t n,const char* file, unsigned int line);
void ARRAYCUDA_API memcpy_d2h_(void* h_dest, void* d_src, size_t n,const char* file, unsigned int line);
void ARRAYCUDA_API memcpy_h2d_(void* d_dest, void* h_src, size_t n,const char* file, unsigned int line);

#define memcpy_d2d(a,b,n) memcpy_d2d_((void*)a, (void*)b, n,__FILE__, __LINE__)
#define memcpy_h2d(a,b,n) memcpy_h2d_((void*)a, (void*)b, n,__FILE__, __LINE__)
#define memcpy_d2h(a,b,n) memcpy_d2h_((void*)a, (void*)b, n,__FILE__, __LINE__)




ARRAYCUDA_API void  registerWS();
ARRAYCUDA_API void  unregisterWS();
// the level argument below prevents WSs from overlapping. This is useful for multi-level 
// operations that all use WSs: example long range interaction. FFTs at lowest level with a WS acting as an accumulator
ARRAYCUDA_API bool getWSMemD_(void** ptr, size_t size, long level);
#define getWSMemD(p1,s1,level) getWSMemD_((void**)p1, s1, level)

ARRAYCUDA_API bool getWSMemH_(void** ptr, size_t size, long level);
#define getWSMemH(p1,s1,level) getWSMemH_((void**)p1, s1, level)


void log_print(const char* msg);


