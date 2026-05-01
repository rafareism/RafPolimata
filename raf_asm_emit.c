/*

RAFcodding — raf_asm_emit.c

IR → x86-64 AT&T ASM text + ARM64 text

raw hex byte encoder (x86-64 opcode table)


Zero alloc — writes into RafAsmBuf/RafBin static arrays
*/
#include "raf_compile.h"
#include <string.h>
#include <stdio.h>


/* ── x86-64 register names ─── */
static const char *_x86_reg64[32] = {
"%rax","%rcx","%rdx","%rbx","%rsp","%rbp","%rsi","%rdi",
"%r8", "%r9", "%r10","%r11","%r12","%r13","%r14","%r15",
"%xmm0","%xmm1","%xmm2","%xmm3",
"%xmm4","%xmm5","%xmm6","%xmm7",
"%xmm8","%xmm9","%xmm10","%xmm11",
"%xmm12","%xmm13","%xmm14","%xmm15"
};
static const char *_x86_reg32[16] = {
"%eax","%ecx","%edx","%ebx","%esp","%ebp","%esi","%edi",
"%r8d","%r9d","%r10d","%r11d","%r12d","%r13d","%r14d","%r15d"
};

/* ── ARM64 register names ─── */
static const char *_a64_xreg[32] = {
"x0","x1","x2","x3","x4","x5","x6","x7",
"x8","x9","x10","x11","x12","x13","x14","x15",
"x16","x17","x18","x19","x20","x21","x22","x23",
"x24","x25","x26","x27","x28","x29","x30","sp"
};
static const char *_a64_vreg[32] = {
"v0","v1","v2","v3","v4","v5","v6","v7",
"v8","v9","v10","v11","v12","v13","v14","v15",
"v16","v17","v18","v19","v20","v21","v22","v23",
"v24","v25","v26","v27","v28","v29","v30","v31"
};

/* helper: append one asm line */
static void _aline(RafAsmBuf *b, const char fmt, ...) {
if (b->n >= RAF_ASM_CAP) return;
va_list ap;
va_start(ap, fmt);
vsnprintf(b->lines[b->n++], RAF_ASM_LINE, fmt, ap);
va_end(ap);
}
/ need va_list */
#include <stdarg.h>

