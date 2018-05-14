#include <cstdlib>
#include <chrono>
#include <iostream>
#include <functional>
#include <map>
#include <string>
#include <intrin.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <windows.h>
#include "mmintrin.h"
#include <vector>
#include <set>
#include <sstream>
#include "fast_memcpy_aligned.h"

#ifdef _WIN32

//  Windows
#define cpuid(info, x)    __cpuidex(info, x, 0)

#else

//  GCC Intrinsics
#include <cpuid.h>
void cpuid(int info[4], int InfoType) {
  __cpuid_count(InfoType, 0, info[0], info[1], info[2], info[3]);
}

#endif

#define BUFFER_SIZE 16777216

#define SSE_MMREG_SIZE 16
#define AVX_MMREG_SIZE 32
#define AVX512_MMREG_SIZE 64
inline static void std_memcpy(void* dst, void* src, size_t len) {
  std::memcpy(dst, src, len);
}

#if _MSC_VER >= 1910
static void mm_avx512_memcpy_aligned(void* dst, void* src, size_t len) {
  len = len / 4;
  for (int i = 0; i < (len); i += 64) {
    __m512 ZMM0 = _mm512_load_ps((int*)src + i);
    __m512 ZMM1 = _mm512_load_ps((int*)src + i + 16);
    __m512 ZMM2 = _mm512_load_ps(((int*)src) + i + 32);
    __m512 ZMM3 = _mm512_load_ps(((int*)src) + i + 48);
    _mm512_stream_ps(((int*)dst) + i, ZMM0);
    _mm512_stream_ps(((int*)dst) + i + 16, ZMM1);
    _mm512_stream_ps(((int*)dst) + i + 32, ZMM2);
    _mm512_stream_ps(((int*)dst) + i + 48, ZMM3);
  }
  _mm_sfence();
}
#endif

static void mm_sse_memcpy_aligned(void* dst, void* src, size_t len) {
  _mm_prefetch((const char*)src, 0);
  _mm_prefetch((const char*)src + 64, 0);
  _mm_prefetch((const char*)src + 128, 0);
  _mm_prefetch((const char*)src + 192, 0);
  _mm_prefetch((const char*)src + 256, 0);


  len = len / 4;
  for (int i = 0; i < (len); i += 16) {
    _mm_prefetch((const char*)src + 320, 0);
    __m128 XMM0 = _mm_load_ps(((float*)src) + i);
    __m128 XMM1 = _mm_load_ps(((float*)src) + i + 4);
    __m128 XMM2 = _mm_load_ps(((float*)src) + i + 8);
    __m128 XMM3 = _mm_load_ps(((float*)src) + i + 12);
    _mm_stream_ps(((float*)dst) + i, XMM0);
    _mm_stream_ps(((float*)dst) + i + 4, XMM1);
    _mm_stream_ps(((float*)dst) + i + 8, XMM2);
    _mm_stream_ps(((float*)dst) + i + 12, XMM3);
  }
  _mm_sfence();
}

static void mm_sse2_memcpy_aligned(void* dst, void* src, size_t len) {
  _mm_prefetch((const char*)src, 0);
  _mm_prefetch((const char*)src + 64, 0);
  _mm_prefetch((const char*)src + 128, 0);
  _mm_prefetch((const char*)src + 192, 0);
  _mm_prefetch((const char*)src + 256, 0);
  /*
  _mm_prefetch((const char*)src + 320, 0);
  _mm_prefetch((const char*)src + 384, 0);
  _mm_prefetch((const char*)src + 448, 0);
  _mm_prefetch((const char*)src + 512, 0);
  */


  len = len / 8;
  for (int i = 0; i < (len); i += 16) {
    _mm_prefetch((const char*)src + 640, 0);
    __m128d XMM0 = _mm_load_pd(((double*)src) + i);
    __m128d XMM1 = _mm_load_pd(((double*)src) + i + 2);
    __m128d XMM2 = _mm_load_pd(((double*)src) + i + 4);
    __m128d XMM3 = _mm_load_pd(((double*)src) + i + 6);
    _mm_stream_pd(((double*)dst) + i, XMM0);
    _mm_stream_pd(((double*)dst) + i + 2, XMM1);
    _mm_stream_pd(((double*)dst) + i + 4, XMM2);
    _mm_stream_pd(((double*)dst) + i + 6, XMM3);
    __m128d XMM4 = _mm_load_pd(((double*)src) + i + 8);
    __m128d XMM5 = _mm_load_pd(((double*)src) + i + 10);
    __m128d XMM6 = _mm_load_pd(((double*)src) + i + 12);
    __m128d XMM7 = _mm_load_pd(((double*)src) + i + 14);
    _mm_stream_pd(((double*)dst) + i + 8, XMM4);
    _mm_stream_pd(((double*)dst) + i + 10, XMM5);
    _mm_stream_pd(((double*)dst) + i + 12, XMM6);
    _mm_stream_pd(((double*)dst) + i + 14, XMM7);
  }
  _mm_sfence();
}

