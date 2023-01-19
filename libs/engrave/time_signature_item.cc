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

#include "engrave/time_signature_item.h"

#include <cassert>
#include <string>

#include "engrave/glyph_item.h"

namespace Engrave {

TimeSignatureItem::TimeSignatureItem (const RenderContext &context, ArdourCanvas::Item *parent,
                                      const TimeSignature &time_signature)
    : ArdourCanvas::Container (parent), _context (context)
{
	std::vector<std::unique_ptr<ArdourCanvas::Item> > top = create_items_for_number (time_signature.divisions_per_bar(), 3);
	std::vector<std::unique_ptr<ArdourCanvas::Item> > bottom = create_items_for_number (time_signature.note_value(), 1);
    // TODO: center whichever one is shorter.
    _items.insert(_items.end(), std::make_move_iterator(top.begin()), std::make_move_iterator(top.end()));
    _items.insert(_items.end(), std::make_move_iterator(bottom.begin()), std::make_move_iterator(bottom.end()));
}

std::vector<std::unique_ptr<ArdourCanvas::Item> >
TimeSignatureItem::create_items_for_number (int number, int line_offset)
{
	std::vector<std::unique_ptr<ArdourCanvas::Item> > result;
	ArdourCanvas::Coord x = 0;
	std::string s = std::to_string (number);
	for (char c : s) {
		assert (c >= '0' && c <= '9');
		auto item = std::make_unique<GlyphItem> (_context, this, Engrave::time_signature_digits[c - '0']);
		item->set_line_offset (line_offset);
		item->set_x_position (x);
		x += item->width();
        result.push_back(std::move(item));
	}
	return result;
}

}