/* ════════════════════════════════════════════════════

x86-64 AT&T ASM EMITTER

════════════════════════════════════════════════════ */
static void _emit_x86(RafAsmBuf *b, const RafIR *ir, uint32_t n,
uint32_t feat) {
_aline(b, "\t.section .text");
_aline(b, "\t.globl _raf_entry");
_aline(b, "\t.type  _raf_entry, @function");
_aline(b, "_raf_entry:");
(void)feat;

for (uint32_t i=0; i<n; i++) {
uint64_t node = ir[i];
RafIROp  op   = (RafIROp)RAF_IR_OP(node);
uint8_t  dst  = RAF_IR_DST(node)  & 0xF;
uint8_t  s0   = RAF_IR_SRC0(node) & 0xF;
uint8_t  s1   = RAF_IR_SRC1(node) & 0xF;
uint64_t imm  = RAF_IR_IMM(node);

switch (op) {  
 case IR_NOP:  
     _aline(b, "\tnop");  
     break;  
 case IR_MOV:  
     _aline(b, "\tmovq %s, %s",  
            _x86_reg64[s0&0x1F], _x86_reg64[dst&0x1F]);  
     break;  
 case IR_MOVIMM:  
     _aline(b, "\tmovq $0x%llx, %s",  
            (unsigned long long)imm, _x86_reg64[dst&0x1F]);  
     break;  
 case IR_LOAD:  
     _aline(b, "\tmovq (%s), %s",  
            _x86_reg64[s0&0x1F], _x86_reg64[dst&0x1F]);  
     break;  
 case IR_STORE:  
     _aline(b, "\tmovq %s, (%s)",  
            _x86_reg64[dst&0x1F], _x86_reg64[s0&0x1F]);  
     break;  
 case IR_ADD:  
     _aline(b, "\taddq %s, %s",  
            _x86_reg64[s1&0x1F], _x86_reg64[dst&0x1F]);  
     break;  
 case IR_SUB:  
     _aline(b, "\tsubq %s, %s",  
            _x86_reg64[s1&0x1F], _x86_reg64[dst&0x1F]);  
     break;  
 case IR_MUL:  
     _aline(b, "\timulq %s",  
            _x86_reg64[s1&0x1F]);  
     break;  
 case IR_DIV:  
     _aline(b, "\tcqto");  
     _aline(b, "\tidivq %s", _x86_reg64[s1&0x1F]);  
     break;  
 case IR_AND:  
     _aline(b, "\tandq %s, %s",  
            _x86_reg64[s1&0x1F], _x86_reg64[dst&0x1F]);  
     break;  
 case IR_OR:  
     _aline(b, "\torq %s, %s",  
            _x86_reg64[s1&0x1F], _x86_reg64[dst&0x1F]);  
     break;  
 case IR_XOR:  
     _aline(b, "\txorq %s, %s",  
            _x86_reg64[s1&0x1F], _x86_reg64[dst&0x1F]);  
     break;  
 case IR_SHL:  
     _aline(b, "\tshlq $%llu, %s",  
            (unsigned long long)imm, _x86_reg64[dst&0x1F]);  
     break;  
 case IR_SHR:  
     _aline(b, "\tshrq $%llu, %s",  
            (unsigned long long)imm, _x86_reg64[dst&0x1F]);  
     break;  
 case IR_CMP:  
     _aline(b, "\tcmpq %s, %s",  
            _x86_reg64[s1&0x1F], _x86_reg64[s0&0x1F]);  
     break;  
 case IR_JMP:  
     _aline(b, "\tjmp .L%llu", (unsigned long long)imm);  
     break;  
 case IR_JEQ:  
     _aline(b, "\tje  .L%llu", (unsigned long long)imm);  
     break;  
 case IR_JNE:  
     _aline(b, "\tjne .L%llu", (unsigned long long)imm);  
     break;  
 case IR_JLT:  
     _aline(b, "\tjl  .L%llu", (unsigned long long)imm);  
     break;  
 case IR_JGE:  
     _aline(b, "\tjge .L%llu", (unsigned long long)imm);  
     break;  
 case IR_CALL:  
     _aline(b, "\tcallq *%s", _x86_reg64[dst&0x1F]);  
     break;  
 case IR_RET:  
     _aline(b, "\tret");  
     break;  
 case IR_PUSH:  
     if (dst < 16)  
         _aline(b, "\tpushq %s", _x86_reg64[dst]);  
     break;  
 case IR_POP:  
     if (dst < 16)  
         _aline(b, "\tpopq %s", _x86_reg64[dst]);  
     break;  
 case IR_FADD:  
     _aline(b, "\tvaddsd %s, %s, %s",  
            _x86_reg64[s1&0x1F], _x86_reg64[s0&0x1F],  
            _x86_reg64[dst&0x1F]);  
     break;  
 case IR_FMUL:  
     _aline(b, "\tvmulsd %s, %s, %s",  
            _x86_reg64[s1&0x1F], _x86_reg64[s0&0x1F],  
            _x86_reg64[dst&0x1F]);  
     break;  
 case IR_CRC32:  
     _aline(b, "\tcrc32q %s, %s",  
            _x86_reg64[s0&0x1F], _x86_reg64[dst&0x1F]);  
     break;  
 /* RAFAELIA geometric stubs */  
 case IR_SPIRAL:  
     /* sqrt(3)/2 = 0.8660254 encoded as double imm */  
     _aline(b, "\t/* SPIRAL: dst=xmm%d *= 0.8660254 */", dst&0xF);  
     _aline(b, "\tmovsd .Lsqrt3_2(%%rip), %%xmm15");  
     _aline(b, "\tmulsd %%xmm15, %s", _x86_reg64[(dst&0xF)+16]);  
     break;  
 case IR_FIBONACCI:  
     _aline(b, "\t/* FIBONACCI-RAFAEL */");  
     _aline(b, "\tmovsd .Lpi(%%rip),    %%xmm14");  
     _aline(b, "\tmovsd .Lsqrt3_2(%%rip),%%xmm15");  
     _aline(b, "\tmulsd %%xmm14, %%xmm15");  
     break;  
 case IR_TOROID:  
     _aline(b, "\t/* TOROID: (R+r*cos(phi))*cos(theta) */");  
     _aline(b, "\t/* caller: xmm0=theta xmm1=phi */");  
     break;  
 case IR_LORENZ:  
     _aline(b, "\t/* LORENZ RK4 dt=0.005 sigma=10 rho=28 beta=8/3 */");  
     break;  
 default:  
     _aline(b, "\t/* IR_0x%02x */", (unsigned)op);  
     break;  
 }

}

/* constants pool */
_aline(b, "\t.section .rodata");
_aline(b, "\t.align 8");
_aline(b, ".Lsqrt3_2: .double 0.8660254037844386");
_aline(b, ".Lpi:       .double 3.141592653589793");
_aline(b, ".Lphi:      .double 1.6180339887498949");
_aline(b, "\t.section .text");
}


