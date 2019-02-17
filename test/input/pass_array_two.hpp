#include "../../json5pp.hpp"

static bool verify(const json5pp::value::ptr& value)
{
  auto array = value->as_array();
  return (array.size() == 2) &&
    (array[0]->as_number().get() == 123) && (array[1]->as_string() == "foo");
}
