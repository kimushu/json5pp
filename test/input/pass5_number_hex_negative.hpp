#include "../../json5pp.hpp"

static bool verify(const json5pp::value& value)
{
  return value.as_number() == -0x0a9f;
}
