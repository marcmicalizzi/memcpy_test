#ifndef _AV_FAST_MEMCPY_ALIGNED_H_
#define _AV_FAST_MEMCPY_ALIGNED_H_

#include <memory.h>

#define AV_MC_SYS	0
#define AV_MC_X86	1
#define AV_MC_MMX	2
#define AV_MC_MMX2	3
#define AV_MC_SSE	4

#ifdef __cplusplus
extern "C" {
#endif


void * asm_memcpy(void * to, const void * from, size_t n);
void * n_rep_movsb(void* d, const void* s, size_t n);
void * sse_memcpy_aligned(void * to, const void * from, size_t len);
void * sse2_memcpy_aligned(void * to, const void * from, size_t len);
void * mmx_memcpy_aligned(void * to, const void * from, size_t len);
void * mmx2_memcpy_aligned(void * to, const void * from, size_t len);
void * avx_memcpy_aligned(void * to, const void * from, size_t n);

#ifdef __cplusplus
}
#endif

#endif
