/*
 * Copyright (C) 2021 Robin Gareus <robin@gareus.org>
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

#ifndef _ardour_plugin_scan_result_h_
#define _ardour_plugin_scan_result_h_

#ifdef WAF_BUILD
#include "libardour-config.h"
#endif

#include <string>

#include "ardour/plugin.h"
#include "ardour/types.h"

namespace ARDOUR {

class LIBARDOUR_API PluginScanLogEntry
{
public:
	PluginScanLogEntry (PluginType const, std::string const& path);
	PluginScanLogEntry (XMLNode const&);

	enum PluginScanResult {
		OK           = 0x000,
		Skipped      = 0x001, // plugin is newer than cache
		Missing      = 0x002, // cache file preset, plugin missing
		Error        = 0x004, // scan failed
		Incompatible = 0x008, // plugin is not compatible (eg 32/64bit) or LV2 in VST2 path
		TimeOut      = 0x010, // scan timed out
		Blacklisted  = 0x100
	};

	void reset ();
	void msg (PluginScanResult, std::string msg = "");
	void add (PluginInfoPtr);

	PluginInfoList const& nfo () const
	{
		return _info;
	}

	XMLNode& state () const;

	PluginType type () const
	{
		return _type;
	}

	std::string path () const
	{
		return _path;
	}

	std::string log () const
	{
		return _scan_log;
	}

	PluginScanResult result () const
	{
		return _result;
	}

	bool operator== (PluginScanLogEntry const& other) const
	{
		return other._type == _type && other._path == _path;
	}

	bool operator!= (PluginScanLogEntry const& other) const
	{
		return other._type != _type || other._path != _path;
	}

	bool operator< (PluginScanLogEntry const& other) const
	{
		if (other._type == _type) {
			return _path < other._path;
		}
		return _type < other._type;
	}

private:
	PluginType       _type;
	std::string      _path;
	std::string      _cache_file; // XXX implicit ?
	PluginScanResult _result;     // TODO -> types.h
	std::string      _scan_log;
	PluginInfoList   _info;
	bool             _recent; // true: touched in this instance, false: loaded from disk
};

} /* namespace ARDOUR */

#endif /* _ardour_plugin_scan_result_h_ */
