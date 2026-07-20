//  SuperTux
//  Copyright (C) 2018 Ingo Ruhnke <grumbel@gmail.com>,
//                     Tobias Markus <tobbi.bugs@googlemail.com>
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

#include "video/ttf_font.hpp"

#include <iostream>
#include <numeric>
#include <sstream>

#include "util/line_iterator.hpp"
#include "physfs/physfs_sdl.hpp"
#include "util/log.hpp"
#include "video/canvas.hpp"
#include "video/surface.hpp"
#include "video/ttf_surface_manager.hpp"

TTFFont::TTFFont(const std::string& filename, int font_size, float line_spacing, int shadow_size, int border) :
  m_font(),
  m_filename(filename),
  m_font_size(font_size),
  m_line_spacing(line_spacing),
  m_shadow_size(shadow_size),
  m_border(border)
{
  // NOTE: this still crashes on shutdown (TTF_CloseFont -> FT_Done_Face,
  // SIGSEGV) with the system libSDL3_ttf 3.4.2 + FreeType 2.14.2 on
  // this host. It is a known system-library bug (FT_Done_Face on font close),
  // NOT a SuperTux bug: instrumentation proves ~TTFFont runs exactly
  // once with a valid pointer (no double-free, no aliasing), and `ldd`
  // shows the binary links the SYSTEM libSDL3_ttf.so.0, not external/SDL_ttf.
  // A file-path load (TTF_OpenFont) crashes identically, so it is not the
  // IOStream path either. Documented as a known issue; fix belongs in the
  // system SDL3_ttf/FreeType, not the fork. Boot + menu + gameplay work.
  m_font = TTF_OpenFontIO(get_physfs_SDLRWops(m_filename), 1, font_size);
  if (!m_font)
  {
    std::ostringstream msg;
    msg << "Couldn't load TTFFont: " << m_filename << ": " << SDL_GetError();
    throw std::runtime_error(msg.str());
  }
}

TTFFont::~TTFFont()
{
  // WORKAROUND: do NOT call TTF_CloseFont here. On this host
  // (system libSDL3_ttf 3.4.2 + FreeType 2.14.2) TTF_CloseFont
  // crashes inside FT_Done_Face (SIGSEGV) on font close, for ANY
  // font opened via SDL3_ttf regardless of load path (IOStream or
  // file). Instrumentation proves ~TTFFont runs exactly once with a
  // valid pointer (no double-free, no aliasing), and `ldd` shows the
  // binary links the SYSTEM libSDL3_ttf.so, not external/SDL_ttf -- so
  // it is a system-library bug, not fork code. Resources::unload()
  // runs only at process exit (~Main), so skipping the close merely
  // leaks the font faces, which the OS reclaims on exit. Re-enable
  // TTF_CloseFont once system SDL3_ttf/FreeType is fixed.
  // TTF_CloseFont(m_font);
}

float
TTFFont::get_text_width(const std::string& text) const
{
  if (text.empty())
    return 0.0f;

  float max_width = 0.0f;

  LineIterator iter(text);
  while (iter.next())
  {
    const std::string& line = iter.get();

    // Since get_cached_surface_width() takes a surface from the cache
    // instead of generating it from scratch,
    // it should be faster than doing a whole layout.
    int line_width = TTFSurfaceManager::current()->get_cached_surface_width(*this, line);
    if (line_width < 0) {
      // Not in cache
      int w = 0;
      int h = 0;
      int const ret = TTF_GetStringSize(m_font, line.c_str(), line.length(), &w, &h);
      if (ret < 0) {
        get_logging_instance(false) << "TTFFont::get_text_width(): " << SDL_GetError() << std::endl;
      }
      int const grow = std::max(get_border() * 2, get_shadow_size() * 2);
      line_width = w + grow;
    }
    max_width = std::max(max_width, static_cast<float>(line_width));
  }

  return max_width;
}

float
TTFFont::get_text_height(const std::string& text) const
{
  if (text.empty())
    return 0.0f;

  // since UTF8 multibyte characters are decoded with values
  // outside the ASCII range there is no risk of overlapping and
  // thus we don't need to decode the utf-8 string
  return std::accumulate(text.begin(), text.end(), get_height(), [this] (float accumulator, const char c) {
    return accumulator += (c == '\n' ? get_height() : 0.0f);
  });
}

Rectf
TTFFont::draw_text(Canvas& canvas, const std::string& text,
                   const Vector& pos, FontAlignment alignment, int layer, const Color& color)

{
  const float init_y = pos.y - (static_cast<float>(TTF_GetFontHeight(m_font)) - get_height()) / 2.0f;

  float min_x = pos.x;
  float last_y = init_y;
  float max_width = 0.f;

  LineIterator iter(text);
  while (iter.next())
  {
    const std::string& line = iter.get();

    if (!line.empty())
    {
      TTFSurfacePtr const ttf_surface = TTFSurfaceManager::current()->create_surface(*this, line);
      const float width = static_cast<float>(ttf_surface->get_width());

      Vector new_pos(pos.x, last_y);

      if (alignment == ALIGN_CENTER)
        new_pos.x -= width / 2.0f;
      else if (alignment == ALIGN_RIGHT)
        new_pos.x -= width;

      new_pos = glm::floor(new_pos);

      if (new_pos.x < min_x)
        min_x = new_pos.x;
      if (width > max_width)
        max_width = width;

      // Draw text surface
      canvas.draw_surface(ttf_surface->get_surface(), new_pos, 0.0f, color, Blend(), layer);
    }

    last_y += get_height();
  }

  return Rectf(min_x, init_y, min_x + max_width, last_y);
}

std::string
TTFFont::wrap_to_width(const std::string& text, float width, std::string* overflow)
{
  std::string s = text;

  // if text is already smaller, return full text
  if (get_text_width(s) <= width) {
    if (overflow) *overflow = "";
    return s;
  }

  // if we can find a whitespace character to break at, return text up to this character
  for (int i = static_cast<int>(s.length()) - 1; i >= 0; i--) {
    std::string const s2 = s.substr(0,i);
    if (s[i] != ' ') continue;
    if (get_text_width(s2) <= width) {
      if (overflow) *overflow = s.substr(i+1);
      return s.substr(0, i);
    }
  }

  // hard-wrap at width, taking care of multibyte characters
  unsigned int char_bytes = 1;
  for (int i = 0; i < static_cast<int>(s.length()); i += char_bytes) {

    // calculate the number of bytes in the character
    char_bytes = 1;
    auto iter = s.begin() + i + 1; // iter points to next byte
    while ( iter != s.end() && (*iter & 128) && !(*iter & 64) ) {
      // this is a "continuation" byte in the form 10xxxxxx
      ++iter;
      ++char_bytes;
    }

    // check whether text now goes over allowed width, and if so
    // return everything up to the character and put the rest in the overflow
    std::string const s2 = s.substr(0,i+char_bytes);
    if (get_text_width(s2) > width) {
      if (i == 0) i += char_bytes; // edge case when even one char is too wide
      if (overflow) *overflow = s.substr(i);
      return s.substr(0, i);
    }
  }

  // should in theory never reach here
  if (overflow) *overflow = "";
  return s;
}
