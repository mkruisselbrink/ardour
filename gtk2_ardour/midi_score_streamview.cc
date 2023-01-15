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

#include "midi_score_streamview.h"

#include "canvas/container.h"
#include "canvas/line_set.h"

#include "ardour/midi_model.h"
#include "ardour/midi_region.h"
#include "ardour/midi_source.h"

#include "engrave/clef.h"

#include "gui_thread.h"
#include "midi_score_bar.h"
#include "midi_score_time_axis.h"
#include "public_editor.h"
#include "route_time_axis.h"
#include "ui_config.h"

MidiScoreStreamView::MidiScoreStreamView (MidiScoreTimeAxisView &tv)
    : StreamView (*dynamic_cast<RouteTimeAxisView *> (tv.get_parent()), tv.canvas_display()), _time_axis_view (tv),
      _key_signature (2)
{
	// TODO: rename to staff lines
	_bar_lines = new ArdourCanvas::LineSet (_canvas_group, ArdourCanvas::LineSet::Horizontal);
	_bar_lines->lower_to_bottom();
	canvas_rect->lower_to_bottom();

	// std::cerr << Engrave::GlyphDescription(Engrave::Glyph::kFClef) << std::endl;
	// Engrave::FontData fd;
	// fd.LoadFromJSON("/home/mek/Source/ardour7/libs/engrave/fonts/Leland/leland_metadata.json");
	// std::cerr << fd;

	color_handler();

	UIConfiguration::instance().ColorsChanged.connect (sigc::mem_fun (*this, &MidiScoreStreamView::color_handler));
}

MidiScoreStreamView::~MidiScoreStreamView()
{
	delete _bar_lines;
}

void
MidiScoreStreamView::redisplay_track()
{
	for (const auto &b : _bars) {
		b->clear();
	}

	// Load models if necessary, and find note range of all our contents.
	_range_dirty = false;
	_data_note_min = 127;
	_data_note_max = 0;
	_data_last_time = {};
	_trackview.track()->playlist()->foreach_region (
	    sigc::mem_fun (*this, &MidiScoreStreamView::update_contents_metrics));

	// No notes, use default range
	if (!_range_dirty) {
		_data_note_min = 60;
		_data_note_max = 71;
		std::cerr << "Using default range" << std::endl;
	}
	std::cerr << "Note range: " << int{ _data_note_min } << " - " << int{ _data_note_max } << std::endl;
	std::cerr << "Time range: " << _data_last_time << " which is " << _data_last_time.beats().get_beats()
		  << " Beats" << std::endl;

	Temporal::TempoMap::SharedPtr tmap (Temporal::TempoMap::use());
	Temporal::BBT_Time bbt = tmap->bbt_at (_data_last_time);
	std::cerr << "Last bar: " << bbt.bars << std::endl;

	int bass_note_range = Engrave::Clef::bass_clef.notes_on_bar (_data_note_min, _data_note_max);
	int treble_note_range = Engrave::Clef::treble_clef.notes_on_bar (_data_note_min, _data_note_max);
	std::cerr << "Bass: " << bass_note_range << ", Treble: " << treble_note_range << std::endl;
	if (bass_note_range > treble_note_range) {
		_clef = &Engrave::Clef::bass_clef;
	} else {
		_clef = &Engrave::Clef::treble_clef;
	}

	update_bars();

	_trackview.track()->playlist()->foreach_region (
	    sigc::mem_fun (*this, &MidiScoreStreamView::update_region_contents));
}

void
MidiScoreStreamView::update_region_contents (boost::shared_ptr<ARDOUR::Region> r)
{
	boost::shared_ptr<ARDOUR::MidiRegion> mr = boost::dynamic_pointer_cast<ARDOUR::MidiRegion> (r);
	if (!mr) {
		return;
	}

	Temporal::Beats offset = r->position().beats();
	ARDOUR::Source::ReaderLock lm (mr->midi_source()->mutex());
	const ARDOUR::MidiModel::Notes &notes = mr->model()->notes();
	auto bar_it = _bars.begin();
	for (const ARDOUR::MidiModel::NotePtr &note : notes) {
		std::cerr << "Note: " << int{ note->note() } << " with length " << note->length() << " at time "
			  << note->time() << std::endl;

		Temporal::Beats note_time = note->time() + offset;

		auto next_bar = bar_it;
		while (next_bar != _bars.end() && (*next_bar)->first_beat() <= note_time) {
			bar_it = next_bar;
			next_bar++;
		}
		// Now `bar_it` should point at the right bar.
		(*bar_it)->add_note (offset, note);
	}
}

