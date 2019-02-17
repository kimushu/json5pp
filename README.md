# json5pp
JSON (ECMA-404 and JSON5) parser &amp; stringifier for C++11.

## Features

* Easy to embed to your project. Only single file `json5pp.hpp` required.
* Parse [standard JSON (ECMA-404)](https://www.json.org/) from `std::istream` or `std::string`.
* Parse [JSON5](https://json5.org/) from `std::istream` or `std::string`.
* Stringify to `std::ostream` or `std::string` as standard JSON.
* Stringify to `std::ostream` or `std::string` as JSON5.

## Requirements

* Compilers with C++11 support

## License

* MIT license

## APIs

### Types
* values
  * `json5pp::value::base`
    * An abstract base class of value which is represented in JSON.
  * `json5pp::value::ptr`
    * A shared pointer holds value which is represented in JSON.
    * Definition: `using json5pp::value::ptr = std::shared_ptr<json5pp::value::base>;`
* policies
  * `json5pp::policy::ecma404`
    * A class defines rules for [standard JSON (ECMA-404)](https://www.json.org/).
  * `json5pp::policy::json5`
    * A class defines rules for [JSON5](https://json5.org/).
* indentation
  * `json5pp::indent::spaces<N>`
    * A template class specifies an indent with spaces(`' '`) for stringifiers.
    * Template parameter `N` defines a number of spaces for one level.
  * `json5pp::indent::tab`
    * A template class specifies an indent with one tab(`'\t'`) for stringifiers.

### Functions

* parse
  * `json5pp::parse<P>(std::istream& in, const bool finish = true)`
    * Parse JSON from an input stream.
    * Template parameter `P` specifies parsing rule. If omitted, rules for standard JSON will be used.
    * `finish` indicates that the stream should be finished after JSON data.
      * If `true`, parse will fail when junk data after JSON value. This is the default.
      * If `false`, parse will succeed after reading the first JSON value. This is useful for reading JSON as streaming data.
    * Returns `json5pp::value::ptr`
  * `json5pp::parse<P>(const std::string& str)`
    * Parse JSON from a string.
    * Returns `json5pp::value::ptr`
  * `json5pp::parse5(std::istream& in, const bool finish = true)`
    * Parse JSON5 from an input stream.
    * About `finish` parameter, see description for `json5pp::parse<P>(std::istream& in, ...)`
    * Returns `json5pp::value::ptr`
  * `json5pp::parse5(const std::string& str)`
    * Parse JSON5 from a string.
    * Returns `json5pp::value::ptr`
* stringify
  * `json5pp::base::stringify<P>()`
    * Stringify value to JSON as a string without indents.
    * Template parameter `P` specifies stringify rule. If omitted, rules for standard JSON will be used.
    * Returns `std::string`
  * `json5pp::base::stringify<P>(std::ostream& out)`
    * Stringify value to JSON via an output stream without indents.
    * Returns `void`
  * `json5pp::base::stringify<P, I>(const I& indent)`
    * Stringify value to JSON as a string with indents specified by `indent`.
    * Returns `std::string`
  * `json5pp::base::stringify<P, I>(std::ostream& out, const I& indent)`
    * Stringify value to JSON via an output stream with indents specified by `indent`.
    * Returns `void`
  * `json5pp::base::stringify5()`
    * Stringify value to JSON5 as a string without indents.
    * Returns `std::string`
  * `json5pp::base::stringify5(std::ostream& out)`
    * Stringify value to JSON5 via an output stream without indents.
    * Returns `void`
  * `json5pp::base::stringify5<I>(const I& indent)`
    * Stringify value to JSON5 as a string with indents specified by `indent`.
    * Returns `std::string`
  * `json5pp::base::stringify5<I>(std::ostream& out, const I& indent)`
    * Stringify value to JSON5 via an output stream with indents specified by `indent`.
    * Returns `void`

## Examples

```cpp
#include <string>
#include <fstream>
#include "json5pp.hpp"

void load_from_json(const std::string& filename)
{
  std::ifstream f(filename);
  auto value = json5pp::parse(f);
  // :
}

void load_from_json5(const std::string& filename)
{
  std::ifstream f(filename);
  auto value = json5pp::parse5(f);
  // :
}

void save_to_json(const json5pp::value::base& value, const std::string& filename)
{
  std::ofstream f(filename);
  value.stringify(f);
}

void save_to_json5(const json5pp::value::base& value, const std::string& filename)
{
  std::ofstream f(filename);
  value.stringify5(f);
}

void save_to_json_with_indents(const json5pp::value::base& value, const std::string& filename)
{
  std::ofstream f(filename);
  value.stringify(f, json5pp::indent::spaces<2>());
}

void save_to_json5_with_indents(const json5pp::value::base& value, const std::string& filename)
{
  std::ofstream f(filename);
  value.stringify5(f, json5pp::indent::spaces<2>());
}
```

## Limitation

* Not fully compatible with unquoted keys in JSON5 (Some unicode will be rejected as keys)

## ToDo

* More tests
* Add support `operator <<` for stringify
