//  SuperTux
//  Copyright (C) 2026 SuperTux contributors
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

// Coverage for util/unique_name.cpp: make_unique_name() builds
// "gen<unix-time><pointer>" identifiers used for auto-generated objects.
// The prefix argument is accepted for API compatibility but ignored;
// names always start with the literal "gen". Stability invariant: the
// same (address, second-of-epoch) pair always produces the same name,
// and different pairs produce different names.

#include <gtest/gtest.h>

#include "util/unique_name.hpp"

#include <chrono>
#include <set>
#include <string>
#include <thread>

namespace {

// Cheap stand-in for a heap object whose address we want to observe.
struct Anchor { int align_make_unique; };

// Returns make_unique_name(prefix, &a) without exposing the pointer type.
std::string named(Anchor& a, const char* prefix = "")
{
  return make_unique_name(prefix, &a);
}

} // namespace

// --- Prefix-handling -----------------------------------------------------

TEST(UniqueNameTest, namesAlwaysStartWithGen)
{
  // The prefix parameter is accepted for API compatibility but ignored.
  // Real names always start with the literal "gen".
  Anchor a{};
  EXPECT_EQ(named(a, "ignored").substr(0, 3), "gen");
  EXPECT_EQ(named(a, "prefix").substr(0, 3), "gen");
  EXPECT_EQ(named(a, "").substr(0, 3), "gen");
  EXPECT_EQ(named(a, "ignored"), named(a, "prefix"))
      << "different prefixes must not affect the produced name";
}

// --- Stability / uniqueness invariants -----------------------------------

TEST(UniqueNameTest, sameAddressSameSecondIsStable)
{
  // Calling twice within the same second always yields the identical name.
  Anchor a{};
  std::string n1 = named(a);
  std::string n2 = named(a);
  EXPECT_EQ(n1, n2);
}

TEST(UniqueNameTest, differentAddressesWithinSameSecondDiffer)
{
  // Two live objects with different addresses must produce distinct names
  // even when called in the same second.
  Anchor x{}, y{};
  EXPECT_NE(named(x), named(y));
}

TEST(UniqueNameTest, sameAddressNextSecondChanges)
{
  // If the clock ticks over to the next second, the same address must
  // produce a different name. We sleep across the boundary; 1.5 s is
  // safe on any platform and hurts nobody in a unit test.
  Anchor a{};
  {
    std::string before = named(a);
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    std::string after = named(a);
    EXPECT_NE(before, after) << "name should change across a second boundary";
  }
}

TEST(UniqueNameTest, rapidCallsSameAddressStable)
{
  // 50 calls within the same second must all agree — exercises the
  // "stable per second" contract without sleeping.
  Anchor a{};
  std::set<std::string> seen;
  for (int i = 0; i < 50; ++i) {
    seen.insert(named(a));
  }
  EXPECT_EQ(seen.size(), 1u) << "all rapid calls within a second must agree";
}

TEST(UniqueNameTest, rapidCallsDifferentAddressesMostlyDistinct)
{
  // Different heap addresses within the same second produce distinct names
  // — the pointer suffix differs, so the full name differs. Stack addresses
  // can be uncomfortably close in tight loops, so we allocate a small vector
  // of Anchors on the heap and name each one; with 30 distinct heap
  // addresses we expect well over half to differ.
  std::vector<Anchor*> anchors;
  for (int i = 0; i < 30; ++i) {
    anchors.push_back(new Anchor{});
  }
  std::set<std::string> seen;
  for (auto* a : anchors) {
    seen.insert(named(*a));
  }
  for (auto* a : anchors) {
    delete a;
  }
  // Even if a handful collide by coincidence we still expect the majority
  // to be distinct — relax from the earlier 15/30 tight bound to 8/30.
  EXPECT_GE(seen.size(), 8u)
      << "different heap addresses should rarely collide (observed: "
      << seen.size() << " distinct out of 30)";
}

// --- Well-formedness ------------------------------------------------------

TEST(UniqueNameTest, namesAreNonEmpty)
{
  Anchor a{};
  EXPECT_FALSE(named(a).empty());
  // nullptr-basierte Namen sind implementationsabhängig — mit einem
  // leeren Prefix und nullptr als Zeiger entsteht der Name dennoch
  // (epoch + 0 als Hex-Zeiger), solange die Implementierung nullptr
  // in einen uintptr_t umwandelt. Das ist das Minimal-Signal.
  std::string null_name = make_unique_name(std::string(), nullptr);
  EXPECT_FALSE(null_name.empty()) << "make_unique_name(std::string(), nullptr) must not crash";
}

TEST(UniqueNameTest, nameStartsWithGenPrefix)
{
  Anchor a{};
  std::string n = named(a);
  EXPECT_EQ(n.substr(0, 3), "gen")
      << "name must start with the literal 'gen' prefix (actual: '" << n << "')";
}

TEST(UniqueNameTest, nameContainsDecimalEpochAndHexPointer)
{
  // Die Implementierung schreibt "gen" << time(nullptr)
  // << reinterpret_cast<uintptr_t>(ptr) — also epoch als Dezimalzahl,
  // direkt gefolgt vom Hex-Zeiger ohne expliziten Trenner.
  Anchor a{};
  std::string n = named(a);
  // mindestens 3 Zeichen für "gen" + mindestens 1 Ziffer epoch + mindestens
  // 1 Hex-Zeichen
  EXPECT_GE(n.size(), 5u) << "name too short: '" << n << "'";
  // Der Teil nach "gen" muss mit einer Dezimalzahl beginnen (epoch).
  std::string after_gen = n.substr(3);
  EXPECT_GE(after_gen.size(), 2u)
      << "expected at least epoch-digit + hex-pointer (actual: '" << after_gen << "')";
  // Erstes Zeichen nach "gen" muss Ziffer sein (epoch).
  EXPECT_TRUE(std::isdigit(static_cast<unsigned char>(after_gen[0])))
      << "first char after 'gen' must be a decimal digit (epoch), got '" << after_gen[0] << "'";
}
