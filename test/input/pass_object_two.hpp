#include "../../json5pp.hpp"

static bool verify(const json5pp::value::ptr& value)
{
  auto object = value->as_object();
  return (object.size() == 2) &&
    (object["a"]->as_number().get() == 123) && (object["b"]->as_string() == "xyz");
}
