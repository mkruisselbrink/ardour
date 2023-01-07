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

#ifndef SMUFL_FONT_DATA_H
#define SMUFL_FONT_DATA_H

#include <string>
#include <vector>

namespace SMuFL
{

#define ENGRAVING_DEFAULTS_FLOAT_FIELD(name, default) float name = default;
#define ENGRAVING_DEFAULT_SELF
struct EngravingDefaults {
	std::vector<std::string> textFontFamily;
#include "smufl/data/font_data_engraving_defaults_fields.h"
};
#undef ENGRAVING_DEFAULT_SELF
#undef ENGRAVING_DEFAULTS_FLOAT_FIELD

class FontData
{
public:
    bool LoadFromJSON(const std::string& file_name);

	const std::string &
	font_name() const
	{
		return _font_name;
	}

	const std::string &
	font_version() const
	{
		return _font_version;
	}

	const EngravingDefaults &
	engraving_defaults() const
	{
		return _engraving_defaults;
	}

private:
	std::string _font_name;
	std::string _font_version;
	EngravingDefaults _engraving_defaults;
};

} // namespace SMuFL

#endif // SMUFL_FONT_DATA_H
