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

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "TSP.hpp"

using namespace std;

namespace eugene {

TSP::TSP(const std::string& filename)
	: _filename(filename)
{
	cout << "Loading TSP problem " << filename << endl;

	ifstream ifs(filename.c_str());

	size_t tokens = 1;
	size_t cities = 0;

	if (ifs.eof())
		throw std::runtime_error("File empty");

	while (true) {
		float in = 0.0f;
		ifs >> in; // city number

		if (ifs.eof())
			break;

		if (tokens % 3 == 1) {
			if (in != cities + 1) {
				std::ostringstream ss;
				ss << "Expected " << cities << " but found " << in
						<< " (after " << tokens << " tokens)";
				ifs.close();
				throw std::runtime_error(ss.str());
			}
		}

		++tokens;

		City city;

		ifs >> in; // x coordinate
		city.first = in;

		if (ifs.eof())
			throw std::runtime_error("Unexpected EOF");

		ifs >> in; // y coordinate
		city.second = in;

		++tokens;

		_cities.push_back(city);
		++cities;
	}

	_gene_size = cities;

	cout << "Loaded " << cities << " cities" << endl;
}

void
TSP::initial_population(Random&     rng,
                        Population& pop,
                        size_t      gene_size,
                        size_t      pop_size) const
{
	pop.assign(pop_size, GeneType(gene_size));
	for (size_t i = 0; i < pop_size; ++i) {
		assert(i < pop.size());
		for (size_t j = 0; j < gene_size; ++j) {
			assert(j < pop[i].size());
			pop[i][j] = j;
		}

		std::random_shuffle(pop[i].begin(), pop[i].end());
		assert(pop[i].size() == _gene_size);
		assert(pop[i].size() == _cities.size());
		assert(assert_gene(pop[i]));
	}
}

float
TSP::evaluate(const GeneType& g) const
{
	//assert(g.size() == _cities.size());

	float d = 0.0f;

	for (size_t i = 0; i < _cities.size() - 1; ++i) {
		assert(i < g.size());
		assert(g[i] < _cities.size());
		assert(g[i+1] < _cities.size());
		d += distance(_cities[g[i]], _cities[g[i+1]]);
	}

	d += distance(_cities[_cities.size()-1], _cities[0]);

	return d;
}

} // namespace eugene
