/*
 * Copyright (C) 2023 Marijn Kruisselbrink <mek@google.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef SMUFL_GLYPH_H
#define SMUFL_GLYPH_H

#include <string>
#include <ostream>

#include <boost/optional.hpp>

namespace SMuFL
{

enum class Glyph {
#include "smufl/data/glyph_ids.h"
    kNumEntries
};

extern const char *GlyphName (Glyph g);
extern const char *GlyphDescription (Glyph g);
extern uint16_t GlyphCodePoint (Glyph g);
extern std::string GlyphAsUTF8 (Glyph g);
extern boost::optional<Glyph> GlyphFromName(const std::string& name);

extern std::ostream& operator<<(std::ostream& os, Glyph g);

} // namespace SMuFL

#endif  // SMUFL_GLYPH_H
