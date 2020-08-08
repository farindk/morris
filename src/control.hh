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

#ifndef CONTROL_HH
#define CONTROL_HH

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>

#include "board.hh"
#include "player.hh"


struct GameState
{
  enum State
    {
      Idle,    /* GameControl is waiting for the next move. */
      Moving,  /* A new move was started, but the player did not return a (complete) move yet. */
      Ended    /* The game is over. */
    };

  State state;

  bool   SUBSTATE_PlayerSet;    // whether the player may set a new stone
  bool   SUBSTATE_PlayerMove;   // whether the player may move a stone
  Player SUBSTATE_Winner;       // for STATE_Ended
};


/* The GameControl manages running a game and controls the players.
   Since some players may run in their own threads, the players probably
   will not return their moves directly, but via some other path.

   Process of conducting a move:
   - Call startNextMove().
     The GameControl will then send a start command to the current player.
   - Once the move was given by the user or the AI finished thinking, the
     method 'doMove()' has to be called to actually carry out the move.
   - If the move was not complete (a human move closed a mill, but the
     move did not contain the take), the move is only partially carried out
     and the number of remaining takes is returned. 'doMove()' then has
     to be called again. This can be repeated until the move is complete.
 */
class GameControl
{
public:
  GameControl();
  ~GameControl();

  /* Call this only if the program wants to close and all threads have to
     be stopped before. */
  void stopThreads();


  // --- configuration ---

  void registerPlayerIF(Player p, boost::shared_ptr<PlayerIF> interf);
  void registerRuleSpec(rulespec_ptr);

  boardspec_ptr getBoardSpec() const { return m_ruleSpec->boardSpec; }
  rulespec_ptr  getRuleSpec() const { return m_ruleSpec; }


  // --- global game control ---

  void resetGame();

  /* Initiate the next move. Usually, this will not actually carry out the
     move, but simply e.g., start a background process for AI, or put the
     user-interface into a different state that allows inputting of moves.
   */
  void startNextMove();

  /* Returns the number of takes the player still has to perform.
     If this is != 0, the GUI has to send a new move with more takes.
     Only of the return value is 0, the move has been actually carried out.

     TODO: I now think that this design is bad, because it mixes a query
     (move complete) with a concrete action. This mixing introduces difficulties,
     e.g., when the GUI wants to enter the move, it cannot really end it before
     it calls doMove(), because it doesn't know, if it is really the end.
     Consequently, startNextMove() cannot immediately start the next move,
     because the GUI is not cleaned up yet. This requires the current
     work-arounds with starting the next move in an idle-function.
  */
  int doMove(Move m);

  /* Force the current player (usually the AI) to carry out the move as
     soon as possible.
   */
  void forceMove() { m_player[ player2Index(m_board->getCurrentPlayer()) ]->forceMove(); }

  GameState     getGameState() const { return m_gameState; }
  Player        getCurrentPlayer() const { return getCurrentBoard().getCurrentPlayer(); }
  player_ptr    getCurrentPlayerInterface() const { return m_player[player2Index(m_board->getCurrentPlayer())]; }

  /* Get the current board. After conducting an incomplete move, this shows the
     situation after the incomplete move. */
  const Board&  getCurrentBoard() const { return m_partialMoveActive ? m_partialMoveBoard : *m_board; }

  /* Same as above, but will never return an intermediate board after a partial move,
     but the situation just before the move. */
  const Board&  getCurrentBoard_noTemporary() const { return *m_board; }

  bool          hasGameEnded() const { return m_gameHasEnded; }
  Player        getGameWinner() const { return m_winner; }

  /* The move-ID is a number that identifies the current move.
     It is used to solve nasty race-conditions, in which a separate AI-thread
     pushes its move into the main-loop while the user just starts a new game
     or performs an undo-operation.
     With the move-ID, the main thread can check if the move sent by the AI-thread
     is still the same game as the currently shown board. To this end, the AI-thread
     stores the move-ID at the time its move was started and forwards this stored ID
     together with its move to the main-thread.
   */
  int getCurrentMoveID() const { return m_moveID; }


  // --- history / undo-buffer ---
  // TODO: extract history to external class (would allow e.g. variation analysis)

  void          undoMove();
  void          redoMove();

  int           getHistoryPos() const { return m_currentHistoryPos; }
  int           getHistorySize() const { return m_history.size(); }
  Move          getHistoryMove(int ply) const { return m_movelog[ply]; }
  boost::shared_ptr<Board> getHistoryBoard(int ply) const { return m_history[ply]; }


  // --- signals ---

  boost::signals2::signal<void (Player)>&     getSignal_gameOver()    { return m_signal_gameOver; }
  boost::signals2::signal<void (GameState)>&  getSignal_changeState() { return m_signal_changeState; }
  boost::signals2::signal<void ()>&           getSignal_changeBoard() { return m_signal_changeBoard; }
  boost::signals2::signal<void (player_ptr)>& getSignal_startMove()   { return m_signal_startMove; }
  boost::signals2::signal<void (player_ptr)>& getSignal_endMove()     { return m_signal_endMove; }

private:
  rulespec_ptr  m_ruleSpec;
  player_ptr    m_player[2];

  Board*        m_board;
  GameState     m_gameState;

  int           m_moveID;

  bool          m_partialMoveActive;
  Board         m_partialMoveBoard;

  bool m_gameHasEnded; /* Do not confuse with GameState::Ended.
			  This flag tells if the game in the history is complete. */
  Player m_winner; // Only valid if m_gameHasEnded==true.


  // Undo-Buffer

  std::vector<Move> m_movelog;
  std::vector< boost::shared_ptr<Board> > m_history;
  int m_currentHistoryPos;


  // signals

  boost::signals2::signal<void (Player)>     m_signal_gameOver;
  boost::signals2::signal<void (GameState)>  m_signal_changeState;
  boost::signals2::signal<void ()>           m_signal_changeBoard;

  boost::signals2::signal<void (player_ptr)> m_signal_startMove;
  boost::signals2::signal<void (player_ptr)> m_signal_endMove;
};

#endif

