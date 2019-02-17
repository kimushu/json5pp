#ifndef _JSON5PP_HPP_
#define _JSON5PP_HPP_

#include <memory>
#include <istream>
#include <string>
#include <sstream>
#include <typeinfo>
#include <vector>
#include <utility>
#include <stdexcept>
#include <limits>
#include <map>
#include <cmath>
#include <iomanip>

namespace json5pp {

/**
 * @class syntax_error
 * A class of objects thrown as exceptions to report a JSON syntax error.
 */
class syntax_error : public std::invalid_argument
{
public:
  /**
   * Construct syntax_error instance with failed character.
   * @param ch Character which raised error or Traits::eof()
   * @param context Context description in human-readable string
   */
  syntax_error(int ch, const char *context)
  : std::invalid_argument(
      std::string("JSON syntax error: ") +
      (ch != std::char_traits<char>::eof() ?
        "illegal character `" + std::string(1, ch) + "'" :
        "unexpected EOS") +
      " in " + context) {}
};

namespace policy {

/**
 * @class json5pp::policy::ecma_404
 * Defines ECMA-404 standard JSON.
 * https://www.json.org/
 */
struct ecma404
{
  /**
   * Allow single line comment starts with "//"
   */
  static constexpr bool allow_single_line_comment = false;

  /**
   * Allow multi-line comment starts with "/*" and ends with "* /"
   */
  static constexpr bool allow_multi_line_comment = false;

  /**
   * Allow explicit plus sign (+) in numbers
   */
  static constexpr bool allow_explicit_plus_sign = false;

  /**
   * Allow leading decimal point in numbers
   */
  static constexpr bool allow_leading_decimal_point = false;

  /**
   * Allow trailing decimal point in numbers
   */
  static constexpr bool allow_trailing_decimal_point = false;

  /**
   * Allow infinity value in numbers
   */
  static constexpr bool allow_infinity = false;

  /**
   * Allow NaN value in numbers
   */
  static constexpr bool allow_nan = false;

  /**
   * Allow hexadecimal in numbers
   */
  static constexpr bool allow_hexadecimal = false;

  /**
   * Allow single quote in strings
   */
  static constexpr bool allow_single_quote = false;

  /**
   * Allow span multi-line in strings
   */
  static constexpr bool allow_multiple_line_string = false;

  /**
   * Allow trailing comma in arrays and objects
   */
  static constexpr bool allow_trailing_comma = false;

