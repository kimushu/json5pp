#include "../../json5pp.hpp"

static bool verify(const json5pp::value::ptr& value)
{
  return value->as_string() == "foo.bar'baz";
}
