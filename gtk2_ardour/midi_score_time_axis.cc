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

#include "midi_score_time_axis.h"

#include "pbd/i18n.h"

#include "ardour/stripable.h"

#include "canvas/container.h"

#include "smufl/clefs.h"

#include "widgets/tooltips.h"

#include "midi_score_streamview.h"
#include "public_editor.h"
#include "utils.h"

class MidiScoreHeader : public Gtk::DrawingArea
{
public:
	MidiScoreHeader (MidiScoreStreamView &);

	bool on_expose_event (GdkEventExpose *) override;
	void on_size_request (Gtk::Requisition *) override;
	void on_size_allocate (Gtk::Allocation &a) override;

	void note_range_changed();
	void
	set_meter (const Temporal::Meter &meter)
	{
		if (meter == _meter)
			return;
		_meter = meter;
		queue_draw();
	}

private:
	MidiScoreStreamView &_view;
	Temporal::Meter _meter;
};

MidiScoreHeader::MidiScoreHeader (MidiScoreStreamView &v) : _view (v), _meter (4, 4)
{
	//_view.NoteRangeChanged.connect (sigc::mem_fun (*this, &PianoRollHeader::note_range_changed));
}

bool
MidiScoreHeader::on_expose_event (GdkEventExpose *ev)
{
	GdkRectangle &rect = ev->area;
	double bottom_line = _view.bottom_line();
	double line_distance = _view.line_distance();

	std::cerr << "Expose event: " << rect.x << "," << rect.y << " " << rect.width << "x" << rect.height
		  << std::endl;

	Cairo::RefPtr<Cairo::Context> cr = get_window()->create_cairo_context();

	cr->rectangle (rect.x, rect.y, rect.width, rect.height);
	cr->set_source_rgba (1, 1, 1, 0.878);
	cr->fill();

	cr->set_source_rgb (0, 0, 0);
	cr->set_line_width (1);
	for (int i = 0; i < 5; ++i) {
		double y = round (bottom_line - line_distance * i) - 0.5;
		cr->move_to (0, y);
		cr->line_to (get_width(), y);
		cr->stroke();
	}

	Glib::RefPtr<Pango::Context> pango = get_pango_context();
	cr->select_font_face ("Leland", Cairo::FONT_SLANT_NORMAL, Cairo::FONT_WEIGHT_NORMAL);
	cr->set_font_size (line_distance * 4);

	double spacing = std::min (line_distance, 15.0);
	double x = get_width();
	{
		// TODO: multi-digit numbers
		std::string ts_top = SMuFL::GlyphAsUTF8 (SMuFL::time_signature_digits[_meter.divisions_per_bar()]);
		std::string ts_bottom = SMuFL::GlyphAsUTF8 (SMuFL::time_signature_digits[_meter.note_value()]);
		Cairo::TextExtents top_extents, bottom_extents;
		cr->get_text_extents (ts_top, top_extents);
		cr->get_text_extents (ts_bottom, bottom_extents);

		x -= std::max (top_extents.width, bottom_extents.width) + spacing;

		cr->move_to (x, bottom_line - line_distance);
		cr->show_text (ts_bottom);
		cr->move_to (x, bottom_line - line_distance * 3);
		cr->show_text (ts_top);
	}

	// TODO: key signature

	const SMuFL::Clef *clef = _view.clef();
	if (clef) {
		std::string s = SMuFL::GlyphAsUTF8 (clef->glyph);
		Cairo::TextExtents extents;
		cr->get_text_extents (s, extents);
		std::cerr << "Clef extents: " << extents.width << "x" << extents.height
			  << ", bearing: " << extents.x_bearing << ", advance: " << extents.x_advance << std::endl;
		x -= extents.width + 2 * spacing;
		cr->move_to (x, bottom_line - line_distance * clef->clef_position / 2);
		cr->show_text (SMuFL::GlyphAsUTF8 (clef->glyph));
	}

	return true;
}

void
MidiScoreHeader::on_size_request (Gtk::Requisition *r)
{
	std::cerr << "On size request" << std::endl;
	r->width = 200;
}

void
MidiScoreHeader::on_size_allocate (Gtk::Allocation &a)
{
	DrawingArea::on_size_allocate (a);

	std::cerr << "Got size: " << get_width() << std::endl;
}

void
MidiScoreHeader::note_range_changed()
{
	std::cerr << "Note range changed" << std::endl;
}

MidiScoreTimeAxisView::MidiScoreTimeAxisView (ARDOUR::Session *s, boost::shared_ptr<ARDOUR::Stripable> strip,
                                              PublicEditor &e, TimeAxisView &parent, ArdourCanvas::Canvas &canvas)
    : TimeAxisView (s, e, &parent, canvas), _stripable (strip)
{
	_view = new MidiScoreStreamView (*this);
	_view->attach();

	_header = new MidiScoreHeader (*_view);
	time_axis_hbox.pack_end (*_header, /*expand=*/false, /*fill=*/false, /*padding=*/0);
	_header->show();

	set_height (preset_height (HeightLarge));

	// name label isn't editable on a midi score track; remove the tooltip
	ArdourWidgets::set_tooltip (name_label, X_ (""));
	name_label.set_text ("Score");
	name_label.show();

	_editor.HorizontalPositionChanged.connect (
	    sigc::mem_fun (*this, &MidiScoreTimeAxisView::on_horizontal_position_changed));
}

MidiScoreTimeAxisView::~MidiScoreTimeAxisView()
{
	delete _view;
}

void
MidiScoreTimeAxisView::set_height (uint32_t h, TrackHeightMode m, bool from_idle)
{
	bool const changed = (height != (uint32_t)h) || _first_call_to_set_height;
	TimeAxisView::set_height (h, m, from_idle);

	_view->set_height (h);
	_view->update_contents_height();

	_first_call_to_set_height = false;
	if (changed) {
		if (_canvas_display->visible() && _stripable) {
			/* only emit the signal if the height really changed and we were visible */
			_stripable->gui_changed ("visible_tracks", (void *)0); /* EMIT_SIGNAL */
		}
	}
}

Gdk::Color
MidiScoreTimeAxisView::color() const
{
	return Gtkmm2ext::gdk_color_from_rgb (_stripable->presentation_info().color());
}

boost::shared_ptr<ARDOUR::Stripable>
MidiScoreTimeAxisView::stripable() const
{
	return _stripable;
}

std::string
MidiScoreTimeAxisView::state_id() const
{
	return "midi score";
}

void
MidiScoreTimeAxisView::set_samples_per_pixel (double fpp)
{
	std::cerr << "Samples per pixel: " << fpp << std::endl;
	if (_view) {
		_view->set_samples_per_pixel (fpp);
	}
	TimeAxisView::set_samples_per_pixel (fpp);
}

void
MidiScoreTimeAxisView::on_horizontal_position_changed()
{
	Temporal::TempoMap::SharedPtr tmap (Temporal::TempoMap::use());
	const Temporal::MeterPoint &meter = tmap->meter_at (Temporal::timepos_t{ _editor.leftmost_sample() });
	// std::cerr << "Horizontal position changed: " << _editor.leftmost_sample()
	//	  << ", meter: " << meter.divisions_per_bar() << "/" << meter.note_value() << std::endl;
	_header->set_meter (meter);
}