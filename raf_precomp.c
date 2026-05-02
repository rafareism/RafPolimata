#include "raf_compile.h"

int raf_precompile(RafCtx *ctx) {
  if (!ctx || !ctx->src) return -1;
  ctx->ir.n = 2;
  ctx->ir.buf[0] = ((uint64_t)IR_MOVIMM << 56) | 42u;
  ctx->ir.buf[1] = ((uint64_t)IR_RET << 56);
  return 0;
}
