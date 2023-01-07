/*
 * Copyright (C) 2021 Marijn Kruisselbrink <mek@google.com>
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

#ifndef __gtk_ardour_midi_score_region_view_h__
#define __gtk_ardour_midi_score_region_view_h__

#include "region_view.h"

namespace ARDOUR
{
class MidiModel;
class MidiRegion;
}

class RouteTimeAxisView;

class MidiScoreRegionView : public RegionView
{
public:
	MidiScoreRegionView (ArdourCanvas::Container *parent, RouteTimeAxisView &tv,
	                     boost::shared_ptr<ARDOUR::MidiRegion> r, double samples_per_pixel, uint32_t basic_color);
	~MidiScoreRegionView() override;

	void init(bool) override;
	const boost::shared_ptr<ARDOUR::MidiRegion> midi_region() const;

	GhostRegion *
	add_ghost (TimeAxisView &) override
	{
		return nullptr;
	}

	void reset_width_dependent_items (double pixel_width) override;
	void _redisplay (bool view_only) override;

private:
	boost::shared_ptr<ARDOUR::MidiModel> _model;
};

#endif /* __gtk_ardour_midi_score_region_view_h__ */