static void mm_avx_memcpy_aligned(void* dst, void* src, size_t len) {
  _mm_prefetch((const char*)src, 0);
  _mm_prefetch((const char*)src + 64, 0);
  _mm_prefetch((const char*)src + 128, 0);
  _mm_prefetch((const char*)src + 192, 0);
  _mm_prefetch((const char*)src + 256, 0);
  _mm_prefetch((const char*)src + 288, 0);

  len = len / 4;
  for (int i = 0; i < (len); i += 32) {
    __m256 YMM0 = _mm256_load_ps(((float*)src) + i);
    __m256 YMM1 = _mm256_load_ps(((float*)src) + i + 8);
    __m256 YMM2 = _mm256_load_ps(((float*)src) + i + 16);
    __m256 YMM3 = _mm256_load_ps(((float*)src) + i + 24);
    _mm256_stream_ps(((float*)dst) + i, YMM0);
    _mm256_stream_ps(((float*)dst) + i + 8, YMM1);
    _mm256_stream_ps(((float*)dst) + i + 16, YMM2);
    _mm256_stream_ps(((float*)dst) + i + 24, YMM3);
  }
  _mm_sfence();
  _mm256_zeroupper();
}


struct memcpy_func {
  const char* name;
  std::function<void(void*, void*, size_t)> func;
  bool supported;
};

struct buffer_t {
  unsigned char** src;
  unsigned char** dst;
  std::string name;
};

