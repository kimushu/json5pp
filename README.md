# json5pp
JSON (ECMA-404 and JSON5) parser &amp; stringifier written in **C++11**.

# Features

* Easy to embed to your project. Only single file `json5pp.hpp` required.
* Parse [standard JSON (ECMA-404)](https://www.json.org/) from `std::istream` or `std::string`.
* Parse [JSON5](https://json5.org/) from `std::istream` or `std::string`.
* Stringify to `std::ostream` or `std::string` as standard JSON.
* Stringify to `std::ostream` or `std::string` as JSON5.

# Requirements

* Compilers with C++11 support

# License

* MIT license

# API

## JSON value type

```cpp
namespace json5pp {
  class value {
  };
}
```
* The class holds a value which is represented in JSON.
* This class can hold all types in JSON:
  * `null`
  * boolean (`true` / `false`)
  * number
  * string
  * array (stored as `std::vector<json5pp::value>`)
  * object (stored as `std::map<std::string, json5pp::value>`)
* Provides type check with `is_xxx()` method. (`xxx` is one of `null`, `boolean`, `number`, `string`, `array` and `object`)
* Provides explicit cast to C++ type with `as_xxx()` method. (`xxx` is one of `null`, `boolean`, `number`, `integer`, `string`, `array` and `object`)
  * If cast failed, throws `std::bad_cast`
* Accepts implicit cast (by overload of `operator=`) from C++ type (`nullptr_t`, `bool`, `double` | `int`, `std::string` | `const char*`)

## Parse functions

```cpp
namespace json5pp {
  value parse(const std::string& str);
}
```
* Parse given string.
* String must be a valid JSON (ECMA-404 standard)
  * If not valid, throws `json5pp::syntax_error`.
* String must be a finished (closed) JSON.
  * i.e. If there are any junk data (except for spaces) after JSON, throws `json5pp::syntax_error`.

```cpp
namespace json5pp {
  value parse(std::istream& istream, bool finish = true)
}
```
* Parse JSON from given input stream.
* Stream must have a valid JSON (ECMA-404 standard).
  * If not valid, throws `json5pp::syntax_error`.
* If `finish` is true, stream must be closed by eof after JSON.

```cpp
namespace json5pp {
  value parse5(const std::string& str);
}
```
* JSON5 version of `parse(const std::string&)`

```cpp
namespace json5pp {
  value parse5(std::istream& istream, bool finish = true);
}
```
* JSON5 version of `parse(std::istream&, bool)`

## Stringify functions

```cpp
namespace json5pp {
  class value {
    template <class... T>
    std::string stringify(T... manip);
  };
}
```
* Stringify value to ECMA-404 standard JSON.
* About `T... manip`, see [iostream overloads and manipulators](#iostream-API-and-manipulators)

```cpp
namespace json5pp {
  class value {
    template <class... T>
    std::string stringify5(T... manip);
  };
}
```
* Stringify value to JSON5.
* About `T... manip`, see [iostream overloads and manipulators](#iostream-API-and-manipulators)

```cpp
namespace json5pp {
  template <class... T>
  std::string stringify(const value& v, T... manip);
}
```
* Global method version of `json5pp::value::stringify()`
* Same as `v.stringify(manip...)`

```cpp
namespace json5pp {
  template <class... T>
  std::string stringify5(const value& v, T... manip);
}
```
* Global method version of `json5pp::value::stringify5()`
* Same as `v.stringify5(manip...)`

## iostream API

### Parse by `operator>>`
```cpp
std::istream& istream = ...;
json5pp::value v;
istream >> v;
```

### Parse by `operator>>` with manipulators
```cpp
std::istream& istream = ...;
json5pp::value v;
istream >> json5pp::rule::json5() >> v; // Parse as JSON5
```

### Stringify by `operator<<`
```cpp
std::ostream& ostream = ...;
const json5pp::value& v = ...;
ostream << v;
```

### Stringify by `operator<<` with manipulators
```cpp
std::ostream& ostream = ...;
const json5pp::value& v = ...;
ostream << json5pp::rule::tab_indent<1>() << v;   // Stringify with tab indent
ostream << json5pp::rule::space_indent<2>() << v; // Stringify with 2-space indent
```

## Manipulators

### Comments
* `json5pp::rule::single_line_comment()`
* `json5pp::rule::no_single_line_comment()`
  * Allow/disallow single line comment starts with `//`
* `json5pp::rule::multi_line_comment()`
* `json5pp::rule::no_multi_line_comment()`
  * Allow/disallow multiple line comment starts with `/*` and ends with `*/`
* `json5pp::rule::comments()`
* `json5pp::rule::no_comments()`
  * Combination of `single_line_comment` and `multi_line_comment`

### Numbers
* `json5pp::rule::explicit_plus_sign()`
* `json5pp::rule::no_explicit_plus_sign()`
  * Allow/disallow explicit plus sign (`+`) before non-negative number (ex: `+123`)
* `json5pp::rule::leading_decimal_point()`
* `json5pp::rule::no_leading_decimal_point()`
  * Allow/disallow leading decimal point before number (ex: `.123`)
* `json5pp::rule::trailing_decimal_point()`
* `json5pp::rule::no_trailing_decimal_point()`
  * Allow/disallow trailing decimal point after number (ex: `123.`)
* `json5pp::rule::decimal_points()`
* `json5pp::rule::no_decimal_points()`
  * Combination of `leading_decimal_point` and `trailing_decimal_point`
* `json5pp::rule::infinity_number()`
* `json5pp::rule::no_infinity_number()`
  * Allow/disallow infinity number
* `json5pp::rule::not_a_number()`
* `json5pp::rule::no_not_a_number()`
  * Allow/disallow NaN
* `json5pp::rule::hexadecimal()`
* `json5pp::rule::no_hexadecimal()`
  * Allow/disallow hexadecimal number (ex: `0x123`)

### Strings
* `json5pp::rule::single_quote()`
* `json5pp::rule::no_single_quote()`
  * Allow/disallow single quoted string (ex: `'foobar'`)
* `json5pp::rule::multi_line_string()`
* `json5pp::rule::no_multi_line_string()`
  * Allow/disallow multiple line string escaped by `\`
  * Example:
    ```json
    "test\
    2nd line"
    ```

### Arrays and objects
* `json5pp::rule::trailing_comma()`
* `json5pp::rule::no_trailing_comma()`
  * Allow/disallow trailing comma at the end of arrays or objects.
  * Example for arrays: `[1,2,3,]`
  * Example for objects: `{"a":123,}`

### Objects
* `json5pp::rule::unquoted_key()`
* `json5pp::rule::no_unquoted_key()`
  * Allow/disallow unquoted keys in objects. (ex: `{a:123}`)

### Rule sets
* `json5pp::rule::ecma404()`
  * ECMA-404 standard rule set.
* `json5pp::rule::json5()`
  * JSON5 rule set.

### Parse options
* `json5pp::rule::finished()`
  * Parse as finished (closed) JSON. If any junk data follows after JSON, parse fails.
  * Opposite to `json5pp::rule::streaming()`
* `json5pp::rule::streaming()`
  * Parse as non-finished (non-closed) JSON. Parse will succeed at the end of JSON.
  * Opposite to `json5pp::rule::finished()`

### Stringify options
* `json5pp::rule::lf_newline()`
  * When indent is enabled, use LF(`\n`) as new-line code.
  * Opposite to `json5pp::rule::crlf_newline`
* `json5pp::rule::crlf_newline()`
  * When indent is enabled, use CR+LF(`\r\n`) as new-line code.
  * Opposite to `json5pp::rule::lf_newline`
* `json5pp::rule::no_indent()`
  * Disable indent. All arrays and objects will be stringified as one-line.
* `json5pp::rule::tab_indent<L>()`
  * Enable indent with tab character(s).
  * `L` means a number of tab (`\t`) characters for one-level indent.
  * If `L` is omitted, treat as `L=1`.
* `json5pp::rule::space_indent<L>()`
  * Enable indent with space character(s).
  * `L` means a number of space (` `) characters for one-level indent.
  * If `L` is omitted, treat as `L=2`.

## Limitation

* Not fully compatible with unquoted keys in JSON5 (Some unicode will be rejected as keys)
* All strings are assumed to be stored in UTF-8 encoding.

## ToDo

* More tests