/* ════════════════════════════════════════════════════

ARM64 ASM EMITTER

════════════════════════════════════════════════════ */
static void _emit_arm64(RafAsmBuf *b, const RafIR *ir, uint32_t n,
uint32_t feat) {
(void)feat;
_aline(b, "\t.section .text");
_aline(b, "\t.globl _raf_entry");
_aline(b, "\t.type  _raf_entry, %function");
_aline(b, "_raf_entry:");
_aline(b, "\tstp x29, x30, [sp, #-16]!");
_aline(b, "\tmov x29, sp");

for (uint32_t i=0; i<n; i++) {
uint64_t node = ir[i];
RafIROp  op   = (RafIROp)RAF_IR_OP(node);
uint8_t  dst  = RAF_IR_DST(node)  & 0x1F;
uint8_t  s0   = RAF_IR_SRC0(node) & 0x1F;
uint8_t  s1   = RAF_IR_SRC1(node) & 0x1F;
uint64_t imm  = RAF_IR_IMM(node);

switch (op) {  
 case IR_NOP:    _aline(b, "\tnop"); break;  
 case IR_MOV:    _aline(b, "\tmov %s, %s",_a64_xreg[dst],_a64_xreg[s0]); break;  
 case IR_MOVIMM: _aline(b, "\tmov %s, #0x%llx",_a64_xreg[dst],(unsigned long long)imm); break;  
 case IR_LOAD:   _aline(b, "\tldr %s, [%s]",_a64_xreg[dst],_a64_xreg[s0]); break;  
 case IR_STORE:  _aline(b, "\tstr %s, [%s]",_a64_xreg[dst],_a64_xreg[s0]); break;  
 case IR_ADD:    _aline(b, "\tadd %s, %s, %s",_a64_xreg[dst],_a64_xreg[s0],_a64_xreg[s1]); break;  
 case IR_SUB:    _aline(b, "\tsub %s, %s, %s",_a64_xreg[dst],_a64_xreg[s0],_a64_xreg[s1]); break;  
 case IR_MUL:    _aline(b, "\tmul %s, %s, %s",_a64_xreg[dst],_a64_xreg[s0],_a64_xreg[s1]); break;  
 case IR_DIV:    _aline(b, "\tsdiv %s, %s, %s",_a64_xreg[dst],_a64_xreg[s0],_a64_xreg[s1]); break;  
 case IR_AND:    _aline(b, "\tand %s, %s, %s",_a64_xreg[dst],_a64_xreg[s0],_a64_xreg[s1]); break;  
 case IR_OR:     _aline(b, "\torr %s, %s, %s",_a64_xreg[dst],_a64_xreg[s0],_a64_xreg[s1]); break;  
 case IR_XOR:    _aline(b, "\teor %s, %s, %s",_a64_xreg[dst],_a64_xreg[s0],_a64_xreg[s1]); break;  
 case IR_SHL:    _aline(b, "\tlsl %s, %s, #%llu",_a64_xreg[dst],_a64_xreg[s0],(unsigned long long)imm); break;  
 case IR_SHR:    _aline(b, "\tlsr %s, %s, #%llu",_a64_xreg[dst],_a64_xreg[s0],(unsigned long long)imm); break;  
 case IR_CMP:    _aline(b, "\tcmp %s, %s",_a64_xreg[s0],_a64_xreg[s1]); break;  
 case IR_JMP:    _aline(b, "\tb   .L%llu",(unsigned long long)imm); break;  
 case IR_JEQ:    _aline(b, "\tb.eq .L%llu",(unsigned long long)imm); break;  
 case IR_JNE:    _aline(b, "\tb.ne .L%llu",(unsigned long long)imm); break;  
 case IR_JLT:    _aline(b, "\tb.lt .L%llu",(unsigned long long)imm); break;  
 case IR_JGE:    _aline(b, "\tb.ge .L%llu",(unsigned long long)imm); break;  
 case IR_CALL:   _aline(b, "\tblr %s",_a64_xreg[dst]); break;  
 case IR_RET:  
     _aline(b, "\tldp x29, x30, [sp], #16");  
     _aline(b, "\tret");  
     break;  
 case IR_PUSH:   _aline(b, "\tstr %s, [sp, #-8]!",_a64_xreg[dst]); break;  
 case IR_POP:    _aline(b, "\tldr %s, [sp], #8",  _a64_xreg[dst]); break;  
 case IR_FADD:   _aline(b, "\tfadd %s.2d, %s.2d, %s.2d",_a64_vreg[dst&0x1F],_a64_vreg[s0&0x1F],_a64_vreg[s1&0x1F]); break;  
 case IR_FMUL:   _aline(b, "\tfmul %s.2d, %s.2d, %s.2d",_a64_vreg[dst&0x1F],_a64_vreg[s0&0x1F],_a64_vreg[s1&0x1F]); break;  
 case IR_CRC32:  _aline(b, "\tcrc32cx %s, %s, %s",_a64_xreg[dst],_a64_xreg[s0],_a64_xreg[s1]); break;  
 case IR_SPIRAL:  
     _aline(b, "\t/* SPIRAL sqrt3/2 */");  
     _aline(b, "\tfldr d15, .Lsqrt3_2");  
     _aline(b, "\tfmul d%d, d%d, d15", dst&0x1F, dst&0x1F);  
     break;  
 case IR_FIBONACCI:  
     _aline(b, "\t/* FIBONACCI-RAFAEL */");  
     break;  
 case IR_LORENZ:  
     _aline(b, "\t/* LORENZ RK4 */");  
     break;  
 default:  
     _aline(b, "\t/* IR_0x%02x */", (unsigned)op);  
     break;  
 }

}
_aline(b, "\t.section .rodata");
_aline(b, "\t.align 8");
_aline(b, ".Lsqrt3_2: .double 0.8660254037844386");
_aline(b, ".Lpi:      .double 3.141592653589793");
}