  /**
   * Allow unquoted key in objects
   */
  static constexpr bool allow_unquoted_key = false;

private:
  ecma404() = delete;
};

/**
 * @class json5pp::policy::json5
 * Defines JSON5 - JSON for Humans
 * https://github.com/json5/json5
 */
struct json5
{
  static constexpr bool allow_single_line_comment = true;
  static constexpr bool allow_multi_line_comment = true;
  static constexpr bool allow_explicit_plus_sign = true;
  static constexpr bool allow_leading_decimal_point = true;
  static constexpr bool allow_trailing_decimal_point = true;
  static constexpr bool allow_infinity = true;
  static constexpr bool allow_nan = true;
  static constexpr bool allow_hexadecimal = true;
  static constexpr bool allow_single_quote = true;
  static constexpr bool allow_multiple_line_string = true;
  static constexpr bool allow_trailing_comma = true;
  static constexpr bool allow_unquoted_key = true;

private:
  json5() = delete;
};

} /* namespace policy */

namespace indent {

/**
 * @class json5pp::indent::spec
 * Defines indent specification
 */
template <char CHAR, unsigned int REPEAT>
class spec
{
public:
  spec() : count(0), str("") {}
  ~spec() = default;
  spec inner() const { return spec<CHAR, REPEAT>(count + REPEAT); }
  const std::string& get() const { return str; }
private:
  explicit spec(unsigned int count) : count(count), str(count, CHAR) {}
  const unsigned int count;
  const std::string str;
};

template <unsigned int repeat>
using spaces = spec<' ', repeat>;
using tab = spec<'\t', 1>;

} /* namespace indent */

namespace value {

class base;
class null;
class boolean;
class number;
class string;
class array;
class object;
using ptr = typename std::shared_ptr<base>;
template <typename P> struct impl;

enum type_id {
  null_id,
  boolean_id,
  number_id,
  string_id,
  array_id,
  object_id,
};

/**
 * @class json5pp::value::base
 * An abstract base class of values in JSON.
 */
class base
{
protected:
  base(type_id id) noexcept : type_id(id) {}
public:
  const value::type_id type_id;
public:
  virtual ~base() noexcept = default;
public:
  virtual bool is_null() const noexcept { return false; };
  virtual bool is_boolean() const noexcept { return false; };
  virtual bool is_number() const noexcept { return false; };
  virtual bool is_string() const noexcept { return false; };
  virtual bool is_array() const noexcept { return false; };
  virtual bool is_object() const noexcept { return false; };
public:
  virtual null& as_null() { throw std::bad_cast(); }
  virtual boolean& as_boolean() { throw std::bad_cast(); }
  virtual number& as_number() { throw std::bad_cast(); }
  virtual string& as_string() { throw std::bad_cast(); }
  virtual array& as_array() { throw std::bad_cast(); }
  virtual object& as_object() { throw std::bad_cast(); }
public:
  virtual const null& as_null() const { throw std::bad_cast(); }
  virtual const boolean& as_boolean() const { throw std::bad_cast(); }
  virtual const number& as_number() const { throw std::bad_cast(); }
  virtual const string& as_string() const { throw std::bad_cast(); }
  virtual const array& as_array() const { throw std::bad_cast(); }
  virtual const object& as_object() const { throw std::bad_cast(); }
public:
  template <typename P = policy::ecma404>
  void stringify(std::ostream& out) const
  {
    value::impl<P>::stringify(*this, out);
  }

  template <typename P = policy::ecma404>
  std::string stringify() const
  {
    std::ostringstream out;
    value::impl<P>::stringify(*this, out);
    return out.str();
  }

  template <typename P = policy::ecma404, typename I>
  void stringify(std::ostream& out, const I& indent) const
  {
    value::impl<P>::stringify(*this, out, indent);
  }

  template <typename P = policy::ecma404, typename I>
  std::string stringify(const I& indent) const
  {
    std::ostringstream out;
    value::impl<P>::stringify(*this, out, indent);
    return out.str();
  }

  void stringify5(std::ostream& out) const
  {
    this->stringify<policy::json5>(out);
  }

  std::string stringify5() const
  {
    return this->stringify<policy::json5>();
  }

  template <typename I>
  void stringify5(std::ostream& out, const I& indent) const
  {
    this->stringify<policy::json5, I>(out, indent);
  }

