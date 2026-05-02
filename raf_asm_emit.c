#include "raf_compile.h"

#include <stdio.h>
#include <string.h>

int raf_asm_emit(RafCtx *ctx) {
  if (!ctx) return -1;
  ctx->asm_out.n = 0;
  snprintf(ctx->asm_out.lines[ctx->asm_out.n++], RAF_ASM_LINE, ".text");
  snprintf(ctx->asm_out.lines[ctx->asm_out.n++], RAF_ASM_LINE, ".globl _raf_entry");
  snprintf(ctx->asm_out.lines[ctx->asm_out.n++], RAF_ASM_LINE, "_raf_entry:");
  snprintf(ctx->asm_out.lines[ctx->asm_out.n++], RAF_ASM_LINE, "  mov $42, %%rax");
  snprintf(ctx->asm_out.lines[ctx->asm_out.n++], RAF_ASM_LINE, "  ret");
  return 0;
}

int raf_hex_encode(RafCtx *ctx) {
  if (!ctx) return -1;
  static const uint8_t code[] = {0x48, 0xC7, 0xC0, 0x2A, 0x00, 0x00, 0x00, 0xC3};
  memcpy(ctx->bin.bytes, code, sizeof(code));
  ctx->bin.n = (uint32_t)sizeof(code);
  return 0;
}
