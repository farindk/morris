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

#ifndef APP_HH
#define APP_HH

#include <boost/shared_ptr.hpp>
#include "boardgui.hh"
#include "util.hh"


class ApplicationGUI
{
public:
  virtual ~ApplicationGUI() { }

  virtual void setProgress(float) { }
  virtual void setStatusbar(const std::string&) { }

  virtual void showGameOverDialog(Player winner) { }
  virtual void preferencesDialog_Display() { }
  virtual void showAboutDialog() { }
  virtual void showMoveLog(bool enable=true, bool quitapp=false) { }
  virtual bool isMoveLogShown() const { return false; }

  virtual boardgui_ptr getBoardGUI() const = 0;

  // run main-loop
  virtual void startApplication() = 0;
  virtual void quitApplication() = 0;
};

typedef boost::shared_ptr<ApplicationGUI> appgui_ptr;

#endif
