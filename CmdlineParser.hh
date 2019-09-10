/*
   copyright 2018 Paul Dreik (earlier Paul Sundvall)
   Distributed under GPL v 2.0 or later, at your option.
   See LICENSE for further details.
*/

#ifndef RDFIND_CMDLINEPARSER_HH_
#define RDFIND_CMDLINEPARSER_HH_

/**
 * Command line parser, designed to be easy to use for rdfind.
 * It will signal user errors by a helpful printout followed by
 * std::exit(EXIT_FAILURE)
 */
class Parser
{
public:
  Parser(int argc, const char* argv[])
    : m_argc(argc)
    , m_argv(argv)
    , m_index(1)
  {}

  /**
   * tries to parse arg.
   * @param arg
   * @return true if argument could be parsed. get it with get_parsed_bool().
   */
  bool try_parse_bool(const char* arg);

  /**
   * tries to parse arg
   * @param arg
   * @return true if argument could be parsed. get it with get_parsed_string().
   */
  bool try_parse_string(const char* arg);
  bool get_parsed_bool() const { return m_last_bool_result; }
  const char* get_parsed_string() const { return m_last_str_result; }
  [[gnu::pure]] bool parsed_string_is(const char* value) const;
  /**
   * advances to the next argument
   * @return
   */
  int advance() { return ++m_index; }
  bool has_args_left() const { return m_index < m_argc; }
  int get_current_index() const { return m_index; }
  const char* get_current_arg() const;

  bool current_arg_is(const char* what) const;

private:
  const int m_argc{};
  const char** m_argv{};
  int m_index = 1;
  bool m_last_bool_result{};
  const char* m_last_str_result{};
};

#endif /* RDFIND_CMDLINEPARSER_HH_ */
