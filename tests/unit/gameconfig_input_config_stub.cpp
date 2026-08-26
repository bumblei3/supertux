//  SuperTux
//  Copyright (C) 2026 SuperTux contributors
//  License: GPL-2+

// Input-config stubs for targets that link gameconfig.cpp WITHOUT the real
// control/* config TUs. Only AutotileParserTest uses this.

#include "control/joystick_config.hpp"
#include "control/keyboard_config.hpp"

JoystickConfig::JoystickConfig() {}
void JoystickConfig::read(const ReaderMapping&) {}
void JoystickConfig::write(Writer&) {}

KeyboardConfig::KeyboardConfig() {}
void KeyboardConfig::read(const ReaderMapping&) {}
void KeyboardConfig::write(Writer&) {}
