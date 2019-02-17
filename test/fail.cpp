#include "../json5pp.hpp"
#include <thread>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>

int main(int argc, char *argv[])
{
  if (argc != 2) {
    std::cerr << "This program should be invoked from Makefile!" << std::endl;
    std::exit(1);
  }

  std::cout << "  \"" << argv[1] << "\":\n    ";

  std::thread([]{
    std::this_thread::sleep_for(std::chrono::seconds(TIMEOUT));
    std::cout << "\"[FAIL] timed out.\"," << std::endl;
    std::exit(3);
  }).detach();

  try {
    std::ifstream in(argv[1], std::ios::binary);
    json5pp::PARSE_METHOD(in);
    std::cout << "\"[FAIL] unexpected success.\"," << std::endl;
    std::exit(1);
  } catch (const json5pp::syntax_error& e) {
    std::cout << "\"[PASS] parse failed expectedly.\"," << std::endl;
    std::exit(0);
  } catch (const std::exception& e) {
    std::cout << "\"[FAIL] unexpected error: " << e.what() << "\"," << std::endl;
    std::exit(2);
  } catch (...) {
    std::cout << "\"[FAIL] unexpected error.\"," << std::endl;
    std::exit(2);
  }
}
