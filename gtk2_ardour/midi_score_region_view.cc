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

#include "midi_score_region_view.h"

#include "ardour/midi_model.h"
#include "ardour/midi_region.h"
#include "ardour/midi_source.h"

#include "route_time_axis.h"

using NoteType = Evoral::Note<Temporal::Beats>;

class ScoreNote : public ArdourCanvas::Rectangle
{
public:
	ScoreNote (
			MidiScoreRegionView& region,
			ArdourCanvas::Item* parent,
			const boost::shared_ptr<NoteType> note);
	~ScoreNote () override;

private:
	MidiScoreRegionView& _region;
	const boost::shared_ptr<NoteType> _note;
};

ScoreNote::ScoreNote (
		MidiScoreRegionView& region,
		ArdourCanvas::Item* parent,
		const boost::shared_ptr<NoteType> note)
	: ArdourCanvas::Rectangle (parent)
	, _region(region)
	, _note(note)
{

}

ScoreNote::~ScoreNote() = default;

MidiScoreRegionView::MidiScoreRegionView(
		ArdourCanvas::Container *parent,
		RouteTimeAxisView &tv,
		boost::shared_ptr<ARDOUR::MidiRegion> r,
		double samples_per_pixel,
		uint32_t basic_color)
	: RegionView (parent, tv, r, samples_per_pixel, basic_color)
{
std::cerr << "Creating midi score region view" << std::endl;

	{
		std::cerr << "Loading model" << std::endl;
		ARDOUR::Source::WriterLock lm(midi_region()->midi_source(0)->mutex());
		midi_region()->midi_source(0)->load_model(lm);
	}
	_model = midi_region()->midi_source(0)->model();
}

MidiScoreRegionView::~MidiScoreRegionView() = default;

const boost::shared_ptr<ARDOUR::MidiRegion>
MidiScoreRegionView::midi_region() const
{
	return boost::dynamic_pointer_cast<ARDOUR::MidiRegion>(_region);
}

void
MidiScoreRegionView::_redisplay (bool view_only)
{

	if (!_model) {
		std::cerr << "No model" << std::endl;
		return;
	}

	ARDOUR::MidiModel::ReadLock lock(_model->read_lock());
	ARDOUR::MidiModel::Notes &notes(_model->notes());

	for (ARDOUR::MidiModel::Notes::iterator n = notes.begin(); n != notes.end(); ++n) {
		boost::shared_ptr<NoteType> note(*n);

		std::cerr << "Note " << int(note->note()) << " with lenght " << note->length() << " at time " << note->time() << std::endl;
	}
}