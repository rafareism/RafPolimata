/*

RAFcodding — raf_frontend.c

Language frontend dispatcher:

C/C++/S  → invoke gcc/g++/as with optimal flags

Rust     → invoke rustc with cargo-free flags

Kotlin   → invoke kotlinc

Java     → invoke javac + extract bytecode

Python   → invoke python3 -m py_compile → .pyc hex dump

All via execvp — zero subprocess library

Also: raf_ctx_report (stats + hex dump)
*/
#include "raf_compile.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>


/* ── time helper ──────────────────────────────────────── /
static uint64_t _ns(void) {
struct timeval tv; gettimeofday(&tv,0);
return (uint64_t)tv.tv_sec1000000000ULL + (uint64_t)tv.tv_usec*1000ULL;
}

/* ── fork+exec helper ─────────────────────────────────── */
static int _run(const char *argv[]) {
pid_t pid = fork();
if (pid<0) return -1;
if (pid==0) {
execvp(argv[0], (char const)argv);
_exit(127);
}
int st=0; waitpid(pid,&st,0);
return WIFEXITED(st) ? WEXITSTATUS(st) : -1;
}

/* ── ASM text → .s file ───────────────────────────────── */
static int _write_asm(RafCtx *ctx) {
if (!ctx->out_asm[0]) snprintf(ctx->out_asm,256,"_raf_out.s");
FILE *f=fopen(ctx->out_asm,"w");
if (!f) return -1;
for (uint64_t i=0;i<ctx->asm_lines;i++)
fprintf(f,"%s\n",ctx->asm_out.lines[i]);
fclose(f);
return 0;
}

/* ── HEX text → .hex file ─────────────────────────────── */
static int _write_hex(RafCtx *ctx) {
if (!ctx->out_hex[0]) snprintf(ctx->out_hex,256,"_raf_out.hex");
FILE *f=fopen(ctx->out_hex,"w");
if (!f) return -1;
for (uint64_t i=0;i<ctx->hex_bytes;i++) {
fprintf(f,"%02X",ctx->bin.bytes[i]);
if ((i&0xF)==0xF) fputc('\n',f);
else              fputc(' ',f);
}
fputc('\n',f);
fclose(f);
return 0;
}

/* ── binary → .bin file ───────────────────────────────── */
static int _write_bin(RafCtx *ctx) {
if (!ctx->out_bin[0]) snprintf(ctx->out_bin,256,"_raf_out.bin");
FILE *f=fopen(ctx->out_bin,"wb");
if (!f) return -1;
fwrite(ctx->bin.bytes,1,(size_t)ctx->hex_bytes,f);
fclose(f);
return 0;
}

/* ── read source file into ctx->src (mmap-style, no malloc) ─ /
/ Uses static buffer: 4MB max source /
static char _src_buf[41024*1024];
static int _read_src(RafCtx *ctx, const char *path) {
FILE *f=fopen(path,"rb");
if (!f) { fprintf(stderr,"[raf] cannot open: %s\n",path); return -1; }
size_t nr=fread(_src_buf,1,sizeof(_src_buf)-1,f);
fclose(f);
_src_buf[nr]=0;
ctx->src     = _src_buf;
ctx->src_len = nr;
return 0;
}

/* ════════════════════════════════════════════════════════

C / C++ frontend

Full pipeline: source → precomp → IR → ASM → HEX

optionally invoke gcc/g++ for native object


════════════════════════════════════════════════════════ */
static int _frontend_c(RafCtx *ctx, const char *src_path,
const char *out_base, int do_native) {
uint64_t t0=_ns();
ctx->lang = raf_lang_from_ext(src_path);

if (_read_src(ctx,src_path)<0) return -1;

/* IR */
if (raf_precompile(ctx)<0) return -2;

/* ASM emit */
raf_asm_emit(ctx);
snprintf(ctx->out_asm,256,"%s.s",out_base);
_write_asm(ctx);

/* HEX */
raf_hex_encode(ctx);
snprintf(ctx->out_hex,256,"%s.hex",out_base);
snprintf(ctx->out_bin,256,"%s.bin",out_base);
_write_hex(ctx);
_write_bin(ctx);

/* native compile via gcc if requested */
if (do_native) {
char flags[512]={0};
raf_flag_matrix_get(ctx->cpu.arch, ctx->lang,
ctx->opt, ctx->feat, flags, sizeof(flags));

/* build argv array — no alloc, static local */  
 static char _flag_copy[512];  
 strncpy(_flag_copy,flags,511);  
 char *argv_buf[64]; int argc=0;  

 argv_buf[argc++] = (ctx->lang==RAF_LANG_CPP) ? "g++" : "gcc";  
 /* split flags by space */  
 char *tok=_flag_copy, *p=_flag_copy;  
 while (*p) {  
     if (*p==' ') {  
         *p=0; if(tok!=p) argv_buf[argc++]=tok;  
         tok=p+1;  
     }  
     p++;  
 }  
 if (*tok) argv_buf[argc++]=tok;  
 /* add source and output */  
 static char _out_obj[256];  
 snprintf(_out_obj,256,"%s.o",out_base);  
 argv_buf[argc++]=(char*)"-c";  
 argv_buf[argc++]=(char*)src_path;  
 argv_buf[argc++]=(char*)"-o";  
 argv_buf[argc++]=_out_obj;  
 argv_buf[argc]  =NULL;  

 int r=_run((const char**)argv_buf);  
 fprintf(stderr,"[raf] gcc/g++ exit=%d  obj=%s\n",r,_out_obj);

}

ctx->elapsed_ns = _ns()-t0;
return 0;
}


