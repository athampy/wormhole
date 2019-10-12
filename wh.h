/*
 * Copyright (c) 2016--2018  Wu, Xingbo <wuxb45@gmail.com>
 *
 * All rights reserved. No warranty, explicit or implicit, provided.
 */
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

// includes {{{
// C headers
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
// }}} includes

// types {{{
typedef uint_least8_t           u8;
typedef uint_least16_t          u16;
typedef uint_least32_t          u32;
typedef uint_least64_t          u64;
// }}} types

// timing {{{
  extern u64
rdtsc(void);

  extern u64
time_nsec(void);

  extern double
time_sec(void);

  extern double
time_diff_sec(const double last);
// }}} timing

// random {{{
  extern u64
random_u64(void);

  extern void
srandom_u64(const u64 seed);
// }}} random

// process/thread {{{
  extern u64
process_affinity_core_count(void);

// if args == true, argx is void **
// if args == false, argx is void *
  extern double
thread_fork_join(const u64 nr, void *(*func) (void *), const bool args, void * const argx);
// }}} process/thread

// kv {{{
/*
 * Some internal union names can be ignored:
 * struct kv {
 *   u32 klen;
 *   u32 vlen;
 *   u64 hash;
 *   u8 kv[];
 * };
 */
struct kv {
  union { // the first u64
    u64 kvlen;
    struct {
      u32 klen;
      union {
        u32 vlen;
        u32 refcnt;
      };
    };
  };
  union {
    u64 hash; // hashvalue of the key
    struct {
      u32 hashlo; // little endian
      u32 hashhi;
    };
  };
  u8 kv[];  // len(kv) == klen + vlen
} __attribute__((packed));

// sized buffer: for returning value only
struct sbuf {
  u32 len;
  u8 buf[];
};

  extern size_t
kv_size(const struct kv * const kv);

  extern size_t
kv_size_align(const struct kv * const kv, const u64 align);

  extern size_t
key_size(const struct kv * const key);

  extern size_t
key_size_align(const struct kv * const key, const u64 align);

  extern void
kv_update_hash(struct kv * const kv);

  extern void
kv_refill(struct kv * const kv, const void * const key, const u32 klen, const void * const value, const u32 vlen);

  extern void
kv_refill_str_str(struct kv * const kv, const char * const key, const char * const value);

  extern void
kv_refill_str_u64(struct kv * const kv, const char * const key, const u64 value);

  extern struct kv *
kv_create(const void * const key, const u32 klen, const void * const value, const u32 vlen);

  extern struct kv *
kv_create_str(const char * const key, const char * const value);

  extern struct kv *
kv_dup(const struct kv * const kv);

  extern struct kv *
kv_dup_key(const struct kv * const kv);

  extern struct kv *
kv_dup2(const struct kv * const from, struct kv * const to);

  extern struct kv *
kv_dup2_key(const struct kv * const from, struct kv * const to);

  extern struct kv *
kv_dup2_key_prefix(const struct kv * const from, struct kv * const to, const u64 plen);

  extern struct sbuf *
kv_dup2_sbuf(const struct kv * const from, struct sbuf * const to);

  extern struct kv *
kv_alloc_malloc(const u64 size, void * const priv);

  extern void
kv_retire_free(struct kv * const kv, void * const priv);

  extern bool
kv_keymatch(const struct kv * const key1, const struct kv * const key2);

  extern bool
kv_keymatch_r(const struct kv * const key1, const struct kv * const key2);

  extern bool
kv_fullmatch(const struct kv * const kv1, const struct kv * const kv2);

typedef int  (*kv_compare_func)(const struct kv * const kv1, const struct kv * const kv2);

  extern int
kv_keycompare(const struct kv * const kv1, const struct kv * const kv2);

  extern void
kv_qsort(const struct kv ** const kvs, const size_t nr);

  extern void *
kv_vptr(struct kv * const kv);

  extern void *
kv_kptr(struct kv * const kv);

  extern const void *
kv_vptr_c(const struct kv * const kv);

  extern const void *
kv_kptr_c(const struct kv * const kv);

  extern void
kv_print(const struct kv * const kv, const char * const cmd, FILE * const out);
// }}} kv

// kvmap {{{
typedef struct kv * (* kv_alloc_func)(const u64, void * const);

typedef void (* kv_retire_func)(struct kv * const, void * const);

struct kvmap_mm {
  kv_alloc_func af;
  void * ap;
  kv_retire_func rf;
  void * rp;
};
// }}} kvmap

// wormhole {{{
struct wormhole;
struct wormref;

  extern struct wormhole *
wormhole_create(const struct kvmap_mm * const mm);

  extern struct kv *
wormhole_get(struct wormref * const ref, const struct kv * const key, struct kv * const out);

  extern struct sbuf *
wormhole_getv(struct wormref * const ref, const struct kv * const key, struct sbuf * const out);

  extern u64
wormhole_getu64(struct wormref * const ref, const struct kv * const key);

  extern bool
wormhole_probe(struct wormref * const ref, const struct kv * const key);

  extern bool
wormhole_set(struct wormref * const ref, const struct kv * const kv);

  extern bool
wormhole_del(struct wormref * const ref, const struct kv * const key);

  extern struct wormhole_iter *
wormhole_iter_create(struct wormref * const ref);

  extern void
wormhole_iter_seek(struct wormhole_iter * const iter, const struct kv * const key);

  extern struct kv *
wormhole_iter_next(struct wormhole_iter * const iter, struct kv * const out);

  extern void
wormhole_iter_destroy(struct wormhole_iter * const iter);

  extern struct wormref *
wormhole_ref(struct wormhole * const map);

  extern struct wormhole *
wormhole_unref(struct wormref * const ref);

  extern void
wormhole_clean(struct wormhole * const map);

  extern void
wormhole_destroy(struct wormhole * const map);

// unsafe API

  extern struct kv *
wormhole_get_unsafe(struct wormhole * const map, const struct kv * const key, struct kv * const out);

  extern struct sbuf *
wormhole_getv_unsafe(struct wormhole * const map, const struct kv * const key, struct sbuf * const out);

  extern void *
wormhole_getp_unsafe(struct wormhole * const map, const struct kv * const key);

  extern bool
wormhole_probe_unsafe(struct wormhole * const map, const struct kv * const key);

  extern bool
wormhole_set_unsafe(struct wormhole * const map, const struct kv * const kv0);

  extern bool
wormhole_del_unsafe(struct wormhole * const map, const struct kv * const key);

  extern struct wormhole_iter *
wormhole_iter_create_unsafe(struct wormhole * const map);

  extern void
wormhole_iter_seek_unsafe(struct wormhole_iter * const iter, const struct kv * const key);

  extern struct kv *
wormhole_iter_next_unsafe(struct wormhole_iter * const iter, struct kv * const out);

  extern void
wormhole_iter_destroy_unsafe(struct wormhole_iter * const iter);
// }}} wormhole

#ifdef __cplusplus
}
#endif
// vim:fdm=marker