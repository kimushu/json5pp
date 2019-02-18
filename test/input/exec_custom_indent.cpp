#include <json5pp.hpp>
#include <ostream>

void exec(std::ostream& out)
{
  json5pp::value::array a;
  a.emplace_back(new json5pp::value::number(1));
  a.emplace_back(new json5pp::value::number(2));
  a.stringify_to(out, json5pp::indent::spec<'\t', 3>());
}
