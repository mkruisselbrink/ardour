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

#ifndef __gtk_ardour_midi_score_time_axis_h__
#define __gtk_ardour_midi_score_time_axis_h__

#include "time_axis_view.h"

namespace ARDOUR
{
class Session;
class Stripable;
}

class MidiScoreHeader;
class MidiScoreStreamView;
class PublicEditor;

class MidiScoreTimeAxisView : public TimeAxisView
{
public:
	MidiScoreTimeAxisView (ARDOUR::Session *, boost::shared_ptr<ARDOUR::Stripable>, PublicEditor &,
	                       TimeAxisView &parent, ArdourCanvas::Canvas &);
	~MidiScoreTimeAxisView() override;

	void set_height (uint32_t, TrackHeightMode m = OnlySelf, bool from_idle = false) override;
	Gdk::Color color() const override;
	boost::shared_ptr<ARDOUR::Stripable> stripable() const override;
	std::string state_id() const override;
	void set_samples_per_pixel (double fpp) override;

private:
	void on_horizontal_position_changed();

	boost::shared_ptr<ARDOUR::Stripable> _stripable;
	bool _first_call_to_set_height = true;

	MidiScoreStreamView *_view = nullptr;
	// Draws the clef, key signature, and first part of lines.
	MidiScoreHeader *_header = nullptr;
};

#endif /* __gtk_ardour_midi_score_time_axis_h__ */
