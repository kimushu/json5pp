#include "../../json5pp.hpp"

static bool verify(const json5pp::value::ptr& value)
{
  return value->as_number().get() == 1;
}
