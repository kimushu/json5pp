#include "../../json5pp.hpp"

static bool verify(const json5pp::value& value)
{
  auto& object = value.as_object();
  auto& inner = object.at("a").as_object();
  return (object.size() == 1) && (inner.size() == 1) && (inner.at("a").as_number() == 123);
}
