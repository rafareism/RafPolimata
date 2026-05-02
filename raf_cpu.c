#include "raf_compile.h"

#include <stdio.h>
#include <string.h>

void raf_cpu_detect(RafCPU *cpu) {
  memset(cpu, 0, sizeof(*cpu));
#if defined(__x86_64__)
  cpu->arch = RAF_ARCH_X86_64;
  cpu->feat = RAF_FEAT_SSE4;
  snprintf(cpu->brand, sizeof(cpu->brand), "generic-x86_64");
#elif defined(__aarch64__)
  cpu->arch = RAF_ARCH_ARM64;
  cpu->feat = RAF_FEAT_NEON;
  snprintf(cpu->brand, sizeof(cpu->brand), "generic-arm64");
#else
  cpu->arch = RAF_ARCH_UNKNOWN;
  snprintf(cpu->brand, sizeof(cpu->brand), "generic");
#endif
  cpu->cores = 1;
}

uint8_t raf_lang_from_ext(const char *path) {
  const char *dot = strrchr(path, '.');
  if (!dot) return RAF_LANG_C;
  if (!strcmp(dot, ".c")) return RAF_LANG_C;
  if (!strcmp(dot, ".cpp") || !strcmp(dot, ".cc")) return RAF_LANG_CPP;
  if (!strcmp(dot, ".s")) return RAF_LANG_S;
  if (!strcmp(dot, ".py")) return RAF_LANG_PY;
  if (!strcmp(dot, ".rs")) return RAF_LANG_RS;
  if (!strcmp(dot, ".kt")) return RAF_LANG_KT;
  if (!strcmp(dot, ".java")) return RAF_LANG_JAVA;
  return RAF_LANG_C;
}

void raf_flag_matrix_get(uint8_t arch, uint8_t lang, uint8_t opt, uint32_t feat,
                         char *out_flags, int cap) {
  (void)arch; (void)lang; (void)feat;
  const char *flags = "-O2";
  if (opt == RAF_OPT_0) flags = "-O0 -g";
  else if (opt == RAF_OPT_1) flags = "-O1";
  else if (opt == RAF_OPT_3) flags = "-O3";
  else if (opt == RAF_OPT_S) flags = "-Os";
  snprintf(out_flags, (size_t)cap, "%s", flags);
}
