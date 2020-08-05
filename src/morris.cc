/***************************************************************************
  This file is part of Morris.
  Copyright (C) 2009 Dirk Farin <dirk.farin@gmail.com>

  Morris is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
***************************************************************************/

#include "config.h"
#include "gtk_appgui.hh"
#include "morris.hh"
#include "mainapp.hh"

#include <assert.h>
#include <iostream>


int main(int argc, char **argv)
{
  srand(time(NULL));

  // init internationalization

  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);

  // start main application control
  MainApp::createMainAppSingleton();

  // init GUI
  //ApplicationGUI_Gnome::initApplicationGUI(argc, argv);
  ApplicationGUI_Gtk::initApplicationGUI(argc, argv);

  // set default state
  MainApp::app().init();

  // start GUI
  MainApp::app().getApplicationGUI()->startApplication();

  return 0;
}

