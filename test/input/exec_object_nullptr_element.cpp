#include <json5pp.hpp>
#include <ostream>

void exec(std::ostream& out)
{
  auto value = json5pp::object({{"a", nullptr}, {"b", 123}, {"c", nullptr}});
  out << value;
}
