//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "gui/menu.hpp"

#include "control/input_manager.hpp"
#include "gui/item_action.hpp"
#include "gui/item_back.hpp"
#include "gui/item_color.hpp"
#include "gui/item_colorchannel_rgba.hpp"
#include "gui/item_colordisplay.hpp"
#include "gui/item_color_picker_2d.hpp"
#include "gui/item_controlfield.hpp"
#include "gui/item_floatfield.hpp"
#include "gui/item_goto.hpp"
#include "gui/item_hl.hpp"
#include "gui/item_horizontalmenu.hpp"
#include "gui/item_inactive.hpp"
#include "gui/item_intfield.hpp"
#include "gui/item_label.hpp"
#include "gui/item_paths.hpp"
#include "gui/item_script.hpp"
#include "gui/item_script_line.hpp"
#include "gui/item_stringselect.hpp"
#include "gui/item_textfield.hpp"
#include "gui/item_list.hpp"
#include "gui/item_toggle.hpp"
#include "gui/item_string_array.hpp"
#include "gui/item_images.hpp"
#include "gui/menu_filesystem.hpp"
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "gui/mousecursor.hpp"
#include "math/util.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/resources.hpp"
#include "video/drawing_context.hpp"
#include "video/renderer.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

#include "supertux/error_handler.hpp"

static const float HELP_MARGIN_Y = 16.f;

// The amount in pixels the mouse has to wiggle after scrolling before it can hover over things again.
constexpr int MOUSE_DEADZONE_AMOUNT = 70;

constexpr int ACTIVE_ITEM_NONE = -1;

Menu::Menu() :
  m_pos(Vector(static_cast<float>(SCREEN_WIDTH) / 2.0f,
               static_cast<float>(SCREEN_HEIGHT) / 2.0f)),
  m_delete_character(0),
  m_mn_input_char('\0'),
  m_menu_width(),
  m_menu_height(),
  m_menu_help_height(0.0f),
  m_items(),
  m_arrange_left(0),
  m_active_item(ACTIVE_ITEM_NONE),
  m_mouse_deadzone(0),
  m_can_click_when_unfocused(false)
{
}

Menu::~Menu()
{
}

/* Add an item to a menu */
MenuItem&
Menu::add_item(std::unique_ptr<MenuItem> new_item)
{
  m_items.push_back(std::move(new_item));
  MenuItem& item = *m_items.back();

  /* If a new menu is being built, the active item shouldn't be set to
   * something that isn't selectable. Set the active_item to the first
   * selectable item added.
   */

  if (get_active_item() == ACTIVE_ITEM_NONE && !item.skippable())
  {
    set_active_item(static_cast<int>(m_items.size()) - 1);
  }

  recalculate_position_and_size();

  return item;
}

MenuItem&
Menu::add_item(std::unique_ptr<MenuItem> new_item, int pos_)
{
  m_items.insert(m_items.begin()+pos_,std::move(new_item));
  MenuItem& item = *m_items[pos_];

  // When the item is inserted before the selected item, the
  // same menu item should be still selected.

  if (m_active_item >= pos_)
  {
    m_active_item++;
  }
  process_action(MenuAction::SELECT);

  recalculate_position_and_size();

  return item;
}

void
Menu::delete_item(int pos_)
{
  m_menu_height -= static_cast<float>(m_items[pos_]->get_height()) + m_items[pos_]->get_distance() * 2;
  m_items.erase(m_items.begin()+pos_);

  // When the item is deleted before the selected item, the
  // same menu item should be still selected.

  if (m_active_item >= pos_)
  {
    do {
      if (m_active_item > 0)
        --m_active_item;
      else
        m_active_item = int(m_items.size()) - 1;
    } while (m_items[m_active_item]->skippable());
    process_action(MenuAction::SELECT);
  }
}

ItemHorizontalLine&
Menu::add_hl()
{
  return add_item<ItemHorizontalLine>();
}

ItemLabel&
Menu::add_label(const std::string& text)
{
  return add_item<ItemLabel>(text);
}

