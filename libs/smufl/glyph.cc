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

#include "smufl/glyph.h"

#include <cassert>
#include <map>

#include <glib.h>

#include <iostream>

namespace SMuFL
{

namespace
{

	const char *const g_glyph_names[] = {
#include "smufl/data/glyph_names.h"
	};
	static_assert (static_cast<size_t> (Glyph::kNumEntries) == std::size (g_glyph_names));

	const char *const g_glyph_descriptions[] = {
#include "smufl/data/glyph_descriptions.h"
	};
	static_assert (static_cast<size_t> (Glyph::kNumEntries) == std::size (g_glyph_descriptions));

	const uint16_t g_code_points[] = {
#include "smufl/data/glyph_codepoints.h"
	};
	static_assert (static_cast<size_t> (Glyph::kNumEntries) == std::size (g_code_points));

	std::map<std::string, Glyph>
	GenerateGlyphMap()
	{
		std::map<std::string, Glyph> result;
		for (size_t i = 0; i < static_cast<size_t> (Glyph::kNumEntries); ++i) {
			result[g_glyph_names[i]] = static_cast<Glyph> (i);
		}
		return result;
	}

} // namespace

const char *
GlyphName (Glyph g)
{
	assert (g <= Glyph::kNumEntries);
	return g_glyph_names[static_cast<size_t> (g)];
}

const char *
GlyphDescription (Glyph g)
{
	assert (g <= Glyph::kNumEntries);
	return g_glyph_descriptions[static_cast<size_t> (g)];
}

uint16_t
GlyphCodePoint (Glyph g)
{
	assert (g <= Glyph::kNumEntries);
	return g_code_points[static_cast<size_t> (g)];
}

std::string
GlyphAsUTF8 (Glyph g)
{
	gunichar2 code_point = GlyphCodePoint (g);
	glong len;
	gchar *utf8 = g_utf16_to_utf8 (&code_point, /*len=*/1, /*items_read=*/nullptr, &len, /*error=*/nullptr);
	if (!utf8 || len == 0) {
		std::cerr << "Failed to get utf8" << std::endl;
		return "";
	}
	std::string rv (utf8, len);
	g_free (utf8);
	return rv;
}

boost::optional<Glyph>
GlyphFromName (const std::string &name)
{
	static std::map<std::string, Glyph> glyph_map = GenerateGlyphMap();
	auto it = glyph_map.find (name);
	if (it == glyph_map.end())
		return {};
	return it->second;
}

std::ostream &
operator<< (std::ostream &os, Glyph g)
{
	return os << GlyphName (g);
}

} // namespace SMuFL
