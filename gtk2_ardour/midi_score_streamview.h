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

#ifndef __gtk_ardour_midi_score_streamview_h__
#define __gtk_ardour_midi_score_streamview_h__

#include "streamview.h"

namespace ArdourCanvas
{
class LineSet;
}

namespace SMuFL
{
class Clef;
}

class MidiScoreTimeAxisView;
class MidiScoreBar;

class MidiScoreStreamView : public StreamView
{
public:
	MidiScoreStreamView (MidiScoreTimeAxisView &);
	~MidiScoreStreamView() override;

	void redisplay_track() override;
	void setup_rec_box() override;
	RegionView *add_region_view_internal (boost::shared_ptr<ARDOUR::Region>, bool wait_for_data,
	                                      bool recording = false) override;
	void color_handler() override;
	void update_contents_metrics (boost::shared_ptr<ARDOUR::Region> r) override;
	int set_samples_per_pixel (double fpp) override;
	bool update_data_note_range (uint8_t min, uint8_t max);
	void update_contents_height();
	void update_bar_lines();

	void update_bars();

	double
	bottom_line() const
	{
		return _bottom_line;
	}
	double
	line_distance() const
	{
		return _line_distance;
	}

	const SMuFL::Clef *
	clef() const
	{
		return _clef;
	}

private:
	MidiScoreTimeAxisView &_time_axis_view;

	ArdourCanvas::LineSet *_bar_lines = nullptr;

	std::vector<std::unique_ptr<MidiScoreBar>> _bars;

	double _line_distance = 10;
	double _bottom_line = 50;

	bool _range_dirty = false;
	uint8_t _data_note_min = 127;
	uint8_t _data_note_max = 0;
	Temporal::timepos_t _data_last_time;

	SMuFL::Clef *_clef = nullptr;
};

#endif /* __gtk_ardour_midi_score_streamview_h__ */
