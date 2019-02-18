#include <json5pp.hpp>
#include <ostream>

void exec(std::ostream& out)
{
  json5pp::value::array a;
  a.resize(3);
  a[1].reset(new json5pp::value::number(123));
  a.stringify_to(out);
}