void
MidiScoreStreamView::update_contents_metrics (boost::shared_ptr<ARDOUR::Region> r)
{
	boost::shared_ptr<ARDOUR::MidiRegion> mr = boost::dynamic_pointer_cast<ARDOUR::MidiRegion> (r);

	if (mr) {
		ARDOUR::Source::ReaderLock lm (mr->midi_source()->mutex());
		_range_dirty = update_data_note_range (mr->model()->lowest_note(), mr->model()->highest_note());
		Temporal::timepos_t end = mr->end();
		if (end > _data_last_time) {
			_data_last_time = end;
		}
	}
}

int
MidiScoreStreamView::set_samples_per_pixel (double fpp)
{
	int result = StreamView::set_samples_per_pixel (fpp);
	update_bars();
	return result;
}

bool
MidiScoreStreamView::update_data_note_range (uint8_t min, uint8_t max)
{
	bool dirty = false;
	if (min < _data_note_min) {
		_data_note_min = min;
		dirty = true;
	}
	if (max > _data_note_max) {
		_data_note_max = max;
		dirty = true;
	}
	return dirty;
}

void
MidiScoreStreamView::setup_rec_box()
{
}

RegionView *
MidiScoreStreamView::add_region_view_internal (boost::shared_ptr<ARDOUR::Region> r, bool wait_for_data, bool recording)
{
	return nullptr;
}

void
MidiScoreStreamView::color_handler()
{
	_canvas_group->set_render_with_alpha (UIConfiguration::instance().modifier ("region alpha").a());
	update_bar_lines();
	canvas_rect->set_fill_color (0xffffffff);
}

void
MidiScoreStreamView::update_contents_height()
{
	StreamView::update_contents_height();

	_bar_lines->set_extent (ArdourCanvas::COORD_MAX);

	double total_bar_height = child_height() / 3;
	_line_distance = total_bar_height / 4;
	_bottom_line = child_height() / 3 + total_bar_height;

	for (const auto &b : _bars) {
		b->set_y_position (_bottom_line);
		b->line_distance_changed();
	}

	update_bar_lines();
}

void
MidiScoreStreamView::update_bar_lines()
{

	_bar_lines->clear();
	for (int i = 0; i < 5; ++i) {
		double y = _bottom_line - _line_distance * i;
		_bar_lines->add_coord (y, 1.0, 0x000000ff);
	}
}

void
MidiScoreStreamView::update_bars()
{
	_bars_by_beats.clear();
	// update position and size of all bars
	// TBD: maybe hide ones that are offscreen?
	Temporal::TempoMap::SharedPtr tmap (Temporal::TempoMap::use());

	// Make sure we get at least until the next bar.
	const Temporal::BBT_Time rightmost_bar = tmap->bbt_at (_data_last_time).round_up_to_bar().next_bar();
	std::cerr << "Last bar: " << rightmost_bar.bars << std::endl;
	std::cerr << "Final superclock: " << tmap->superclock_at (rightmost_bar) << std::endl;
	const samplecnt_t sr = _time_axis_view.editor().session()->sample_rate();

	Temporal::TempoMapPoints grid;
	tmap->get_grid (grid, 0, tmap->superclock_at (rightmost_bar), /*bar_mod=*/1);
	double last_pos = 0;
	Temporal::Beats last_bar_start;
	size_t last_bar = 0;
	for (const auto &p : grid) {
		size_t bar = p.bbt().bars - 1;
		last_bar = bar;
		double pos = _time_axis_view.editor().sample_to_pixel (p.sample (sr));
		std::cerr << "Bar: " << p.bbt().bars << ", sample: " << p.sample (sr) << ", pos: " << pos
			  << ", superclock: " << p.sclock() << std::endl;
		if (bar > 0 && bar - 1 < _bars.size()) {
			std::cerr << "Resizing to " << (pos - last_pos) << std::endl;
			_bars[bar - 1]->set_width (pos - last_pos);
			_bars[bar - 1]->set_beat_length (last_bar_start - p.beats());
		}
		if (bar >= _bars.size()) {
			_bars.push_back (std::make_unique<MidiScoreBar> (
			    *this, _canvas_group, ArdourCanvas::Duple{ pos, _bottom_line }, pos - last_pos));
		} else {
			std::cerr << "Moving" << std::endl;
			last_pos = pos;
			_bars[bar]->set_x_position (pos);
		}
		last_bar_start = p.beats();
		_bars[bar]->set_first_beat (last_bar_start);
		_bars_by_beats[p.beats()] = _bars[bar].get();
	}

	if (_bars.size() > last_bar) {
		_bars.resize (last_bar);
	}
}