/* This file is part of Eugene
 * Copyright 2007-2012 David Robillard <http://drobilla.net>
 *
 * Eugene is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Eugene is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with Eugene.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ganv/ganv.hpp"
#include "eugene/GA.hpp"
#include "eugene/Problem.hpp"
#include "eugene/Selection.hpp"
#include "eugene/Mutation.hpp"
#include "eugene/Crossover.hpp"
#include "TSPCanvas.hpp"

#include "../src/TSP.hpp"

using namespace Ganv;

namespace eugene {
namespace GUI {

typedef GA<TSP::GeneType> TSPGA;

std::shared_ptr<TSPCanvas>
TSPCanvas::load(std::shared_ptr<TSP> tsp)
{
	std::shared_ptr<TSPCanvas> canvas(new TSPCanvas(2000, 2000));

	size_t city_count = 0;
	canvas->_nodes.resize(tsp->cities().size());

	for (TSP::Cities::const_iterator i = tsp->cities().begin(); i != tsp->cities().end(); ++i) {
		std::shared_ptr<Circle> node(new Circle(*canvas.get(), "", i->first+8, i->second+8));
		node->set_border_color(0xFFFFFFFF);
		node->set_fill_color(0x333333FF);
		node->show();
		canvas->_nodes[city_count++] = node;
	}

	return canvas;
}

static void
remove_edge_cb(GanvEdge* edge, void* data)
{
	TSPCanvas* canvas = (TSPCanvas*)data;
	Edge*      edgemm = Glib::wrap(edge);
	canvas->remove_edge(edgemm);
}

void
TSPCanvas::update(const TSP::GeneType& best)
{
	this->for_each_edge(remove_edge_cb, this);
	for (size_t i = 0; i < best.size(); ++i) {
		assert(best.at(i) < _nodes.size());
		assert(best.at((i+1) % best.size()) < _nodes.size());

		const size_t c1 = best[i];
		const size_t c2 = best[(i+1) % best.size()];

		Edge* edge = new Edge(*this,
		                      _nodes[c1].get(), _nodes[c2].get(),
		                      0xFFFFFFAA, false);
		edge->set_handle_radius(0.0);
		edge->show();
	}
}

} // namespace GUI
} // namespace eugene