int main()
{

  //  Misc.
  bool HW_MMX;
  bool HW_x64;
  bool HW_ABM;      // Advanced Bit Manipulation
  bool HW_RDRAND;
  bool HW_BMI1;
  bool HW_BMI2;
  bool HW_ADX;
  bool HW_PREFETCHWT1;

  //  SIMD: 128-bit
  bool HW_SSE;
  bool HW_SSE2;
  bool HW_SSE3;
  bool HW_SSSE3;
  bool HW_SSE41;
  bool HW_SSE42;
  bool HW_SSE4a;
  bool HW_AES;
  bool HW_SHA;

  //  SIMD: 256-bit
  bool HW_AVX;
  bool HW_XOP;
  bool HW_FMA3;
  bool HW_FMA4;
  bool HW_AVX2;

  //  SIMD: 512-bit
  bool HW_AVX512F;    //  AVX512 Foundation
  bool HW_AVX512CD;   //  AVX512 Conflict Detection
  bool HW_AVX512PF;   //  AVX512 Prefetch
  bool HW_AVX512ER;   //  AVX512 Exponential + Reciprocal
  bool HW_AVX512VL;   //  AVX512 Vector Length Extensions
  bool HW_AVX512BW;   //  AVX512 Byte + Word
  bool HW_AVX512DQ;   //  AVX512 Doubleword + Quadword
  bool HW_AVX512IFMA; //  AVX512 Integer 52-bit Fused Multiply-Add
  bool HW_AVX512VBMI; //  AVX512 Vector Byte Manipulation Instructions

  int info[4];
  cpuid(info, 0);
  int nIds = info[0];

  cpuid(info, 0x80000000);
  unsigned nExIds = info[0];

  //  Detect Features
  if (nIds >= 0x00000001) {
    cpuid(info, 0x00000001);
    HW_MMX = (info[3] & ((int)1 << 23)) != 0;
    HW_SSE = (info[3] & ((int)1 << 25)) != 0;
    HW_SSE2 = (info[3] & ((int)1 << 26)) != 0;
    HW_SSE3 = (info[2] & ((int)1 << 0)) != 0;

    HW_SSSE3 = (info[2] & ((int)1 << 9)) != 0;
    HW_SSE41 = (info[2] & ((int)1 << 19)) != 0;
    HW_SSE42 = (info[2] & ((int)1 << 20)) != 0;
    HW_AES = (info[2] & ((int)1 << 25)) != 0;

    HW_AVX = (info[2] & ((int)1 << 28)) != 0;
    HW_FMA3 = (info[2] & ((int)1 << 12)) != 0;

    HW_RDRAND = (info[2] & ((int)1 << 30)) != 0;
  }
  if (nIds >= 0x00000007) {
    cpuid(info, 0x00000007);
    HW_AVX2 = (info[1] & ((int)1 << 5)) != 0;

    HW_BMI1 = (info[1] & ((int)1 << 3)) != 0;
    HW_BMI2 = (info[1] & ((int)1 << 8)) != 0;
    HW_ADX = (info[1] & ((int)1 << 19)) != 0;
    HW_SHA = (info[1] & ((int)1 << 29)) != 0;
    HW_PREFETCHWT1 = (info[2] & ((int)1 << 0)) != 0;

    HW_AVX512F = (info[1] & ((int)1 << 16)) != 0;
    HW_AVX512CD = (info[1] & ((int)1 << 28)) != 0;
    HW_AVX512PF = (info[1] & ((int)1 << 26)) != 0;
    HW_AVX512ER = (info[1] & ((int)1 << 27)) != 0;
    HW_AVX512VL = (info[1] & ((int)1 << 31)) != 0;
    HW_AVX512BW = (info[1] & ((int)1 << 30)) != 0;
    HW_AVX512DQ = (info[1] & ((int)1 << 17)) != 0;
    HW_AVX512IFMA = (info[1] & ((int)1 << 21)) != 0;
    HW_AVX512VBMI = (info[2] & ((int)1 << 1)) != 0;
  }
  if (nExIds >= 0x80000001) {
    cpuid(info, 0x80000001);
    HW_x64 = (info[3] & ((int)1 << 29)) != 0;
    HW_ABM = (info[2] & ((int)1 << 5)) != 0;
    HW_SSE4a = (info[2] & ((int)1 << 6)) != 0;
    HW_FMA4 = (info[2] & ((int)1 << 16)) != 0;
    HW_XOP = (info[2] & ((int)1 << 11)) != 0;
  }

  int NUM_FUNCS = 0;
  memcpy_func funcs[] = {
    { "std::memcpy\t\t", std_memcpy, true },
  { "asm_memcpy (asm)\t", asm_memcpy, true },
  { "sse_memcpy (intrinsic)\t", mm_sse_memcpy_aligned, HW_SSE },
  { "sse_memcpy (asm)\t", sse_memcpy_aligned, HW_SSE },
  { "sse2_memcpy (intrinsic)\t", mm_sse2_memcpy_aligned, HW_SSE2 },
  { "sse2_memcpy (asm)\t", sse2_memcpy_aligned, HW_SSE2 },
  { "mmx_memcpy (asm)\t", mmx_memcpy_aligned, HW_MMX },
  { "mmx2_memcpy (asm)\t", mmx2_memcpy_aligned, HW_MMX && HW_SSE },
  { "avx_memcpy (intrinsic)\t", mm_avx_memcpy_aligned, HW_AVX },
  { "avx_memcpy (asm)\t", avx_memcpy_aligned, HW_AVX },
#if _MSC_VER >= 1910
  { "avx512_memcpy (intrinsic)", mm_avx512_memcpy_aligned, HW_AVX512F },
#endif
  { "rep movsb (asm)\t\t", n_rep_movsb, true },
  //{ "rep stosb (asm)", n_rep_stosb, true },
  { 0, 0 }
  };

  for (size_t i = 0; funcs[i].func; i++) {
    NUM_FUNCS++;
  }

  std::set<UCHAR> numa_nodes;
  ULONG HighestNodeNumber = -1;
  ULONG NumberOfProcessors = -1;
  UCHAR local_numa = -1;
  SYSTEM_INFO SystemInfo;
  GetSystemInfo(&SystemInfo);
  NumberOfProcessors = SystemInfo.dwNumberOfProcessors;
  DWORD PageSize = SystemInfo.dwPageSize;

  //
  // Get the highest node number.
  //

  if (GetNumaHighestNodeNumber(&HighestNodeNumber)) {
    if (HighestNodeNumber > 0) {
      std::cerr << "System has " << NumberOfProcessors << " processors and " << HighestNodeNumber << " highest NUMA node" << std::endl;
      for (UCHAR i = 0; i < NumberOfProcessors; i++) {
        UCHAR NodeNumber;
        if (GetNumaProcessorNode(i, &NodeNumber)) {
          std::cerr << "Processor " << (int)i << " on node " << (int)NodeNumber << std::endl;
          numa_nodes.insert(NodeNumber);
          if (i == GetCurrentProcessorNumber()) {
            local_numa = NodeNumber;
          }
        }
      }
      for (size_t i = 0; i <= HighestNodeNumber; i++) {
        numa_nodes.insert(i);
      }
    }
  }

  __declspec(align(64)) buffer_t* buf = 0;
  size_t i = 0;
  size_t allocd = 0;
  size_t nn = numa_nodes.size() + 2;
  std::cerr << "Allocating " << nn << " buffer sets" << std::endl;
  buf = new buffer_t[nn];
  if (!numa_nodes.empty()) {
    for (auto& it : numa_nodes) {
      long long avg_alloc = 0;
      std::stringstream name;
      name << "VirtualAllocExNuma to NUMA node " << (int)it;
      if (it == local_numa) {
        name << "(local)";
      }
      buf[i].name = name.str();
      buf[i].src = (unsigned char**)VirtualAllocExNuma(GetCurrentProcess(), 0, 100 * sizeof(intptr_t), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE, it);
      buf[i].dst = (unsigned char**)VirtualAllocExNuma(GetCurrentProcess(), 0, 100 * sizeof(intptr_t), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE, it);
      if (buf[i].src && buf[i].dst) {
        std::cerr << "Allocating memory to buffer set " << i << " " << name.str() << std::endl;
        for (size_t j = 0; j < 100; j++) {
          buf[i].src[j] = (unsigned char*)VirtualAllocExNuma(GetCurrentProcess(), 0, BUFFER_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE, it);
          buf[i].dst[j] = (unsigned char*)VirtualAllocExNuma(GetCurrentProcess(), 0, BUFFER_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE, it);
          if (buf[i].src[j] && buf[i].dst[j]) {
            for (size_t k = 0; k < BUFFER_SIZE; k++) {
              buf[i].src[j][k] = k % 256;
              buf[i].dst[j][k] = 0xff;
              allocd++;
            }
          }
        }
      }
      i++;
    }
  }

  std::cerr << "Allocating memory to buffer set " << i << " using new" << std::endl;
  buf[i].name = "operator new";
  buf[i].src = new unsigned char*[100];
  buf[i].dst = new unsigned char*[100];

  for (size_t j = 0; j < 100; j++) {
    buf[i].src[j] = new unsigned char[BUFFER_SIZE];
    buf[i].dst[j] = new unsigned char[BUFFER_SIZE];
    for (size_t k = 0; k < BUFFER_SIZE; k++) {
      buf[i].src[j][k] = k % 256;
      buf[i].dst[j][k] = 0xff;
    }
  }

  buf[i + 1].src = 0;
  buf[i + 1].dst = 0;


  std::map<std::string, std::map<const char*, std::vector<long long> > > times;
  std::map<size_t, size_t> n;
  for (;;) {
    for (size_t nn = 0; buf[nn].src && buf[nn].dst; nn++) {
      memcpy_func f = funcs[n[nn] % NUM_FUNCS];
      if (f.supported) {
        for (size_t i = 0; i < 100; i++) {
          std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
          f.func(buf[nn].dst[i], buf[nn].src[i], BUFFER_SIZE);
          std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
          auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
          times[buf[nn].name][f.name].push_back(duration);
        }
        if (n[nn] < NUM_FUNCS) {
          for (size_t i = 0; i < BUFFER_SIZE; i++) {
            if (buf[nn].dst[0][i] != buf[nn].src[0][i]) {
              std::cerr << f.name << ": dst does not match src" << std::endl;
              break;
            }
          }
        }
      }
      else {
        times[buf[nn].name][f.name].push_back(-1);
      }
      n[nn] = (n[nn] + 1) % SIZE_MAX;
      if (n[nn] % NUM_FUNCS == 0) {
        std::cerr << "---------------------------------------------------------------------------" << std::endl;
        std::cerr << "Averaging " << times[buf[nn].name].begin()->second.size() << " copies of " << BUFFER_SIZE / 1024 / 1024 << "MB of data per function for " << buf[nn].name.c_str() << std::endl;
        std::cerr << "---------------------------------------------------------------------------" << std::endl;
        for (auto& it : times[buf[nn].name]) {
          double avg = 0;
          size_t vals = it.second.size();
          for (size_t j = 0; j < vals; j++) {
            avg += (double)it.second[j] / (double)vals;
          }
          if (avg > 0) {
            std::cerr << it.first << "\t averaging " << avg << " microseconds" << std::endl;
          }
          else {
            std::cerr << it.first << "\t unsupported on this CPU" << std::endl;
          }
        }
      }
    }
  }
}

