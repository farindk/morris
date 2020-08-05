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

#ifndef PLAYER_HH
#define PLAYER_HH

#include "board.hh"
#include "rules.hh"
#include "threadtunnel.hh"


/* The main interface for all kinds of players.

   Communication of the player to the outside world is usually through
   the ThreadTunnel interface. This is because some players will run
   as separate threads and have to send their messages to the main
   application thread. This inter-thread-communication is performed
   by the thread-tunnel.
   NOTE: player do not have to use the thread-tunnel, if they do not
   need. But it can be convenient to use this interface even if it
   is not strictly required.

   The main operation is that GameControl initiates a new move by
   calling startMove(). Once the move is computed, entered, or received,
   it is sent to the main application with the doMove() method of the
   thread-tunnel.
 */
class PlayerIF
{
public:
  PlayerIF() { m_selfPlayer=PL_None; }

  virtual ~PlayerIF() { }

  void setPlayer(Player p) { m_selfPlayer=p; }
  void setRuleSpec(rulespec_ptr rs) { m_ruleSpec = rs; }

  virtual void registerThreadTunnel(ThreadTunnel& tunnel) { }
  virtual bool isInteractivePlayer() const { return false; }

  virtual void resetGame() { }
  virtual void startMove(const Board& current, int moveID) = 0; // start this player's move
  virtual void forceMove() { }  // stop thinking process and move as soon as possible
  virtual void cancelMove() { } // stop thinking and do not move, will join the algo-thread

  virtual void notifyWinner(Player p) { }

protected:
  Player       m_selfPlayer;
  rulespec_ptr m_ruleSpec;
};

typedef boost::shared_ptr<PlayerIF> player_ptr;

#endif
