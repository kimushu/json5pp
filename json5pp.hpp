#ifndef _JSON5PP_HPP_
#define _JSON5PP_HPP_

#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <initializer_list>
#include <cmath>
#include <limits>

namespace json5pp {

/*
 * json5pp uses semantic versioning
 * See: https://semver.org/
 */
namespace version {
  static constexpr auto major = 2;
  static constexpr auto minor = 0;
  static constexpr auto patch = 0;
}

/**
 * @class syntax_error
 * A class of objects thrown as exceptions to report a JSON syntax error.
 */
class syntax_error : public std::invalid_argument
{
public:
  /**
   * Construct syntax_error object with failed character.
   * @param ch Character which raised error or Traits::eof()
   * @param context_name Context description in human-readable string
   */
  syntax_error(int ch, const char *context_name)
  : std::invalid_argument(
      std::string("JSON syntax error: ") +
      (ch != std::char_traits<char>::eof() ?
        "illegal character `" + std::string(1, ch) + "'" :
        "unexpected EOS") +
      " in " + context_name) {}
};

namespace impl {

template <class T> class parse_context;
/*template <class T> */class stringify_context;

} /* namespace impl */

/**
 * @brief A class to hold JSON value
 */
class value
{
private:
  enum type_enum {
    TYPE_NULL,
    TYPE_BOOLEAN,
    TYPE_NUMBER,
    TYPE_STRING,
    TYPE_ARRAY,
    TYPE_OBJECT,
  } type;

public:
  using null_type = std::nullptr_t;
  using boolean_type = bool;
  using number_type = double;
  using number_i_type = int;
  using string_type = std::string;
  using array_type = std::vector<value>;
  using object_type = std::map<string_type, value>;
  using pair_type = object_type::value_type;
  using json_type = std::string;

  /*================================================================================
   * Construction
   */
public:
  /**
   * @brief JSON value default constructor for "null" type.
   */
  value() noexcept : value(nullptr) {}

  /**
   * @brief JSON value constructor for "null" type.
   * @param null A dummy argument for nullptr
   */
  value(null_type null) noexcept : type(TYPE_NULL) {}

  /**
   * @brief JSON value constructor for "boolean" type.
   * @param boolean A boolean value to be set.
   */
  value(boolean_type boolean) noexcept : type(TYPE_BOOLEAN), content(boolean) {}

  /**
   * @brief JSON value constructor for "number" type.
   * @param number A number to be set.
   */
  value(number_type number) noexcept : type(TYPE_NUMBER), content(number) {}

  /**
   * @brief JSON value constructor with integer for "number" type.
   * @param number An integer value to be set.
   */
  value(number_i_type number) noexcept : type(TYPE_NUMBER), content(number) {}

  /**
   * @brief JSON value constructor for "string" type.
   * @param string A string value to be set.
   */
  value(const string_type& string) : type(TYPE_STRING)
  {
    new(&content.string) string_type(string);
  }

  /**
   * @brief JSON value constructor for "array" type.
   * @param array An initializer list of elements.
   */
  explicit value(std::initializer_list<value> array) : type(TYPE_ARRAY)
  {
    new(&content.array) array_type(array);
  }

  /**
   * @brief JSON value constructor with key,value pair for "object" type.
   * @param elements An initializer list of key,value pair.
   */
  explicit value(std::initializer_list<pair_type> elements) : type(TYPE_OBJECT)
  {
    new(&content.object) object_type(elements);
  }

  /**
   * @brief JSON value copy constructor.
   * @param src A value to be copied from.
   */
  value(const value& src) : type(TYPE_NULL)
  {
    *this = src;
  }

  /**
   * @brief JSON value move constructor.
   * @param src A value to be moved from.
   */
  value(value&& src) : type(src.type)
  {
    switch (type) {
    case TYPE_BOOLEAN:
      new (&content.boolean) boolean_type(std::move(src.content.boolean));
      break;
    case TYPE_NUMBER:
      new (&content.number) number_type(std::move(src.content.number));
      break;
    case TYPE_STRING:
      new (&content.string) string_type(std::move(src.content.string));
      break;
    case TYPE_ARRAY:
      new (&content.array) array_type(std::move(src.content.array));
      break;
    case TYPE_OBJECT:
      new (&content.object) object_type(std::move(src.content.object));
      break;
    default:
      break;
    }
    src.type = TYPE_NULL;
  }

  friend value array(std::initializer_list<value> elements);
  friend value object(std::initializer_list<pair_type> elements);

  /*================================================================================
   * Destruction
   */
public:
  /**
   * @brief JSON value destructor.
   */
  ~value()
  {
    release();
  }

private:
  /**
   * @brief Release content
   * 
   * @param new_type A new type id
   */
  void release(type_enum new_type = TYPE_NULL)
  {
    switch (type) {
    case TYPE_BOOLEAN:
      content.boolean.~boolean_type();
      break;
    case TYPE_NUMBER:
      content.number.~number_type();
      break;
    case TYPE_STRING:
      content.string.~string_type();
      break;
    case TYPE_ARRAY:
      content.array.~array_type();
      break;
    case TYPE_OBJECT:
      content.object.~object_type();
      break;
    default:
      break;
    }
    type = new_type;
  }

