/*
   copyright 2018 Paul Dreik
   Distributed under GPL v 2.0 or later, at your option.
   See LICENSE for further details.
*/

#include "config.h"

// std
#include <algorithm>
#include <array>
#include <random>

// project
#include "EasyRandom.hh"

class EasyRandom::GlobalRandom final
{
public:
  char randomFileChar() { return getChar(m_dist(m_gen)); }
  GlobalRandom()
  {
    // there are pitfalls of random device - may be nondeterministic. for that,
    // one needs platform dependent initialization.
    const std::size_t state_size_in_bytes =
      std::mt19937::state_size * sizeof(std::mt19937::default_seed);
    using array_element = std::random_device::result_type;
    std::array<array_element, state_size_in_bytes / sizeof(array_element)> seed;

    std::random_device rd;
    std::generate_n(seed.data(), seed.size(), [&]() { return rd(); });
    std::seed_seq seq(seed.cbegin(), seed.cend());
    m_gen.seed(seq);
  }

  // this is expensive to copy, so disable that.
  GlobalRandom(const GlobalRandom&) = delete;

private:
  std::mt19937 m_gen{};
  static constexpr int nchars = 64;
  static char getChar(int i)
  {
    const char acceptable_filename_chars[nchars + 1] =
      "abcdefghijklmnopqrstuvwxyz"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "0123456789"
      "_-";
    return acceptable_filename_chars[i];
  }
  std::uniform_int_distribution<int> m_dist{ 0, nchars - 1 };
};

EasyRandom::GlobalRandom&
EasyRandom::getGlobalObject()
{
  // thread safe (magic static)
  static GlobalRandom global{};
  return global;
}

EasyRandom::EasyRandom()
  : m_rand(getGlobalObject())
{
}

std::string
EasyRandom::makeRandomFileString(std::size_t N)
{
  std::string ret(N, '\0');
  std::generate(
    begin(ret), end(ret), [this]() { return m_rand.randomFileChar(); });
  return ret;
}
