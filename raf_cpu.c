/*

RAFcodding — raf_cpu.c

Bare-metal CPU detection + 4D Flag Matrix

arch × lang × opt × feat → optimal compiler flags

Zero stdlib dependencies on hot path — only CPUID inline ASM
*/
#include "raf_compile.h"
#include <string.h>
#include <stdio.h>


/* ════════════════════════════════════════════════════

CPUID — x86-64 bare metal

════════════════════════════════════════════════════ */
#if defined(x86_64) || defined(i386)
#include <cpuid.h>


static void _cpuid(uint32_t leaf, uint32_t sub,
uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d) {
asm volatile(
"cpuid"
: "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d)
: "a"(leaf), "c"(sub)
);
}

static void _detect_x86(RafCPU *cpu) {
uint32_t a,b,c,d;
cpu->arch = RAF_ARCH_X86_64;

/* vendor string */  
_cpuid(0,0,&a,&b,&c,&d);  
uint32_t max_leaf = a;  
/* Intel: GenuineIntel  b=0x756E6547 d=0x49656E69 c=0x6C65746E */  
cpu->vendor = (b==0x756E6547u) ? 0 : 1; /* 0=Intel 1=AMD/other */  

/* brand string */  
_cpuid(0x80000002,0,&a,&b,&c,&d);  
memcpy(cpu->brand,    &a, 4); memcpy(cpu->brand+4,  &b, 4);  
memcpy(cpu->brand+8,  &c, 4); memcpy(cpu->brand+12, &d, 4);  
_cpuid(0x80000003,0,&a,&b,&c,&d);  
memcpy(cpu->brand+16, &a, 4); memcpy(cpu->brand+20, &b, 4);  
memcpy(cpu->brand+24, &c, 4); memcpy(cpu->brand+28, &d, 4);  
_cpuid(0x80000004,0,&a,&b,&c,&d);  
memcpy(cpu->brand+32, &a, 4); memcpy(cpu->brand+36, &b, 4);  
memcpy(cpu->brand+40, &c, 4); memcpy(cpu->brand+44, &d, 4);  
cpu->brand[47]=0;  

/* feature bits */  
if (max_leaf >= 1) {  
    _cpuid(1,0,&a,&b,&c,&d);  
    cpu->family = ((a>>8)&0xF) + ((a>>20)&0xFF);  
    cpu->model  = ((a>>4)&0xF) | ((a>>12)&0xF0);  
    if (c & (1u<<20)) cpu->feat |= RAF_FEAT_SSE4;  
    if (c & (1u<<12)) cpu->feat |= RAF_FEAT_CRC32;  
    /* cores from EBX[23:16] */  
    cpu->cores = (b>>16)&0xFF;  
    if (!cpu->cores) cpu->cores=1;  
}  
if (max_leaf >= 7) {  
    _cpuid(7,0,&a,&b,&c,&d);  
    if (b & (1u<<5))  cpu->feat |= RAF_FEAT_AVX2;  
    if (b & (1u<<16)) cpu->feat |= RAF_FEAT_AVX512;  
}  

/* L1d cache via leaf 4 */  
_cpuid(4,0,&a,&b,&c,&d);  
if (a) {  
    uint32_t sets     = d+1;  
    uint32_t assoc    = ((b>>22)&0x3FF)+1;  
    uint32_t part     = ((b>>12)&0x3FF)+1;  
    uint32_t line_sz  = (b&0xFFF)+1;  
    cpu->l1d_kb = (sets*assoc*part*line_sz)>>10;  
    cpu->cache_line = line_sz;  
}  
/* L2: leaf 4 sub-leaf 2 */  
_cpuid(4,2,&a,&b,&c,&d);  
if (a) {  
    cpu->l2_kb = ((d+1)*((b>>22&0x3FF)+1)*((b>>12&0x3FF)+1)*((b&0xFFF)+1))>>10;  
}  
/* L3: sub-leaf 3 */  
_cpuid(4,3,&a,&b,&c,&d);  
if (a) {  
    cpu->l3_kb = ((d+1)*((b>>22&0x3FF)+1)*((b>>12&0x3FF)+1)*((b&0xFFF)+1))>>10;  
}  

/* register geometry */  
cpu->gpr_count = 16;  
if (cpu->feat & RAF_FEAT_AVX512) {  
    cpu->vec_width = 64; cpu->vec_count = 32;  
} else if (cpu->feat & RAF_FEAT_AVX2) {  
    cpu->vec_width = 32; cpu->vec_count = 16;  
} else {  
    cpu->vec_width = 16; cpu->vec_count = 16;  
}

}
#else
static void _detect_x86(RafCPU *cpu) { (void)cpu; }
#endif

