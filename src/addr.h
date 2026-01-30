#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// printf formatting
#define ADDR_FMT      "%04x:%04x"
#define ADDR_ARG(s)   (s).seg, (s).off
#define ADDR_MAKE(_seg, _off) ({ addr_t a; a.seg = (_seg); a.off = (_off); a; })

typedef struct addr addr_t;
struct addr
{
  uint16_t seg;
  uint16_t off;
};

// Parse string in the form "xxxx:yyyy"
addr_t parse_addr(const char *s);

// Compute absoulte address
size_t addr_abs(addr_t s);

// Compute relative addr from some base segment
addr_t addr_relative_to_segment(addr_t s, uint16_t seg);

// Equal??
static inline bool addr_equal(addr_t a, addr_t b)
{
  return a.seg == b.seg && a.off == b.off;
}
