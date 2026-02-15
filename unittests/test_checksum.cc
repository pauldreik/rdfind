#include <catch2/catch_test_macros.hpp>

#include "Checksum.hh"
#include <set>

namespace {
using enum Checksum::checksumtypes;
const auto types = { MD5,
                     SHA1,
                     SHA256,
                     SHA512
#ifdef HAVE_LIBXXHASH
                     ,
                     XXH128
#endif
};

// helper function to store the result in a string
std::string
finalize_checksum(Checksum& ck)
{
  std::string ret(static_cast<std::size_t>(ck.getDigestLength()), ' ');
  REQUIRE(0 == ck.printToBuffer(ret.data(), ret.size()));
  return ret;
}

}

TEST_CASE("different checksums are distinct")
{
  std::set<std::string> answers;
  for (auto type : types) {
    Checksum ck(type);
    const auto answer = finalize_checksum(ck);
    answers.insert(answer);
  }
  REQUIRE(types.size() == answers.size());
}

TEST_CASE("update with zero bytes is fine")
{
  for (auto type : types) {
    const auto s1 = [type]() {
      Checksum ck(type);
      return finalize_checksum(ck);
    }();
    const auto s2 = [type]() {
      Checksum ck(type);
      REQUIRE(0 == ck.update(0, static_cast<const char*>(nullptr)));
      return finalize_checksum(ck);
    }();
    REQUIRE(s1 == s2);
  }
}

TEST_CASE("creating and not using it is fine")
{
  for (auto type : types) {
    Checksum ck(type);
  }
}
