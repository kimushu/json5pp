#include "../../json5pp.hpp"

static bool verify(const json5pp::value& value)
{
  auto object = value.as_object();
  return (object.size() == 0);
}
