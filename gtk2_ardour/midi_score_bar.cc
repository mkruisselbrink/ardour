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

#include "score/clef.h"
#include "smufl/glyph.h"

#include "score/key_signature.h"

#include "midi_score_streamview.h"

namespace
{
using Accidental = Score::KeySignature::Accidental;
using Beats = Temporal::Beats;

struct PosAndAcc {
	int position;
	Accidental accidental = Accidental::kNone;
	bool tie_to_prev = false;
	bool tie_to_next = false;
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

struct Rest {
	SMuFL::Glyph g;
	Beats b;
	int line = 2;
};

Rest kRests[] = {
	{ SMuFL::Glyph::kRestWhole, Beats (4, 0), 3 },
	{ SMuFL::Glyph::kRestHalf, Beats (2, 0) },
	{ SMuFL::Glyph::kRestQuarter, Beats (1, 0) },
	{ SMuFL::Glyph::kRest8th, Beats (0, Temporal::ticks_per_beat / 2) },
	{ SMuFL::Glyph::kRest16th, Beats (0, Temporal::ticks_per_beat / 4) },
	{ SMuFL::Glyph::kRest32nd, Beats (0, Temporal::ticks_per_beat / 8) },
	{ SMuFL::Glyph::kRest64th, Beats (0, Temporal::ticks_per_beat / 16) },
	{ SMuFL::Glyph::kRest128th, Beats (0, Temporal::ticks_per_beat / 32) },
};

struct NoteShape {
	SMuFL::Glyph notehead_glyph;
	Beats b;
	bool uses_stem = true;
	int flag_count = 0;
};

NoteShape kNotes[] = {
	{ SMuFL::Glyph::kNoteheadDoubleWhole, Beats (8, 0), false },
	{ SMuFL::Glyph::kNoteheadWhole, Beats (4, 0), false },
	{ SMuFL::Glyph::kNoteheadHalf, Beats (2, 0) },
	{ SMuFL::Glyph::kNoteheadBlack, Beats (1, 0) },
	{ SMuFL::Glyph::kNoteheadBlack, Beats (0, Temporal::ticks_per_beat / 2), true, 1 },
	{ SMuFL::Glyph::kNoteheadBlack, Beats (0, Temporal::ticks_per_beat / 4), true, 2 },
	{ SMuFL::Glyph::kNoteheadBlack, Beats (0, Temporal::ticks_per_beat / 8), true, 3 },
	{ SMuFL::Glyph::kNoteheadBlack, Beats (0, Temporal::ticks_per_beat / 16), true, 4 },
	{ SMuFL::Glyph::kNoteheadBlack, Beats (0, Temporal::ticks_per_beat / 32), true, 5 },
};

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
		const Score::Clef *clef = _view.clef();
		const Score::KeySignature *ks = _view.key_signature();
		double ld = _view.line_distance();

		// simplest algorithm:
		// A: convert notes to position+accidentals
		// B:
		//  - make list of (timestamp-rounded-to-small-number, notes-on-at-time)
		//  - then iterate over that list, emitting notes and rests as needed

		// need a time signature class, which not only has the total time,
		// but also subdivisions. I.e. 6/8 is dotted-quarter+dotted-quarter,
		// while 4/4 is quarters

		// TODO: separate out layout from rendering

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
					// Going from something to none means we're going back to the key signature.
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
			// TODO: something better than rounding to 32nd
			c.position = (np.note->time() + np.offset).round_to_subdivision (4, Temporal::RoundNearest);
			c.duration = np.note->length().round_to_subdivision (4, Temporal::RoundNearest);
			if (c.duration < Beats (0, Temporal::ticks_per_beat / 4)) {
				c.duration = Beats (0, Temporal::ticks_per_beat / 4);
			}
			if (!chords.empty() && chords.back().position == c.position
			    && chords.back().duration == c.duration) {
				chords.back().notes.push_back (pa);
			} else {
				c.notes.push_back (pa);
				chords.push_back (c);
			}
		}

		for (auto &c : chords) {
			sort (c.notes.begin(), c.notes.end(),
			      [] (const PosAndAcc &a, const PosAndAcc &b) { return a.position < b.position; });

			// TODO: deal with clashing positions. Change one of the clashing notes to the alternate option
			// for that note.
		}