/* ════════════════════════════════════════════════════

ARM64 detection via /proc/cpuinfo

════════════════════════════════════════════════════ */
#if defined(aarch64)
#include <stdio.h>
static void _detect_arm64(RafCPU cpu) {
cpu->arch  = RAF_ARCH_ARM64;
cpu->vendor= 2; / ARM /
cpu->gpr_count = 31;
cpu->cache_line = 64;
cpu->feat |= RAF_FEAT_NEON;  / always on AArch64 */

FILE *f = fopen("/proc/cpuinfo","r");
if (!f) { snprintf(cpu->brand,48,"AArch64"); return; }
char line[256];
while (fgets(line,sizeof(line),f)) {
if (!strncmp(line,"Hardware",8)) {
char p=strchr(line,':'); if(p){p+=2;
size_t l=strlen(p); if(l>47)l=47;
memcpy(cpu->brand,p,l); cpu->brand[l]=0;
/ strip newline */ for(int i=0;cpu->brand[i];i++)
if(cpu->brand[i]=='\n'){cpu->brand[i]=0;break;}
}
}
if (!strncmp(line,"Features",8)) {
if (strstr(line,"crc32")) cpu->feat |= RAF_FEAT_CRC32;
if (strstr(line,"sve"))   cpu->feat |= RAF_FEAT_SVE;
}
if (!strncmp(line,"CPU cores",9)) {
char *p=strchr(line,':');
if(p) cpu->cores=(uint32_t)atoi(p+1);
}
}
fclose(f);
if (!cpu->cores) cpu->cores=4;
cpu->vec_width = (cpu->feat&RAF_FEAT_SVE)?64:16;
cpu->vec_count = 32;
}
#else
static void _detect_arm64(RafCPU *cpu) { (void)cpu; }
#endif


/* ════════════════════════════════════════════════════

RISC-V 64 stub

════════════════════════════════════════════════════ */
static void _detect_rv64(RafCPU *cpu) {
cpu->arch  = RAF_ARCH_RV64;
cpu->vendor= 4;
snprintf(cpu->brand,48,"RISC-V 64");
cpu->gpr_count=32; cpu->cache_line=64; cpu->cores=1;
cpu->vec_width=16; cpu->vec_count=32;
}


void raf_cpu_detect(RafCPU *cpu) {
memset(cpu,0,sizeof(*cpu));
cpu->arch = RAF_ARCH_UNKNOWN;
#if defined(x86_64)
_detect_x86(cpu);
#elif defined(aarch64)
_detect_arm64(cpu);
#elif defined(__riscv) && __riscv_xlen==64
_detect_rv64(cpu);
#else
snprintf(cpu->brand,48,"UNKNOWN_ARCH");
#endif
if (!cpu->cache_line) cpu->cache_line=64;
if (!cpu->cores)      cpu->cores=1;
}

/* ════════════════════════════════════════════════════

FLAG MATRIX 4D

F[arch][lang][opt][feat_idx]

feat_idx = 0 (no simd), 1 (sse4/neon), 2 (avx2/sve), 3 (avx512)

4 × 7 × 5 × 4 = 560 cells

Each cell: {flags, linker, feat_req}

════════════════════════════════════════════════════ */


