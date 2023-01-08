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

#include "smufl/clefs.h"
#include "smufl/font_data.h"
#include "smufl/glyph.h"

#include "gui_thread.h"
#include "midi_score_time_axis.h"
#include "public_editor.h"
#include "route_time_axis.h"
#include "ui_config.h"

class MidiScoreBar : public ArdourCanvas::Item
{
public:
	MidiScoreBar (MidiScoreStreamView &view, ArdourCanvas::Item *parent, const ArdourCanvas::Duple &position,
	              double width)
	    : ArdourCanvas::Item (parent, position), _view (view), _width (width)
	{
		/*_rest = new ArdourCanvas::Text (this);
		_rest->set_position ({_width / 2, 0});
		_rest->set (SMuFL::GlyphAsUTF8(SMuFL::Glyph::kRestWhole));
		Pango::FontDescription font;
		font.set_family("Leland");
		font.set_size(40);
		_rest->set_font_description(font);*/
	}

	void
	line_distance_changed()
	{
		// TODO: bounding box should be full height, so this shouldn't effect bounding box
		begin_change();
		compute_bounding_box();
		end_change();
	}

	void
	set_width (double w)
	{
		if (w == _width)
			return;
		begin_change();
		_width = w;
		compute_bounding_box();
		end_change();
	}

	void
	render (const ArdourCanvas::Rect &area, Cairo::RefPtr<Cairo::Context> cr) const override
	{
		ArdourCanvas::Rect self
		    = item_to_window (ArdourCanvas::Rect (0, -_view.line_distance() * 4, _width, 0));
		ArdourCanvas::Rect isect = self.intersection (area);
		if (!isect)
			return;

		cr->set_source_rgb (0, 0, 0);
		cr->set_line_width (1);
		cr->move_to (self.x1 - 0.5, self.y0);
		cr->line_to (self.x1 - 0.5, self.y1);
		cr->stroke();

		cr->select_font_face ("Leland", Cairo::FONT_SLANT_NORMAL, Cairo::FONT_WEIGHT_NORMAL);
		cr->set_font_size (_view.line_distance() * 4);

		{
			Cairo::TextExtents extents;
			std::string s = SMuFL::GlyphAsUTF8 (SMuFL::Glyph::kRestWhole);
			cr->get_text_extents (s, extents);
			cr->move_to ((self.x1 - self.x0) / 2 + self.x0 - extents.width / 2,
			             self.y1 - _view.line_distance() * 3);
			cr->show_text (s);
		}
	}

	void
	compute_bounding_box() const override
	{
		_bounding_box = { 0, -_view.line_distance() * 4, _width, 0 };
		set_bbox_clean();
	}

private:
	MidiScoreStreamView &_view;
	double _width;
};

MidiScoreStreamView::MidiScoreStreamView (MidiScoreTimeAxisView &tv)
    : StreamView (*dynamic_cast<RouteTimeAxisView *> (tv.get_parent()), tv.canvas_display()), _time_axis_view (tv)
{
	_bar_lines = new ArdourCanvas::LineSet (_canvas_group, ArdourCanvas::LineSet::Horizontal);
	_bar_lines->lower_to_bottom();
	canvas_rect->lower_to_bottom();

	// std::cerr << SMuFL::GlyphDescription(SMuFL::Glyph::kFClef) << std::endl;
	// SMuFL::FontData fd;
	// fd.LoadFromJSON("/home/mek/Source/ardour7/libs/smufl/fonts/Leland/leland_metadata.json");
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

	int bass_note_range = SMuFL::Clef::bass_clef.notes_on_bar (_data_note_min, _data_note_max);
	int treble_note_range = SMuFL::Clef::treble_clef.notes_on_bar (_data_note_min, _data_note_max);
	std::cerr << "Bass: " << bass_note_range << ", Treble: " << treble_note_range << std::endl;
	if (bass_note_range > treble_note_range) {
		_clef = &SMuFL::Clef::bass_clef;
	} else {
		_clef = &SMuFL::Clef::treble_clef;
	}

	update_bars();
}

void
MidiScoreStreamView::update_contents_metrics (boost::shared_ptr<ARDOUR::Region> r)
{
	boost::shared_ptr<ARDOUR::MidiRegion> mr = boost::dynamic_pointer_cast<ARDOUR::MidiRegion> (r);

	if (mr) {
		ARDOUR::Source::ReaderLock lm (mr->midi_source (0)->mutex());
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

	for (const auto& b : _bars) {
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
	// update position and size of all bars
	// TBD: maybe hide ones that are offscreen?
	Temporal::TempoMap::SharedPtr tmap (Temporal::TempoMap::use());

	// Make sure we get at least until the next bar.
	const Temporal::BBT_Time rightmost_bar = tmap->bbt_at (_data_last_time).round_up_to_bar().next_bar();
	const samplecnt_t sr = _time_axis_view.editor().session()->sample_rate();

	Temporal::TempoMapPoints grid;
	tmap->get_grid (grid, 0, tmap->superclock_at (rightmost_bar), /*bar_mod=*/1);
	double last_pos = 0;
	size_t last_bar = 0;
	for (const auto &p : grid) {
		size_t bar = p.bbt().bars - 1;
		last_bar = bar;
		double pos = _time_axis_view.editor().sample_to_pixel (p.sample (sr));
		std::cerr << "Bar: " << p.bbt().bars << ", sample: " << p.sample (sr) << ", pos: " << pos << std::endl;
		if (bar > 0 && bar - 1 < _bars.size()) {
			std::cerr << "Resizing to " << (pos - last_pos) << std::endl;
			_bars[bar - 1]->set_width (pos - last_pos);
		}
		if (bar >= _bars.size()) {
			_bars.push_back (std::make_unique<MidiScoreBar> (
			    *this, _canvas_group, ArdourCanvas::Duple{ pos, _bottom_line }, pos - last_pos));
		} else {
			std::cerr << "Moving" << std::endl;
			last_pos = pos;
			_bars[bar]->set_x_position (pos);
		}
	}

	if (_bars.size() > last_bar) {
		_bars.resize (last_bar);
	}
}