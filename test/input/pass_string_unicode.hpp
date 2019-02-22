#include "../../json5pp.hpp"

static bool verify(const json5pp::value& value)
{
  return value.as_string() == u8"fooあ123";
}
