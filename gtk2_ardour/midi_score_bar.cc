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

#include "midi_score_bar.h"

#include "smufl/clefs.h"
#include "smufl/glyph.h"
#include "smufl/key_signature.h"

#include "midi_score_streamview.h"

namespace
{
using Accidental = SMuFL::KeySignature::Accidental;
using Beats = Temporal::Beats;

struct PosAndAcc {
	int position;
	Accidental accidental = Accidental::kNone;
};

struct Chord {
	Beats position;
	Beats duration;
	std::vector<PosAndAcc> notes;
};

SMuFL::Glyph
glyph_for_accidental (Accidental a)
{
	assert (a != Accidental::kNone);
	switch (a) {
	case Accidental::kFlat:
		return SMuFL::Glyph::kAccidentalFlat;
	case Accidental::kSharp:
		return SMuFL::Glyph::kAccidentalSharp;
	case Accidental::kNatural:
		return SMuFL::Glyph::kAccidentalNatural;
	default:
		return SMuFL::Glyph::kNumEntries;
	}
}

} // namespace

MidiScoreBar::MidiScoreBar (MidiScoreStreamView &view, ArdourCanvas::Item *parent, const ArdourCanvas::Duple &position,
                            double width)
    : ArdourCanvas::Item (parent, position), _view (view), _width (width)
{
}

void
MidiScoreBar::line_distance_changed()
{
	// TODO: bounding box should be full height, so this shouldn't effect bounding box
	begin_change();
	compute_bounding_box();
	end_change();
}

void
MidiScoreBar::set_width (double w)
{
	if (w == _width)
		return;
	begin_change();
	_width = w;
	compute_bounding_box();
	end_change();
}

void
MidiScoreBar::render (const ArdourCanvas::Rect &area, Cairo::RefPtr<Cairo::Context> cr) const
{
	ArdourCanvas::Rect self = item_to_window (ArdourCanvas::Rect (0, -_view.line_distance() * 4, _width, 0));
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

	if (_notes.empty()) {
		Cairo::TextExtents extents;
		std::string s = SMuFL::GlyphAsUTF8 (SMuFL::Glyph::kRestWhole);
		cr->get_text_extents (s, extents);
		cr->move_to ((self.x1 - self.x0) / 2 + self.x0 - extents.width / 2,
		             self.y1 - _view.line_distance() * 3);
		cr->show_text (s);
	} else {
		double x = self.x0 + 5;
		const SMuFL::Clef *clef = _view.clef();
		const SMuFL::KeySignature *ks = _view.key_signature();
		double ld = _view.line_distance();

		// simplest algorithm:
		// A: convert notes to position+accidentals
		// B:
		//  - make list of (timestamp-rounded-to-small-number, notes-on-at-time)
		//  - then iterate over that list, emitting notes and rests as needed

		// start out with no accidentals
		std::map<int, Accidental> accidentals;
		std::vector<Chord> chords;
		for (const auto &np : _notes) {
			PosAndAcc pa;
			auto note_to_render = ks->note_and_accidentals_to_render (np.note->note());
			pa.position = clef ? clef->position_for_note (note_to_render.note) : 4;
			// go from "something" to "none" is problematic
			Accidental &a = accidentals[pa.position];
			if (a != note_to_render.accidental) {
				a = note_to_render.accidental;
				if (a == Accidental::kNone) {
					// Going from soemthing to none means we're going back to the key signature.
					// That means we do need an accidental.
					if (ks->flats_or_sharps_count() < 0) {
						pa.accidental = Accidental::kFlat;
					} else {
						pa.accidental = Accidental::kSharp;
					}
				} else {
					pa.accidental = a;
				}
			}

			Chord c;
			c.position = np.note->time() + np.offset;
			c.duration = np.note->length();
			c.notes.push_back (pa);
			chords.push_back (c);
		}

		for (const auto &c : chords) {
			SMuFL::Glyph note_head_glyph = SMuFL::Glyph::kNoteheadBlack;
			std::string s = SMuFL::GlyphAsUTF8 (note_head_glyph);
			Cairo::TextExtents note_extents;
			cr->get_text_extents (s, note_extents);

			double x_offset = 0;
			for (const auto &n : c.notes) {
				if (n.accidental == Accidental::kNone)
					continue;

				auto g = glyph_for_accidental (n.accidental);
				std::string gs = SMuFL::GlyphAsUTF8 (g);
				Cairo::TextExtents acc_extents;
				cr->get_text_extents (gs, acc_extents);
				x_offset = std::max (x_offset, acc_extents.width + 4);

				double y = self.y1 - n.position * _view.line_distance() / 2;
				cr->move_to (x, y);
				cr->show_text (gs);
			}
			x += x_offset;

			int bottom_line = 0;
			int top_line = 4;

			int top_note_position = -9999;
			int bottom_note_position = 9999;

			for (const auto &n : c.notes) {
				top_note_position = std::max (top_note_position, n.position);
				bottom_note_position = std::min (bottom_note_position, n.position);

				// Leger lines
				cr->set_line_width (0.16 * ld);
				double extension = 0.35 * ld;
				for (int i = n.position / 2; i < bottom_line; ++i) {
					double y = self.y1 - i * _view.line_distance();
					cr->move_to (x - extension, y);
					cr->line_to (x + note_extents.width + extension, y);
					cr->stroke();
				}
				for (int i = top_line + 1; i <= n.position / 2; ++i) {
					double y = self.y1 - i * _view.line_distance();
					cr->move_to (x - extension, y);
					cr->line_to (x + note_extents.width + extension, y);
					cr->stroke();
				}
				bottom_line = std::min (bottom_line, n.position / 2);
				top_line = std::max (top_line, n.position / 2);

				// Note head
				double y = self.y1 - n.position * ld / 2;
				cr->move_to (x, y);
				cr->show_text (s);
			}

			// Chord stem
			double stem_width = 0.1 * ld;
			cr->set_line_width (stem_width);
			bool stem_down = top_note_position > 4;
			double top_y = self.y1 - top_note_position * ld / 2;
			double bottom_y = self.y1 - bottom_note_position * ld / 2;
			if (stem_down) {
				// down
				double stemx = 0.0 + 0.5 * stem_width;
				cr->move_to (x + stemx, top_y + 0.168 * ld);
				cr->line_to (x + stemx, bottom_y + 4 * ld);
			} else {
				// up
				double stemx = 1.3 * ld - 0.5 * stem_width;
				cr->move_to (x + stemx, bottom_y - 0.16 * ld);
				cr->line_to (x + stemx, top_y - 4 * ld);
			}
			cr->stroke();

			x += note_extents.width + 10;
		}
	}
}

void
MidiScoreBar::compute_bounding_box() const
{
	_bounding_box = { 0, -_view.line_distance() * 4, _width, 0 };
	set_bbox_clean();
}

void
MidiScoreBar::clear()
{
	begin_visual_change();
	_notes.clear();
	_dirty = true;
	end_visual_change();
}

void
MidiScoreBar::add_note (Temporal::Beats offset, NotePtr note)
{
	std::cerr << "Bar starts at " << first_beat() << ", adding note at " << (offset + note->time()) << std::endl;
	begin_visual_change();
	_notes.push_back ({ note, offset });
	_dirty = true;
	end_visual_change();
}
