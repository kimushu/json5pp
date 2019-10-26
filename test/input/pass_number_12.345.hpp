#include "../../json5pp.hpp"

static bool verify(const json5pp::value& value)
{
  return (value.is_number()) && (!value.is_integer()) &&
    (value.as_number() == 12.345) && (value.as_integer() == 12);
}
