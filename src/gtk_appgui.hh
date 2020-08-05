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

#ifndef GTK_APP_HH
#define GTK_APP_HH

#include "gtk_appgui_interface.hh"
#include "gtk_movelog.hh"
#include <gtk/gtk.h>
#include <string>


class ApplicationGUI_Gtk : public ApplicationGUI_GtkIF
{
public:
  ApplicationGUI_Gtk();
  ~ApplicationGUI_Gtk();

  static void initApplicationGUI(int& argc, char**& argv);

  virtual void startApplication();
  virtual void quitApplication();

  virtual void setProgress(float p)
  {
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progressbar), p);
  }

  virtual void setStatusbar(const std::string& str)
  {
    gtk_statusbar_pop (GTK_STATUSBAR(statusbar), statusbar_context);
    gtk_statusbar_push(GTK_STATUSBAR(statusbar), statusbar_context, str.c_str());
  }

  void showGameOverDialog(Player winner);
  void enableGameOverDialog(bool flag) { options.showGameOverRequester = flag; }
  bool getGameOverDialogFlag() const { return options.showGameOverRequester; }

  void preferencesDialog_Display();
  void showMoveLog(bool enable=true, bool quitApp=false);
  bool isMoveLogShown() const { return gui_movelog != NULL; }

  GtkWidget* getMainWindow() { return window; }
  GtkWidget* getMainWindowVBox() { return main_vbox; }
  void       showWindow() { gtk_widget_show_all(window); }

  void setBoardGUI(boardgui_ptr gui) { gui_board=gui; }
  boardgui_ptr getBoardGUI() const { return gui_board; }
  void showAboutDialog();

private:
  GtkWidget* window;
  GtkWidget* main_vbox;

  GtkWidget* statusbar;
  gint statusbar_context;

  GtkWidget* progressbar;

  boardgui_ptr  gui_board;
  boost::shared_ptr<MoveLog> gui_movelog;

  struct Options
  {
    Options() : showGameOverRequester(true) { }

    bool showGameOverRequester;
  } options;

  friend void gtk_initMainMenu(GtkWidget* vbox, GtkWidget* window);

  void changeState(const struct GameState& state);
  void changeBoard();
  void startMove();
};

#endif
