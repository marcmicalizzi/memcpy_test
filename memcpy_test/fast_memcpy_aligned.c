#include "fast_memcpy_aligned.h"

#define rep_movsb(to,from,n)\
  __asm__ __volatile__ (\
    "cld\n\trep movsb"\
    : "=D" (to), "=S" (from), "=c" (n)\
    : "0"  (to),  "1" (from), "2"  (n)\
    : "cc", "memory"\
  )
  
void * n_rep_movsb(void* d, const void* s, size_t n) {
	  asm volatile ("rep movsb"
                : "=D" (d),
                  "=S" (s),
                  "=c" (n)
                : "0" (d),
                  "1" (s),
                  "2" (n)
                : "memory");
  return d;
}
  
#define SSE_MMREG_SIZE 16
#define MMX_MMREG_SIZE 8

#define MMX1_MIN_LEN 0x800  /* 2K blocks */
#define MIN_LEN 0x40  /* 64-byte blocks */

void * asm_memcpy (void * to, const void * from, size_t n)
{
int d0, d1, d2;

  if( n < 4 ) {
    rep_movsb(to,from,n);
  }
  else
    __asm__ __volatile__(
    "rep ; movsl\n\t"
    "testb $2,%b4\n\t"
    "je 1f\n\t"
    "movsw\n"
    "1:\ttestb $1,%b4\n\t"
    "je 2f\n\t"
    "movsb\n"
    "2:"
    : "=&c" (d0), "=&D" (d1), "=&S" (d2)
    :"0" (n/4), "q" (n),"1" ((uintptr_t) to),"2" ((uintptr_t) from)
    : "memory");

  return (to);
}


void * sse_memcpy_aligned(void * to, const void * from, size_t len)
{
  void *retval;
  size_t i;
  retval = to;

  __asm__ __volatile__ (
    "   prefetchnta (%0)\n"
    "   prefetchnta 32(%0)\n"
    "   prefetchnta 64(%0)\n"
    "   prefetchnta 96(%0)\n"
    "   prefetchnta 128(%0)\n"
    "   prefetchnta 160(%0)\n"
    "   prefetchnta 192(%0)\n"
    "   prefetchnta 224(%0)\n"
    "   prefetchnta 256(%0)\n"
    "   prefetchnta 288(%0)\n"
    : : "r" (from) );

  if(len >= MIN_LEN)
  {
    i = len >> 6; /* len/64 */
    len&=63;
	  for(; i>0; i--)
	  {
		__asm__ __volatile__ (
		"prefetchnta 320(%0)\n"
	   "prefetchnta 352(%0)\n"
		"movaps (%0), %%xmm0\n"
		"movaps 16(%0), %%xmm1\n"
		"movaps 32(%0), %%xmm2\n"
		"movaps 48(%0), %%xmm3\n"
		"movntps %%xmm0, (%1)\n"
		"movntps %%xmm1, 16(%1)\n"
		"movntps %%xmm2, 32(%1)\n"
		"movntps %%xmm3, 48(%1)\n"
		:: "r" (from), "r" (to) : "memory");
		from = ((const unsigned char *)from) + 64;
		to = ((unsigned char *)to) + 64;
	  }
	/* since movntq is weakly-ordered, a "sfence"
	 * is needed to become ordered again. */
	__asm__ __volatile__ ("sfence":::"memory");
  }
  return retval;
}

void * sse2_memcpy_aligned(void * to, const void * from, size_t len)
{
     void *retval;
     size_t i;
     retval = to;
     __asm__ __volatile__ (
                          "   prefetchnta (%0)\n"
                          "   prefetchnta 64(%0)\n"
                          "   prefetchnta 128(%0)\n"
                          "   prefetchnta 192(%0)\n"
                          "   prefetchnta 256(%0)\n"
			  /*
                          "   prefetchnta 320(%0)\n"
                          "   prefetchnta 384(%0)\n"
                          "   prefetchnta 448(%0)\n"
                          "   prefetchnta 512(%0)\n"
			  */
                          : : "r" (from) );

     if (len >= MIN_LEN) {
          i = len >> 7; /* len/128 */
          len&=127;
		   for (; i>0; i--) {
				__asm__ __volatile__ (
									 "prefetchnta 640(%0)\n"
				 
									 "movapd (%0), %%xmm0\n"
									 "movapd 16(%0), %%xmm1\n"
									 "movapd 32(%0), %%xmm2\n"
									 "movapd 48(%0), %%xmm3\n"
				 
									 "movntdq %%xmm0, (%1)\n"
									 "movntdq %%xmm1, 16(%1)\n"
									 "movntdq %%xmm2, 32(%1)\n"
									 "movntdq %%xmm3, 48(%1)\n"
				 
									 "movapd 64(%0), %%xmm4\n"
									 "movapd 80(%0), %%xmm5\n"
									 "movapd 96(%0), %%xmm6\n"
									 "movapd 112(%0), %%xmm7\n"

									 "movntdq %%xmm4, 64(%1)\n"
									 "movntdq %%xmm5, 80(%1)\n"
									 "movntdq %%xmm6, 96(%1)\n"
									 "movntdq %%xmm7, 112(%1)\n"
									 :: "r" (from), "r" (to) : "memory");
		  from = ((const unsigned char *)from)+128;
		  to   = ((unsigned char *)to)+128;
		   }
		__asm__ __volatile__ ("mfence":::"memory");
		__asm__ __volatile__ ("emms":::"memory");
	 }
     return retval;
}

