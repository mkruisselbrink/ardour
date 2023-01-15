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

#include "engrave/clef_item.h"

#include "engrave/clef.h"

namespace Engrave {

ClefItem::ClefItem (const RenderContext &context, ArdourCanvas::Item *parent, const Clef &clef)
    : GlyphItem (context, parent, clef.glyph)
{
	set_line_offset (clef.clef_position * 0.5);
}

} // namespace Engrave
