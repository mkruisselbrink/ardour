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

#define debug warning
#define info warning

namespace SMuFL
{

bool
FontData::LoadFromJSON (const std::string &file_name)
{
	PBD::info << "Loading JSON " << file_name << endmsg;

	pt::ptree root;
	// TODO catch parsing errors
	try {
		pt::read_json (file_name, root);
	} catch (...) {
		PBD::error << "Failed to read font file metadata from " << file_name << endmsg;
		return false;
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

	return true;
}

} // namespace SMuFL
