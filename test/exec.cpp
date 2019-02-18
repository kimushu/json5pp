#include <json5pp.hpp>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iostream>

void exec(std::ostream& out);

int main(int argc, char *argv[])
{
  if ((argc != 2) && (argc != 3)) {
    std::cerr << "This program should be invoked from Makefile!" << std::endl;
    std::exit(1);
  }

  std::string source(argv[1]);
  source.replace(source.size() - 4, 4, "cpp");
  std::cout << "  \"" << source << "\":\n    ";

  std::thread([]{
    std::this_thread::sleep_for(std::chrono::seconds(TIMEOUT));
    std::cout << "\"[FAIL] timed out.\"," << std::endl;
    std::exit(3);
  }).detach();

  try {
    if (argc == 3) {
      std::ofstream out(argv[2], std::ios::binary);
      exec(out);
    } else {
      std::ostringstream out;
      exec(out);
    }
    std::cout << "\"[PASS] execution succeeded.\"," << std::endl;
    std::exit(0);
  } catch (const std::exception& e) {
    std::cout << "\"[FAIL] unexpected error: " << e.what() << "\"," << std::endl;
    std::exit(2);
  } catch (...) {
    std::cout << "\"[FAIL] unexpected error.\"," << std::endl;
    std::exit(2);
  }
}
