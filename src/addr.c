#include "addr.h"
#include "header.h"

addr_t parse_addr(const char *s)
{
  const char *end = s + strlen(s);

  const char *colon = strchr(s, ':');
  if (!colon) FAIL("Invalid addr: '%s'", s);

  addr_t ret;
  ret.seg = parse_hex_u16(s, colon-s);
  ret.off = parse_hex_u16(colon+1, end-(colon+1));
  return ret;
}

size_t addr_abs(addr_t s)
{
  return (size_t)s.seg * 16 + (size_t)s.off;
}

addr_t addr_relative_to_segment(addr_t s, u16 seg)
{
  if (s.seg < seg) {
    FAIL("Cannot compute relative segment, expected >= %04x, got %04x", seg, s.seg);
  }

  assert(s.seg >= seg);
  s.seg -= seg;
  return s;
}
