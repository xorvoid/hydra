#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// printf formatting
#define ADDR_FMT      "%04x:%04x"
#define ADDR_ARG(s)   (s).seg, (s).off

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
