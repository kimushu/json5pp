#include <json5pp.hpp>
#include <ostream>

void exec(std::ostream& out)
{
  json5pp::value::object o;
  o.emplace("a", nullptr);
  o.emplace("b", new json5pp::value::number(123));
  o.emplace("c", nullptr);
  o.stringify_to(out);
}