  /*================================================================================
   * Type checks
   */
public:
  /**
   * @brief Check if stored value is null.
   */
  bool is_null() const noexcept { return type == TYPE_NULL; }

  /**
   * @brief Check if type of stored value is boolean.
   */
  bool is_boolean() const noexcept { return type == TYPE_BOOLEAN; }

  /**
   * @brief Check if type of stored value is number.
   */
  bool is_number() const noexcept { return type == TYPE_NUMBER; }

  /**
   * @brief Check if type of stored value is string.
   */
  bool is_string() const noexcept { return type == TYPE_STRING; }

  /**
   * @brief Check if type of stored value is array.
   */
  bool is_array() const noexcept { return type == TYPE_ARRAY; }

  /**
   * @brief Check if type of stored value is object.
   */
  bool is_object() const noexcept { return type == TYPE_OBJECT; }

  /*================================================================================
   * Type casts
   */
public:
  null_type as_null() const
  {
    if (type != TYPE_NULL) { throw std::bad_cast(); }
    return nullptr;
  }

  boolean_type as_boolean() const
  {
    if (type != TYPE_BOOLEAN) { throw std::bad_cast(); }
    return content.boolean;
  }

  boolean_type& as_boolean()
  {
    if (type != TYPE_BOOLEAN) { throw std::bad_cast(); }
    return content.boolean;
  }

  number_type as_number() const
  {
    if (type != TYPE_NUMBER) { throw std::bad_cast(); }
    return content.number;
  }

  number_type& as_number()
  {
    if (type != TYPE_NUMBER) { throw std::bad_cast(); }
    return content.number;
  }

  number_i_type as_integer() const
  {
    if (type != TYPE_NUMBER) { throw std::bad_cast(); }
    return static_cast<number_i_type>(content.number);
  }

  const string_type& as_string() const
  {
    if (type != TYPE_STRING) { throw std::bad_cast(); }
    return content.string;
  }

  string_type& as_string()
  {
    if (type != TYPE_STRING) { throw std::bad_cast(); }
    return content.string;
  }

  const array_type& as_array() const
  {
    if (type != TYPE_ARRAY) { throw std::bad_cast(); }
    return content.array;
  }

  array_type& as_array()
  {
    if (type != TYPE_ARRAY) { throw std::bad_cast(); }
    return content.array;
  }

  const object_type& as_object() const
  {
    if (type != TYPE_OBJECT) { throw std::bad_cast(); }
    return content.object;
  }

  object_type& as_object()
  {
    if (type != TYPE_OBJECT) { throw std::bad_cast(); }
    return content.object;
  }

  /*================================================================================
   * Assignment (Copying)
   */
public:
  /**
   * @brief Copy from another JSON value object.
   * @param src A value object.
   */
  value& operator =(const value& src)
  {
    release(src.type);
    switch (type) {
    case TYPE_BOOLEAN:
      new (&content.boolean) boolean_type(src.content.boolean);
      break;
    case TYPE_NUMBER:
      new (&content.number) number_type(src.content.number);
      break;
    case TYPE_STRING:
      new (&content.string) string_type(src.content.string);
      break;
    case TYPE_ARRAY:
      new (&content.array) array_type(src.content.array);
      break;
    case TYPE_OBJECT:
      new (&content.object) object_type(src.content.object);
      break;
    default:
      break;
    }
    return *this;
  }

  /**
   * @brief Assign null value.
   * @param null A dummy value.
   */
  value& operator =(null_type null)
  {
    release();
    return *this;
  }

  /**
   * @brief Assign boolean value.
   * @param boolean A boolean value to be set.
   */
  value& operator =(boolean_type boolean)
  {
    release(TYPE_BOOLEAN);
    new (&content.boolean) boolean_type(boolean);
    return *this;
  }

  /**
   * @brief Assign number value.
   * @param number A number to be set.
   */
  value& operator =(number_type number)
  {
    release(TYPE_NUMBER);
    new (&content.number) number_type(number);
    return *this;
  }

  /**
   * @brief Assign number value by integer type.
   * @param number A integer number to be set.
   */
  value& operator =(number_i_type number)
  {
    release(TYPE_NUMBER);
    new (&content.number) number_type(number);
    return *this;
  }

  /**
   * @brief Assign string value.
   * @param string A string to be set.
   */
  value& operator =(const string_type& string)
  {
    if (type == TYPE_STRING) {
      content.string = string;
    } else {
      release(TYPE_STRING);
      new (&content.string) string_type(string);
    }
    return *this;
  }

  /**
   * @brief Assign string value from const char*
   * @param string A string to be set.
   */
  value& operator =(const char* string)
  {
    return (*this = std::string(string));
  }

  /**
   * @brief Assign array value by deep copy.
   * @param array An array to be set.
   */
  value& operator =(std::initializer_list<value> array)
  {
    if (type == TYPE_ARRAY) {
      content.array = array;
    } else {
      release(TYPE_ARRAY);
      new (&content.array) array_type(array);
    }
    return *this;
  }