  template <typename I>
  std::string stringify5(const I& indent) const
  {
    return this->stringify<policy::json5, I>(indent);
  }
};  /* class base */

/**
 * @class json5pp::value::null
 * A class of objects which represent `null' value in JSON.
 */
class null : public base
{
  static constexpr auto context = "null";
public:
  null() noexcept : base(type_id::null_id) {}
  null(const null& src) noexcept : base(type_id::null_id) {}
  virtual bool is_null() const noexcept { return true; }
  virtual null& as_null() { return *this; }
  virtual const null& as_null() const { return *this; }
public:
  operator bool() const { return false; }
public:
  template <typename P>
  static ptr parse(std::istream& in)
  {
    int ch = impl<P>::skip_spaces(in);
    // ["null"]
    if ((ch == 'n') && impl<P>::equals(in, 'u', 'l', 'l')) {
      return std::make_shared<null>();
    }
    throw syntax_error(ch, context);
  }
  template <typename P>
  static void stringify(const base& value, std::ostream& out)
  {
    out << "null";
  }
};  /* class null */

/**
 * @class json5pp::value::boolean
 * A class of objects which represent `true' or `false' value in JSON.
 */
class boolean : public base
{
  static constexpr auto context = "boolean";
public:
  explicit boolean(bool value = false) noexcept : base(type_id::boolean_id), value(value) {}
  boolean(const boolean& src) noexcept : base(type_id::boolean_id), value(src.value) {}
  virtual bool is_boolean() const noexcept { return true; }
  virtual boolean& as_boolean() { return *this; }
  virtual const boolean& as_boolean() const { return *this; }
public:
  boolean& operator =(bool value) noexcept { set(value); return *this; }
  void set(bool value) noexcept { this->value = value; }
  operator bool() const noexcept { return get(); }
  bool get() const noexcept { return value; }
private:
  bool value;
public:
  template <typename P>
  static ptr parse(std::istream& in)
  {
    int ch = impl<P>::skip_spaces(in);
    if (ch == 't') {
      // ["true"]
      if (impl<P>::equals(in, 'r', 'u', 'e')) {
        return std::make_shared<boolean>(true);
      }
    } else if (ch == 'f') {
      // ["false"]
      if (impl<P>::equals(in, 'a', 'l', 's', 'e')) {
        return std::make_shared<boolean>(false);
      }
    }
    throw syntax_error(ch, context);
  }
  template <typename P>
  static void stringify(const boolean& value, std::ostream& out)
  {
    out << (value.value ? "true" : "false");
  }
};  /* class boolean */

/**
 * @class json5pp::value::number
 * A class of objects which represent numeric value in JSON.
 */
class number : public base
{
  static constexpr auto context = "number";
public:
  explicit number(double value = 0.0) noexcept : base(type_id::number_id), value(value) {}
  number(const number& src) noexcept : base(type_id::number_id), value(src.value) {}
  virtual bool is_number() const noexcept { return true; }
  virtual number& as_number() { return *this; }
  virtual const number& as_number() const { return *this; }
public:
  number& operator =(double value) noexcept { set(value); return *this; }
  void set(double value) noexcept { this->value = value; }
  number& operator =(int value) noexcept { set(value); return *this; }
  void set(int value) noexcept { this->value = (double)value; }
  operator double() const noexcept { return get(); }
  double get() const noexcept { return value; }
  operator int() const noexcept { return get_as_int(); }
  int get_as_int() const noexcept { return (int)value; }
private:
  double value;
public:
  template <typename P>
  static ptr parse(std::istream& in)
  {
    unsigned long long int_part = 0;
    unsigned long long frac_part = 0;
    int frac_divs = 0;
    int exp_part = 0;
    bool exp_negative = false;
    bool negative = false;

    int ch = impl<P>::skip_spaces(in);
    // [int]
    if (ch == '-') {
      negative = true;
      ch = in.get();
    } else if ((P::allow_explicit_plus_sign) && (ch == '+')) {
      ch = in.get();
    }
    // [digit|digits]
    for (;;) {
      if (ch == '0') {
        // ["0"]
        ch = in.get();
        break;
      } else if (impl<P>::is_digit(ch)) {
        // [onenine]
        int_part = impl<P>::to_number(ch);
        for (; ch = in.get(), impl<P>::is_digit(ch);) {
          int_part *= 10;
          int_part += impl<P>::to_number(ch);
        }
        break;
      } else if ((P::allow_leading_decimal_point) && (ch == '.')) {
        // ['.'] (JSON5)
        break;
      } else if ((P::allow_infinity) && (ch == 'i')) {
        // ["infinity"] (JSON5)
        if (impl<P>::equals(in, 'n', 'f', 'i', 'n', 'i', 't', 'y')) {
          return std::make_shared<number>(negative ?
            -std::numeric_limits<double>::infinity() :
            +std::numeric_limits<double>::infinity()
          );
        }
      } else if ((P::allow_nan) && (ch == 'N')) {
        // ["NaN"] (JSON5)
        if (impl<P>::equals(in, 'a', 'N')) {
          return std::make_shared<number>(std::numeric_limits<double>::quiet_NaN());
        }
      }
      throw syntax_error(ch, context);
    }
    if (ch == '.') {
      // [frac]
      for (; ch = in.get(), impl<P>::is_digit(ch); ++frac_divs) {
        frac_part *= 10;
        frac_part += impl<P>::to_number(ch);
      }
      if ((!P::allow_trailing_decimal_point) && (frac_divs == 0)) {
        throw syntax_error(ch, context);
      }
    }
    if ((ch == 'e') || (ch == 'E')) {
      // [exp]
      ch = in.get();
      switch (ch) {
      case '-':
        exp_negative = true;
        /* no-break */
      case '+':
        ch = in.get();
        break;
      }
      bool no_digit = true;
      for (; impl<P>::is_digit(ch); no_digit = false, ch = in.get()) {
        exp_part *= 10;
        exp_part += impl<P>::to_number(ch);
      }
      if (no_digit) {
        throw syntax_error(ch, context);
      }
    }
    in.unget();
    double value = (double)int_part;
    if (frac_part > 0) {
      value += (frac_part * std::pow(10, -frac_divs));
    }
    if (exp_part > 0) {
      value *= std::pow(10, exp_negative ? -exp_part : +exp_part);
    }
    return std::make_shared<number>(negative ? -value : +value);
  }

