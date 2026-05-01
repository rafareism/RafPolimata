/*

RAFcodding — raf_compile.h

Master types: zero abstractions, register-level geometry

Ω = Amor · ∆RafaelVerboΩ · ΣΩΔΦBITRAF

DESIGN:

Flag matrix F[arch][lang][opt][feat]  — 4D tensor

IR node: opcode(8) | dst_reg(5) | src0(5) | src1(5) | imm(39)

All names are indices — no string vars on hot path
*/
#ifndef RAF_COMPILE_H
#define RAF_COMPILE_H
#include <stdint.h>
#include <stddef.h>


/* ── arch ids ─────────────────────────── */
#define RAF_ARCH_X86_64  0
#define RAF_ARCH_ARM64   1
#define RAF_ARCH_RV64    2
#define RAF_ARCH_UNKNOWN 3
#define RAF_ARCH_N       4

/* ── language ids ────────────────────── /
#define RAF_LANG_C    0
#define RAF_LANG_CPP  1
#define RAF_LANG_S    2   / raw ASM */
#define RAF_LANG_PY   3
#define RAF_LANG_RS   4
#define RAF_LANG_KT   5
#define RAF_LANG_JAVA 6
#define RAF_LANG_N    7

/* ── opt levels ──────────────────────── /
#define RAF_OPT_0  0   / debug: no opt /
#define RAF_OPT_1  1   / basic   /
#define RAF_OPT_2  2   / default /
#define RAF_OPT_3  3   / aggressive /
#define RAF_OPT_S  4   / size   */
#define RAF_OPT_N  5

/* ── feature flags (bitmask dimension) ─ */
#define RAF_FEAT_NONE    0x00u
#define RAF_FEAT_SSE4    0x01u
#define RAF_FEAT_AVX2    0x02u
#define RAF_FEAT_AVX512  0x04u
#define RAF_FEAT_CRC32   0x08u
#define RAF_FEAT_NEON    0x10u
#define RAF_FEAT_SVE     0x20u
#define RAF_FEAT_LTO     0x40u
#define RAF_FEAT_PIE     0x80u
#define RAF_FEAT_N       8

/* ── register maps ───────────────────── /
/ x86-64: 16 GPR + 16 XMM encoded as index 0..31 /
#define R_X86_RAX  0
#define R_X86_RCX  1
#define R_X86_RDX  2
#define R_X86_RBX  3
#define R_X86_RSP  4
#define R_X86_RBP  5
#define R_X86_RSI  6
#define R_X86_RDI  7
#define R_X86_R8   8
#define R_X86_R9   9
#define R_X86_R10  10
#define R_X86_R11  11
#define R_X86_R12  12
#define R_X86_R13  13
#define R_X86_R14  14
#define R_X86_R15  15
#define R_X86_XMM0 16
#define R_X86_XMM1 17
#define R_X86_XMM2 18
#define R_X86_XMM3 19
/ ARM64: X0..X30, SP=31, XZR=31 (context-dep), V0..V31 = 32..63 /
#define R_A64_X0   0
#define R_A64_X1   1
#define R_A64_X2   2
#define R_A64_X3   3
#define R_A64_X4   4
#define R_A64_X5   5
#define R_A64_X8   8    / syscall nr /
#define R_A64_X16  16   / ip0 /
#define R_A64_X17  17   / ip1 /
#define R_A64_X29  29   / fp  /
#define R_A64_X30  30   / lr  */
#define R_A64_SP   31
#define R_A64_V0   32
#define R_A64_V1   33

/* ── IR opcode set (64 ops) ──────────── /
typedef enum {
IR_NOP=0, IR_MOV, IR_MOVIMM, IR_LOAD, IR_STORE,
IR_ADD, IR_SUB, IR_MUL, IR_DIV, IR_MOD,
IR_AND, IR_OR,  IR_XOR, IR_NOT, IR_SHL, IR_SHR,
IR_CMP, IR_JMP, IR_JEQ, IR_JNE, IR_JLT, IR_JGE,
IR_CALL, IR_RET, IR_PUSH, IR_POP,
IR_FADD, IR_FSUB, IR_FMUL, IR_FDIV,
IR_CRC32, IR_HASH, IR_PHI,
/ RAFAELIA geometric ops /
IR_SPIRAL,    / dst = src * (sqrt3/2)^n /
IR_FIBONACCI, / dst = fibonacci_rafael(n) /
IR_TOROID,    / maps {x,y,z} → torus surface /
IR_LORENZ,    / one RK4 Lorenz step /
IR_EOF_OP     / sentinel */
} RafIROp;

