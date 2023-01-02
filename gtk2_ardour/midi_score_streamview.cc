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

#include "ardour/midi_region.h"

#include "gui_thread.h"
#include "midi_score_region_view.h"
#include "midi_score_time_axis.h"
#include "route_time_axis.h"

MidiScoreStreamView::MidiScoreStreamView (MidiScoreTimeAxisView &tv)
    : StreamView (*dynamic_cast<RouteTimeAxisView *> (tv.get_parent()), tv.canvas_display()), _time_axis_view (tv)
{
	_bar_lines = new ArdourCanvas::LineSet (_canvas_group, ArdourCanvas::LineSet::Horizontal);
	_bar_lines->lower_to_bottom();
	canvas_rect->lower_to_bottom();

	color_handler();
}

MidiScoreStreamView::~MidiScoreStreamView()
{
	delete _bar_lines;
}

void
MidiScoreStreamView::redisplay_track()
{
	// Add and display region views, and flag them as valid
	_trackview.track()->playlist()->foreach_region (
	    sigc::hide_return (sigc::mem_fun (*this, &StreamView::add_region_view)));

	// Stack regions by layer, and remove invalid regions
	layer_regions();

	for (std::list<RegionView *>::iterator i = region_views.begin(); i != region_views.end(); ++i) {
		//((MidiScoreRegionView *)(*i))->redisplay_model ();
	}
}

void
MidiScoreStreamView::setup_rec_box()
{
}

RegionView *
MidiScoreStreamView::add_region_view_internal (boost::shared_ptr<ARDOUR::Region> r, bool wait_for_data, bool recording)
{
	boost::shared_ptr<ARDOUR::MidiRegion> region = boost::dynamic_pointer_cast<ARDOUR::MidiRegion> (r);
	if (!region) {
		return nullptr;
	}

	for (std::list<RegionView *>::iterator i = region_views.begin(); i != region_views.end(); ++i) {
		if ((*i)->region() == r) {

			/* great. we already have a MidiRegionView for this Region. use it again. */

			(*i)->set_valid (true);

			// display_region(dynamic_cast<MidiRegionView*>(*i), wait_for_data);

			return 0;
		}
	}

	MidiScoreRegionView *region_view = create_region_view (r, wait_for_data, recording);
	if (!region_view) {
		return nullptr;
	}

	region_views.push_front (region_view);

	// display_region (region_view, wait_for_data);

	/* catch regionview going away */

	boost::weak_ptr<ARDOUR::Region> wr (region); // make this explicit
	region->DropReferences.connect (*this, invalidator (*this),
	                                boost::bind (&MidiScoreStreamView::remove_region_view, this, wr),
	                                gui_context());

	RegionViewAdded (region_view);

	return region_view;
}

MidiScoreRegionView *
MidiScoreStreamView::create_region_view (boost::shared_ptr<ARDOUR::Region> r, bool wait_for_waves, bool recording)
{
	boost::shared_ptr<ARDOUR::MidiRegion> region = boost::dynamic_pointer_cast<ARDOUR::MidiRegion> (r);
	if (!region) {
		return nullptr;
	}

	MidiScoreRegionView *region_view
	    = new MidiScoreRegionView (_canvas_group, _trackview, region, _samples_per_pixel, region_color);

	region_view->init (false);

	return region_view;
}

void
MidiScoreStreamView::color_handler()
{
	update_bar_lines();
	canvas_rect->set_fill_color (0xffffffe0);
}

void
MidiScoreStreamView::update_contents_height()
{
	StreamView::update_contents_height();

	_bar_lines->set_extent (ArdourCanvas::COORD_MAX);

	double total_bar_height = child_height() / 2;
	_line_distance = total_bar_height / 4;
	_bottom_line = child_height() / 2 + total_bar_height / 2;

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