ItemControlField&
Menu::add_controlfield(int id, const std::string& text,
                       const std::string& mapping)
{
  return add_item<ItemControlField>(text, mapping, id);
}

ItemTextField&
Menu::add_textfield(const std::string& text, std::string* input, int id)
{
  return add_item<ItemTextField>(text, input, id);
}

ItemScript&
Menu::add_script(UID uid, const std::string& key, const std::string& text, std::string* script, int id)
{
  return add_item<ItemScript>(uid, key, text, script, id);
}

ItemIntField&
Menu::add_intfield(const std::string& text, int* input, int id, bool positive, ItemIntFieldRange range)
{
  return add_item<ItemIntField>(text, input, id, positive, range);
}

ItemFloatField&
Menu::add_floatfield(const std::string& text, float* input, int id)
{
  return add_item<ItemFloatField>(text, input, id);
}

ItemGoto&
Menu::add_goto(const std::string& text, std::string* input, int id, int max)
{
  return add_item<ItemGoto>(text, input, id, max);
}

ItemStringSelect&
Menu::add_stringselect(const std::string& text, std::string* input, int id, const std::vector<std::string>& values)
{
  return add_item<ItemStringSelect>(text, input, id, values);
}

ItemStringArray&
Menu::add_string_array(const std::string& text, std::vector<std::string>* input, int id)
{
  return add_item<ItemStringArray>(text, input, id);
}

ItemToggle&
Menu::add_toggle(const std::string& text, bool* input, int id, const std::string& on, const std::string& off)
{
  return add_item<ItemToggle>(text, input, id, on, off);
}

ItemImages&
Menu::add_images(const std::string& text, std::vector<std::string>* input, int id)
{
  return add_item<ItemImages>(text, input, id);
}

ItemPaths&
Menu::add_paths(const std::string& text, std::string* input, int id)
{
  return add_item<ItemPaths>(text, input, id);
}

ItemScriptLine&
Menu::add_script_line(const std::string& text, std::string* input, int id)
{
  return add_item<ItemScriptLine>(text, input, id);
}

ItemHL&
Menu::add_hl_item(const std::string& text)
{
  return add_item<ItemHL>(text);
}

ItemInactive&
Menu::add_inactive(const std::string& text)
{
  return add_item<ItemInactive>(text);
}

ItemLabel&
Menu::add_label_item(const std::string& text)
{
  return add_item<ItemLabel>(text);
}

ItemColorDisplay&
Menu::add_color_display(const std::string& text, const Color& color)
{
  return add_item<ItemColorDisplay>(text, color);
}

ItemColorPicker2D&
Menu::add_color_picker_2d(const std::string& text, Color* color, int id)
{
  return add_item<ItemColorPicker2D>(text, color, id);
}

ItemColorChannelRGBA&
Menu::add_color_channel_rgba(const std::string& text, Color* color, int id, ColorChannel channel)
{
  return add_item<ItemColorChannelRGBA>(text, color, id, channel);
}

void
Menu::recalculate_position_and_size()
{
  // 1. determine max width of any item
  float max_width = 0;
  float max_height = 0;
  for (auto& item : m_items)
  {
    float width = item->get_width();
    float height = item->get_height();
    if (width > max_width)
      max_width = width;
    max_height += height + item->get_distance();
  }

  // 2. set menu's size based on the items' sizes.
  m_menu_width = max_width + 2 * HELP_MARGIN_Y;
  m_menu_height = max_height;

  // adjust items
  float x = m_pos.x - m_menu_width / 2;
  float y = m_pos.y - m_menu_height / 2;
  m_arrange_left = (SCREEN_WIDTH - m_menu_width) / 2;
  for (auto& item : m_items)
  {
    item->arrange(x, y);
    y += item->get_height() + item->get_distance();
  }

  // 3. arrange "help" text
  float help_height = 0;
  for (auto& item : m_items)
  {
    float help_text_height = item->get_help_height();
    if (help_text_height > help_height)
      help_height = help_text_height;
  }
  m_menu_help_height = help_height;
}

