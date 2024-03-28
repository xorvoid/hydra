#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// printf formatting
#define SEGOFF_FMT      "%04x:%04x"
#define SEGOFF_ARG(s)   (s).seg, (s).off

typedef struct segoff segoff_t;
struct segoff
{
  uint16_t seg;
  uint16_t off;
};

// Parse string in the form "xxxx:yyyy"
segoff_t parse_segoff(const char *s);

// Compute absoulte address
size_t segoff_abs(segoff_t s);

// Compute relative segoff from some base segment
segoff_t segoff_relative_to_segment(segoff_t s, uint16_t seg);
