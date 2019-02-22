#include <json5pp.hpp>
#include <ostream>

void exec(std::ostream& out)
{
  auto value = json5pp::array({});
  auto& array = value.as_array();
  array.resize(3);
  array[1] = 123;
  out << value;
}