/* ── x86-64 C/C++ flags ── */
static const RafFlagCell _F_X86_C[RAF_OPT_N][4] = {
/O0/ {{"-O0 -g -std=c11","-lm",0},
{"-O0 -g -std=c11 -msse4.2","-lm",RAF_FEAT_SSE4},
{"-O0 -g -std=c11 -mavx2","-lm",RAF_FEAT_AVX2},
{"-O0 -g -std=c11 -mavx512f","-lm",RAF_FEAT_AVX512}},
/O1/ {{"-O1 -std=c11 -fomit-frame-pointer","-lm",0},
{"-O1 -std=c11 -msse4.2 -fomit-frame-pointer","-lm",RAF_FEAT_SSE4},
{"-O1 -std=c11 -mavx2 -fomit-frame-pointer","-lm",RAF_FEAT_AVX2},
{"-O1 -std=c11 -mavx512f -fomit-frame-pointer","-lm",RAF_FEAT_AVX512}},
/O2/ {{"-O2 -std=c11 -fomit-frame-pointer -fstrict-aliasing","-lm",0},
{"-O2 -std=c11 -msse4.2 -fstrict-aliasing","-lm",RAF_FEAT_SSE4},
{"-O2 -std=c11 -mavx2 -fstrict-aliasing -funroll-loops","-lm",RAF_FEAT_AVX2},
{"-O2 -std=c11 -mavx512f -funroll-loops","-lm",RAF_FEAT_AVX512}},
/O3/ {{"-O3 -std=c11 -fomit-frame-pointer -funroll-loops -fvectorize","-lm",0},
{"-O3 -std=c11 -msse4.2 -funroll-loops","-lm",RAF_FEAT_SSE4},
{"-O3 -std=c11 -mavx2 -funroll-loops -fvectorize","-lm",RAF_FEAT_AVX2},
{"-O3 -std=c11 -mavx512f -funroll-loops -fvectorize","-lm",RAF_FEAT_AVX512}},
/Os/ {{"-Os -std=c11 -ffunction-sections -fdata-sections","--gc-sections -lm",0},
{"-Os -std=c11 -msse4.2 -ffunction-sections","--gc-sections -lm",RAF_FEAT_SSE4},
{"-Os -std=c11 -mavx2 -ffunction-sections","--gc-sections -lm",RAF_FEAT_AVX2},
{"-Os -std=c11 -mavx512f -ffunction-sections","--gc-sections -lm",RAF_FEAT_AVX512}},
};

static const RafFlagCell _F_X86_CPP[RAF_OPT_N][4] = {
/O0/ {{"-O0 -g -std=c++17","-lm",0},
{"-O0 -g -std=c++17 -msse4.2","-lm",RAF_FEAT_SSE4},
{"-O0 -g -std=c++17 -mavx2","-lm",RAF_FEAT_AVX2},
{"-O0 -g -std=c++17 -mavx512f","-lm",RAF_FEAT_AVX512}},
/O1/ {{"-O1 -std=c++17","-lm",0},{"-O1 -std=c++17 -msse4.2","-lm",RAF_FEAT_SSE4},
{"-O1 -std=c++17 -mavx2","-lm",RAF_FEAT_AVX2},
{"-O1 -std=c++17 -mavx512f","-lm",RAF_FEAT_AVX512}},
/O2/ {{"-O2 -std=c++17 -fomit-frame-pointer","-lm",0},
{"-O2 -std=c++17 -msse4.2","-lm",RAF_FEAT_SSE4},
{"-O2 -std=c++17 -mavx2 -funroll-loops","-lm",RAF_FEAT_AVX2},
{"-O2 -std=c++17 -mavx512f","-lm",RAF_FEAT_AVX512}},
/O3/ {{"-O3 -std=c++17 -funroll-loops","-lm",0},
{"-O3 -std=c++17 -msse4.2 -funroll-loops","-lm",RAF_FEAT_SSE4},
{"-O3 -std=c++17 -mavx2 -funroll-loops","-lm",RAF_FEAT_AVX2},
{"-O3 -std=c++17 -mavx512f -funroll-loops","-lm",RAF_FEAT_AVX512}},
/Os/ {{"-Os -std=c++17 -ffunction-sections","--gc-sections -lm",0},
{"-Os -std=c++17 -msse4.2 -ffunction-sections","--gc-sections -lm",RAF_FEAT_SSE4},
{"-Os -std=c++17 -mavx2 -ffunction-sections","--gc-sections -lm",RAF_FEAT_AVX2},
{"-Os -std=c++17 -mavx512f -ffunction-sections","--gc-sections -lm",RAF_FEAT_AVX512}},
};