  /**
   * @brief Assign object value by deep copy.
   * @param object An object to be set.
   */
  value& operator =(std::initializer_list<value::pair_type> elements)
  {
    if (type == TYPE_OBJECT) {
      content.object = elements;
    } else {
      release(TYPE_OBJECT);
      new (&content.object) object_type(elements);
    }
    return *this;
  }

  /*================================================================================
   * Parse
   */
private:
  friend value parse(std::istream& istream, bool finished);
  friend value parse5(std::istream& istream, bool finished);

  /*================================================================================
   * Stringify
   */
  friend class impl::stringify_context;

  friend impl::stringify_context operator<<(std::ostream& ostream, const value& value);

  template <class... T>
  json_type stringify(T... args) const;

private:

  /**
   * @brief Do stringify (A specialized function for manipulator expansion)
   * 
   * @param context A stringify context
   */
  void stringify_with_manip(impl::stringify_context& context) const
  {
    stringify_to(context);
  }

  /**
   * @brief A function for manipulator expansion
   * 
   * @tparam M A typename of manipulator
   * @tparam T... A list of typenames of manipulator
   * @param context A stringify context
   * @param manip The first manipulator to be applied
   * @param args The remainings
   */
  template <class M, class... T>
  void stringify_with_manip(impl::stringify_context& context, const M& manip, T... args) const
  {
    stringify_with_manip(context << manip, args...);
  }

  void stringify_to(impl::stringify_context& context) const;
  void stringify_to(impl::stringify_context& context, const std::string indent) const;

  /**
   * @brief Stringify string
   * 
   * @param out An output stream
   * @param string A string to be stringified
   */
  static void stringify_string(std::ostream& out, const string_type& string)
  {
    out << "\"";
    for (const auto& i : string) {
      const auto ch = (unsigned char)i;
      static const char hex[] = "0123456789abcdef";
      switch (ch) {
      case '"':  out << "\\\""; break;
      case '\\': out << "\\\\"; break;
      case '\b': out << "\\b";  break;
      case '\f': out << "\\f";  break;
      case '\n': out << "\\n";  break;
      case '\r': out << "\\r";  break;
      case '\t': out << "\\t";  break;
      default:
        if (ch < ' ') {
          out << "\\u00";
          out.put(hex[(ch >> 4) & 0xf]);
          out.put(hex[ch & 0xf]);
        } else {
          out.put(ch);
        }
        break;
      }
    }
    out << "\"";
  }

  /*================================================================================
   * Internal data structure
   */
private:
  union content {
    boolean_type boolean;
    number_type number;
    string_type string;
    array_type array;
    object_type object;
    content() {}
    content(boolean_type boolean) : boolean(boolean) {}
    content(number_type number) : number(number) {}
    content(number_i_type number) : number(number) {}
    ~content() {}
  } content;
};

/**
 * @brief Make JSON array
 * 
 * @param elements An initializer list of elements
 * @return JSON value object
 */
inline value array(std::initializer_list<value> elements)
{
  value v;
  v.release(value::TYPE_ARRAY);
  new (&v.content.array) value::array_type(elements);
  return std::move(v);
}

/**
 * @brief Make JSON object
 * 
 * @param elements An initializer list of key:value pairs
 * @return JSON value object
 */
inline value object(std::initializer_list<value::pair_type> elements)
{
  value v;
  v.release(value::TYPE_OBJECT);
  new (&v.content.object) value::object_type(elements);
  return std::move(v);
}

namespace impl {

enum flags : std::uint16_t
{
  single_line_comment     = (1u<<0),
  multi_line_comment      = (1u<<1),
  explicit_plus_sign      = (1u<<2),
  leading_decimal_point   = (1u<<3),
  trailing_decimal_point  = (1u<<4),
  infinity_number         = (1u<<5),
  not_a_number            = (1u<<6),
  hexadecimal             = (1u<<7),
  single_quote            = (1u<<8),
  multi_line_string       = (1u<<9),
  trailing_comma          = (1u<<10),
  unquoted_key            = (1u<<11),

  json5_rules = ((unquoted_key << 1) - 1),
  all_rules = json5_rules,

  crlf_newline            = (1u<<14),
  finished                = (1u<<15),
};

using flags_type = std::underlying_type<flags>::type;
using indent_type = std::int8_t;

// Forward declarations
class context_base;
template <class T> class parse_context;
class stringify_context;

/**
 * @brief Manipulator for parse/stringify contexts
 */
class manipulator
{
  friend class context_base;
public:
  /**
   * @brief Construct a new manipulator with all member values
   * 
   * @param flags_set Flags to be set
   * @param flags_clear Flags to be cleared
   * @param indent_set If true, indent is valid
   * @param indent Indent specification
   */
  manipulator(flags_type flags_set, flags_type flags_clear, bool indent_set, indent_type indent = 0)
  : flags_set(flags_set), flags_clear(flags_clear), indent_set(indent_set), indent(indent)
  {
  }