		Beats last_end = _beat;
		for (const auto &c : chords) {
			// Fill in gap with rests
			if (c.position > last_end) {
				Beats rest_length = c.position - last_end;
				//std::cerr << rest_length << " worth of rests" << std::endl;
				// TODO: take time signature into account, and round to whole beats first etc.
				for (const auto &r : kRests) {
					//std::cerr << "Checking for " << r.b << " as rest." << std::endl;
					while (rest_length >= r.b) {
						rest_length -= r.b;

						std::string s = SMuFL::GlyphAsUTF8 (r.g);
						Cairo::TextExtents rest_extents;
						cr->get_text_extents (s, rest_extents);
						cr->move_to (x, self.y1 - r.line * ld);
						cr->show_text (s);
						x += rest_extents.width + 2;
					}
				}
			}
			last_end = c.position + c.duration;

			// TODO: preprocess into correct note segments
			NoteShape *note_shape = kNotes;
			while (note_shape->b > c.duration)
				note_shape++;

			std::string s = SMuFL::GlyphAsUTF8 (note_shape->notehead_glyph);
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

			double stem_width = 0.1 * ld;
			double right_note_offset = note_extents.width - stem_width;

			bool prev_left = true;
			bool has_notes_on_right = false;
			int last_position = -9999;
			for (const auto &n : c.notes) {
				if (n.position == last_position + 1) {
					has_notes_on_right = true;
					break;
				}
				last_position = n.position;
			}

			last_position = -9999;

			double leger_note_length = note_extents.width;
			if (has_notes_on_right)
				leger_note_length += right_note_offset;

			for (const auto &n : c.notes) {
				top_note_position = std::max (top_note_position, n.position);
				bottom_note_position = std::min (bottom_note_position, n.position);

				// Leger lines
				cr->set_line_width (0.16 * ld);
				double extension = 0.35 * ld;
				for (int i = n.position / 2; i < bottom_line; ++i) {
					double y = self.y1 - i * _view.line_distance();
					cr->move_to (x - extension, y);
					cr->line_to (x + leger_note_length + extension, y);
					cr->stroke();
				}
				for (int i = top_line + 1; i <= n.position / 2; ++i) {
					double y = self.y1 - i * _view.line_distance();
					cr->move_to (x - extension, y);
					cr->line_to (x + leger_note_length + extension, y);
					cr->stroke();
				}
				bottom_line = std::min (bottom_line, n.position / 2);
				top_line = std::max (top_line, n.position / 2);

				double note_x = x;
				if (n.position == last_position + 1 && prev_left) {
					note_x = x + right_note_offset;
					prev_left = false;
				} else {
					prev_left = true;
				}
				// Note head
				double y = self.y1 - n.position * ld / 2;
				cr->move_to (note_x, y);
				cr->show_text (s);

				last_position = n.position;
			}

			// Chord stem
			if (note_shape->uses_stem) {
				int stem_length = 4;
				if (note_shape->flag_count > 2)
					stem_length += note_shape->flag_count - 2;

				cr->set_line_width (stem_width);
				bool stem_down = top_note_position > 4;
				double top_y = self.y1 - top_note_position * ld / 2;
				double bottom_y = self.y1 - bottom_note_position * ld / 2;
				if (stem_down) {
					// down
					double stemx = 0.0 + 0.5 * stem_width;
					if (has_notes_on_right)
						stemx += right_note_offset;
					// TODO: depending on where the top-note is, this migth have to use the other
					// corner of the note head
					cr->move_to (x + stemx, top_y + 0.168 * ld);
					cr->line_to (x + stemx, bottom_y + stem_length * ld);

					if (note_shape->flag_count) {
						SMuFL::Glyph flag_g = note_shape->flag_count == 1
						                          ? SMuFL::Glyph::kFlag8thDown
						                          : SMuFL::Glyph::kFlag16thDown;
						cr->move_to (x + stemx - 0.5 * stem_width,
						             bottom_y + stem_length * ld);
						std::string flag_s = SMuFL::GlyphAsUTF8 (flag_g);
						cr->show_text (flag_s);
					}
				} else {
					// up
					double stemx = 1.3 * ld - 0.5 * stem_width;
					cr->move_to (x + stemx, bottom_y - 0.16 * ld);
					cr->line_to (x + stemx, top_y - stem_length * ld);

					if (note_shape->flag_count) {
						SMuFL::Glyph flag_g = note_shape->flag_count == 1
						                          ? SMuFL::Glyph::kFlag8thUp
						                          : SMuFL::Glyph::kFlag16thUp;
						cr->move_to (x + stemx - 0.5 * stem_width, top_y - stem_length * ld);
						std::string flag_s = SMuFL::GlyphAsUTF8 (flag_g);
						cr->show_text (flag_s);
					}
				}
				cr->stroke();
			}

			// TODO: evenly distribute
			x += note_extents.width + 2 + x_offset;
			if (has_notes_on_right)
				x += right_note_offset;
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