  template <typename P>
  static void stringify(const number& value, std::ostream& out)
  {
    if (std::isinf(value.value)) {
      if (P::allow_infinity) {
        if (value.value < 0) {
          out << "-";
        }
        out << "infinity";
      } else {
        null::stringify<P>(value, out);
      }
      return;
    } else if (std::isnan(value.value)) {
      if (P::allow_nan) {
        out << "NaN";
      } else {
        null::stringify<P>(value, out);
      }
      return;
    }
    out << value.value;
  }
};  /* class number */

/**
 * @class json5pp::value::string
 * A class of objects which represent string in JSON.
 */
class string : public base, public std::string
{
  static constexpr auto context = "string";
public:
  string() noexcept : base(type_id::string_id) {}
  string(const std::string& value) noexcept : base(type_id::string_id), std::string(value) {}
  virtual bool is_string() const noexcept { return true; }
  virtual string& as_string() { return *this; }
  virtual const string& as_string() const { return *this; }
public:
  string& operator =(const std::string& value) noexcept { set(value); return *this; }
  void set(const std::string& value) noexcept { this->value = value; }
  operator const std::string&() const noexcept { return get(); }
  const std::string& get() const noexcept { return this->value; }
  operator const char *() const noexcept { return get().c_str(); }
private:
  std::string value;
public:
  template <typename P>
  static void parse(std::istream& in, std::string& buffer)
  {
    int quote = impl<P>::skip_spaces(in);
    if ((quote != '"') && ((!P::allow_single_quote) || (quote != '\''))) {
      throw syntax_error((char)quote, context);
    }
    buffer.clear();
    for (;;) {
      int ch = in.get();
      if (ch == quote) {
        break;
      } else if (ch < ' ') {
        throw syntax_error(ch, context);
      } else if (ch == '\\') {
        // [escape]
        ch = in.get();
        switch (ch) {
        case '\'':
          if (!P::allow_single_quote) {
            throw syntax_error(ch, context);
          }
          break;
        case '"':
        case '\\':
        case '/':
          break;
        case 'b':
          ch = '\b';
          break;
        case 'f':
          ch = '\f';
          break;
        case 'n':
          ch = '\n';
          break;
        case 'r':
          ch = '\r';
          break;
        case 't':
          ch = '\t';
          break;
        case 'u':
          // ['u' hex hex hex hex]
          {
            char16_t code = 0;
            for (int i = 0; i < 4; ++i) {
              ch = in.get();
              int n = impl<P>::to_number_hex(ch);
              if (n < 0) {
                throw syntax_error(ch, context);
              }
              code = (code << 4) + n;
            }
            if (code < 0x80) {
              buffer.append(1, (char)code);
            } else if (code < 0x800) {
              buffer.append(1, (char)(0xc0 | (code >> 6)));
              buffer.append(1, (char)(0x80 | (code & 0x3f)));
            } else {
              buffer.append(1, (char)(0xe0 | (code >> 12)));
              buffer.append(1, (char)(0x80 | ((code >> 6) & 0x3f)));
              buffer.append(1, (char)(0x80 | (code & 0x3f)));
            }
          }
          continue;
        case '\r':
          if (P::allow_multiple_line_string) {
            ch = in.get();
            if (ch != '\n') {
              in.unget();
            }
            continue;
          }
          /* no-break */
        case '\n':
          if (P::allow_multiple_line_string) {
            continue;
          }
          /* no-break */
        default:
          throw syntax_error(ch, context);
        }
      }
      buffer.append(1, (char)ch);
    }
  }