  /**
   * @brief Construct a new manipulator with flags and set/clear selection
   * 
   * @param flags Flags to be set or cleared
   * @param set If true, flags will be used to set.
   */
  manipulator(flags_type flags, bool set)
  : flags_set(set ? flags : 0), flags_clear(set ? 0 : flags), indent_set(false), indent(0)
  {
  }

public:
  friend parse_context<void> operator>>(std::istream& istream, const manipulator& manip);
  friend stringify_context operator<<(std::ostream& ostream, const manipulator& manip);

private:
  flags_type flags_set;     ///< Flags to be set
  flags_type flags_clear;   ///< Flags to be cleared
  bool indent_set;          ///< If true, indent is valid
  indent_type indent;       ///< Indent specification
};

/**
 * @brief A base class for JSON parse/stringify context
 */
class context_base
{
public:
  flags_type flags;         ///< JSON flags
  indent_type indent;       ///< Indent specification

  /**
   * @brief Get an indent string
   * 
   * @return std::string 
   */
  std::string get_indent() const
  {
    if (indent > 0) {
      return std::string(indent, ' ');
    } else if (indent < 0) {
      return std::string(-indent, '\t');
    }
    return std::string();
  }

  /**
   * @brief Get newline string
   */
  const char* get_newline() const
  {
    return (flags & flags::crlf_newline) ? "\r\n" : "\n";
  }

  /**
   * @brief Check if flags are set
   * 
   * @param flags Flags to check
   * @retval true One of more flags are set
   * @retval false No flags are set
   */
  inline bool has_flag(flags_type flags) const
  {
    return (this->flags & flags) != 0;
  }

protected:
  /**
   * @brief Construct a new context_base object
   */
  context_base()
  : flags(0), indent(0)
  {
  }

  /**
   * @brief Apply JSON manipulator to this context
   * 
   * @param manip A manipulator
   */
  void apply_manipulator(const manipulator& manip)
  {
    flags = (flags & ~manip.flags_clear) | manip.flags_set;
    if (manip.indent_set) {
      indent = manip.indent;
    }
  }
};

/**
 * @brief JSON parse context
 * @note This class is defined as template to make symbols weak.
 * 
 * @tparam T Ignored.
 */
template <class T>
class parse_context : public context_base
{
public:
  std::istream& istream;  ///< ios stream

  /**
   * @brief Construct a new parse_context object
   * 
   * @param istream A reference to ios stream
   */
  parse_context(std::istream& istream)
  : istream(istream)
  {
  }

  /**
   * @brief Apply JSON manipulator
   * 
   * @param manip A JSON manipulator to be applied
   * @return A referenece to object itself
   */
  parse_context<T>& operator>>(const manipulator& manip)
  {
    apply_manipulator(manip);
    return *this;
  }

  // Parse JSON value (written after "class value" definition)
  parse_context<T>& operator>>(value& value);

  /**
   * @brief Delegate operator>> to ios stream
   * 
   * @tparam TT A typename of value
   * @param value A value to delegate
   * @return A reference to ios stream
   */
  template <class TT>
  std::istream& operator>>(TT& value)
  {
    return istream >> value;
  }

