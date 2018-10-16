/*
 * CmdlineParser.cc
 *
 *  Created on: 16 okt. 2018
 *      Author: paul
 */

#include "CmdlineParser.hh"
#include <cstring>
#include <iostream>
#include <cstdlib>

bool
Parser::try_parse_bool(const char* arg)
{
  if (m_index >= m_argc) {
    // out of bounds - programming error.
    std::cerr << "out of bounds: m_index=" << m_index << " m_argc=" << m_argc
              << '\n';
    std::exit(EXIT_FAILURE);
  }
  if (0 == std::strcmp(arg, m_argv[m_index])) {
    // yep - match!
    if (1 + m_index >= m_argc) {
      // out of bounds - user gave to few arguments.
      std::cerr << "expected true or false after " << arg
                << ", not end of argument list.\n";
      std::exit(EXIT_FAILURE);
    }
    auto value = m_argv[++m_index];
    if (0 == std::strcmp(value, "true")) {
      m_last_bool_result = true;
      return true;
    }
    if (0 == std::strcmp(value, "false")) {
      m_last_bool_result = false;
      return true;
    }
    std::cerr << "expected true or false after " << arg << ", not \"" << value
              << "\"\n";
    std::exit(EXIT_FAILURE);
  } else {
    // no match. keep searching.
    return false;
  }
}

bool
Parser::try_parse_string(const char* arg)
{
  if (m_index >= m_argc) {
    // out of bounds - programming error.
    std::cerr << "out of bounds: m_index=" << m_index << " m_argc=" << m_argc
              << '\n';
    std::exit(EXIT_FAILURE);
  }
  if (0 == std::strcmp(arg, m_argv[m_index])) {
    // yep - match!
    if (1 + m_index >= m_argc) {
      // out of bounds. user supplied to few arguments.
      std::cerr << "expected string after " << arg
                << ", not end of argument list.\n";
      std::exit(EXIT_FAILURE);
    }
    m_last_str_result = m_argv[++m_index];
    return true;
  } else {
    // no match. keep searching.
    return false;
  }
}

bool
Parser::parsed_string_is(const char* value) const
{
  return 0 == std::strcmp(m_last_str_result, value);
}

const char*
Parser::get_current_arg() const
{
  if (m_index >= m_argc) {
    // out of bounds.
    std::cerr << "out of bounds: m_index=" << m_index << " m_argc=" << m_argc
              << '\n';
    std::exit(EXIT_FAILURE);
  }
  return m_argv[m_index];
}

bool
Parser::current_arg_is(const char* what) const
{
  return 0 == std::strcmp(get_current_arg(), what);
}
