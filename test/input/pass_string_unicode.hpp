#include "../../json5pp.hpp"
#include <iostream>

static bool verify(const json5pp::value::ptr& value)
{
  return value->as_string() == u8"fooあ123";
}
