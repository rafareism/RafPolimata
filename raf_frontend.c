#include "raf_compile.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

static int read_src(RafCtx *ctx, const char *path) {
  static char buf[1024 * 1024];
  FILE *f = fopen(path, "rb");
  if (!f) return -1;
  size_t n = fread(buf, 1, sizeof(buf) - 1, f);
  fclose(f);
  buf[n] = '\0';
  ctx->src = buf;
  ctx->src_len = n;
  return 0;
}

int raf_ir_lower(RafCtx *ctx) { return raf_precompile(ctx); }

void raf_ctx_init(RafCtx *ctx) {
  memset(ctx, 0, sizeof(*ctx));
  ctx->opt = RAF_OPT_2;
  raf_cpu_detect(&ctx->cpu);
}

void raf_ctx_report(const RafCtx *ctx) {
  printf("[raf] arch=%u brand=%s ir=%u asm=%u bin=%u\n", ctx->cpu.arch,
         ctx->cpu.brand, ctx->ir.n, ctx->asm_out.n, ctx->bin.n);
}

int raf_compile_file(RafCtx *ctx, const char *src_path, const char *out_base,
                     int do_native) {
  (void)do_native;
  if (read_src(ctx, src_path) != 0) return -1;
  clock_t t0 = clock();
  ctx->lang = raf_lang_from_ext(src_path);
  if (raf_ir_lower(ctx) != 0) return -2;
  if (raf_asm_emit(ctx) != 0) return -3;
  if (raf_hex_encode(ctx) != 0) return -4;

  snprintf(ctx->out_asm, sizeof(ctx->out_asm), "%s.s", out_base);
  snprintf(ctx->out_hex, sizeof(ctx->out_hex), "%s.hex", out_base);

  FILE *fa = fopen(ctx->out_asm, "w");
  for (uint32_t i = 0; fa && i < ctx->asm_out.n; ++i) fprintf(fa, "%s\n", ctx->asm_out.lines[i]);
  if (fa) fclose(fa);

  FILE *fh = fopen(ctx->out_hex, "w");
  for (uint32_t i = 0; fh && i < ctx->bin.n; ++i) fprintf(fh, "%02X%s", ctx->bin.bytes[i], ((i + 1) % 16) ? " " : "\n");
  if (fh) fclose(fh);

  ctx->elapsed_ns = (uint64_t)((clock() - t0) * (1000000000.0 / CLOCKS_PER_SEC));
  return 0;
}
