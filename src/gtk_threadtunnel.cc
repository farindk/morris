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

#include "mainapp.hh"
#include "gtk_threadtunnel.hh"

#include <assert.h>
#include <iostream>

#include <gtk/gtk.h>


static gboolean cb_setProgress(gpointer p)
{
  float* progress = (float*)p;

  MainApp::app().getApplicationGUI()->setProgress(*progress);
  delete progress;

  return FALSE;
}

static gboolean cb_setStatusbar(gpointer p)
{
  std::string* str = (std::string*)p;

  MainApp::app().getApplicationGUI()->setStatusbar(*str);
  delete str;

  return FALSE;
}

struct MoveData
{
  Move move;
  int  gameID;
};

static gboolean cb_doMove(gpointer p)
{
  MoveData* m = (MoveData*)p;

  gdk_threads_enter(); // see gtk_boardgui.cc for why this is needed
  MainApp::app().visualizeMove(m->move, m->gameID);
  gdk_threads_leave();
  delete m;

  return FALSE;
}


// ---------------------------------------------------------------------------


class ThreadTunnel_Gtk : public ThreadTunnel
{
public:
  void setStatusbar(const char* buf)
  {
    g_idle_add( cb_setStatusbar, new std::string(buf) );
  }

  void setProgress(float progress)
  {
    float* p = new float;
    *p = progress;

    g_idle_add( cb_setProgress, p );
  }

  void doMove(Move m, int gameID)
  {
    MoveData* md = new MoveData;
    md->move = m;
    md->gameID = gameID;

    g_idle_add( cb_doMove, md );
  }

  void showThinkingInfo(const std::string& str)
  {
    class IdleFunc_showThinking : public IdleFunc
    {
    public:
      IdleFunc_showThinking(const std::string& s) : m_string(s) { }

      void operator()() { MainApp::app().setThinkingInfo(m_string); }

      std::string m_string;
    };

    IdleFunc::install(new IdleFunc_showThinking(str));
  }
};


static ThreadTunnel_Gtk guicb_gtk;

ThreadTunnel& getThreadTunnel_Gtk()
{
  return guicb_gtk;
}



// ---------------------------------------------------------------------------

static gboolean cb_idlefunc(gpointer p)
{
  IdleFunc* func = (IdleFunc*)p;

  (*func)();

  delete func;
  return FALSE;
}

void IdleFunc::install(IdleFunc* func)
{
  g_idle_add( cb_idlefunc, func );
}
