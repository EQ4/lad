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

#include <time.h>

#include <iostream>

#include <glibmm.h>
#include <gtkmm.h>

#include "eugene/Crossover.hpp"
#include "eugene/GA.hpp"
#include "eugene/GA.hpp"
#include "eugene/Gene.hpp"
#include "eugene/Mutation.hpp"
#include "eugene/Problem.hpp"
#include "ganv/Canvas.hpp"
#include "ganv/Node.hpp"

#include "../src/TSP.hpp"

#include "MainWindow.hpp"

using namespace std;
using namespace Ganv;
using namespace eugene;
using namespace eugene::GUI;

int
main(int argc, char** argv)
{
	Glib::thread_init();

	Gtk::Main main(argc, argv);

	MainWindow* gui = new MainWindow();
	gui->main_win->present();

	main.run(*gui->main_win);

	return 0;
}
