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

#ifndef THREADTUNNEL_HH
#define THREADTUNNEL_HH

#include "board.hh"


/* The thread-tunnel provides the interface through which the players
   communicate to the main application. The thread-tunnel object is
   required, because many players will run in separate threads and
   have to send their messages as inter-thread messages. The thread-tunnel
   performs this inter-thread message forwarding.
 */
class ThreadTunnel
{
public:
  ThreadTunnel() {} 
  virtual ~ThreadTunnel() { }

  // --- thread tunnels ---

  // Set the progress bar in the status-line (value is in [0;1]).
  virtual void setProgress(float progress) { }

  // Show some additional information about the AI thinking-process in the statusbar.
  virtual void showThinkingInfo(const std::string&) { }

  // Send move to main application.
  virtual void doMove(Move m, int moveID) = 0;

  // currently unused
  // virtual void setStatusbar(const char*) { }
};



/* Define and install a run-once function that is executed
   once when the program is idle.
   This is an alternative way of inter-thread communication.
 */
class IdleFunc
{
public:
  virtual ~IdleFunc() { }

  // The idle function...
  virtual void operator() () = 0;

  /* The install function is defined in one of the
     GUI-framework specific modules (e.g., gtk_threadtunnel.cc).
  */
  static void install(IdleFunc*);
};

#endif
