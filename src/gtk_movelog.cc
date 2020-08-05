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

#include "gtk_movelog.hh"
#include "util.hh"
#include "rules.hh"
#include "mainapp.hh"

#include <iostream>
#include <boost/bind.hpp>

const int PADDING=10;


static gboolean quit_callback(GtkWidget *widget, gpointer data)
{
  return FALSE;
}


gboolean cbMoveLog_gtk_destroy(GtkWidget *widget, gpointer data)
{
  ((MoveLog_Gtk*)(data))->cbDestroy();

  return FALSE;
}


MoveLog_Gtk::MoveLog_Gtk()
{
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), _("Move List"));
  gtk_container_set_border_width(GTK_CONTAINER(window), PADDING);
  gtk_window_set_default_size(GTK_WINDOW(window), 210,400);

  scrolledWindow = gtk_scrolled_window_new(NULL,NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledWindow),
				 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  treeview = gtk_tree_view_new();

  gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)), GTK_SELECTION_NONE);

  {
    GtkCellRenderer* renderer;
    GtkTreeViewColumn* column;

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Move"),
						       renderer,
						       "text", 0,
						       //"foreground", COLOR_COLUMN,
						       NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("White"),
						       renderer,
						       "text", 1,
						       "background-gdk", 4,
						       NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Black"),
						       renderer,
						       "text", 2,
						       "background-gdk", 6,
						       NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
  }

  liststore = gtk_list_store_new(7,
				 G_TYPE_INT, // move number
				 G_TYPE_STRING, G_TYPE_STRING, // white move, black move
				 GDK_TYPE_COLOR, GDK_TYPE_COLOR, // white fgr, bkg
				 GDK_TYPE_COLOR, GDK_TYPE_COLOR  // black fgr, bkg
				 );

  {
    refresh();
  }

  gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(liststore));

  // do not g_object_unref(liststore); -> keep lock on liststore
  g_object_ref(window);

  gtk_container_add(GTK_CONTAINER(scrolledWindow), treeview);
  gtk_container_add(GTK_CONTAINER(window), scrolledWindow);
  gtk_widget_show_all(window);

  g_signal_connect(window, "delete_event", G_CALLBACK(quit_callback), NULL);
  destroyHandler = g_signal_connect(window, "destroy", G_CALLBACK(cbMoveLog_gtk_destroy), this);

  GameControl& control = MainApp::app().getControl();
  refreshConnection = control.getSignal_changeBoard().connect(boost::bind(&MoveLog_Gtk::refresh, this));
}

MoveLog_Gtk::~MoveLog_Gtk()
{
  refreshConnection.disconnect();
  g_signal_handler_disconnect(window, destroyHandler);  // avoid endless recursive destroying loop

  gtk_widget_destroy(window);

  g_object_unref(liststore);
  g_object_unref(window);
}


void MoveLog_Gtk::cbDestroy()
{
  m_signal_windowClosed();
}


/* TODO: the refresh is really inefficient, because the complete list is rebuilt, adding the content
   line by line. Since gtk seems to make a refresh for each line, this is very slow.
 */
void MoveLog_Gtk::refresh()
{
  if (!liststore) { return; }

  gtk_list_store_clear(liststore);

  const GameControl& control = MainApp::app().getControl();
  boardspec_ptr boardspec = control.getBoardSpec();

  GdkColor activeColor, passiveColor;
  gdk_color_parse("#c0c0ff", &activeColor);
  gdk_color_parse("#ffffff", &passiveColor);

  const bool ended = (control.hasGameEnded());

  std::string winner;
  if (ended)
    {
      if (control.getGameWinner()==PL_None)
	winner=_("tie");
      else
	winner="+++";
    }
  else
    winner="...";

  for (int ply=0;ply<control.getHistorySize();ply+=2)
    {
      Move whitemove;
      Move blackmove;

      std::string whiteStr,blackStr;

      if (ply<control.getHistorySize()-1)
	{
	  whitemove = control.getHistoryMove(ply);
	  whiteStr = writeMove(whitemove,boardspec);
	}

      if (ply==control.getHistorySize()-1)
	{
	  whiteStr = winner;
	}

      if (ply+1<control.getHistorySize()-1)
	{
	  blackmove = control.getHistoryMove(ply+1);
	  blackStr  = writeMove(blackmove,boardspec);
	}

      if (ply+1==control.getHistorySize()-1)
	{
	  blackStr = winner;
	}

      bool whiteCurrent = (ply   == control.getHistoryPos());
      bool blackCurrent = (ply+1 == control.getHistoryPos());

      GtkTreeIter iter;
      gtk_list_store_append(liststore, &iter);
      gtk_list_store_set(liststore, &iter,
			 0, ply/2+1,
			 1, whiteStr.c_str(),
			 2, blackStr.c_str(),
			 4, (whiteCurrent ? &activeColor : &passiveColor),
			 6, (blackCurrent ? &activeColor : &passiveColor),
			 -1);
    }
}
