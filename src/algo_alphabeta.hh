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

#ifndef ALGO_ALPHABETA_HH
#define ALGO_ALPHABETA_HH

#include "control.hh"
#include "ttable.hh"
#include "learn.hh"

#include <stdlib.h>
#include <iostream>
#include <glib.h>
#include <sys/time.h>


/* A quite standard alpha-beta search algo with a quite basic evaluation function.
   Still, it is a quite competitive player.
   Features:
   - use of transposition table
   - PV display
   - learning of good/bad games and avoiding previous bad situations.
 */
class PlayerIF_AlgoAB : public PlayerIF
{
public:
  PlayerIF_AlgoAB();

  typedef float eval_t;
  typedef SmallVec<Move, MAXSEARCHDEPTH> Variation;

  // --- configuration ---

  void registerTTable(ttable_ptr tt) { m_ttable=tt; }
  void registerThreadTunnel(ThreadTunnel& tunnel) { m_tunnel=&tunnel; }
  void registerExperience(experience_ptr e) { m_experience=e; }

  // --- AI parameters ---

  void setMaxTime_msec(int msecs) { m_maxMSecs=msecs; }
  void setMaxDepth(int d) { m_maxDepth=d; }

  int  askMaxTime_msec() const { return m_maxMSecs; }
  int  askMaxDepth() const { return m_maxDepth; }

  enum Weight {
    Weight_Material,
    Weight_Freedom,
    Weight_Mills,
    Weight_Experience,
    Weight_NWEIGHTS
  };

  void  setEvalWeight(Weight w, float val) { m_weight[w] = val; }
  float askEvalWeight(Weight w) const { return m_weight[w]; }

  // --- standard methods ---

  bool isInteractivePlayer() const { return false; }

  // start a new game
  void resetGame();

  void startMove(const Board& curr, int moveID);

  // Carry out the move as soon as possible.
  void forceMove();

  // Cancel the current move (do not send the currently computed move).
  void cancelMove();

  void notifyWinner(Player p);

private:
  void doSearch();

  float NegaMax(const Board& board,float alpha,float beta,
		int currDepth, int levels_to_go,Variation&, bool useTT);

  float Eval(const Board& board, int levelsToGo) const;


  Board m_startBoard;
  Move  m_move;       // the move that is currently computed

  //PositionMemory m_posMemory;  // TODO: disabled, because not as effective as Experience
  experience_ptr m_experience;

  // multi-threading management

  friend void startSearchThread(class PlayerIF_AlgoAB*);
  void installJoinThreadHandler();
  void joinThread();

  class ThreadTunnel* m_tunnel;
  GThread* thread;
  int  m_moveID;

  bool m_stopThread;
  bool m_ignoreMove;
  bool m_computedSomeMove;

  // time management

  struct timeval m_startTime; // time when move was started
  void checkTime();

  // configuration

  ttable_ptr m_ttable;
  int m_maxMSecs;
  int m_maxDepth;
  float m_weight[Weight_NWEIGHTS];

  // visualization

  void logBestMove(const Variation&, eval_t, int depth, const char* suffix="") const;
  void logBestMove(const Move&, eval_t, int depth) const;
  void logBestMoveFromTable(const Board&, const Move&, eval_t, int depth) const;

  // statistics

  mutable int  m_nodesEvaluated;

  // debug
  int moveCnt;
};

#endif