void * mmx_memcpy_aligned(void * to, const void * from, size_t len)
{
  void *retval;
  size_t i;
  retval = to;

  if(len >= MMX1_MIN_LEN)
  {
    i = len >> 6; /* len/64 */
    len&=63;
    for(; i>0; i--)
    {
      __asm__ __volatile__ (
      "movq (%0), %%mm0\n"
      "movq 8(%0), %%mm1\n"
      "movq 16(%0), %%mm2\n"
      "movq 24(%0), %%mm3\n"
      "movq 32(%0), %%mm4\n"
      "movq 40(%0), %%mm5\n"
      "movq 48(%0), %%mm6\n"
      "movq 56(%0), %%mm7\n"
      "movq %%mm0, (%1)\n"
      "movq %%mm1, 8(%1)\n"
      "movq %%mm2, 16(%1)\n"
      "movq %%mm3, 24(%1)\n"
      "movq %%mm4, 32(%1)\n"
      "movq %%mm5, 40(%1)\n"
      "movq %%mm6, 48(%1)\n"
      "movq %%mm7, 56(%1)\n"
      :: "r" (from), "r" (to) : "memory");
      from = ((const unsigned char *)from) + 64;
      to = ((unsigned char *)to) + 64;
    }
    __asm__ __volatile__ ("emms":::"memory");
  }
  /*
   *	Now do the tail of the block
   */
  return retval;
}

void * mmx2_memcpy_aligned(void * to, const void * from, size_t len)
{
  void *retval;
  size_t i;
  retval = to;

  __asm__ __volatile__ (
    "   prefetchnta (%0)\n"
    "   prefetchnta 32(%0)\n"
    "   prefetchnta 64(%0)\n"
    "   prefetchnta 96(%0)\n"
    "   prefetchnta 128(%0)\n"
    "   prefetchnta 160(%0)\n"
    "   prefetchnta 192(%0)\n"
    "   prefetchnta 224(%0)\n"
    "   prefetchnta 256(%0)\n"
    "   prefetchnta 288(%0)\n"
    : : "r" (from) );

  if(len >= MIN_LEN)
  {
    i = len >> 6; /* len/64 */
    len&=63;
    for(; i>0; i--)
    {
      __asm__ __volatile__ (
      "prefetchnta 320(%0)\n"
      "prefetchnta 352(%0)\n"
      "movq (%0), %%mm0\n"
      "movq 8(%0), %%mm1\n"
      "movq 16(%0), %%mm2\n"
      "movq 24(%0), %%mm3\n"
      "movq 32(%0), %%mm4\n"
      "movq 40(%0), %%mm5\n"
      "movq 48(%0), %%mm6\n"
      "movq 56(%0), %%mm7\n"
      "movntq %%mm0, (%1)\n"
      "movntq %%mm1, 8(%1)\n"
      "movntq %%mm2, 16(%1)\n"
      "movntq %%mm3, 24(%1)\n"
      "movntq %%mm4, 32(%1)\n"
      "movntq %%mm5, 40(%1)\n"
      "movntq %%mm6, 48(%1)\n"
      "movntq %%mm7, 56(%1)\n"
      :: "r" (from), "r" (to) : "memory");
      from = ((const unsigned char *)from) + 64;
      to = ((unsigned char *)to) + 64;
    }
    __asm__ __volatile__ ("sfence":::"memory");
    __asm__ __volatile__ ("emms":::"memory");
  }
  return retval;
}

void * avx_memcpy_aligned(void * to, const void * from, size_t len)
{
  void *retval = to;
  const unsigned char *pf;
  uintptr_t i;
  pf = (const unsigned char *)((uintptr_t)from & ~(uintptr_t)31);
  __asm__ __volatile__ (
    "   prefetchnta (%0)\n"
    "   prefetchnta 32(%0)\n"
    "   prefetchnta 64(%0)\n"
    "   prefetchnta 96(%0)\n"
    "   prefetchnta 128(%0)\n"
    "   prefetchnta 160(%0)\n"
    "   prefetchnta 192(%0)\n"
    "   prefetchnta 224(%0)\n"
    "   prefetchnta 256(%0)\n"
    "   prefetchnta 288(%0)\n"
    : : "r" (pf));
  pf += 320;

  if(len >= MIN_LEN)
  {
    i = len >> 7; /* len/128 */
      while (i--) {
        __asm__ __volatile__ (
        "\n\tprefetchnta (%2)\n"
        "\n\tprefetchnta 32(%2)\n"
        "\n\tprefetchnta 64(%2)\n"
		"\n\tprefetchnta 96(%2)\n"
        "\n\tvmovaps (%0), %%ymm0"
        "\n\tvmovaps 32(%0), %%ymm1"
		"\n\tleaq\t128(%2), %2"
        "\n\tvmovaps\t64(%0), %%ymm2"
        "\n\tvmovaps\t96(%0), %%ymm3"
        "\n\tvmovntps\t%%ymm0, (%1)"
        "\n\tvmovntps\t%%ymm1, 32(%1)"
        "\n\tleaq\t128(%0), %0"
        "\n\tvmovntps\t%%ymm2, 64(%1)"
        "\n\tvmovntps\t%%ymm3, 96(%1)"
        "\n\tleaq\t128(%1), %1"
        : "=r" (from), "=r" (to), "=r" (pf)
        : "0"  (from), "1"  (to), "2"  (pf)
        : "memory");
      }
    __asm__ __volatile__ ("sfence":::"memory");
    __asm__ __volatile__ ("vzeroupper");
    len &= 127;
  }
  return retval;
}


