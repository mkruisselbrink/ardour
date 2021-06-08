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

#ifndef _ardour_auv2_scan_h_
#define _ardour_auv2_scan_h_

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "pbd/xml++.h"

#include "ardour/libardour_visibility.h"
#include "ardour/types.h"

namespace ARDOUR {

struct AUv2Info {
	AUv2Info ()
		: version (0)
		, n_inputs (0)
		, n_outputs (0)
		, n_midi_inputs (0)
		, n_midi_outputs (0)
		, max_outputs (0)
	{}

	AUv2Info (XMLNode const&);
	XMLNode& state () const;

	std::string id;
	std::string name;
	std::string creator;
	std::string category;

	uint32_t version;

	int n_inputs;
	int n_outputs;
	int n_midi_inputs; // has_midi_in
	int n_midi_outputs; // == 0

	int max_outputs;

	std::vector<std::pair<int,int> > io_configs;

};

struct AUv2DescStr {
	std::string type;
	std::string subt;
	std::string manu;

	std::string to_s () const {
		return type + "-" + subt + "-" + manu;
	}
};

LIBARDOUR_API extern void
auv2_blacklist (std::string const&);

LIBARDOUR_API extern void
auv2_whitelist (std::string);

LIBARDOUR_API extern bool
auv2_is_blacklisted (std::string const&);

LIBARDOUR_API extern std::string
auv2_cache_file (CAComponentDescription const&);

LIBARDOUR_API extern std::string
auv2_valid_cache_file (CAComponentDescription const&, bool verbose = false);

LIBARDOUR_API extern bool
auv2_scan_and_cache (CAComponentDescription&, boost::function<void (CAComponentDescription const&, AUv2Info const&)> cb, bool verbose = false);

LIBARDOUR_API extern void
auv2_list_plugins (std::vector<AUv2DescStr>& rv);

} // namespace ARDOUR

#endif
