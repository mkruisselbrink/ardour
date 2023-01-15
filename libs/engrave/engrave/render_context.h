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

#ifndef ENGRAVE_RENDER_CONTEXT_H_
#define ENGRAVE_RENDER_CONTEXT_H_

#include <string>

#include <cairomm/fontface.h>
#include <cairomm/scaledfont.h>

#include "engrave/font_data.h"

namespace Engrave {

class RenderContext {
public:
	RenderContext (const std::string &family, double line_distance);

	void set_font_family (const std::string &family);
	void set_line_distance (double d);

	double
	line_distance() const
	{
		return _line_distance;
	}
	double
	staff_line_thickness() const
	{
		return _line_distance * _font_data.engraving_defaults().staffLineThickness;
	}
	Cairo::RefPtr<Cairo::ScaledFont>
	font() const
	{
		return _font;
	}
	const FontData &
	font_data() const
	{
		return _font_data;
	}

	void text_extents (const std::string &text, Cairo::TextExtents &extents) const;

private:
	double _line_distance = 10;
	Cairo::RefPtr<Cairo::FontFace> _font_face;
	Cairo::RefPtr<Cairo::ScaledFont> _font;
	FontData _font_data;
};

} // namespace Engrave

#endif // ENGRAVE_RENDER_CONTEXT_H_