/* ── IR node (64-bit packed) ─────────── /
/  bits 63..56: opcode (8)

bits 55..51: dst   (5) — register index

bits 50..46: src0  (5)

bits 45..41: src1  (5)

bits 40..0:  imm39 (39) — immediate / label offset
*/
typedef uint64_t RafIR;


#define RAF_IR_PACK(op,dst,s0,s1,imm) \
( ((uint64_t)(op)  << 56) \
| ((uint64_t)(dst) << 51) \
| ((uint64_t)(s0)  << 46) \
| ((uint64_t)(s1)  << 41) \
| ((uint64_t)((imm)&0x7FFFFFFFFFull)) )
#define RAF_IR_OP(ir)   (((ir)>>56)&0xFF)
#define RAF_IR_DST(ir)  (((ir)>>51)&0x1F)
#define RAF_IR_SRC0(ir) (((ir)>>46)&0x1F)
#define RAF_IR_SRC1(ir) (((ir)>>41)&0x1F)
#define RAF_IR_IMM(ir)  ((ir)&0x7FFFFFFFFFull)

/* ── Flag entry ──────────────────────── /
/ One cell of the 4D flag matrix */
typedef struct {
const char flags;   / compiler flags string */
const char linker;  / linker flags /
uint32_t    feat_req;/ required feature bitmask */
} RafFlagCell;

/* ── CPU info ────────────────────────── /
typedef struct {
uint8_t  arch;        / RAF_ARCH_* /
uint8_t  vendor;      / 0=intel 1=amd 2=arm 3=apple 4=riscv /
uint8_t  family;
uint8_t  model;
uint32_t feat;        / RAF_FEAT_* bitmask /
uint32_t l1d_kb;
uint32_t l2_kb;
uint32_t l3_kb;
uint32_t cores;
uint32_t cache_line;  / bytes /
char     brand[48];
/ register file geometry /
uint8_t  gpr_count;   / available GPRs /
uint8_t  vec_width;   / SIMD width: 16/32/64 bytes /
uint8_t  vec_count;   / SIMD registers */
} RafCPU;

/* ── Precompiler token ───────────────── /
typedef struct {
uint8_t  type;   / 0=PUNCT 1=NUM 2=IDENT 3=STR 4=OP 5=KW 6=EOF */
uint8_t  lang;
uint16_t len;
uint32_t line;
const char ptr; / pointer into source (no copy) */
} RafTok;

/* ── IR buffer ───────────────────────── /
#define RAF_IR_CAP (1<<17)  / 128K nodes, 1MB */
typedef struct {
RafIR    buf[RAF_IR_CAP];
uint32_t n;
uint8_t  arch;
uint8_t  opt;
uint32_t feat;
} RafIRBuf;

/* ── ASM line ────────────────────────── */
#define RAF_ASM_CAP  (1<<18)
#define RAF_ASM_LINE 128
typedef struct {
char     lines[RAF_ASM_CAP][RAF_ASM_LINE];
uint32_t n;
} RafAsmBuf;

/* ── HEX / binary output ─────────────── /
#define RAF_HEX_CAP (1<<20)  / 1MB hex output */
typedef struct {
uint8_t  bytes[RAF_HEX_CAP];
uint32_t n;
} RafBin;

/* ── top-level context ───────────────── /
typedef struct {
RafCPU    cpu;
uint8_t   lang;
uint8_t   opt;
uint32_t  feat;
RafIRBuf  ir;
RafAsmBuf asm_out;
RafBin    bin;
/ source */
const char src;
size_t      src_len;
/ output paths /
char  out_asm[256];
char  out_hex[256];
char  out_bin[256];
/ stats */
uint64_t ir_nodes;
uint64_t asm_lines;
uint64_t hex_bytes;
uint64_t elapsed_ns;
} RafCtx;

/* ── public API ──────────────────────── */
void     raf_cpu_detect(RafCPU *cpu);
uint8_t  raf_lang_from_ext(const char *path);
void     raf_flag_matrix_get(uint8_t arch, uint8_t lang,
uint8_t opt, uint32_t feat,
char *out_flags, int cap);
int      raf_precompile(RafCtx *ctx);
int      raf_ir_lower(RafCtx *ctx);
int      raf_asm_emit(RafCtx *ctx);
int      raf_hex_encode(RafCtx *ctx);
void     raf_ctx_init(RafCtx *ctx);
void     raf_ctx_report(const RafCtx *ctx);

#endif /* RAF_COMPILE_H */