int raf_asm_emit(RafCtx *ctx) {
if (!ctx) return -1;
RafAsmBuf *b = &ctx->asm_out;
b->n = 0;
if (ctx->cpu.arch == RAF_ARCH_ARM64)
_emit_arm64(b, ctx->ir.buf, ctx->ir.n, ctx->feat);
else
_emit_x86(b, ctx->ir.buf, ctx->ir.n, ctx->feat);
ctx->asm_lines = b->n;
return 0;
}

/* ════════════════════════════════════════════════════

HEX ENCODER

IR → raw x86-64 machine code bytes

Minimal 1-1 mapping for core opcodes

(full encoding: use actual assembler; this is the

RAFcodding hex preview layer)

════════════════════════════════════════════════════ */


/* REX.W prefix for 64-bit ops /
#define REX_W    0x48
#define REX_WR   0x4C
#define REX_WRB  0x4D
/ ModRM: mod=3 (register), reg, rm */
#define MODRM(mod,reg,rm) (((mod)<<6)|((reg)<<3)|(rm))

static void _hb(RafBin *b, uint8_t byte) {
if (b->n < RAF_HEX_CAP) b->bytes[b->n++]=byte;
}
static void _hw(RafBin *b, uint16_t w) {
_hb(b,(uint8_t)(w));_hb(b,(uint8_t)(w>>8));
}
static void _hd(RafBin *b, uint32_t d) {
_hb(b,d&0xFF);_hb(b,(d>>8)&0xFF);
_hb(b,(d>>16)&0xFF);_hb(b,(d>>24)&0xFF);
}
static void _hq(RafBin *b, uint64_t q) {
_hd(b,(uint32_t)q);_hd(b,(uint32_t)(q>>32));
}

