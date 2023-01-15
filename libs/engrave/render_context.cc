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

#include "engrave/render_context.h"

#include <glib.h>

#include "pbd/error.h"
#include "pbd/file_utils.h"

#include "ardour/filesystem_paths.h"

namespace Engrave {

RenderContext::RenderContext (const std::string &family, double line_distance) : _line_distance (line_distance)
{
	set_font_family (family);
}

void
RenderContext::set_font_family (const std::string &family)
{
	_font_face = Cairo::ToyFontFace::create (family, Cairo::FONT_SLANT_NORMAL, Cairo::FONT_WEIGHT_NORMAL);
	_font = Cairo::ScaledFont::create (_font_face, Cairo::scaling_matrix (_line_distance * 4, _line_distance * 4),
	                                   Cairo::identity_matrix());

	std::string metadata_path;
	std::string metadata_file_name = family + "_metadata.json";
	for (size_t i = 0; i < metadata_file_name.size(); ++i) {
		metadata_file_name[i] = g_ascii_tolower (metadata_file_name[i]);
	}
	if (PBD::find_file (ARDOUR::ardour_data_search_path(), metadata_file_name, metadata_path)) {
		PBD::debug << "Found " << metadata_path << endmsg;
		if (!_font_data.LoadFromJSON (metadata_path)) {
			PBD::error << "Failed to load font metadata from " << metadata_file_name << endmsg;
		}
	} else {
		PBD::error << "Failed to find font metadata " << metadata_file_name << endmsg;
	}
}

void
RenderContext::set_line_distance (double d)
{
	if (d == _line_distance) {
		return;
	}
	_font = Cairo::ScaledFont::create (_font_face, Cairo::scaling_matrix (_line_distance * 4, _line_distance * 4),
	                                   Cairo::identity_matrix());
}

void
RenderContext::text_extents (const std::string &text, Cairo::TextExtents &extents) const
{
	cairo_scaled_font_text_extents (const_cast<Cairo::ScaledFont::cobject *> (_font->cobj()), text.c_str(),
	                                &extents);
}

} // namespace Engrave