  template <typename P>
  static ptr parse(std::istream& in)
  {
    std::string buffer;
    parse<P>(in, buffer);
    return std::make_shared<string>(buffer);
  }

  template <typename P>
  static void stringify(const std::string& value, std::ostream& out)
  {
    static const auto hex = "0123456789abcdef";
    out << '"';
    for (const auto& i : value) {
      unsigned char ch = i;
      if (ch == '"') {
        out << "\\\"";
      } else if (ch == '\\') {
        out << "\\\\";
      } else if (ch == '\b') {
        out << "\\b";
      } else if (ch == '\f') {
        out << "\\f";
      } else if (ch == '\n') {
        out << "\\n";
      } else if (ch == '\r') {
        out << "\\r";
      } else if (ch == '\t') {
        out << "\\t";
      } else if (ch < ' ') {
        out << "\\u00";
        out.put(hex[ch >> 4]);
        out.put(hex[ch & 0xf]);
      } else {
        out.put(ch);
      }
    }
    out << '"';
  }
};  /* class string */

/**
 * @class json5pp::value::array
 * A class of objects which represent array in JSON.
 */
class array : public base, public std::vector<ptr>
{
  static constexpr auto context = "array";
public:
  using container_t = typename std::vector<ptr>;
  array() noexcept : base(type_id::array_id) {}
  array(const container_t& values) noexcept : base(type_id::array_id), container_t(values) {}
  array(const array& src) noexcept : base(type_id::array_id), container_t(src) {}
  virtual bool is_array() const noexcept { return true; }
  virtual array& as_array() { return *this; }
  virtual const array& as_array() const { return *this; }
public:
  template <typename P>
  static ptr parse(std::istream& in)
  {
    int ch = impl<P>::skip_spaces(in);
    if (ch != '[') {
      throw syntax_error(ch, context);
    }
    container_t values;
    for (;;) {
      ch = impl<P>::skip_spaces(in);
      if (ch == ']') {
        break;
      }
      if (values.empty()) {
        in.unget();
      } else if (ch != ',') {
        throw syntax_error(ch, context);
      } else if (P::allow_trailing_comma) {
        ch = in.get();
        if (ch == ']') {
          break;
        }
        in.unget();
      }
      // [value]
      values.push_back(value::impl<P>::parse(in));
    }
    return std::make_shared<array>(values);
  }

  template <typename P>
  static void stringify(const array& value, std::ostream& out)
  {
    out << "[";
    bool comma = false;
    for (const auto& i : value) {
      if (comma) {
        out << ",";
      }
      i->stringify<P>(out);
      comma = true;
    }
    out << "]";
  }

