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

#include "smufl/font_data.h"

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "pbd/error.h"

namespace pt = boost::property_tree;

#define debug info

namespace SMuFL {

namespace {

	boost::optional<Offset>
	ReadOptionalOffset (const pt::ptree &tree, const char *property_name)
	{
		boost::optional<const pt::ptree &> prop = tree.get_child_optional (property_name);
		if (!prop || prop->size() != 2) {
			return {};
		}
		try {
			float x = prop->front().second.get_value<float>();
			float y = prop->back().second.get_value<float>();
			return Offset{ x, y };
		} catch (...) {
			return {};
		}
	}

} // namespace

bool
FontData::LoadFromJSON (const std::string &file_name)
{
	PBD::info << "Loading JSON " << file_name << endmsg;

	pt::ptree root;
	try {
		pt::read_json (file_name, root);
	} catch (...) {
		PBD::error << "Failed to read font file metadata from " << file_name << endmsg;
		return false;
	}

	try {
		_font_name = root.get<std::string> ("fontName");
	} catch (...) {
		PBD::error << "Failed to read font name" << endmsg;
	}
	try {
		_font_version = root.get<std::string> ("fontVersion");
	} catch (...) {
		PBD::error << "Failed to read font name" << endmsg;
	}

	const pt::ptree &engraving_defaults_pt = root.get_child ("engravingDefaults");

#define ENGRAVING_DEFAULTS_FLOAT_FIELD(name, default)                                                                 \
	{                                                                                                             \
		float val;                                                                                            \
		try {                                                                                                 \
			val = engraving_defaults_pt.get<float> (#name);                                               \
		} catch (...) {                                                                                       \
			PBD::debug << "Falling back to default for " #name << endmsg;                                 \
			val = default;                                                                                \
		}                                                                                                     \
		_engraving_defaults.name = val;                                                                       \
	}
#define ENGRAVING_DEFAULT_SELF _engraving_defaults.
#include "smufl/data/font_data_engraving_defaults_fields.h"
#undef ENGRAVING_DEFAULT_SELF
#undef ENGRAVING_DEFAULTS_FLOAT_FIELD

	_engraving_defaults.textFontFamily.clear();
	try {
		for (const auto &ff : engraving_defaults_pt.get_child ("textFontFamily")) {
			try {
				std::string val = ff.second.get_value<std::string>();
				_engraving_defaults.textFontFamily.push_back (std::move (val));
			} catch (...) {
				PBD::debug << "Failed to parse font family" << endmsg;
			}
		}
	} catch (...) {
		PBD::debug << "No font families" << endmsg;
	}

	_glyph_info.clear();
	try {
		for (const auto &glyph_data : root.get_child ("glyphsWithAnchors")) {
			std::string glyph_name = glyph_data.first;
			boost::optional<Glyph> glyph = GlyphFromName (glyph_name);
			if (!glyph) {
				PBD::debug << "Unknown glyph name: " << glyph_name << endmsg;
				continue;
			}
			GlyphInfo &info = _glyph_info[*glyph];
			info.cutOutNE = ReadOptionalOffset (glyph_data.second, "cutOutNE");
			info.cutOutNW = ReadOptionalOffset (glyph_data.second, "cutOutNW");
			info.cutOutSE = ReadOptionalOffset (glyph_data.second, "cutOutSE");
			info.cutOutSW = ReadOptionalOffset (glyph_data.second, "cutOutSW");
			info.stemUpSE = ReadOptionalOffset (glyph_data.second, "stemUpSE");
			info.stemUpNW = ReadOptionalOffset (glyph_data.second, "stemUpNW");
			info.stemDownNW = ReadOptionalOffset (glyph_data.second, "stemDownNW");
			info.stemDownSW = ReadOptionalOffset (glyph_data.second, "stemDownSW");
			info.opticalCenter = ReadOptionalOffset (glyph_data.second, "opticalCenter");
			info.repeatOffset = ReadOptionalOffset (glyph_data.second, "repeatOffset");
		}
	} catch (...) {
		PBD::debug << "No glyph anchor data" << endmsg;
	}

	return true;
}

std::ostream &
operator<< (std::ostream &os, const EngravingDefaults &ed)
{
#define ENGRAVING_DEFAULTS_FLOAT_FIELD(name, default) os << "    " << #name " = " << ed.name << std::endl;
#define ENGRAVING_DEFAULT_SELF
#include "smufl/data/font_data_engraving_defaults_fields.h"
#undef ENGRAVING_DEFAULT_SELF
#undef ENGRAVING_DEFAULTS_FLOAT_FIELD
	os << "    textFontFamily: ";
	std::string sep;
	for (const auto &s : ed.textFontFamily) {
		os << sep << s;
		sep = ", ";
	}
	os << std::endl;
	return os;
}

std::ostream &
operator<< (std::ostream &os, const Offset &o)
{
	return os << "[" << o.x << ", " << o.y << "]";
}

std::ostream &
operator<< (std::ostream &os, const GlyphInfo &gi)
{
	if (gi.cutOutNE) {
		os << " cutOutNE = " << *gi.cutOutNE;
	}
	if (gi.cutOutNW) {
		os << " cutOutNW = " << *gi.cutOutNW;
	}
	if (gi.cutOutSE) {
		os << " cutOutSE = " << *gi.cutOutSE;
	}
	if (gi.cutOutSW) {
		os << " cutOutSW = " << *gi.cutOutSW;
	}
	if (gi.stemUpSE) {
		os << " stemUpSE = " << *gi.stemUpSE;
	}
	if (gi.stemUpNW) {
		os << " stemUpNW = " << *gi.stemUpNW;
	}
	if (gi.stemDownNW) {
		os << " stemDownNW = " << *gi.stemDownNW;
	}
	if (gi.stemDownSW) {
		os << " stemDownSW = " << *gi.stemDownSW;
	}
	if (gi.opticalCenter) {
		os << " opticalCenter = " << *gi.opticalCenter;
	}
	if (gi.repeatOffset) {
		os << " repeatOffset = " << *gi.repeatOffset;
	}
	return os;
}

std::ostream &
operator<< (std::ostream &os, const FontData &fd)
{
	os << "Font Name: " << fd._font_name << std::endl;
	os << "Font Version: " << fd._font_version << std::endl;
	os << "Engraving defaults:" << std::endl;
	os << fd._engraving_defaults;
	os << "Glyph Anchors: " << std::endl;
	for (const auto &gi : fd._glyph_info) {
		os << "    " << gi.first << ":" << gi.second << std::endl;
	}
	return os;
}

} // namespace SMuFL
