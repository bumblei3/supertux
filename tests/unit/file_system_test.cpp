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

#include <gtest/gtest.h>

#include "util/file_system.hpp"

// These exercise the pure path-manipulation helpers in util/file_system.cpp.
// They are used pervasively by the engine (every file open / addon load /
// save path goes through join/dirname/basename/normalize), but had no
// unit coverage -- a regression in slash handling or ".." resolution would
// only surface as a mis-loaded asset deep in gameplay.

TEST(FileSystemTest, dirname)
{
  EXPECT_EQ("a/b/", FileSystem::dirname("a/b/c"));
  EXPECT_EQ("./", FileSystem::dirname("c"));          // no separator -> cwd
  EXPECT_EQ("a/", FileSystem::dirname("a/c"));
  EXPECT_EQ("dir/", FileSystem::dirname("dir/file.txt"));
}

TEST(FileSystemTest, basename)
{
  EXPECT_EQ("c", FileSystem::basename("a/b/c"));
  EXPECT_EQ("file.txt", FileSystem::basename("a/b/file.txt"));
  EXPECT_EQ("c", FileSystem::basename("c"));          // no separator
  // greedy=true first strips trailing dir separators from the input
  EXPECT_EQ("file", FileSystem::basename("a/b/file/", true));
  EXPECT_EQ("file", FileSystem::basename("a/b/file", true));
}

TEST(FileSystemTest, join)
{
  EXPECT_EQ("a/b", FileSystem::join("a", "b"));
  EXPECT_EQ("a/b", FileSystem::join("a/", "b"));       // collapse lhs slash
  EXPECT_EQ("a/b", FileSystem::join("a", "/b"));       // collapse rhs slash
  EXPECT_EQ("a/b", FileSystem::join("a/", "/b"));      // collapse both
  EXPECT_EQ("x", FileSystem::join("", "x"));           // empty lhs -> rhs
  EXPECT_EQ("x/", FileSystem::join("x", ""));          // empty rhs -> lhs+/
  EXPECT_EQ("a/b/c", FileSystem::join("a/b", "c"));
}

TEST(FileSystemTest, extension)
{
  EXPECT_EQ(".gz", FileSystem::extension("foo.tar.gz")); // last dot only
  EXPECT_EQ(".txt", FileSystem::extension("file.txt"));
  EXPECT_EQ("", FileSystem::extension("noextension"));
  EXPECT_EQ(".dot/", FileSystem::extension("trailing.dot/")); // last dot wins
  EXPECT_EQ(".png", FileSystem::extension("a/b/c.png"));
}

TEST(FileSystemTest, strip_extension)
{
  EXPECT_EQ("foo.tar", FileSystem::strip_extension("foo.tar.gz"));
  EXPECT_EQ("file", FileSystem::strip_extension("file.txt"));
  EXPECT_EQ("noextension", FileSystem::strip_extension("noextension"));
}

TEST(FileSystemTest, normalize)
{
  // "./" segments are dropped, the result is always absolute (leading /).
  EXPECT_EQ("/y", FileSystem::normalize("./x/../y"));
  EXPECT_EQ("/a/b", FileSystem::normalize("a/b"));
  EXPECT_EQ("/a/b", FileSystem::normalize("a//b"));     // double slash
  EXPECT_EQ("/a/b/c", FileSystem::normalize("/a/b/c/")); // trailing slash
  // ".." past the root is kept literally (the code logs a warning and
  // pushes it so the error is visible to the user), then "a" is appended.
  EXPECT_EQ("/../a", FileSystem::normalize("../a"));
}

TEST(FileSystemTest, strip_leading_dirs)
{
  EXPECT_EQ("a/b", FileSystem::strip_leading_dirs("a/b/"));
  EXPECT_EQ("a/b", FileSystem::strip_leading_dirs("a/b\\\\"));
  EXPECT_EQ("a/b", FileSystem::strip_leading_dirs("a/b")); // idempotent
}

TEST(FileSystemTest, relpath)
{
  // fs::relative: path of target relative to base.
  EXPECT_EQ("c.txt", FileSystem::relpath("a/b/c.txt", "a/b"));
  EXPECT_EQ("..", FileSystem::relpath("a", "a/b"));
}

/* EOF */
