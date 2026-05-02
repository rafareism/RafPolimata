/* raf_hash.h — FNV1a-64 + PHI64 Knuth mix + Merkle chain, zero heap
 * PHI64 = floor(2^64/phi): Teorema Knuth — dispersão uniforme em [0,2^n)
 * Avalanche: cada bit de entrada afeta ~50% dos bits de saída em 1 passo
 * HashVivo chain: hash[i] = mix(hash[i-1] XOR data[i]) — imutável          */
#pragma once
#include "raf_types.h"

#define FNV1A_BASIS  0xCBF29CE484222325ULL
#define FNV1A_PRIME  0x100000001B3ULL    /* Eq.32 */

/* FNV1a-64: O(n), zero alloc, 1 mul + 1 xor por byte                       */
static __attribute__((always_inline)) inline
u64 fnv1a_byte(u64 h, u8 b) {
    return (h ^ b) * FNV1A_PRIME;
}
static u64 fnv1a_buf(const u8 *buf, usize len) {
    u64 h = FNV1A_BASIS;
    while (len--) h = fnv1a_byte(h, *buf++);
    return h;
}

/* PHI64 Knuth mix — Eq.15: h = (h XOR x) * PHI64
 * Extrai n bits superiores: dispersão uniforme provada em [0, 2^n)          */
static __attribute__((always_inline)) inline
u64 phi64_mix(u64 h, u64 x) {
    return (h ^ x) * PHI64;
}
static __attribute__((always_inline)) inline
u32 phi64_to_idx(u64 h, u32 bits) {
    return (u32)(h >> (64 - bits)); /* bits superiores: dispersão máxima */
}

/* Merkle chain step — Eq.17: R = Merkle(H0,H1,...,Hn)
 * chain[i] = phi64_mix(chain[i-1], data_hash[i])
 * Propriedade: alterar data[k] invalida chain[k..n] — detecção de tamper   */
#define MERKLE_DEPTH  8
typedef struct {
    u64 h[MERKLE_DEPTH];
    u32 count;
} MerkleChain;

static inline void merkle_init(MerkleChain *m) {
    __builtin_memset(m, 0, sizeof(*m));
    m->h[0] = FNV1A_BASIS;
}
static inline void merkle_feed(MerkleChain *m, u64 data_hash) {
    u32 i = m->count & (MERKLE_DEPTH - 1);
    u32 prev = (i + MERKLE_DEPTH - 1) & (MERKLE_DEPTH - 1);
    m->h[i] = phi64_mix(m->h[prev], data_hash);
    m->count++;
}
static inline u64 merkle_root(const MerkleChain *m) {
    u32 i = (m->count - 1) & (MERKLE_DEPTH - 1);
    return m->h[i];
}

/* HashVivo: chain de sessão RAFAELIA — equivalente ao hashchain[] do kernel  */
typedef struct {
    u64  chain;      /* acumulador running                                    */
    u32  crc_a;      /* CRC32C seed 0x00000000                               */
    u32  crc_b;      /* CRC32C seed 0xFFFFFFFF (AETHER dual)                 */
    u32  count;      /* número de feeds                                       */
} HashVivo;

static inline void hashvivo_init(HashVivo *hv) {
    hv->chain = FNV1A_BASIS;
    hv->crc_a = hv->crc_b = 0;
    hv->count = 0;
}
static inline void hashvivo_feed(HashVivo *hv, u64 v) {
    hv->chain = phi64_mix(hv->chain, v);
    hv->count++;
}
static inline u64 hashvivo_digest(const HashVivo *hv) {
    return phi64_mix(hv->chain, (u64)hv->count);
}
