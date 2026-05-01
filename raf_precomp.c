/*

RAFcodding — raf_precomp.c

Tokenizer (C/C++/S) → IR nodes (packed 64-bit, no malloc)

x86-64 / ARM64 ASM text emitter


raw hex encoder


TOKEN TYPES: 0=PUNCT 1=NUM 2=IDENT 3=STR 4=OP 5=KW 6=EOF

IR is pure index-based: no string names on hot path
*/
#include "raf_compile.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>


/* ════════════════════════════════════════════════════

TOKENIZER (C/C++/ASM subset — zero alloc)

════════════════════════════════════════════════════ */


typedef struct {
const char *cur;
const char *end;
uint32_t    line;
uint8_t     lang;
} Lex;

/* C keywords → keyword id (1..32) */
static uint8_t _kw_id(const char p, uint16_t len) {
/ small table: only what the IR needs */
static const struct { const char *s; uint8_t id; } KW[] = {
{"return",1},{"if",2},{"else",3},{"while",4},{"for",5},
{"int",6},{"long",7},{"float",8},{"double",9},{"void",10},
{"char",11},{"unsigned",12},{"static",13},{"inline",14},
{"const",15},{"struct",16},{"typedef",17},{"sizeof",18},
{"do",19},{"break",20},{"continue",21},{"switch",22},{"case",23},
{"uint8_t",24},{"uint16_t",25},{"uint32_t",26},{"uint64_t",27},
{"int32_t",28},{"int64_t",29},{"size_t",30},
{0,0}
};
for (int i=0; KW[i].s; i++) {
const char *s=KW[i].s;
uint16_t sl=(uint16_t)strlen(s);
if (sl==len && !memcmp(s,p,len)) return KW[i].id;
}
return 0;
}

static RafTok _next_tok(Lex L) {
RafTok t; t.lang=L->lang; t.line=L->line;
/ skip whitespace + line comments */
restart:
while (L->cur < L->end && (*L->cur==' '||*L->cur=='\t'||
*L->cur=='\r'||*L->cur=='\n')) {
if (*L->cur=='\n') L->line++;
L->cur++;
}
if (L->cur >= L->end) { t.type=6; t.ptr=L->cur; t.len=0; return t; }

/* line comment */  
if (L->cur[0]=='/' && L->cur+1<L->end && L->cur[1]=='/') {  
    while (L->cur<L->end && *L->cur!='\n') L->cur++;  
    goto restart;  
}  
/* block comment */  
if (L->cur[0]=='/' && L->cur+1<L->end && L->cur[1]=='*') {  
    L->cur+=2;  
    while (L->cur+1<L->end && !(L->cur[0]=='*'&&L->cur[1]=='/')) {  
        if (*L->cur=='\n') L->line++;  
        L->cur++;  
    }  
    if (L->cur+1<L->end) L->cur+=2;  
    goto restart;  
}  
/* preprocessor line — skip entirely */  
if (*L->cur=='#') {  
    while (L->cur<L->end && *L->cur!='\n') L->cur++;  
    goto restart;  
}  

t.ptr = L->cur;  

/* number literal */  
if (isdigit((unsigned char)*L->cur) ||  
    (*L->cur=='-' && L->cur+1<L->end && isdigit((unsigned char)L->cur[1]))) {  
    t.type=1;  
    if (*L->cur=='-') L->cur++;  
    /* hex? */  
    if (*L->cur=='0' && L->cur+1<L->end &&  
        (L->cur[1]=='x'||L->cur[1]=='X')) {  
        L->cur+=2;  
        while (L->cur<L->end && isxdigit((unsigned char)*L->cur)) L->cur++;  
    } else {  
        while (L->cur<L->end && (isdigit((unsigned char)*L->cur)||  
               *L->cur=='.'||*L->cur=='e'||*L->cur=='E'||  
               *L->cur=='u'||*L->cur=='l'||*L->cur=='L')) L->cur++;  
    }  
    t.len=(uint16_t)(L->cur-t.ptr);  
    return t;  
}  

/* string literal */  
if (*L->cur=='"') {  
    t.type=3; L->cur++;  
    while (L->cur<L->end && *L->cur!='"') {  
        if (*L->cur=='\\') L->cur++;  
        L->cur++;  
    }  
    if (L->cur<L->end) L->cur++;  
    t.len=(uint16_t)(L->cur-t.ptr);  
    return t;  
}  

/* identifier or keyword */  
if (isalpha((unsigned char)*L->cur) || *L->cur=='_') {  
    while (L->cur<L->end && (isalnum((unsigned char)*L->cur)||*L->cur=='_'))  
        L->cur++;  
    t.len=(uint16_t)(L->cur-t.ptr);  
    uint8_t kid = _kw_id(t.ptr, t.len);  
    t.type = kid ? 5 : 2;  
    return t;  
}  

/* operator (1 or 2 char) */  
t.type=4; L->cur++;  
/* two-char ops */  
if (L->cur<L->end) {  
    char a=t.ptr[0], b=L->cur[0];  
    if ((a=='='&&b=='=')||(a=='!'&&b=='=')||(a=='<'&&b=='=')||  
        (a=='>'&&b=='=')||(a=='+'&&b=='+')||(a=='-'&&b=='-')||  
        (a=='+'&&b=='=')||(a=='-'&&b=='=')||(a=='*'&&b=='=')||  
        (a=='/'&&b=='=')||(a=='&'&&b=='&')||(a=='|'&&b=='|')||  
        (a=='<'&&b=='<')||(a=='>'&&b=='>')||(a=='-'&&b=='>'))  
        L->cur++;  
}  
t.len=(uint16_t)(L->cur-t.ptr);  
return t;

}