static const RafFlagCell _F_X86_RS[RAF_OPT_N][4] = {
/O0/ {{"opt-level=0 debuginfo=2","",0},
{"opt-level=0 target-feature=+sse4.2","",RAF_FEAT_SSE4},
{"opt-level=0 target-feature=+avx2","",RAF_FEAT_AVX2},
{"opt-level=0 target-feature=+avx512f","",RAF_FEAT_AVX512}},
/O1/ {{"opt-level=1","",0},{"opt-level=1 target-feature=+sse4.2","",RAF_FEAT_SSE4},
{"opt-level=1 target-feature=+avx2","",RAF_FEAT_AVX2},
{"opt-level=1 target-feature=+avx512f","",RAF_FEAT_AVX512}},
/O2/ {{"opt-level=2","",0},{"opt-level=2 target-feature=+sse4.2","",RAF_FEAT_SSE4},
{"opt-level=2 target-feature=+avx2","",RAF_FEAT_AVX2},
{"opt-level=2 target-feature=+avx512f","",RAF_FEAT_AVX512}},
/O3/ {{"opt-level=3","",0},{"opt-level=3 target-feature=+sse4.2","",RAF_FEAT_SSE4},
{"opt-level=3 target-feature=+avx2","",RAF_FEAT_AVX2},
{"opt-level=3 target-feature=+avx512f","",RAF_FEAT_AVX512}},
/Os/ {{"opt-level=s","",0},{"opt-level=s target-feature=+sse4.2","",RAF_FEAT_SSE4},
{"opt-level=s target-feature=+avx2","",RAF_FEAT_AVX2},
{"opt-level=s target-feature=+avx512f","",RAF_FEAT_AVX512}},
};

/* ARM64 C/C++ flags */
static const RafFlagCell _F_A64_C[RAF_OPT_N][4] = {
/O0/ {{"-O0 -g -std=c11 -march=armv8-a","-lm",0},
{"-O0 -g -std=c11 -march=armv8-a+crc","-lm",RAF_FEAT_CRC32},
{"-O0 -g -std=c11 -march=armv8-a+crypto","-lm",RAF_FEAT_NEON},
{"-O0 -g -std=c11 -march=armv9-a+sve2","-lm",RAF_FEAT_SVE}},
/O1/ {{"-O1 -std=c11 -march=armv8-a","-lm",0},
{"-O1 -std=c11 -march=armv8-a+crc","-lm",RAF_FEAT_CRC32},
{"-O1 -std=c11 -march=armv8-a+crypto+simd","-lm",RAF_FEAT_NEON},
{"-O1 -std=c11 -march=armv9-a+sve2","-lm",RAF_FEAT_SVE}},
/O2/ {{"-O2 -std=c11 -march=armv8-a -fomit-frame-pointer","-lm",0},
{"-O2 -std=c11 -march=armv8-a+crc","-lm",RAF_FEAT_CRC32},
{"-O2 -std=c11 -march=armv8-a+simd+crypto","-lm",RAF_FEAT_NEON},
{"-O2 -std=c11 -march=armv9-a+sve2","-lm",RAF_FEAT_SVE}},
/O3/ {{"-O3 -std=c11 -march=armv8-a -funroll-loops","-lm",0},
{"-O3 -std=c11 -march=armv8-a+crc -funroll-loops","-lm",RAF_FEAT_CRC32},
{"-O3 -std=c11 -march=armv8-a+simd -funroll-loops","-lm",RAF_FEAT_NEON},
{"-O3 -std=c11 -march=armv9-a+sve2 -funroll-loops","-lm",RAF_FEAT_SVE}},
/Os/ {{"-Os -std=c11 -march=armv8-a -ffunction-sections","--gc-sections -lm",0},
{"-Os -std=c11 -march=armv8-a+crc -ffunction-sections","--gc-sections -lm",RAF_FEAT_CRC32},
{"-Os -std=c11 -march=armv8-a+simd -ffunction-sections","--gc-sections -lm",RAF_FEAT_NEON},
{"-Os -std=c11 -march=armv9-a+sve2 -ffunction-sections","--gc-sections -lm",RAF_FEAT_SVE}},
};