/* ════════════════════════════════════════════════════════

ASM (.s) frontend — direct assemble → hex

════════════════════════════════════════════════════════ */
static int _frontend_s(RafCtx *ctx, const char *src_path,
const char *out_base) {
uint64_t t0=_ns();
ctx->lang=RAF_LANG_S;
static char _obj[256], _bin_local[256];
snprintf(_obj,256,"%s.o",out_base);
snprintf(_bin_local,256,"%s.bin",out_base);

/* as → object */
const char *argv_as[]={
"as", src_path, "-o", _obj, NULL
};
int r=_run(argv_as);
fprintf(stderr,"[raf] as exit=%d  obj=%s\n",r,_obj);

/* objcopy → binary for hex dump */
const char *argv_oc[]={
"objcopy","-O","binary","--only-section=.text",
_obj, _bin_local, NULL
};
_run(argv_oc);

/* read binary → hex output */
FILE *f=fopen(_bin_local,"rb");
if (f) {
ctx->bin.n=0;
ctx->hex_bytes=fread(ctx->bin.bytes,1,RAF_HEX_CAP,f);
ctx->bin.n=(uint32_t)ctx->hex_bytes;
fclose(f);
snprintf(ctx->out_hex,256,"%s.hex",out_base);
_write_hex(ctx);
}
ctx->elapsed_ns=_ns()-t0;
return r;
}


/* ════════════════════════════════════════════════════════

Rust frontend

rustc with --emit=asm,llvm-ir,obj + optimal flags

════════════════════════════════════════════════════════ */
static int _frontend_rs(RafCtx *ctx, const char *src_path,
const char *out_base) {
uint64_t t0=_ns();
ctx->lang=RAF_LANG_RS;
char flags[512]={0};
raf_flag_matrix_get(ctx->cpu.arch,RAF_LANG_RS,ctx->opt,ctx->feat,
flags,sizeof(flags));

/* target triple */
const char *triple = (ctx->cpu.arch==RAF_ARCH_ARM64)
? "aarch64-unknown-linux-gnu"
: "x86_64-unknown-linux-gnu";

static char _cg[512], _out_asm[256], _out_obj[256];
snprintf(_cg,sizeof(_cg),"target-cpu=native,%s",flags);
snprintf(_out_asm,256,"%s.rs.s",out_base);
snprintf(_out_obj, 256,"%s.rs.o",out_base);

const char *argv[]={
"rustc",
"--edition","2021",
"--crate-type","cdylib",
"--target",triple,
"-C",_cg,
"--emit","asm,obj",
"-o",_out_obj,
src_path, NULL
};
int r=_run(argv);
fprintf(stderr,"[raf] rustc exit=%d  asm=%s\n",r,_out_asm);
ctx->elapsed_ns=_ns()-t0;
return r;
}


