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
#include "gtkcairo_boardgui.hh"
#include "gtk_menutoolbar.hh"
#include "gtk_appgui.hh"
#include "mainapp.hh"
#include "gtk_threadtunnel.hh"
#include "gtk_prefDisplay.hh"
#include "appgtk_configmgr.hh"

#if HAVE_GCONF
#  include "gconf_configmgr.hh"
#endif

#include <assert.h>
#include <iostream>
#include <boost/bind.hpp>
#include <glib/gthread.h>


static gboolean delete_callback(GtkWidget *widget, GdkEvent* event, gpointer data)
{
  return FALSE;
}


static void cb_destroy(GtkWidget *widget, gpointer data)
{
  ((ApplicationGUI_Gtk*)(data))->quitApplication();
}


/* This function should also work if MainApp is not fully functional yet.
 */
ApplicationGUI_Gtk::ApplicationGUI_Gtk()
{
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  main_vbox = gtk_vbox_new(false,0);
  gtk_container_add (GTK_CONTAINER (window), main_vbox);

  gtk_window_set_policy(GTK_WINDOW(window), TRUE, TRUE, FALSE);
  gtk_widget_realize(window);

  // statusbar

  statusbar = gtk_statusbar_new();
  statusbar_context = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), "mainapp");
  gtk_box_pack_end(GTK_BOX(main_vbox), statusbar, false,true,0);

  progressbar = gtk_progress_bar_new();
  gtk_box_pack_start(GTK_BOX(statusbar), progressbar, false,true,0);
  gtk_box_reorder_child(GTK_BOX(statusbar), progressbar, 0);

  // signals

  gtk_signal_connect(GTK_OBJECT(window), "destroy",      GTK_SIGNAL_FUNC(cb_destroy), this);
  gtk_signal_connect(GTK_OBJECT(window), "delete_event", GTK_SIGNAL_FUNC(delete_callback), this);
}


ApplicationGUI_Gtk::~ApplicationGUI_Gtk()
{
}


void ApplicationGUI_Gtk::initApplicationGUI(int& argc, char**& argv)
{
  g_thread_init(NULL);
  gdk_threads_init();

  gtk_init(&argc,&argv);

  // attach app-GUI and board-GUI to main application

  ApplicationGUI_Gtk* appgui = new ApplicationGUI_Gtk;
  MainApp::app().setApplicationGUI( appgui_ptr(appgui) );
  MainApp::app().setThreadTunnel( getThreadTunnel_Gtk() );

  BoardGUI_GtkCairo* boardgui = new BoardGUI_GtkCairo;
  appgui->setBoardGUI( boardgui_ptr(boardgui));

  gtk_box_pack_end(GTK_BOX(appgui->getMainWindowVBox()), boardgui->getBoardWidget(), true,true,0);


  // install configuration manager using GConf

#if HAVE_GCONF
  ConfigManager_GConf* config = new ConfigManager_GConf(0,NULL);
  config->setAppConfigDelegate(configmanager_ptr(new ConfigManager_AppGtk));
#else
  ConfigManager_Chained* config = new ConfigManager_Application;
  config->setDelegate(configmanager_ptr(new ConfigManager_AppGtk));
#endif

  MainApp::app().registerConfigManager(configmanager_ptr(config));
}


void ApplicationGUI_Gtk::startApplication()
{
  GameControl& control = MainApp::app().getControl();

  control.getSignal_changeState().connect(boost::bind(&ApplicationGUI_Gtk::changeState, this, _1));
  control.getSignal_changeBoard().connect(boost::bind(&ApplicationGUI_Gtk::changeBoard, this));
  control.getSignal_startMove()  .connect(boost::bind(&ApplicationGUI_Gtk::startMove,   this));

  gtk_initMainMenu(main_vbox, window);
  gtk_connectMenuSignals();

  showWindow();

  MainApp::app().getConfigManager()->readInitialValues(); // TODO: this should probably be in MainApp.init()

  control.startNextMove();

  // === main loop ===

  gdk_threads_enter();
  gtk_main();
  gdk_threads_leave();
}


void ApplicationGUI_Gtk::quitApplication()
{
  showMoveLog(false, true);
  MainApp::app().getControl().stopThreads();
  gtk_main_quit();
}


void ApplicationGUI_Gtk::changeState(const GameState& state)
{
}


void ApplicationGUI_Gtk::changeBoard()
{
}


void ApplicationGUI_Gtk::startMove()
{
}


void ApplicationGUI_Gtk::preferencesDialog_Display()
{
  ::preferencesDialog_Display();
  gui_board->resetDisplay();
}


void ApplicationGUI_Gtk::showMoveLog(bool enable, bool quitApp)
{
  if (enable)
    {
      gui_movelog = boost::shared_ptr<MoveLog>(new MoveLog_Gtk);

      gui_movelog->getSignal_windowClosed().connect(boost::bind(&ApplicationGUI_Gtk::showMoveLog, this,false,false));
      gui_movelog->getSignal_windowClosed().connect(boost::bind(menu_closedMoveLog, false));
    }
  else
    {
      gui_movelog.reset();
    }

  if (!quitApp)
    {
      MainApp::app().getConfigManager()->store(ConfigManager::itemPref_showLogOfMoves, enable);
    }
}


void ApplicationGUI_Gtk::showGameOverDialog(Player winner)
{
  if (!options.showGameOverRequester)
    { return; }

  const char* text;
  switch (winner)
    {
    case PL_White: text=_("White has won."); break;
    case PL_Black: text=_("Black has won."); break;
    case PL_None:  text=_("Tie between both players."); break;
    }

  GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(window),
					     GTK_DIALOG_DESTROY_WITH_PARENT,
					     GTK_MESSAGE_INFO,
					     GTK_BUTTONS_OK,
					     _("Game over"));
  gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), text);

  gtk_dialog_run(GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}


void ApplicationGUI_Gtk::showAboutDialog()
{
  const char* authors[] =
    {
      "Dirk Farin <dirk.farin@gmail.com>",
      NULL
    };

  gtk_show_about_dialog (GTK_WINDOW(window),
			 "program-name", PACKAGE_NAME,
			 //"logo", example_logo,
			 //"title", _("About Gnome Nine-Mens-Morris"),  // tag does not exist ?
			 "authors", authors,
			 "version", VERSION,
			 "comments", _("A computer adaptation of the Nine-Mens-Morris board game and its variants."),
			 "copyright", "Copyright (c) 2009 Dirk Farin",
			 //"website", "morris.org",
			 NULL);
}