bool
Menu::handle_event(const SDL_Event& event)
{
  SDL_EventType type = event.type;

  // pass events to menu items for event handling
  for (auto& item : m_items)
  {
    if (item->handle_event(event))
      return true;
  }

  if (type == SDL_EVENT_MOUSE_BUTTON_DOWN || type == SDL_EVENT_MOUSE_BUTTON_UP)
  {
    int x, y;
    SDL_GetMouseState(&x, &y);
    Vector mouse_pos = VideoSystem::current()->get_viewport().to_logical(x, y);

    for (auto& item : m_items)
    {
      if (item->contains(mouse_pos))
      {
        if (type == SDL_EVENT_MOUSE_BUTTON_DOWN)
        {
          item->pressed();
          process_action(MenuAction::PRESS);
          return true;
        }
        if (type == SDL_EVENT_MOUSE_BUTTON_UP && item->is_pressed())
        {
          item->released();
          process_action(MenuAction::HIT);
        }
      }
    }
    return false;
  }

  if (type == SDL_EVENT_MOUSE_WHEEL)
  {
    float yrel = event.wheel.y;
    int new_active_item = m_active_item;

    if (yrel > 0)
    {
      // scroll down
      for (int i = 0; i < static_cast<int>(m_items.size()); ++i)
      {
        if (m_items[(m_active_item + 1 + i) % m_items.size()]->skippable())
          continue;
        new_active_item = (m_active_item + 1 + i) % m_items.size();
        break;
      }
    }
    else
    {
      // scroll up
      for (int i = 0; i < static_cast<int>(m_items.size()); ++i)
      {
        if (m_items[(m_active_item - 1 - i + m_items.size()) % m_items.size()]->skippable())
          continue;
        new_active_item = (m_active_item - 1 - i + m_items.size()) % m_items.size();
        break;
      }
    }
    set_active_item(new_active_item);
    return true;
  }

  if (type == SDL_EVENT_MOUSE_MOTION)
  {
    if (m_mouse_deadzone > 0)
    {
      m_mouse_deadzone -= abs(event.motion.xrel);
      m_mouse_deadzone -= abs(event.motion.yrel);

      if (m_mouse_deadzone < 0)
        m_mouse_deadzone = 0;

        return;
      }
      Vector const mouse_pos = VideoSystem::current()->get_viewport().to_logical(event.motion.x, event.motion.y);
      float const x = mouse_pos.x;
      float const y = mouse_pos.y;

      if (x > m_pos.x - get_width()/2 &&
         x < m_pos.x + get_width()/2 &&
         y > m_pos.y - get_height()/2 &&
         y < m_pos.y + get_height()/2)
      {
        int new_active_item = 0;
        // This is probably not the most efficient way of finding active item
        // but I can't think of something better right now ~ mrkubax10
        float item_y = m_pos.y - get_height()/2;
        for (unsigned i = 0; i < m_items.size(); i++)
        {
          if (y >= item_y && y <= item_y + static_cast<float>(m_items[i]->get_height()))
          {
            new_active_item = i;
            break;
          }
          item_y += static_cast<float>(m_items[i]->get_height());
        }

        /* only change the mouse focus to a selectable item */
        if (!m_items[new_active_item]->skippable() &&
            new_active_item != m_active_item) {

          set_active_item(new_active_item);
        }

        if (MouseCursor::current())
          MouseCursor::current()->set_state(MouseCursorState::LINK);
      }
      else
      {
        if (MouseCursor::current())
          MouseCursor::current()->set_state(MouseCursorState::NORMAL);
      }
    }
    break;

    default:
      break;
  }
  return false;
}

void
Menu::set_active_item_id(int id)
{
  for (size_t i = 0; i < m_items.size(); ++i)
  {
    if (m_items[i]->get_id() == id)
    {
      set_active_item(static_cast<int>(i));
      break;
    }
  }
}

void
Menu::set_active_item(int item_idx)
{
  if (item_idx == m_active_item)
    return;

  if (m_active_item > -1)
  {
    process_action(MenuAction::UNSELECT);
  }

  m_active_item = item_idx;
  if (m_active_item > -1)
  {
    process_action(MenuAction::SELECT);
  }
}
