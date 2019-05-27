#include "../../json5pp.hpp"

static bool verify(const json5pp::value& value)
{
  auto& array = value.as_array();
  auto& inner = array[0].as_array();
  return (array.size() == 1) && (inner.size() == 1) && (inner[0].as_number() == 123);
}
