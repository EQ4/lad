/*
  This file is part of Ingen.
  Copyright 2007-2012 David Robillard <http://drobilla.net/>

  Ingen is free software: you can redistribute it and/or modify it under the
  terms of the GNU Affero General Public License as published by the Free
  Software Foundation, either version 3 of the License, or any later version.

  Ingen is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU Affero General Public License for details.

  You should have received a copy of the GNU Affero General Public License
  along with Ingen.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef INGEN_SERIALISATION_SERIALISER_HPP
#define INGEN_SERIALISATION_SERIALISER_HPP

#include <stdexcept>
#include <string>

#include "raul/Path.hpp"

#include "sord/sordmm.hpp"

#include "ingen/Node.hpp"

namespace Ingen {

class Arc;
class Node;
class Store;
class World;

namespace Serialisation {

/**
   Write Ingen objects to Turtle files or strings.

   @ingroup IngenSerialisation
*/
class Serialiser
{
public:
	explicit Serialiser(World& world);
	virtual ~Serialiser();

	/** Write a graph and all its contents as a complete bundle. */
	virtual void write_bundle(SPtr<const Node>   graph,
	                          const std::string& path);

	/** Begin a serialization to a string.
	 *
	 * This must be called before any serializing methods.
	 *
	 * The results of the serialization will be returned by the finish() method after
	 * the desired objects have been serialised.
	 *
	 * All serialized paths will have the root path chopped from their prefix
	 * (therefore all serialized paths must be descendants of the root)
	 */
	virtual void start_to_string(const Raul::Path&  root,
	                             const std::string& base_uri);

	/** Begin a serialization to a file.
	 *
	 * This must be called before any serializing methods.
	 *
	 * All serialized paths will have the root path chopped from their prefix
	 * (therefore all serialized paths must be descendants of the root)
	 */
	virtual void start_to_file(const Raul::Path&  root,
	                           const std::string& filename);

	/** Serialize an object (graph, block, or port). */
	virtual void serialise(SPtr<const Node> object)
			throw (std::logic_error);

	/** Serialize an arc. */
	virtual void serialise_arc(const Sord::Node& parent,
	                           SPtr<const Arc>   arc)
			throw (std::logic_error);

	/** Finish serialization.
	 *
	 * If this is a file serialization, this must be called to finish and close
	 * the output file, and the empty string is returned.
	 *
	 * If this is a string serialization, the serialized result is returned.
	 */
	virtual std::string finish();

private:
	struct Impl;
	Impl* me;
};

} // namespace Serialisation
} // namespace Ingen

#endif // INGEN_SERIALISATION_SERIALISER_HPP
