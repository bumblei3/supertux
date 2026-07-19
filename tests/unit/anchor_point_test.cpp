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

// Coverage for math/anchor_point.hpp: the 9 anchor positions over a known
// Rectf, string<->anchor round-trips, and error handling for bad input.
// gettext/logging are stubbed (anchor_point_test_stub.cpp) so no engine
// libraries are linked.

#include "st_assert.hpp"
#include "math/anchor_point.hpp"
#include "math/rectf.hpp"

#include <stdexcept>
#include <string>

namespace {

bool approx(float a, float b, float eps = 0.001f)
{
  float d = a - b;
  return (d < 0 ? -d : d) < eps;
}

} // namespace

int main(void)
{
  // A 100x40 rect anchored at (10, 20): left=10 right=110 top=20 bottom=60,
  // horizontal middle = 60, vertical middle = 40.
  Rectf rect(10.0f, 20.0f, 110.0f, 60.0f);

  // get_anchor_pos: all nine corners/edges/centre.
  {
    Vector tl = get_anchor_pos(rect, ANCHOR_TOP_LEFT);
    ST_ASSERT("top-left x", approx(tl.x, 10.0f));
    ST_ASSERT("top-left y", approx(tl.y, 20.0f));

    Vector top = get_anchor_pos(rect, ANCHOR_TOP);
    ST_ASSERT("top x (h-middle)", approx(top.x, 60.0f));
    ST_ASSERT("top y", approx(top.y, 20.0f));

    Vector tr = get_anchor_pos(rect, ANCHOR_TOP_RIGHT);
    ST_ASSERT("top-right x", approx(tr.x, 110.0f));
    ST_ASSERT("top-right y", approx(tr.y, 20.0f));

    Vector mid = get_anchor_pos(rect, ANCHOR_MIDDLE);
    ST_ASSERT("middle x", approx(mid.x, 60.0f));
    ST_ASSERT("middle y", approx(mid.y, 40.0f));

    Vector br = get_anchor_pos(rect, ANCHOR_BOTTOM_RIGHT);
    ST_ASSERT("bottom-right x", approx(br.x, 110.0f));
    ST_ASSERT("bottom-right y", approx(br.y, 60.0f));

    Vector bl = get_anchor_pos(rect, ANCHOR_BOTTOM_LEFT);
    ST_ASSERT("bottom-left x", approx(bl.x, 10.0f));
    ST_ASSERT("bottom-left y", approx(bl.y, 60.0f));
  }

  // get_anchor_pos with a placed object of given width/height. For a 20x10
  // object anchored bottom-right, the top-left corner sits at
  // (right - width, bottom - height) = (110-20, 60-10) = (90, 50).
  {
    Vector br = get_anchor_pos(rect, 20.0f, 10.0f, ANCHOR_BOTTOM_RIGHT);
    ST_ASSERT("placed bottom-right x", approx(br.x, 90.0f));
    ST_ASSERT("placed bottom-right y", approx(br.y, 50.0f));

    Vector mid = get_anchor_pos(rect, 20.0f, 10.0f, ANCHOR_MIDDLE);
    ST_ASSERT("placed middle x", approx(mid.x, 60.0f - 10.0f));
    ST_ASSERT("placed middle y", approx(mid.y, 40.0f - 5.0f));

    Vector tl = get_anchor_pos(rect, 20.0f, 10.0f, ANCHOR_TOP_LEFT);
    ST_ASSERT("placed top-left x", approx(tl.x, 10.0f));
    ST_ASSERT("placed top-left y", approx(tl.y, 20.0f));
  }

  // get_anchor_center_pos: quarter/half offsets into the rect (width=100,
  // height=40). Left column -> left + width/4 = 10 + 25 = 35.
  {
    Vector tl = get_anchor_center_pos(rect, ANCHOR_TOP_LEFT);
    ST_ASSERT("center top-left x (quarter)", approx(tl.x, 35.0f));
    ST_ASSERT("center top-left y (quarter)", approx(tl.y, 30.0f));

    Vector mid = get_anchor_center_pos(rect, ANCHOR_MIDDLE);
    ST_ASSERT("center middle x", approx(mid.x, 60.0f));
    ST_ASSERT("center middle y", approx(mid.y, 40.0f));
  }

  // anchor_point_to_string <-> string_to_anchor_point round-trip for all 9.
  {
    for (int i = 0; i <= ANCHOR_LAST; ++i)
    {
      AnchorPoint ap = static_cast<AnchorPoint>(i);
      std::string s = anchor_point_to_string(ap);
      AnchorPoint back = string_to_anchor_point(s);
      ST_ASSERT("anchor round-trip", back == ap);
    }
  }

  // Specific string mappings pinned.
  {
    ST_ASSERT("topleft -> ANCHOR_TOP_LEFT",
              string_to_anchor_point("topleft") == ANCHOR_TOP_LEFT);
    ST_ASSERT("bottomright -> ANCHOR_BOTTOM_RIGHT",
              string_to_anchor_point("bottomright") == ANCHOR_BOTTOM_RIGHT);
    ST_ASSERT("ANCHOR_MIDDLE -> middle",
              anchor_point_to_string(ANCHOR_MIDDLE) == "middle");
  }

  // Unknown string must throw.
  {
    bool threw = false;
    try { string_to_anchor_point("nonsense"); }
    catch (const std::exception&) { threw = true; }
    ST_ASSERT("unknown anchor string throws", threw);
  }
}

/* EOF */
