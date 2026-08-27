//  SuperTux
//  Copyright (C) 2026 SuperTux contributors
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY EVEN THE IMPLIED WARRANTY OF MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with this program.  If not, see <http://www.gnu.org/licenses/>.

// Engine-free coverage for util/file_watcher.cpp: mtime probing of existing
// and missing files, plus the start_monitoring/poll callback contract
// (callback fires on mtime change, is suppressed when unchanged, and clear()
// stops monitoring). Uses real temp files on disk, so it exercises the actual
// stat() path rather than a stub.

#include <gtest/gtest.h>

#include "util/file_watcher.hpp"

#include <cstdio>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <utime.h>

namespace {

// RAII temp file so the test cannot leak files into the source tree.
class TempFile
{
public:
  explicit TempFile(const std::string& name) : m_path("/tmp/" + name) {}
  ~TempFile() { std::remove(m_path.c_str()); }

  void write(const std::string& content)
  {
    std::ofstream out(m_path, std::ios::trunc);
    out << content;
  }

  // Force a new mtime regardless of filesystem timestamp resolution (which can
  // be coarse, e.g. 1s). The poller compares mtime, so the test must guarantee
  // a change rather than relying on append-within-the-same-second.
  void bump_mtime() const
  {
    struct stat st;
    if (stat(m_path.c_str(), &st) != 0)
      return;
    struct utimbuf u;
    u.actime = st.st_mtime + 1;
    u.modtime = st.st_mtime + 1;
    utime(m_path.c_str(), &u);
  }

  const std::string& path() const { return m_path; }

private:
  std::string m_path;
};

TEST(FileWatcherTest, get_mtime_missing_file_returns_zero)
{
  // No such file: stat() fails and the code returns 0.
  FileWatcher w;
  EXPECT_EQ(w.get_mtime("/tmp/__supertux_fw_nonexistent_xyz"), 0);
}

TEST(FileWatcherTest, get_mtime_existing_file_positive)
{
  TempFile f("__supertux_fw_existing.txt");
  f.write("hello");
  FileWatcher w;
  EXPECT_GT(w.get_mtime(f.path()), 0);
}

TEST(FileWatcherTest, poll_fires_callback_on_mtime_change)
{
  TempFile f("__supertux_fw_poll.txt");
  f.write("v1");
  FileWatcher w;

  int calls = 0;
  w.start_monitoring(f.path(), [&](FileWatcher::FileInfo&) { calls++; });

  // Same content, same mtime -> no callback.
  w.poll();
  EXPECT_EQ(calls, 0);

  // Change mtime -> callback fires exactly once.
  f.bump_mtime();
  w.poll();
  EXPECT_EQ(calls, 1);

  // Still changed -> fires again on the next poll.
  f.bump_mtime();
  w.poll();
  EXPECT_EQ(calls, 2);
}

TEST(FileWatcherTest, poll_suppressed_when_unchanged)
{
  TempFile f("__supertux_fw_unchanged.txt");
  f.write("stable");
  FileWatcher w;

  int calls = 0;
  w.start_monitoring(f.path(), [&](FileWatcher::FileInfo&) { calls++; });

  w.poll();
  w.poll();
  w.poll();
  EXPECT_EQ(calls, 0);
}

TEST(FileWatcherTest, clear_stops_monitoring)
{
  TempFile f("__supertux_fw_clear.txt");
  f.write("data");
  FileWatcher w;

  int calls = 0;
  w.start_monitoring(f.path(), [&](FileWatcher::FileInfo&) { calls++; });

  w.clear();
  f.bump_mtime();
  w.poll();
  EXPECT_EQ(calls, 0);
}

TEST(FileWatcherTest, monitor_then_clear_then_monitor_again)
{
  TempFile f("__supertux_fw_cycle.txt");
  f.write("a");
  FileWatcher w;

  int calls = 0;
  w.start_monitoring(f.path(), [&](FileWatcher::FileInfo&) { calls++; });
  f.bump_mtime();
  w.poll();
  EXPECT_EQ(calls, 1);

  w.clear();
  f.bump_mtime();
  w.poll();
  EXPECT_EQ(calls, 1); // still 1: cleared, not monitoring

  w.start_monitoring(f.path(), [&](FileWatcher::FileInfo&) { calls++; });
  f.bump_mtime();
  w.poll();
  EXPECT_EQ(calls, 2); // re-armed -> fires again
}

} // namespace
