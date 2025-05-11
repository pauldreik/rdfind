/*
   copyright 2018 Paul Dreik (earlier Paul Sundvall)
   Distributed under GPL v 2.0 or later, at your option.
   See LICENSE for further details.
*/

#include "config.h"

// std
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>

// project
#include "CmdlineParser.hh"

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

namespace {

    enum FILESIZE : long long
    {
        k = 1024,
        K = k,
        M = k * k,
        G = k * M,
        T = k * G,
        P = k * T,
        E = k * P,
        // too big for signed long long // Z = k * E,
        // too big for signed long long // Y = k * Z,
        // too big for signed long long // R = k * Y,
        // too big for signed long long // Q = k * R,
    };

    
    Fileinfo::filesizetype value_of_file_size_suffix(char suffix)
    {
        switch (toupper(suffix))
        {
        // No 'B': too ambiguous.
        case 'K': return FILESIZE::k;
        case 'M': return FILESIZE::M;
        case 'G': return FILESIZE::G;
        case 'T': return FILESIZE::T;
        case 'P': return FILESIZE::P;
        case 'E': return FILESIZE::E;
        // case 'Z': return FILESIZE::Z;
        // case 'Y': return FILESIZE::Y;
        // case 'R': return FILESIZE::R;
        // case 'Q': return FILESIZE::Q;
        default: throw std::runtime_error(
            "The program only understands file size suffixes "
            "K, M, G, T, P or E. Lower case works too.");
        };
    }

}

Fileinfo::filesizetype long Parser::read_file_size(char const *text)
{

    using filesizetype = Fileinfo::filesizetype;
    
    size_t pos = 0;
    filesizetype const without_suffix = std::stoll(text, &pos);

    size_t const len = strlen(text);
    filesizetype const multiplier = 
        (len - pos > 1) ?
        throw std::runtime_error(
            "The program only understands single-letter file size suffixes.")
        :
        (len > pos) ?
        value_of_file_size_suffix(text[pos])
        :
        1;

    filesizetype const retval = multiplier * without_suffix;

    if (retval / multiplier != without_suffix)
        throw std::runtime_error("File size overflows long long.");

    return retval;
}
