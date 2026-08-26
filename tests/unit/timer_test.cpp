//  SuperTux
//  Unit tests for supertux/timer.cpp — engine-free: Timer only depends on
//  the global float g_game_time, which we control directly in the tests.
//  Also covers supertux/sequence.cpp (Sequence<->string mapping).

#include <gtest/gtest.h>

#include "supertux/globals.hpp"
#include "supertux/timer.hpp"
#include "supertux/sequence.hpp"

namespace {

class TimerTest : public ::testing::Test
{
protected:
  void SetUp() override { g_game_time = 0.0f; }
};

TEST_F(TimerTest, DefaultConstructorIsStopped)
{
  Timer t;
  EXPECT_FLOAT_EQ(t.get_period(), 0.0f);
  EXPECT_FALSE(t.started());
  EXPECT_FALSE(t.paused());
}

TEST_F(TimerTest, ZeroPeriodNeverFires)
{
  g_game_time = 100.0f;
  Timer t;
  EXPECT_FALSE(t.check()); // period == 0 -> early out
  EXPECT_FALSE(t.started());
}

TEST_F(TimerTest, CheckFiresAfterPeriodElapses)
{
  Timer t;
  t.start(5.0f);
  EXPECT_TRUE(t.started());

  g_game_time = 3.0f;
  EXPECT_FALSE(t.check());

  g_game_time = 4.9999f;
  EXPECT_FALSE(t.check());

  g_game_time = 5.0f; // >= period -> fires exactly on the boundary
  EXPECT_TRUE(t.check());

  // One-shot timer: after firing, period resets to 0.
  EXPECT_FLOAT_EQ(t.get_period(), 0.0f);
  EXPECT_FALSE(t.started());
  EXPECT_FALSE(t.check());
}

TEST_F(TimerTest, StopResetsPeriod)
{
  Timer t;
  t.start(10.0f);
  t.stop();
  EXPECT_FLOAT_EQ(t.get_period(), 0.0f);
  EXPECT_FALSE(t.started());
}

TEST_F(TimerTest, CyclicTimerKeepsFiringEachPeriod)
{
  Timer t;
  t.start(2.0f, true);

  g_game_time = 2.0f;
  EXPECT_TRUE(t.check());

  // cycle_start is rewound by whole periods, not reset to now:
  // fire again at 4.0 without touching the timer.
  g_game_time = 4.0f;
  EXPECT_TRUE(t.check());

  // Mid-cycle: no fire.
  g_game_time = 4.5f;
  EXPECT_FALSE(t.check());

  g_game_time = 6.0f;
  EXPECT_TRUE(t.check());
}

TEST_F(TimerTest, CyclicTimerSkippedPeriodsFireOnce)
{
  Timer t;
  t.start(1.0f, true);
  g_game_time = 7.3f; // skipped ~7 periods
  EXPECT_TRUE(t.check());
  // Next boundary is aligned to the original grid: 8.0
  g_game_time = 7.9f;
  EXPECT_FALSE(t.check());
  g_game_time = 8.0f;
  EXPECT_TRUE(t.check());
}

TEST_F(TimerTest, PauseResumePreservesTimeleft)
{
  Timer t;
  t.start(10.0f);

  g_game_time = 4.0f; // 6s left
  t.pause();
  EXPECT_TRUE(t.paused());
  EXPECT_FALSE(t.started()); // stopped while paused

  g_game_time = 50.0f; // time passes while paused — must not matter
  t.resume();
  EXPECT_FALSE(t.paused());
  EXPECT_FLOAT_EQ(t.get_period(), 6.0f); // resumes with the remaining left

  // Real semantics: resume() calls start(left) anchored at the CURRENT
  // game time, so the full remaining time elapses again from now.
  g_game_time = 56.0f; // 6s since resume -> fires
  EXPECT_TRUE(t.check());
}

TEST_F(TimerTest, UnstartedTimerGetTimeleftIsZero)
{
  Timer t;
  EXPECT_FLOAT_EQ(t.get_timeleft(), 0.0f);
  EXPECT_FLOAT_EQ(t.get_timegone(), 0.0f);
  EXPECT_FLOAT_EQ(t.get_progress(), 0.0f);
}

TEST_F(TimerTest, PauseOnUnstartedTimerIsHarmless)
{
  Timer t;
  t.pause();
  EXPECT_FALSE(t.paused());
  EXPECT_FALSE(t.started());
}

TEST_F(TimerTest, MultiplePauseResumeCycles)
{
  Timer t;
  t.start(10.0f);

  g_game_time = 3.0f;
  t.pause();
  EXPECT_FLOAT_EQ(t.get_period(), 0.0f);  // stop() resets period
  EXPECT_TRUE(t.paused());

  g_game_time = 100.0f;
  t.resume();
  EXPECT_FLOAT_EQ(t.get_period(), 7.0f);  // remaining = 10 - 3
  EXPECT_FALSE(t.paused());

  g_game_time = 103.0f;  // 3s since resume
  EXPECT_FALSE(t.check());

  t.pause();
  g_game_time = 200.0f;
  t.resume();
  EXPECT_FLOAT_EQ(t.get_period(), 4.0f);  // 7 - 3 = 4 remaining

  g_game_time = 204.0f;
  EXPECT_TRUE(t.check());
}

TEST_F(TimerTest, GetProgressBeforeStartIsZero)
{
  Timer t;
  EXPECT_FLOAT_EQ(t.get_progress(), 0.0f);
}

TEST_F(TimerTest, StopWhilePausedClearsPauseState)
{
  Timer t;
  t.start(5.0f);
  g_game_time = 2.0f;
  t.pause();
  EXPECT_TRUE(t.paused());

  t.stop();
  EXPECT_FALSE(t.paused());
  EXPECT_FALSE(t.started());
  EXPECT_FLOAT_EQ(t.get_period(), 0.0f);
}

// --- supertux/sequence.cpp: Sequence <-> string mapping -------------------

TEST(SequenceTest, StringToSequenceKnownNames)
{
  EXPECT_EQ(string_to_sequence("endsequence"), SEQ_ENDSEQUENCE);
  EXPECT_EQ(string_to_sequence("stoptux"), SEQ_STOPTUX);
  EXPECT_EQ(string_to_sequence("fireworks"), SEQ_FIREWORKS);
}

TEST(SequenceTest, StringToSequenceUnknownFallsBackToEndsequence)
{
  // Real semantics: unknown names log a warning and default to endsequence.
  EXPECT_EQ(string_to_sequence(""), SEQ_ENDSEQUENCE);
  EXPECT_EQ(string_to_sequence("bogus"), SEQ_ENDSEQUENCE);
}

TEST(SequenceTest, SequenceToStringRoundtrip)
{
  for (Sequence seq : {SEQ_ENDSEQUENCE, SEQ_STOPTUX, SEQ_FIREWORKS})
  {
    EXPECT_EQ(string_to_sequence(sequence_to_string(seq)), seq);
  }
}

TEST(SequenceTest, SequenceToStringUnknownValue)
{
  std::string s = sequence_to_string(static_cast<Sequence>(42));
  EXPECT_NE(s.find("unknown sequence"), std::string::npos);
  EXPECT_NE(s.find("42"), std::string::npos);
}

} // namespace
