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
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

// Coverage for util/string_util.hpp using the st_assert.hpp harness (no gtest/glm/SDL).
// Focuses on replace_all edge cases (including the empty-needle infinite loop fix)
// and split behaviour not exercised by the existing string_util_test.cpp.

#include "st_assert.hpp"
#include "util/string_util.hpp"

#include <string>
#include <vector>

int main(void)
{
  // replace_all: empty needle must not loop forever (treated as no-op).
  ST_ASSERT("replace_all: empty needle is a no-op",
            StringUtil::replace_all("abc", "", "x") == "abc");
  ST_ASSERT("replace_all: basic replacement",
            StringUtil::replace_all("a.b.c", ".", "/") == "a/b/c");
  ST_ASSERT("replace_all: empty replacement deletes needle",
            StringUtil::replace_all("aXXb", "XX", "") == "ab");

  // split: empty delimiter field is preserved.
  {
    std::vector<std::string> out;
    StringUtil::split(out, "x,,y", ',');
    ST_ASSERT("split: keeps empty middle field", out.size() == 3 && out[1] == "");
  }

  return 0;
}