  /**
   * @brief Delegate operator>> for manipulators to ios stream
   * 
   * @param manip A manipulator to delegate
   * @return A reference to ios stream
   */
  std::istream& operator>>(std::istream& (*manip)(std::istream&))
  {
    return istream >> manip;
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

  static bool is_alpha(int ch)
  {
    return (('A' <= ch) && (ch <= 'Z')) || (('a' <= ch) && (ch <= 'z'));
  }

  template <class... C>
  bool equals(int& ch, char expected, C... sequence)
  {
    return equals(ch, expected) && equals(ch, sequence...);
  }

  bool equals(int& ch, char expected)
  {
    return ((ch = istream.get()) == expected);
  }

  /**
   * @brief Skip spaces (and comments) from input stream
   * 
   * @return the first non-space character
   */
  int skip_spaces()
  {
    for (;;) {
      int ch = istream.get();
    reeval_space:
      switch (ch) {
      case '\t':
      case '\n':
      case '\r':
      case ' ':
        continue;
      case '/':
        if (has_flag(flags::single_line_comment | flags::multi_line_comment)) {
          ch = istream.get();
          if (has_flag(flags::single_line_comment) && (ch == '/')) {
            // [single_line_comment] (JSON5)
            for (;;) {
              ch = istream.get();
              if ((ch == std::char_traits<char>::eof()) || (ch == '\r') || (ch == '\n')) {
                break;
              }
            }
            goto reeval_space;
          } else if (has_flag(flags::multi_line_comment) && (ch == '*')) {
            // [multi_line_comment] (JSON5)
            for (;;) {
              ch = istream.get();
            reeval_asterisk:
              if (ch == std::char_traits<char>::eof()) {
                throw syntax_error(ch, "comment");
              }
              if (ch != '*') {
                continue;
              }
              ch = istream.get();
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
      } /* switch(ch) */
    } /* for(;;) */
  }

  void parse(value& v, bool finished)
  {
    static const char context_name[] = "value";
    parse(v, context_name);
    if (finished) {
      int ch = skip_spaces();
      if (ch != std::char_traits<char>::eof()) {
        throw syntax_error(ch, context_name);
      }
    }
  }

  void parse(value& v, const char *context_name)
  {
    int ch = skip_spaces();

    // [value]
    switch (ch) {
    case '{':
      // [object]
      return parse_object(v);
    case '[':
      // [array]
      return parse_array(v);
    case '"':
    case '\'':
      // [string]
      return parse_string(v, ch);
    case 'n':
      // ["null"]?
      return parse_null(v);
    case 't':
    case 'f':
      // ["true"] or ["false"]?
      return parse_boolean(v, ch);
    default:
      if (is_digit(ch) || (ch == '-') || (ch == '+') ||
          (ch == '.') || (ch == 'i') || (ch == 'N')) {
        // [number]?
        return parse_number(v, ch);
      }
      throw syntax_error(ch, context_name);
    }
  }

  void parse_null(value& v)
  {
    static const char context_name[] = "null";
    int ch;
    if (equals(ch, 'u', 'l', 'l')) {
      v = nullptr;
      return;
    }
    throw syntax_error(ch, context_name);
  }

  void parse_boolean(value& v, int ch)
  {
    static const char context_name[] = "boolean";
    if (ch == 't') {
      if (equals(ch, 'r', 'u', 'e')) {
        v = true;
        return;
      }
    } else if (ch == 'f') {
      if (equals(ch, 'a', 'l', 's', 'e')) {
        v = false;
        return;
      }
    }
    throw syntax_error(ch, context_name);
  }

  void parse_number(value& v, int ch)
  {
    static const char context_name[] = "number";
    unsigned long long int_part = 0;
    unsigned long long frac_part = 0;
    int frac_divs = 0;
    int exp_part = 0;
    bool exp_negative = false;
    bool negative = false;

    // [int]
    if (ch == '-') {
      negative = true;
      ch = istream.get();
    } else if (has_flag(flags::explicit_plus_sign) && (ch == '+')) {
      ch = istream.get();
    }
    // [digit|digits]
    for (;;) {
      if (ch == '0') {
        // ["0"]
        ch = istream.get();
        break;
      } else if (is_digit(ch)) {
        // [onenine]
        int_part = to_number(ch);
        for (; ch = istream.get(), is_digit(ch);) {
          int_part *= 10;
          int_part += to_number(ch);
        }
        break;
      } else if (has_flag(flags::leading_decimal_point) && (ch == '.')) {
        // ['.'] (JSON5)
        break;
      } else if (has_flag(flags::infinity_number) && (ch == 'i')) {
        // ["infinity"] (JSON5)
        if (equals(ch, 'n', 'f', 'i', 'n', 'i', 't', 'y')) {
          v = negative ?
            -std::numeric_limits<value::number_type>::infinity() :
            +std::numeric_limits<value::number_type>::infinity();
          return;
        }
      } else if (has_flag(flags::not_a_number) && (ch == 'N')) {
        // ["NaN"] (JSON5)
        if (equals(ch, 'a', 'N')) {
          v = std::numeric_limits<value::number_type>::quiet_NaN();
          return;
        }
      }
      throw syntax_error(ch, context_name);
    }
    if (ch == '.') {
      // [frac]
      for (; ch = istream.get(), is_digit(ch); ++frac_divs) {
        frac_part *= 10;
        frac_part += to_number(ch);
      }
      if ((!has_flag(flags::trailing_decimal_point)) && (frac_divs == 0)) {
        throw syntax_error(ch, context_name);
      }
    }
    if ((ch == 'e') || (ch == 'E')) {
      // [exp]
      ch = istream.get();
      switch (ch) {
      case '-':
        exp_negative = true;
        /* no-break */
      case '+':
        ch = istream.get();
        break;
      }
      bool no_digit = true;
      for (; is_digit(ch); no_digit = false, ch = istream.get()) {
        exp_part *= 10;
        exp_part += to_number(ch);
      }
      if (no_digit) {
        throw syntax_error(ch, context_name);
      }
    }
    istream.unget();
    double number_value = (double)int_part;
    if (frac_part > 0) {
      number_value += (frac_part * std::pow(10, -frac_divs));
    }
    if (exp_part > 0) {
      number_value *= std::pow(10, exp_negative ? -exp_part : +exp_part);
    }
    v = negative ? -number_value : +number_value;
  }

  void parse_string(std::string& buffer, int quote, const char *context_name)
  {
    if (!((quote == '"') || (has_flag(flags::single_quote) && quote == '\''))) {
      throw syntax_error(quote, context_name);
    }
    buffer.clear();
    for (;;) {
      int ch = istream.get();
      if (ch == quote) {
        break;
      } else if (ch < ' ') {
        throw syntax_error(ch, context_name);
      } else if (ch == '\\') {
        // [escape]
        ch = istream.get();
        switch (ch) {
        case '\'':
          if (!has_flag(flags::single_quote)) {
            throw syntax_error(ch, context_name);
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
              ch = istream.get();
              int n = to_number_hex(ch);
              if (n < 0) {
                throw syntax_error(ch, context_name);
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
          if (has_flag(flags::multi_line_string)) {
            ch = istream.get();
            if (ch != '\n') {
              istream.unget();
            }
            continue;
          }
          /* no-break */
        case '\n':
          if (has_flag(flags::multi_line_string)) {
            continue;
          }
          /* no-break */
        default:
          throw syntax_error(ch, context_name);
        }
      }
      buffer.append(1, (char)ch);
    }
  }

  void parse_string(value& v, int quote)
  {
    static const char context_name[] = "string";
    v = "";
    parse_string(v.as_string(), quote, context_name);
  }

  void parse_array(value& v)
  {
    static const char context_name[] = "array";
    v = array({});
    auto& elements = v.as_array();
    for (;;) {
      int ch = skip_spaces();
      if (ch == ']') {
        break;
      }
      if (elements.empty()) {
        istream.unget();
      } else if (ch != ',') {
        throw syntax_error(ch, context_name);
      } else if (has_flag(trailing_comma)) {
        ch = istream.get();
        if (ch == ']') {
          break;
        }
        istream.unget();
      }
      // [value]
      elements.emplace_back(nullptr);
      parse(elements.back(), context_name);
    }
  }

  std::string parse_key()
  {
    static const char context_name[] = "object-key";
    std::string buffer;
    int ch = skip_spaces();
    if (has_flag(flags::unquoted_key)) {
      if ((ch != '"') && (ch != '\'')) {
        for (;; ch = istream.get()) {
          if ((ch == '_') || (ch == '$') || (is_alpha(ch))) {
            // [IdentifierStart]
          } else if (is_digit(ch) && (!buffer.empty())) {
            // [UnicodeDigit]
          } else if (ch == ':') {
            break;
          } else {
            throw syntax_error(ch, context_name);
          }
          buffer.append(1, (char)ch);
        }
        istream.unget();
        return buffer;
      }
    }
    parse_string(buffer, ch, context_name);
    return buffer;
  }

  void parse_object(value& v)
  {
    static const char context_name[] = "object";
    v = object({});
    auto& elements = v.as_object();
    for (;;) {
      int ch = skip_spaces();
      if (ch == '}') {
        break;
      }
      if (elements.empty()) {
        istream.unget();
      } else if (ch != ',') {
        throw syntax_error(ch, context_name);
      } else if (has_flag(flags::trailing_comma)) {
        ch = istream.get();
        if (ch == '}') {
          break;
        }
        istream.unget();
      }
      // [string]
      // [key] (JSON5)
      const std::string key = parse_key();
      ch = skip_spaces();
      if (ch != ':') {
        throw syntax_error(ch, context_name);
      }
      // [value]
      auto result = elements.emplace(key, nullptr);
      parse(result.first->second, context_name);
    }
  }
};

/**
 * @brief JSON stringify context
 */
class stringify_context : public context_base
{
public:
  /**
   * @brief Construct a new stringify_context object
   * 
   * @param istream A reference to ios stream
   */
  stringify_context(std::ostream& ostream)
  : ostream(ostream)
  {
  }

  /**
   * @brief Apply JSON manipulator
   * 
   * @param manip A JSON manipulator to be applied
   * @return A referenece to object itself
   */
  stringify_context& operator<<(const manipulator manip)
  {
    apply_manipulator(manip);
    return *this;
  }

  // Stringify JSON value (written after "class value" definition)
  stringify_context& operator<<(const value& value);

  /**
   * @brief Delegate operator<< for values to ios stream
   * 
   * @tparam T A typename of value
   * @param value A value to delegate
   * @return A reference to ios stream
   */
  template <class T>
  std::ostream& operator<<(const T& value)
  {
    return ostream << value;
  }

  /**
   * @brief Delegate operator<< for manipulators to ios stream
   * 
   * @param manip A manipulator to delegate
   * @return A reference to ios stream
   */
  std::ostream& operator<<(std::ostream& (*manip)(std::ostream&))
  {
    return ostream << manip;
  }

  std::ostream& ostream;  ///< ios stream
};

template <class T>
parse_context<T> operator>>(std::istream& istream, const manipulator& manip)
{
  return parse_context<T>(istream) >> manip;
}

inline stringify_context operator<<(std::ostream& ostream, const manipulator& manip)
{
  return stringify_context(ostream) << manip;
}

} /* namespace impl */

/**
 * @brief Parse JSON
 * 
 * @param value A value object to hold parsed value
 * @return A referenece to object itself
 */
template <class T>
impl::parse_context<T>& impl::parse_context<T>::operator>>(value& v)
{
  parse(v, false);
  return *this;
}

/**
 * @brief Stringify JSON
 * 
 * @param value A value object to stringify
 * @return A referenece to object itself
 */
inline impl::stringify_context& impl::stringify_context::operator<<(const value& value)
{
  value.stringify_to(*this);
  return *this;
}

inline impl::stringify_context operator<<(std::ostream& ostream, const value& value)
{
  return impl::stringify_context(ostream) << value;
}

/**
 * @brief Stringify JSON value with manipulators
 * 
 * @tparam T A list of typename of arguments
 * @param args A list of manipulators
 * @return JSON text
 */
template <class... T>
value::json_type value::stringify(T... args) const
{
  std::ostringstream out;
  stringify_with_manip(impl::stringify_context(out), args...);
  return out.str();
}

inline void value::stringify_to(impl::stringify_context& context) const
{
  class fmtsaver
  {
  public:
    fmtsaver(std::ios_base& ios)
    : ios(ios), flags(ios.flags()), width(ios.width())
    {
    }
    ~fmtsaver()
    {
      ios.width(width);
      ios.flags(flags);
    }
  private:
    std::ios_base& ios;
    const std::ios_base::fmtflags flags;
    const std::streamsize width;
  };
  fmtsaver saver(context.ostream);
  context.ostream.flags(std::ios_base::fmtflags(0));
  context.ostream.width(0);
  stringify_to(context, "");
}

inline void value::stringify_to(impl::stringify_context& context, const std::string indent) const
{
  std::ostream& out = context.ostream;

  switch (type) {
  case TYPE_BOOLEAN:
    out << (content.boolean ? "true" : "false");
    break;
  case TYPE_NUMBER:
    if (std::isnan(content.number)) {
      if (!context.has_flag(impl::flags::not_a_number)) {
        goto null;
      }
      out << "NaN";
    } else if (!std::isfinite(content.number)) {
      if (!context.has_flag(impl::flags::infinity_number)) {
        goto null;
      }
      out << ((content.number > 0) ? "infinity" : "-infinity");
    } else {
      out << content.number;
    }
    break;
  case TYPE_STRING:
    stringify_string(out, content.string);
    break;
  case TYPE_ARRAY:
    if (content.array.empty()) {
      out << "[]";
    } else if (context.indent == 0) {
      const char *delim = "[";
      for (const auto& item : content.array) {
        out << delim;
        item.stringify_to(context);
        delim = ",";
      }
      out << "]";
    } else {
      const char *const newline = context.get_newline();
      const char *delim = "[";
      const std::string inner_indent = indent + context.get_indent();
      for (const auto& item : content.array) {
        out << delim << newline << inner_indent;
        item.stringify_to(context, inner_indent);
        delim = ",";
      }
      out << newline << indent << "]";
    }
    break;
  case TYPE_OBJECT:
    if (content.object.empty()) {
      out << "{}";
    } else if (context.indent == 0) {
      const char *delim = "{";
      for (const auto& pair : content.object) {
        out << delim;
        stringify_string(out, pair.first);
        out << ":";
        pair.second.stringify_to(context);
        delim = ",";
      }
      out << "}";
    } else {
      const char *const newline = context.get_newline();
      const char *delim = "{";
      const std::string inner_indent = indent + context.get_indent();
      for (const auto& pair : content.object) {
        out << delim << newline << inner_indent;
        stringify_string(out, pair.first);
        out << ": ";
        pair.second.stringify_to(context, inner_indent);
        delim = ",";
      }
      out << newline << indent << "}";
    }
    break;
  default:
  null:
    out << "null";
    break;
  }
}

namespace rule {

/**
 * @brief Allow/disallow single line comment starts with "//"
 * 
 * @param allow Allow(true) or disallow(false)
 * @return A JSON manipulator object
 */
inline impl::manipulator single_line_comment(bool allow)
{
  return impl::manipulator(impl::flags::single_line_comment, allow);
}

/**
 * @brief Allow/disallow multi line comment starts with "/*" and ends with "* /"
 * 
 * @param allow Allow(true) or disallow(false)
 * @return A JSON manipulator object
 */
inline impl::manipulator multi_line_comment(bool allow)
{
  return impl::manipulator(impl::flags::multi_line_comment, allow);
}

/**
 * @brief Allow/disallow any comments
 * 
 * @param allow Allow(true) or disallow(false)
 * @return A JSON manipulator object
 */
inline impl::manipulator comments(bool allow)
{
  return impl::manipulator(
    (impl::flags::single_line_comment | impl::flags::multi_line_comment), allow);
}

/**
 * @brief Allow/disallow explicit plus sign(+) before non-negative number
 * 
 * @param allow Allow(true) or disallow(false)
 * @return A JSON manipulator object
 */
inline impl::manipulator explicit_plus_sign(bool allow)
{
  return impl::manipulator(impl::flags::explicit_plus_sign, allow);
}

/**
 * @brief Allow/disallow leading decimal point before number
 * 
 * @param allow Allow(true) or disallow(false)
 * @return A JSON manipulator object
 */
inline impl::manipulator leading_decimal_point(bool allow)
{
  return impl::manipulator(impl::flags::leading_decimal_point, allow);
}

/**
 * @brief Allow/disallow trailing decimal point after number
 * 
 * @param allow Allow(true) or disallow(false)
 * @return A JSON manipulator object
 */
inline impl::manipulator trailing_decimal_point(bool allow)
{
  return impl::manipulator(impl::flags::trailing_decimal_point, allow);
}

/**
 * @brief Allow/disallow leading/trailing decimal points beside number
 * 
 * @param allow Allow(true) or disallow(false)
 * @return A JSON manipulator object
 */
inline impl::manipulator decimal_points(bool allow)
{
  return impl::manipulator(
    (impl::flags::leading_decimal_point | impl::flags::trailing_decimal_point), allow);
}

/**
 * @brief Allow/disallow infinity (infinity/-infinity) as number value
 * 
 * @param allow Allow(true) or disallow(false)
 * @return A JSON manipulator object
 */
inline impl::manipulator infinity_number(bool allow)
{
  return impl::manipulator(impl::flags::infinity_number, allow);
}

/**
 * @brief Allow/disallow NaN as number value
 * 
 * @param allow Allow(true) or disallow(false)
 * @return A JSON manipulator object
 */
inline impl::manipulator not_a_number(bool allow)
{
  return impl::manipulator(impl::flags::not_a_number, allow);
}

/**
 * @brief Allow/disallow hexadeciaml number
 * 
 * @param allow Allow(true) or disallow(false)
 * @return A JSON manipulator object
 */
inline impl::manipulator hexadecimal(bool allow)
{
  return impl::manipulator(impl::flags::hexadecimal, allow);
}

/**
 * @brief Allow/disallow single-quoted string
 * 
 * @param allow Allow(true) or disallow(false)
 * @return A JSON manipulator object
 */
inline impl::manipulator single_quote(bool allow)
{
  return impl::manipulator(impl::flags::single_quote, allow);
}

/**
 * @brief Allow/disallow multi line string escaped by "\"
 * 
 * @param allow Allow(true) or disallow(false)
 * @return A JSON manipulator object
 */
inline impl::manipulator multi_line_string(bool allow)
{
  return impl::manipulator(impl::flags::multi_line_string, allow);
}

/**
 * @brief Allow/disallow trailing comma in arrays and objects
 * 
 * @param allow Allow(true) or disallow(false)
 * @return A JSON manipulator object
 */
inline impl::manipulator trailing_comma(bool allow)
{
  return impl::manipulator(impl::flags::trailing_comma, allow);
}

/**
 * @brief Allow/disallow unquoted keys in objects
 * 
 * @param allow Allow(true) or disallow(false)
 * @return A JSON manipulator object
 */
inline impl::manipulator unquoted_key(bool allow)
{
  return impl::manipulator(impl::flags::unquoted_key, allow);
}

/**
 * @brief Set ECMA-404 standard rules
 * @see https://www.json.org/
 * 
 * @return A JSON manipulator object
 */
inline impl::manipulator ecma404()
{
  return impl::manipulator(0, impl::flags::all_rules, false);
}

/**
 * @brief Set JSON5 rules
 * @see https://json5.org/
 * 
 * @return A JSON manipulator object
 */
inline impl::manipulator json5()
{
  return impl::manipulator(impl::flags::json5_rules, impl::flags::all_rules, false);
}

/**
 * @brief Disable indent
 * 
 * @return A JSON manipulator object
 */
inline impl::manipulator no_indent()
{
  return impl::manipulator(0, 0, true, 0);
}

/**
 * @brief Enable indent with tab "\t" character
 * 
 * @param tabs A number of tab characters for one level (default:1)
 * @return A JSON manipulator object
 */
inline impl::manipulator tab_indent(std::uint8_t tabs = 1)
{
  return impl::manipulator(0, 0, true, -tabs);
}

/**
 * @brief Enable indent with space " " character
 * 
 * @param tabs A number of space characters for one level (default:2)
 * @return A JSON manipulator object
 */
inline impl::manipulator space_indent(std::uint8_t spaces = 2)
{
  return impl::manipulator(0, 0, true, spaces);
}

/**
 * @brief Use LF as newline code (when indent enabled)
 * 
 * @return A JSON manipulator object
 */
inline impl::manipulator lf_newline()
{
  return impl::manipulator(impl::flags::crlf_newline, false);
}

/**
 * @brief Use CR+LF as newline code (when indent enabled)
 * 
 * @return A JSON manipulator object
 */
inline impl::manipulator crlf_newline()
{
  return impl::manipulator(impl::flags::crlf_newline, true);
}

/**
 * @brief Parse as finished (closed) JSON
 * 
 * @return A JSON manipulator object
 */
inline impl::manipulator finished()
{
  return impl::manipulator(impl::flags::finished, true);
}

} /* namespace rule */

/**
 * @brief Parse string as JSON (ECMA-404 standard)
 * 
 * @param istream An input stream
 * @param finished If true, parse as finished(closed) JSON
 * @return JSON value
 */
inline value parse(std::istream& istream, bool finished = true)
{
  value v;
  (impl::parse_context<void>(istream) >> rule::ecma404()).parse(v, finished);
  return v;
}

/**
 * @brief Parse string as JSON (ECMA-404 standard)
 * 
 * @param string A string to be parsed
 * @return JSON value
 */
inline value parse(const value::json_type& string)
{
  std::istringstream istream(string);
  return parse(istream, true);
}

/**
 * @brief Parse string as JSON (JSON5)
 * 
 * @param istream An input stream
 * @param finished If true, parse as finished(closed) JSON
 * @return JSON value
 */
inline value parse5(std::istream& istream, bool finished = true)
{
  value v;
  (impl::parse_context<void>(istream) >> rule::json5()).parse(v, finished);
  return v;
}

/**
 * @brief Parse string as JSON (JSON5)
 * 
 * @param string A string to be parsed
 * @return JSON value
 */
inline value parse5(const value::json_type& string)
{
  std::istringstream istream(string);
  return parse5(istream, true);
}

} /* namespace json5pp */

#endif  /* _JSON5PP_HPP_ */