/* JVM languages: Java/Kotlin — JVM flags */
static const RafFlagCell _F_JVM[RAF_OPT_N][4] = {
/O0/ {{"-Xss512k -Xmx256m","",0},
{"-Xss512k -Xmx512m -XX:+UseSSE42","",RAF_FEAT_SSE4},
{"-Xss512k -Xmx1g -XX:+UseAVX=2","",RAF_FEAT_AVX2},
{"-Xss1m -Xmx2g -XX:+UseAVX=3","",RAF_FEAT_AVX512}},
/O1/ {{"-Xss512k -Xmx512m -server","",0},
{"-Xss512k -Xmx512m -server -XX:+UseSSE42","",RAF_FEAT_SSE4},
{"-Xss512k -Xmx1g -server -XX:+UseAVX=2","",RAF_FEAT_AVX2},
{"-Xss1m -Xmx2g -server -XX:+UseAVX=3","",RAF_FEAT_AVX512}},
/O2/ {{"-Xss512k -Xmx512m -server -XX:+OptimizeStringConcat","",0},
{"-server -XX:+UseSSE42 -XX:+OptimizeStringConcat","",RAF_FEAT_SSE4},
{"-server -XX:+UseAVX=2 -XX:+OptimizeStringConcat","",RAF_FEAT_AVX2},
{"-server -XX:+UseAVX=3 -XX:+OptimizeStringConcat","",RAF_FEAT_AVX512}},
/O3/ {{"-server -XX:+AggressiveOpts -XX:+UseCompressedOops","",0},
{"-server -XX:+AggressiveOpts -XX:+UseSSE42","",RAF_FEAT_SSE4},
{"-server -XX:+AggressiveOpts -XX:+UseAVX=2","",RAF_FEAT_AVX2},
{"-server -XX:+AggressiveOpts -XX:+UseAVX=3","",RAF_FEAT_AVX512}},
/Os/ {{"-client -Xss256k -Xmx128m","",0},
{"-client -Xss256k -Xmx128m","",RAF_FEAT_SSE4},
{"-client -Xss256k -Xmx128m","",RAF_FEAT_AVX2},
{"-client -Xss256k -Xmx128m","",RAF_FEAT_AVX512}},
};

/* Python: no flags — CPython vs PyPy dispatch */
static const RafFlagCell _F_PY[RAF_OPT_N][4] = {
/O0/ {{"-B","",0},{"-B","",0},{"-B","",0},{"-B","",0}},
/O1/ {{"-O","",0},{"-O","",0},{"-O","",0},{"-O","",0}},
/O2/ {{"-OO","",0},{"-OO","",0},{"-OO","",0},{"-OO","",0}},
/O3/ {{"-OO -X no_debug_ranges","",0},{"-OO","",0},{"-OO","",0},{"-OO","",0}},
/Os/ {{"-OO","",0},{"-OO","",0},{"-OO","",0},{"-OO","",0}},
};

