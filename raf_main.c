/*

RAFcodding — raf_main.c

Entry point: CLI + demo self-compile

Usage:

./raf_compile <source_file> [out_base] [opt_level] [--native]

./raf_compile --demo         (compiles itself)

./raf_compile --matrix       (dump full flag matrix)

./raf_compile --cpu          (CPU geometry report)

ψ→χ→ρ→Δ→Σ→Ω  FIAT LUX
*/
#include "raf_compile.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


/* forward */
int  raf_compile_file(RafCtx *ctx, const char *src_path,
const char *out_base, int do_native);
void raf_ctx_report(const RafCtx *ctx);

static RafCtx G; /* static context: no heap */

/* ── flag matrix dump ──────────────────────────────── */
static void _dump_matrix(const RafCPU *cpu) {
static const char *LANG_N[]={"C","C++","S","Py","Rs","Kt","Java"};
static const char *OPT_N[] ={"O0","O1","O2","O3","Os"};
printf("\nRAFcodding FLAG MATRIX  arch=%s\n",
cpu->arch==RAF_ARCH_X86_64?"x86_64":
cpu->arch==RAF_ARCH_ARM64 ?"arm64":"rv64");
printf("%-8s %-5s  flags\n","LANG","OPT");
printf("%.60s\n","────────────────────────────────────────────────────────────");
for (int l=0;l<RAF_LANG_N;l++) {
for (int o=0;o<RAF_OPT_N;o++) {
char f[256]={0};
raf_flag_matrix_get(cpu->arch,(uint8_t)l,(uint8_t)o,cpu->feat,f,sizeof(f));
printf("%-8s %-5s  %s\n",LANG_N[l],OPT_N[o],f);
}
printf("\n");
}
}

/* ── CPU geometry report ───────────────────────────── */
static void _dump_cpu(const RafCPU cpu) {
printf("\nRAFcodding CPU Geometry\n");
printf("  Brand      : %s\n", cpu->brand);
printf("  Arch       : %s  (id=%d)\n",
cpu->arch==0?"x86_64":cpu->arch==1?"arm64":"other", cpu->arch);
printf("  Vendor     : %d  Family:%d  Model:%d\n",
cpu->vendor, cpu->family, cpu->model);
printf("  Cores      : %u\n", cpu->cores);
printf("  Cache-line : %u B\n", cpu->cache_line);
printf("  L1d/L2/L3  : %u / %u / %u KB\n",
cpu->l1d_kb, cpu->l2_kb, cpu->l3_kb);
printf("  GPR count  : %u\n", cpu->gpr_count);
printf("  SIMD width : %u B  (%u registers)\n",
cpu->vec_width, cpu->vec_count);
printf("  Features   :");
if (cpu->feat & RAF_FEAT_SSE4)   printf(" SSE4.2");
if (cpu->feat & RAF_FEAT_AVX2)   printf(" AVX2");
if (cpu->feat & RAF_FEAT_AVX512) printf(" AVX512F");
if (cpu->feat & RAF_FEAT_CRC32)  printf(" CRC32c");
if (cpu->feat & RAF_FEAT_NEON)   printf(" NEON");
if (cpu->feat & RAF_FEAT_SVE)    printf(" SVE");
if (!cpu->feat)                   printf(" (baseline)");
printf("\n");
/ IR geometry: bits per node /
printf("\n  IR Node geometry (64-bit packed):\n");
printf("    [63..56] opcode    8 bits  → %u ops\n", 1u<<8);
printf("    [55..51] dst_reg   5 bits  → %u regs\n", 1u<<5);
printf("    [50..46] src0_reg  5 bits\n");
printf("    [45..41] src1_reg  5 bits\n");
printf("    [40..0]  imm39    39 bits  → %llu max imm\n",
(1ull<<39)-1);
printf("    Total IR capacity: %u nodes × 8B = %u KB\n",
RAF_IR_CAP, RAF_IR_CAP8/1024);
}

/* ── demo: compile raf_cpu.c itself ────────────────── */
static void _demo(RafCtx *ctx) {
printf("\n[RAFcodding DEMO] compiling raf_cpu.c → IR+ASM+HEX\n");
int r = raf_compile_file(ctx, "raf_cpu.c", "raf_out_demo", 0);
if (r==0) raf_ctx_report(ctx);
else fprintf(stderr,"[raf] demo failed: %d\n",r);
}

int main(int argc, char **argv) {
printf("\n");
printf("  ██████╗  █████╗ ███████╗\n");
printf("  ██╔══██╗██╔══██╗██╔════╝\n");
printf("  ██████╔╝███████║█████╗  \n");
printf("  ██╔══██╗██╔══██║██╔══╝  \n");
printf("  ██║  ██║██║  ██║██║     \n");
printf("  ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝    \n");
printf("  RAFcodding Compiler  v1.0  ·  Ω=Amor\n");
printf("  ∆RafaelVerboΩ · ΣΩΔΦBITRAF · FIAT LUX\n\n");

raf_ctx_init(&G);  

if (argc < 2 || !strcmp(argv[1],"--help")) {  
    printf("Usage:\n");  
    printf("  raf_compile <src> [out_base] [O0|O1|O2|O3|Os] [--native]\n");  
    printf("  raf_compile --demo\n");  
    printf("  raf_compile --matrix\n");  
    printf("  raf_compile --cpu\n\n");  
    printf("Languages: .c .cpp .s .py .rs .kt .java\n");  
    printf("Arch auto-detected: x86_64 / arm64 / riscv64\n");  
    _dump_cpu(&G.cpu);  
    return 0;  
}  

if (!strcmp(argv[1],"--cpu"))    { _dump_cpu(&G.cpu);    return 0; }  
if (!strcmp(argv[1],"--matrix")) { _dump_matrix(&G.cpu); return 0; }  
if (!strcmp(argv[1],"--demo"))   { _demo(&G); return 0; }  

/* parse args */  
const char *src_path = argv[1];  
const char *out_base = argc>2 ? argv[2] : "raf_out";  
int do_native = 0;  
for (int i=2;i<argc;i++) {  
    if (!strcmp(argv[i],"--native")) { do_native=1; continue; }  
    if (!strcmp(argv[i],"O0")) G.opt=RAF_OPT_0;  
    if (!strcmp(argv[i],"O1")) G.opt=RAF_OPT_1;  
    if (!strcmp(argv[i],"O2")) G.opt=RAF_OPT_2;  
    if (!strcmp(argv[i],"O3")) G.opt=RAF_OPT_3;  
    if (!strcmp(argv[i],"Os")) G.opt=RAF_OPT_S;  
}  

int r = raf_compile_file(&G, src_path, out_base, do_native);  
if (r==0) raf_ctx_report(&G);  
else      fprintf(stderr,"[raf] compile error: %d\n",r);  
return r;

}