  template <typename P, typename I>
  static void stringify(const array& value, std::ostream& out, const I& indent)
  {
    out << "[";
    bool comma = false;
    auto indent_inner = indent.inner();
    for (const auto& i : value) {
      out << (comma ? ",\n" : "\n") << indent_inner.get();
      i->stringify<P, I>(out, indent_inner);
      comma = true;
    }
    out << "\n" << indent.get() << "]";
  }
};  /* class array */

/**
 * @class json5pp::value::object
 * A class of objects which represent object in JSON.
 */
class object : public base, public std::map<std::string, ptr>
{
  static constexpr auto context = "object";
public:
  using key_t = std::string;
  using container_t = typename std::map<key_t, ptr>;
  object() noexcept : base(type_id::object_id) {}
  object(const container_t& values) noexcept : base(type_id::object_id), container_t(values) {}
  object(const object& src) noexcept : base(type_id::object_id), container_t(src) {}
  virtual bool is_object() const noexcept { return true; }
  virtual object& as_object() { return *this; }
  virtual const object& as_object() const { return *this; }
private:
  template <typename P>
  static std::string parse_key(std::istream& in)
  {
    std::string buffer;
    if (P::allow_unquoted_key) {
      int ch = impl<P>::skip_spaces(in);
      if ((ch != '"') && (ch != '\'')) {
        for (;; ch = in.get()) {
          if ((ch == '_') || (ch == '$') || (impl<P>::is_alpha(ch))) {
            // [IdentifierStart]
          } else if (impl<P>::is_digit(ch) && (!buffer.empty())) {
            // [UnicodeDigit]
          } else if (ch == ':') {
            break;
          } else {
            throw syntax_error(ch, context);
          }
          buffer.append(1, ch);
        }
        in.unget();
        return buffer;
      }
      in.unget();
    }
    string::parse<P>(in, buffer);
    return buffer;
  }
public:
  template <typename P>
  static ptr parse(std::istream& in)
  {
    int ch = impl<P>::skip_spaces(in);
    if (ch != '{') {
      throw syntax_error(ch, context);
    }
    object::container_t values;
    for (;;) {
      ch = impl<P>::skip_spaces(in);
      if (ch == '}') {
        break;
      }
      if (values.empty()) {
        in.unget();
      } else if (ch != ',') {
        throw syntax_error(ch, context);
      } else if (P::allow_trailing_comma) {
        ch = in.get();
        if (ch == '}') {
          break;
        }
        in.unget();
      }
      // [string]
      // [key] (JSON5)
      const std::string key = parse_key<P>(in);
      ch = impl<P>::skip_spaces(in);
      if (ch != ':') {
        throw syntax_error(ch, context);
      }
      // [value]
      values.emplace(key, value::impl<P>::parse(in));
    }
    return std::make_shared<object>(values);
  }

  template <typename P>
  static void stringify(const object& value, std::ostream& out)
  {
    out << "{";
    bool comma = false;
    for (const auto& i : value) {
      if (comma) {
        out << ",";
      }
      string::stringify<P>(i.first, out);
      out << ":";
      i.second->stringify<P>(out);
      comma = true;
    }
    out << "}";
  }

  template <typename P, typename I>
  static void stringify(const object& value, std::ostream& out, const I& indent)
  {
    out << "{";
    bool comma = false;
    auto indent_inner = indent.inner();
    for (const auto& i : value) {
      out << (comma ? ",\n" : "\n") << indent_inner.get();
      string::stringify<P>(i.first, out);
      out << ": ";
      i.second->stringify<P, I>(out, indent_inner);
      comma = true;
    }
    out << "\n" << indent.get() << "}";
  }
};  /* class object */

template <typename P>
struct impl
{
  friend class value::base;
  friend class value::null;
  friend class value::boolean;
  friend class value::number;
  friend class value::string;
  friend class value::array;
  friend class value::object;
  friend value::ptr parse(std::istream& in);

  static ptr parse(std::istream& in)
  {
    int ch = skip_spaces(in);
    in.unget();

    // [value]
    switch (ch) {
    case '{':
      // [object]
      return object::parse<P>(in);
    case '[':
      // [array]
      return array::parse<P>(in);
    case '"':
    case '\'':
      // [string]
      return string::parse<P>(in);
    case 'n':
      // ["null"]?
      return null::parse<P>(in);
    case 't':
    case 'f':
      // ["true"] or ["false"]?
      return boolean::parse<P>(in);
    default:
      // [number]?
      return number::parse<P>(in);
    }
  }

  static void stringify(const base& value, std::ostream& out)
  {
    switch (value.type_id) {
    case type_id::null_id:
      return value::null::stringify<P>(value.as_null(), out);
    case type_id::boolean_id:
      return value::boolean::stringify<P>(value.as_boolean(), out);
    case type_id::number_id:
      return value::number::stringify<P>(value.as_number(), out);
    case type_id::string_id:
      return value::string::stringify<P>(value.as_string(), out);
    case type_id::array_id:
      return value::array::stringify<P>(value.as_array(), out);
    case type_id::object_id:
      return value::object::stringify<P>(value.as_object(), out);
    default:
      throw std::invalid_argument("unexpected JSON value");
    }
  }

