#include <catch2/catch_test_macros.hpp>

#include "Checksum.hh"
#include <set>

namespace {
using enum checksumtypes;
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

TEST_CASE("copying a checksum is fine")
{
  static const char* content = "abcd";
  for (auto type : types) {
    const auto expected = [type]() {
      Checksum ck(type);
      REQUIRE(0 == ck.update(std::strlen(content), content));
      return finalize_checksum(ck);
    }();
    Checksum original(type);
    REQUIRE(0 == original.update(std::strlen(content), content));
    Checksum copy(original);
    REQUIRE(expected == finalize_checksum(copy));
    REQUIRE(expected == finalize_checksum(original));
  }
}

TEST_CASE("copy from an rval is fine")
{
  static const char* content = "abcd";
  for (auto type : types) {
    const auto expected = [type]() {
      Checksum ck(type);
      REQUIRE(0 == ck.update(std::strlen(content), content));
      return finalize_checksum(ck);
    }();
    Checksum original(type);
    REQUIRE(0 == original.update(std::strlen(content), content));

    // move copy should be ok
    Checksum movedto(std::move(original));
    REQUIRE(expected == finalize_checksum(movedto));
  }
}

TEST_CASE("resetting the state works as intended")
{
  static const char* content = "abcd";
  for (auto type : types) {

    Checksum ck1(type);
    const auto v1 = finalize_checksum(ck1);

    Checksum ck2(type);
    REQUIRE(0 == ck2.update(std::strlen(content), content));
    const auto v2 = finalize_checksum(ck2);
    REQUIRE(v1 != v2);

    // resetting should get the same checksum as an empty object.
    Checksum ck3(type);
    REQUIRE(0 == ck3.update(std::strlen(content), content));
    ck3.reset();
    const auto v3 = finalize_checksum(ck3);
    REQUIRE(v1 == v3);
  }
}
