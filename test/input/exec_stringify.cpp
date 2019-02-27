#include <json5pp.hpp>
#include <ostream>

void exec(std::ostream& out)
{
  auto value = json5pp::object({{"foo", "bar"}});
  out << value.stringify();
}