/* encode one x86-64 IR node → bytes */
static void _hex_x86_node(RafBin *b, RafIR node) {
RafIROp op  = (RafIROp)RAF_IR_OP(node);
uint8_t dst = RAF_IR_DST(node)  & 0xF;
uint8_t s0  = RAF_IR_SRC0(node) & 0xF;
uint8_t s1  = RAF_IR_SRC1(node) & 0xF;
uint64_t imm= RAF_IR_IMM(node);
(void)s0;(void)s1;

switch (op) {  
case IR_NOP:  
    _hb(b, 0x90);  
    break;  
case IR_MOVIMM:  
    /* REX.W + MOV r/m64, imm64: opcode=0xB8+rd */  
    if (dst < 8)  _hb(b, REX_W);  
    else          _hb(b, REX_WR);  
    _hb(b, 0xB8 + (dst&7));  
    _hq(b, imm);  
    break;  
case IR_MOV:  
    /* REX.W 0x89 /r */  
    _hb(b, REX_W);  
    _hb(b, 0x89);  
    _hb(b, MODRM(3, s0&7, dst&7));  
    break;  
case IR_ADD:  
    /* REX.W 0x01 /r */  
    _hb(b, REX_W);  
    _hb(b, 0x01);  
    _hb(b, MODRM(3, s1&7, dst&7));  
    break;  
case IR_SUB:  
    _hb(b, REX_W);  
    _hb(b, 0x29);  
    _hb(b, MODRM(3, s1&7, dst&7));  
    break;  
case IR_XOR:  
    _hb(b, REX_W);  
    _hb(b, 0x31);  
    _hb(b, MODRM(3, s1&7, dst&7));  
    break;  
case IR_AND:  
    _hb(b, REX_W);  
    _hb(b, 0x21);  
    _hb(b, MODRM(3, s1&7, dst&7));  
    break;  
case IR_OR:  
    _hb(b, REX_W);  
    _hb(b, 0x09);  
    _hb(b, MODRM(3, s1&7, dst&7));  
    break;  
case IR_SHL:  
    /* REX.W 0xC1 /4 ib */  
    _hb(b, REX_W);  
    _hb(b, 0xC1);  
    _hb(b, MODRM(3, 4, dst&7));  
    _hb(b, (uint8_t)(imm&0x3F));  
    break;  
case IR_SHR:  
    _hb(b, REX_W);  
    _hb(b, 0xC1);  
    _hb(b, MODRM(3, 5, dst&7));  
    _hb(b, (uint8_t)(imm&0x3F));  
    break;  
case IR_PUSH:  
    if (dst < 8)  _hb(b, 0x50+(dst&7));  
    else         { _hb(b, 0x41); _hb(b, 0x50+(dst&7)); }  
    break;  
case IR_POP:  
    if (dst < 8)  _hb(b, 0x58+(dst&7));  
    else         { _hb(b, 0x41); _hb(b, 0x58+(dst&7)); }  
    break;  
case IR_RET:  
    _hb(b, 0xC3);  
    break;  
case IR_CMP:  
    _hb(b, REX_W);  
    _hb(b, 0x39);  
    _hb(b, MODRM(3, s1&7, s0&7));  
    break;  
case IR_JMP:  
    _hb(b, 0xE9);  
    _hd(b, (uint32_t)(imm & 0xFFFFFFFF));  
    break;  
case IR_JEQ:  
    _hb(b, 0x0F); _hb(b, 0x84);  
    _hd(b, (uint32_t)(imm & 0xFFFFFFFF));  
    break;  
case IR_JNE:  
    _hb(b, 0x0F); _hb(b, 0x85);  
    _hd(b, (uint32_t)(imm & 0xFFFFFFFF));  
    break;  
case IR_CRC32:  
    /* F2 REX.W 0F 38 F1 /r */  
    _hb(b,0xF2); _hb(b,REX_W);  
    _hb(b,0x0F); _hb(b,0x38); _hb(b,0xF1);  
    _hb(b,MODRM(3,dst&7,s0&7));  
    break;  
/* RAFAELIA ops: inline float ops */  
case IR_SPIRAL:  
    /* VMULSD xmm,xmm,xmm: VEX.128 0xF2 0x59 */  
    _hb(b,0xC5); _hb(b,0xFB); _hb(b,0x59);  
    _hb(b,MODRM(3,(dst&7),(dst&7)));  
    break;  
default:  
    _hb(b, 0x90); /* NOP for unhandled */  
    break;  
}  
(void)_hw; /* suppress unused */

}

int raf_hex_encode(RafCtx *ctx) {
if (!ctx) return -1;
RafBin *b = &ctx->bin;
b->n = 0;

/* ELF-like header marker: magic 4 bytes */  
_hb(b, 0x7F); _hb(b, 'R'); _hb(b, 'A'); _hb(b, 'F');  
/* version + arch */  
_hb(b, 0x01);  
_hb(b, ctx->cpu.arch);  
_hb(b, ctx->opt);  
_hb(b, 0x00); /* padding */  

if (ctx->cpu.arch != RAF_ARCH_ARM64) {  
    for (uint32_t i=0; i<ctx->ir.n; i++)  
        _hex_x86_node(b, ctx->ir.buf[i]);  
} else {  
    /* ARM64: emit fixed-width 32-bit NOP for each IR node */  
    for (uint32_t i=0; i<ctx->ir.n; i++)  
        _hd(b, 0xD503201Fu); /* ARM64 NOP */  
}  

ctx->hex_bytes = b->n;  
return 0;

}