/* ════════════════════════════════════════════════════════

Kotlin frontend

kotlinc → .jar → d8 → dex → hex

════════════════════════════════════════════════════════ */
static int _frontend_kt(RafCtx *ctx, const char *src_path,
const char *out_base) {
uint64_t t0=_ns();
ctx->lang=RAF_LANG_KT;
static char _jar[256], _dex[256];
snprintf(_jar,256,"%s.jar",out_base);
snprintf(_dex,256,"%s_dex",out_base);

char jvm_flags[256];
raf_flag_matrix_get(ctx->cpu.arch,RAF_LANG_KT,ctx->opt,ctx->feat,
jvm_flags,sizeof(jvm_flags));

const char *argv_kc[]={
"kotlinc","-include-runtime",
"-jvm-target","17",
src_path,"-output",_jar,NULL
};
int r=_run(argv_kc);
fprintf(stderr,"[raf] kotlinc exit=%d  jar=%s\n",r,_jar);

/* d8 (Android dex) if available */
const char *argv_d8[]={
"d8","--min-api","26",_jar,"--output",_dex,NULL
};
int rd=_run(argv_d8);
fprintf(stderr,"[raf] d8 exit=%d  dex=%s\n",rd,_dex);

ctx->elapsed_ns=_ns()-t0;
return r;
}


/* ════════════════════════════════════════════════════════

Java frontend

javac → .class → javap -c → hex dump of bytecode

════════════════════════════════════════════════════════ */
static int _frontend_java(RafCtx *ctx, const char *src_path,
const char *out_base) {
uint64_t t0=_ns();
ctx->lang=RAF_LANG_JAVA;
static char _cls_dir[256];
snprintf(_cls_dir,256,"%s_cls",out_base);

/* mkdir */
const char *argv_mkdir[]={"mkdir","-p",_cls_dir,NULL};
_run(argv_mkdir);

const char *argv_jc[]={
"javac",
"--release","17",
"-d",_cls_dir,
src_path,NULL
};
int r=_run(argv_jc);
fprintf(stderr,"[raf] javac exit=%d  cls=%s\n",r,_cls_dir);

ctx->elapsed_ns=_ns()-t0;
return r;
}


/* ════════════════════════════════════════════════════════

Python frontend

python3 -m py_compile → .pyc → hex dump

════════════════════════════════════════════════════════ */
static int _frontend_py(RafCtx *ctx, const char *src_path,
const char *out_base) {
uint64_t t0=_ns();
ctx->lang=RAF_LANG_PY;
char opt_flag[8];
snprintf(opt_flag,8,"-%s", ctx->opt==RAF_OPT_0 ? "B" :
ctx->opt<=RAF_OPT_2 ? "O" : "OO");
const char *argv[]={
"python3",opt_flag,"-m","py_compile",src_path,NULL
};
int r=_run(argv);
fprintf(stderr,"[raf] py_compile exit=%d\n",r);
(void)out_base;
ctx->elapsed_ns=_ns()-t0;
return r;
}


/* ════════════════════════════════════════════════════════

PUBLIC: raf_ir_lower

Decides which frontend to run based on lang.

Also does full IR→ASM→HEX pipeline for C/C++/S.

════════════════════════════════════════════════════════ */
int raf_ir_lower(RafCtx ctx) {
/ For non-C langs, IR is empty — just flag dispatch */
return 0;
}


/* ════════════════════════════════════════════════════════

raf_ctx_report — full stats + hex preview

════════════════════════════════════════════════════════ */
static const char *_arch_str(uint8_t a) {
switch(a){
case RAF_ARCH_X86_64: return "x86_64";
case RAF_ARCH_ARM64:  return "aarch64";
case RAF_ARCH_RV64:   return "riscv64";
default:              return "unknown";
}
}
static const char *_lang_str(uint8_t l) {
static const char *T[]={"C","C++","ASM","Python","Rust","Kotlin","Java","?"};
return l<7?T[l]:T[7];
}
static const char *_opt_str(uint8_t o) {
static const char *T[]={"O0","O1","O2","O3","Os","?"};
return o<5?T[o]:T[5];
}


