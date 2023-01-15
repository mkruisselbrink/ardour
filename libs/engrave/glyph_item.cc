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

#include "engrave/glyph_item.h"

#include "engrave/render_context.h"

namespace Engrave {

GlyphItem::GlyphItem (const RenderContext &context, ArdourCanvas::Item *parent, Glyph glyph)
    : ArdourCanvas::Item (parent), _context (context)
{
	_text = GlyphAsUTF8 (glyph);
}

void
GlyphItem::render (const ArdourCanvas::Rect &area, Cairo::RefPtr<Cairo::Context> cr) const
{
	double offset = _line_offset * _context.line_distance();
	ArdourCanvas::Duple pos = item_to_window (ArdourCanvas::Duple (0, -offset));
	cr->set_source_rgb (0, 0, 0);
	cr->set_scaled_font (_context.font());

	cr->move_to (pos.x, pos.y);
	cr->show_text (_text);
}

void
GlyphItem::compute_bounding_box() const
{
	Cairo::TextExtents extents;
	_context.text_extents (_text, extents);
	double offset = _line_offset * _context.line_distance();
	_bounding_box = { extents.x_bearing, extents.y_bearing - offset, extents.x_bearing + extents.width,
		          extents.y_bearing + extents.height - offset };
	set_bbox_clean();
}

} // namespace Engrave
