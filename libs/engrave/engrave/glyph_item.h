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

#ifndef ENGRAVE_GLYPH_ITEM_H_
#define ENGRAVE_GLYPH_ITEM_H_

#include "engrave/glyph.h"

#include "canvas/item.h"

namespace Engrave {

class RenderContext;

class GlyphItem : public ArdourCanvas::Item {
public:
	GlyphItem (const RenderContext &context, ArdourCanvas::Item *parent, Glyph glyph);

	// ArdourCanvas::Item
	void render (const ArdourCanvas::Rect &area, Cairo::RefPtr<Cairo::Context> cr) const override;
	void compute_bounding_box() const override;

	// Should only be called during construction.
	void
	set_line_offset (double offset)
	{
		_line_offset = offset;
	}

private:
	const RenderContext &_context;
	std::string _text;
	double _line_offset = 0;
};

} // namespace Engrave

#endif // ENGRAVE_GLYPH_ITEM_H_
