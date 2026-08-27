//  SuperTux
//  Copyright (C) 2015 SuperTux Development Team
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

// Verifies that the generated version header carries a real version string
// (not empty) and that the tag field is populated. The values come from
// ${SUPERTUX_PACKAGE_VERSION} / ${SUPERTUX_PACKAGE_VERSION_TAG} injected
// by the build system (configure_file in CMakeLists.txt).

#include <gtest/gtest.h>

#include "version.h"

TEST(VersionTest, package_version_nonempty)
{
  ASSERT_FALSE(PACKAGE_VERSION[0] == '\0')
      << "PACKAGE_VERSION is empty — version.h was not configured";
}

TEST(VersionTest, package_version_tag_present)
{
  ASSERT_FALSE(PACKAGE_VERSION_TAG[0] == '\0')
      << "PACKAGE_VERSION_TAG is empty — version.h was not configured";
}

TEST(VersionTest, version_strings_are_c_headers_safe)
{
  // Both macros must be valid C-string literals usable in printf-style
  // logging and editor display. The null-termination + non-empty check
  // above already covers the common failure modes; this test pins the
  // invariant without assuming a particular format.
  std::string const v(PACKAGE_VERSION);
  std::string const t(PACKAGE_VERSION_TAG);
  EXPECT_FALSE(v.empty());
  EXPECT_FALSE(t.empty());
  // The tag must be a substring of the full version or follow a
  // recognizable delimiter, so downstream code can extract it.
  EXPECT_TRUE(v.find(t) != std::string::npos
              || v.find('@') != std::string::npos
              || v.find(' ') != std::string::npos);
}

