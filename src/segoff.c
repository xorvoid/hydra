#include "segoff.h"
#include "header.h"

segoff_t parse_segoff(const char *s)
{
  const char *end = s + strlen(s);

  const char *colon = strchr(s, ':');
  if (!colon) FAIL("Invalid segoff: '%s'", s);

  segoff_t ret;
  ret.seg = parse_hex_u16(s, colon-s);
  ret.off = parse_hex_u16(colon+1, end-(colon+1));
  return ret;
}

size_t segoff_abs(segoff_t s)
{
  return (size_t)s.seg * 16 + (size_t)s.off;
}

segoff_t segoff_relative_to_segment(segoff_t s, u16 seg)
{
  if (s.seg < seg) {
    FAIL("Cannot compute relative segment, expected >= %04x, got %04x", seg, s.seg);
  }

  assert(s.seg >= seg);
  s.seg -= seg;
  return s;
}
