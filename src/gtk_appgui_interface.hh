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

#ifndef APPLICATIONGUI_GTKIF
#define APPLICATIONGUI_GTKIF

#include <gtk/gtk.h>
#include "appgui.hh"

// Trivial helper class in case we want to easily support both Gtk-only
// and Gnome applications.
class ApplicationGUI_GtkIF : public ApplicationGUI
{
public:
  virtual GtkWidget* getMainWindow() = 0;
};

#endif