/* ── flag matrix lookup ────────────────────────────────────── /
/

Geometric dimension mapping:

feat_idx = popcount(feat & {SSE4,AVX2,AVX512,NEON,SVE}) clamped to 3

This maps the feature bitmask → the closest cell in the 4th dimension
*/
static int _feat_idx(uint32_t feat) {
int n=0;
if (feat & (RAF_FEAT_AVX512|RAF_FEAT_SVE))  return 3;
if (feat & (RAF_FEAT_AVX2))                  return 2;
if (feat & (RAF_FEAT_SSE4|RAF_FEAT_NEON))    return 1;
(void)n;
return 0;
}


void raf_flag_matrix_get(uint8_t arch, uint8_t lang,
uint8_t opt, uint32_t feat,
char *out, int cap) {
if (!out || cap < 2) return;
if (opt >= RAF_OPT_N) opt = RAF_OPT_2;
int fi = _feat_idx(feat);
const RafFlagCell *cell = 0;

/* select table by arch × lang */  
if (arch == RAF_ARCH_X86_64) {  
    switch (lang) {  
        case RAF_LANG_C:  cell = &_F_X86_C[opt][fi]; break;  
        case RAF_LANG_CPP:cell = &_F_X86_CPP[opt][fi]; break;  
        case RAF_LANG_RS: cell = &_F_X86_RS[opt][fi]; break;  
        case RAF_LANG_JAVA:  
        case RAF_LANG_KT: cell = &_F_JVM[opt][fi]; break;  
        case RAF_LANG_PY: cell = &_F_PY[opt][fi]; break;  
        default: /* S: pass-through */  
            snprintf(out,cap,"-O2 -march=native"); return;  
    }  
} else if (arch == RAF_ARCH_ARM64) {  
    switch (lang) {  
        case RAF_LANG_C:  
        case RAF_LANG_CPP: cell = &_F_A64_C[opt][fi]; break;  
        case RAF_LANG_RS:  cell = &_F_X86_RS[opt][fi]; break;/*rustc handles target*/  
        case RAF_LANG_JAVA:  
        case RAF_LANG_KT:  cell = &_F_JVM[opt][fi]; break;  
        case RAF_LANG_PY:  cell = &_F_PY[opt][fi]; break;  
        default: snprintf(out,cap,"-O2 -march=armv8-a"); return;  
    }  
} else {  
    snprintf(out,cap,"-O2"); return;  
}  

if (cell && cell->flags)  
    snprintf(out,cap,"%s",cell->flags);  
else  
    snprintf(out,cap,"-O2");

}

uint8_t raf_lang_from_ext(const char *path) {
if (!path) return RAF_LANG_C;
const char *p = path + strlen(path);
while (p > path && p != '.') p--;
if      (!strcmp(p,".c"))    return RAF_LANG_C;
else if (!strcmp(p,".cpp") ||
!strcmp(p,".cc") ||
!strcmp(p,".cxx"))  return RAF_LANG_CPP;
else if (!strcmp(p,".s") ||
!strcmp(p,".S") ||
!strcmp(p,".asm"))  return RAF_LANG_S;
else if (!strcmp(p,".py"))   return RAF_LANG_PY;
else if (!strcmp(p,".rs"))   return RAF_LANG_RS;
else if (!strcmp(p,".kt"))   return RAF_LANG_KT;
else if (!strcmp(p,".java")) return RAF_LANG_JAVA;
return RAF_LANG_C; / default */
}

void raf_ctx_init(RafCtx *ctx) {
memset(ctx,0,sizeof(*ctx));
raf_cpu_detect(&ctx->cpu);
ctx->arch = ctx->cpu.arch;
ctx->feat = ctx->cpu.feat;
ctx->opt  = RAF_OPT_2;
}