  template <typename I>
  static void stringify(const base& value, std::ostream& out, const I& indent)
  {
    switch (value.type_id) {
    case type_id::null_id:
      return value::null::stringify<P>(value.as_null(), out);
    case type_id::boolean_id:
      return value::boolean::stringify<P>(value.as_boolean(), out);
    case type_id::number_id:
      return value::number::stringify<P>(value.as_number(), out);
    case type_id::string_id:
      return value::string::stringify<P>(value.as_string(), out);
    case type_id::array_id:
      return value::array::stringify<P, I>(value.as_array(), out, indent);
    case type_id::object_id:
      return value::object::stringify<P, I>(value.as_object(), out, indent);
    default:
      throw std::invalid_argument("unexpected JSON value");
    }
  }

  static int skip_spaces(std::istream& in)
  {
    for (;;) {
      int ch = in.get();
    reeval_space:
      switch (ch) {
      case '\t':
      case '\n':
      case '\r':
      case ' ':
        continue;
      case '/':
        if (P::allow_single_line_comment || P::allow_multi_line_comment) {
          ch = in.get();
          if (P::allow_single_line_comment && (ch == '/')) {
            // [single_line_comment] (JSON5)
            for (;;) {
              ch = in.get();
              if ((ch == std::char_traits<char>::eof()) || (ch == '\r') || (ch == '\n')) {
                break;
              }
            }
            goto reeval_space;
          } else if (P::allow_multi_line_comment && (ch == '*')) {
            // [multi_line_comment] (JSON5)
            for (;;) {
              ch = in.get();
            reeval_asterisk:
              if (ch == std::char_traits<char>::eof()) {
                throw syntax_error(ch, "comment");
              }
              if (ch != '*') {
                continue;
              }
              ch = in.get();
              if (ch == '*') {
                goto reeval_asterisk;
              }
              if (ch == '/') {
                break;
              }
            }
            continue;
          }
          // no valid comments
        }
        /* no-break */
      default:
        return ch;
      }
    }
  }

  static bool is_digit(int ch)
  {
    return (('0' <= ch) && (ch <= '9'));
  }

  static int to_number(int ch)
  {
    return ch - '0';
  }

  static int to_number_hex(int ch)
  {
    if (is_digit(ch)) {
      return to_number(ch);
    } else if (('A' <= ch) && (ch <= 'F')) {
      return ch - 'A' + 10;
    } else if (('a' <= ch) && (ch <= 'f')) {
      return ch - 'a' + 10;
    }
    return -1;
  }

  static int is_alpha(int ch)
  {
    return (('A' <= ch) && (ch <= 'Z')) || (('a' <= ch) && (ch <= 'z'));
  }

  template <typename ...Types>
  static bool equals(std::istream& in, char expected, Types... sequence)
  {
    return equals(in, expected) && equals(in, sequence...);
  }

  template <typename=void>
  static bool equals(std::istream& in, char expected)
  {
    return (in.get() == expected);
  }

  impl() = delete;
};  /* class private */

} /* namespace value */

template <typename P = policy::ecma404>
value::ptr parse(std::istream& in, const bool finish = true)
{
  const auto result = value::impl<P>::parse(in);
  if (finish) {
    const auto ch = value::impl<P>::skip_spaces(in);
    if (ch != std::char_traits<char>::eof()) {
      throw syntax_error(ch, "JSON");
    }
  }
  return result;
}

template <typename P = policy::ecma404>
value::ptr parse(const std::string& str, const bool finish = true)
{
  std::istringstream in(str);
  return parse<P>(in, finish);
}

static value::ptr parse5(std::istream& in, const bool finish = true)
{
  return parse<policy::json5>(in, finish);
}

static value::ptr parse5(const std::string& str, const bool finish = true)
{
  return parse<policy::json5>(str, finish);
}

} /* namespace json */

#endif  /* _JSON5PP_HPP_ */