/* ════════════════════════════════════════════════════

IR BUILDER

Maps C-like token stream → RafIR packed nodes

No recursion, no alloc — single-pass linear transform

════════════════════════════════════════════════════ */


/* simple register allocator: stack of 32 virtual regs /
typedef struct {
uint8_t  top;
uint8_t  next;   / next free virtual reg */
uint8_t  stk[32];
} RegStk;

static uint8_t _ralloc(RegStk rs) {
uint8_t r = rs->next++;
if (rs->next > 31) rs->next=0; / wrap */
rs->stk[rs->top++&31]=r;
return r;
}
static uint8_t _rpop(RegStk *rs) {
return rs->stk[--rs->top&31];
}

static void _ir_push(RafIRBuf *b, RafIROp op,
uint8_t dst, uint8_t s0, uint8_t s1, uint64_t imm) {
if (b->n >= RAF_IR_CAP) return;
b->buf[b->n++] = RAF_IR_PACK(op,dst,s0,s1,imm);
}

/*

Single-pass IR builder for C/C++ expression + statement level.

Covers: assignments, arithmetic, comparisons, calls, returns.

Unrecognized constructs → IR_NOP (pass-through).
*/
int raf_precompile(RafCtx *ctx) {
if (!ctx||!ctx->src) return -1;
Lex L;
L.cur  = ctx->src;
L.end  = ctx->src + ctx->src_len;
L.line = 1;
L.lang = ctx->lang;

RafIRBuf *ir = &ctx->ir;
ir->n    = 0;
ir->arch = ctx->cpu.arch;
ir->opt  = ctx->opt;
ir->feat = ctx->feat;

RegStk rs; memset(&rs,0,sizeof(rs)); rs.next=0;

/* function prologue IR */
_ir_push(ir, IR_PUSH, R_X86_RBP, R_X86_RSP, 0, 0);
_ir_push(ir, IR_MOV,  R_X86_RBP, R_X86_RSP, 0, 0);

RafTok t, t2;
uint8_t dst, s0, s1;

while (1) {
t = _next_tok(&L);
if (t.type==6) break; /* EOF */

/* RETURN statement */  
 if (t.type==5 && t.len==6 && !memcmp(t.ptr,"return",6)) {  
     t2 = _next_tok(&L);  
     if (t2.type==1) { /* numeric literal */  
         uint64_t imm=0;  
         const char *p=t2.ptr;  
         if (p[0]=='0'&&(p[1]=='x'||p[1]=='X')) {  
             const char *q=p+2;  
             while (*q) { imm=(imm<<4)|(*q<='9'?*q-'0':(*q|32)-'a'+10); q++; }  
         } else {  
             while (*p>='0'&&*p<='9') { imm=imm*10+(*p-'0'); p++; }  
         }  
         _ir_push(ir, IR_MOVIMM, R_X86_RAX, 0, 0, imm);  
     } else if (t2.type==2) { /* ident → load from vreg */  
         s0 = _ralloc(&rs);  
         _ir_push(ir, IR_LOAD, R_X86_RAX, s0, 0, 0);  
     }  
     _ir_push(ir, IR_RET, 0, 0, 0, 0);  
     continue;  
 }  

 /* ASSIGNMENT: ident = expr ; */  
 if (t.type==2) {  
     RafTok op_tok = _next_tok(&L);  
     if (op_tok.type==4 && op_tok.len==1 && op_tok.ptr[0]=='=') {  
         t2 = _next_tok(&L);  
         dst = _ralloc(&rs);  
         if (t2.type==1) {  
             uint64_t imm=0;  
             const char *p=t2.ptr;  
             while (*p>='0'&&*p<='9'){imm=imm*10+(*p-'0');p++;}  
             _ir_push(ir, IR_MOVIMM, dst, 0, 0, imm);  
             _ir_push(ir, IR_STORE, dst, 0, 0, 0);  
         } else if (t2.type==2) {  
             s0=_ralloc(&rs);  
             _ir_push(ir, IR_LOAD,  dst, s0, 0, 0);  
             _ir_push(ir, IR_STORE, dst,  0, 0, 0);  
         }  
         continue;  
     }  
 }  

 /* BINARY OP: detect a OP b pattern */  
 if (t.type==2) {  
     RafTok op_tok = _next_tok(&L);  
     if (op_tok.type==4) {  
         t2 = _next_tok(&L);  
         s0=_ralloc(&rs); s1=_ralloc(&rs); dst=_ralloc(&rs);  
         RafIROp iop = IR_NOP;  
         char op = op_tok.ptr[0];  
         if      (op=='+') iop=IR_ADD;  
         else if (op=='-') iop=IR_SUB;  
         else if (op=='*') iop=IR_MUL;  
         else if (op=='/') iop=IR_DIV;  
         else if (op=='&') iop=IR_AND;  
         else if (op=='|') iop=IR_OR;  
         else if (op=='^') iop=IR_XOR;  
         if (iop!=IR_NOP) {  
             if (t2.type==1) {  
                 uint64_t imm=0;  
                 const char *p=t2.ptr;  
                 while(*p>='0'&&*p<='9'){imm=imm*10+(*p-'0');p++;}  
                 _ir_push(ir, IR_MOVIMM, s1, 0, 0, imm);  
             } else {  
                 _ir_push(ir, IR_LOAD, s1, 0, 0, 0);  
             }  
             _ir_push(ir, IR_LOAD, s0, 0, 0, 0);  
             _ir_push(ir, iop, dst, s0, s1, 0);  
             _ir_push(ir, IR_STORE, dst, 0, 0, 0);  
         }  
         continue;  
     }  
 }  

 /* CALL: ident ( ... ) */  
 if (t.type==2 || (t.type==5 && t.len)) {  
     RafTok p2 = _next_tok(&L);  
     if (p2.type==4 && p2.ptr[0]=='(') {  
         /* argument count heuristic: count commas until ) */  
         uint8_t argc=0;  
         RafTok a;  
         while (1) {  
             a=_next_tok(&L);  
             if (a.type==6) break;  
             if (a.type==4 && a.ptr[0]==')') break;  
             if (a.type==4 && a.ptr[0]==',') { argc++; continue; }  
             if (a.type==1 || a.type==2) {  
                 /* push arg to arg register */  
                 _ir_push(ir, IR_PUSH, (uint8_t)(R_X86_RDI+argc), 0, 0, 0);  
             }  
         }  
         _ir_push(ir, IR_CALL, R_X86_RAX, 0, 0, (uint64_t)argc);  
         continue;  
     }  
 }  

 /* SPIRAL / RAFAELIA ops: detect raf_ prefix */  
 if (t.type==2 && t.len>=4 && !memcmp(t.ptr,"raf_",4)) {  
     if (t.len==10 && !memcmp(t.ptr,"raf_spiral",10))  
         _ir_push(ir, IR_SPIRAL, R_X86_XMM0, 0, 0, 0);  
     else if (t.len>=13 && !memcmp(t.ptr,"raf_fibonacci",13))  
         _ir_push(ir, IR_FIBONACCI, R_X86_XMM1, 0, 0, 0);  
     else if (t.len>=10 && !memcmp(t.ptr,"raf_toroid",10))  
         _ir_push(ir, IR_TOROID, R_X86_XMM2, 0, 0, 0);  
     else if (t.len>=10 && !memcmp(t.ptr,"raf_lorenz",10))  
         _ir_push(ir, IR_LORENZ, R_X86_XMM3, 0, 0, 0);  
     continue;  
 }  

 /* everything else → NOP marker */  
 _ir_push(ir, IR_NOP, 0, 0, 0, (uint64_t)(t.ptr - ctx->src));

}

/* function epilogue */
_ir_push(ir, IR_POP,  R_X86_RBP, 0, 0, 0);
_ir_push(ir, IR_RET, 0, 0, 0, 0);

ctx->ir_nodes = ir->n;
return 0;
}
