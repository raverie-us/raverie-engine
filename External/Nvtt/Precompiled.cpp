// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

// This was added for the Empty/Stub platform.
int strcasecmp(const void *s1, const void *s2)
{
  const unsigned char *p1 = (const unsigned char *)s1;
  const unsigned char *p2 = (const unsigned char *)s2;
  
  int result = 0;
  if (p1 == p2)
    return 0;
  
  while ((result = tolower(*p1) - tolower(*p2++)) == 0)
  {
    if (*p1++ == '\0')
      break;
  }
  return result;
}