void raf_ctx_report(const RafCtx *ctx) {
printf("\n");
printf("┌──────────────────────────────────────────────────┐\n");
printf("│  RAFcodding Compiler Report  ·  Ω = Amor          │\n");
printf("├──────────────────────────────────────────────────┤\n");
printf("│  CPU   : %-40s│\n", ctx->cpu.brand);
printf("│  Arch  : %-8s  Cores:%-4u  Cache-line:%-4u B │\n",
_arch_str(ctx->cpu.arch), ctx->cpu.cores, ctx->cpu.cache_line);
printf("│  L1d:%-4uKB  L2:%-6uKB  L3:%-8uKB          │\n",
ctx->cpu.l1d_kb, ctx->cpu.l2_kb, ctx->cpu.l3_kb);
printf("│  GPR:%-4u  VEC_w:%-4uB  VEC_n:%-4u             │\n",
ctx->cpu.gpr_count, ctx->cpu.vec_width, ctx->cpu.vec_count);
printf("├──────────────────────────────────────────────────┤\n");
printf("│  Lang  : %-8s  Opt:%-4s                       │\n",
_lang_str(ctx->lang), _opt_str(ctx->opt));
printf("│  Features:");
if (ctx->feat & RAF_FEAT_SSE4)   printf(" SSE4.2");
if (ctx->feat & RAF_FEAT_AVX2)   printf(" AVX2");
if (ctx->feat & RAF_FEAT_AVX512) printf(" AVX512");
if (ctx->feat & RAF_FEAT_CRC32)  printf(" CRC32");
if (ctx->feat & RAF_FEAT_NEON)   printf(" NEON");
if (ctx->feat & RAF_FEAT_SVE)    printf(" SVE");
if (!ctx->feat)                   printf(" none");
printf("\n");
printf("├──────────────────────────────────────────────────┤\n");
printf("│  IR nodes  : %-8llu                              │\n",
(unsigned long long)ctx->ir_nodes);
printf("│  ASM lines : %-8llu                              │\n",
(unsigned long long)ctx->asm_lines);
printf("│  Hex bytes : %-8llu  (%.1f KB)                   │\n",
(unsigned long long)ctx->hex_bytes,
(double)ctx->hex_bytes/1024.0);
printf("│  Elapsed   : %-8llu ns (%.3f ms)                │\n",
(unsigned long long)ctx->elapsed_ns,
(double)ctx->elapsed_ns/1e6);
printf("├──────────────────────────────────────────────────┤\n");

/* Flag matrix preview */  
char flags[512]={0};  
raf_flag_matrix_get(ctx->cpu.arch,ctx->lang,ctx->opt,ctx->feat,  
                    flags,sizeof(flags));  
printf("│  FLAGS: %.49s │\n",flags);  
if (strlen(flags)>49)  
    printf("│         %.49s │\n",flags+49);  

printf("├──────────────────────────────────────────────────┤\n");  
/* HEX preview: first 32 bytes */  
printf("│  HEX preview (first 32B):                        │\n│  ");  
uint32_t lim = ctx->bin.n < 32 ? ctx->bin.n : 32;  
for (uint32_t i=0;i<lim;i++) {  
    printf("%02X ",ctx->bin.bytes[i]);  
    if ((i&0xF)==0xF && i+1<lim) printf("\n│  ");  
}  
printf("\n");  
printf("└──────────────────────────────────────────────────┘\n");  
printf("  ASM  → %s\n", ctx->out_asm);  
printf("  HEX  → %s\n", ctx->out_hex);  
printf("  BIN  → %s\n", ctx->out_bin);  
printf("\n");

}

/* ── expose frontend dispatcher ─────────────────────── */
int raf_compile_file(RafCtx *ctx, const char *src_path,
const char *out_base, int do_native) {
uint8_t lang = raf_lang_from_ext(src_path);
ctx->lang = lang;
switch(lang) {
case RAF_LANG_C:
case RAF_LANG_CPP:  return _frontend_c(ctx,src_path,out_base,do_native);
case RAF_LANG_S:    return _frontend_s(ctx,src_path,out_base);
case RAF_LANG_RS:   return _frontend_rs(ctx,src_path,out_base);
case RAF_LANG_KT:   return _frontend_kt(ctx,src_path,out_base);
case RAF_LANG_JAVA: return _frontend_java(ctx,src_path,out_base);
case RAF_LANG_PY:   return _frontend_py(ctx,src_path,out_base);
default:            return _frontend_c(ctx,src_path,out_base,do_native);
}
}
