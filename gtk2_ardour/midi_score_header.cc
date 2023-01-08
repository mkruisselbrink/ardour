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

#include "midi_score_header.h"

#include "smufl/clefs.h"
#include "smufl/glyph.h"
#include "smufl/key_signature.h"

#include "midi_score_streamview.h"

void
MidiScoreHeader::set_meter (const Temporal::Meter &meter)
{
	if (meter == _meter)
		return;
	_meter = meter;
	queue_draw();
}

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

	const SMuFL::Clef *clef = _view.clef();
	if (clef) {
		uint8_t lowest = clef->lowest_note_on_bar();

		const SMuFL::KeySignature *ks = _view.key_signature();
		if (ks) {
			x -= 5;

			std::vector<uint8_t> sharps = ks->sharps();
			std::string sharp = SMuFL::GlyphAsUTF8 (SMuFL::Glyph::kAccidentalSharp);
			Cairo::TextExtents sharp_extents;
			cr->get_text_extents (sharp, sharp_extents);
			for (int i = sharps.size() - 1; i >= 0; --i) {
				x -= sharp_extents.width;
				uint8_t note = sharps[i];
				while (note < lowest)
					note += 12;
				int pos = clef->position_for_note (note);
				if (pos < clef->key_signature_sharp_offset)
					pos += 7;
				cr->move_to (x, bottom_line - line_distance / 2 * pos);
				cr->show_text (sharp);
			}

			std::vector<uint8_t> flats = ks->flats();
			std::string flat = SMuFL::GlyphAsUTF8 (SMuFL::Glyph::kAccidentalFlat);
			Cairo::TextExtents flat_extents;
			cr->get_text_extents (flat, flat_extents);
			for (int i = flats.size() - 1; i >= 0; --i) {
				x -= flat_extents.width;
				uint8_t note = flats[i];
				while (note < lowest)
					note += 12;
				int pos = clef->position_for_note (note);
				if (pos < clef->key_signature_flat_offset)
					pos += 7;
				cr->move_to (x, bottom_line - line_distance / 2 * pos);
				cr->show_text (flat);
			}
		}

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
