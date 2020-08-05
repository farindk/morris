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

#include "control.hh"
#include "threadtunnel.hh"

#include <iostream>


GameControl::GameControl()
{
  m_partialMoveActive = false;  // TODO: move the partial moves into board-gui ?

  m_history.push_back( boost::shared_ptr<Board>(new Board) );
  m_currentHistoryPos=0;
  m_board=m_history[0].get();

  m_moveID=0;

  // setup some game ...

  m_ruleSpec = RuleSpec::createPresetRule(RuleSpec::Preset_Standard);
  resetGame();
}

GameControl::~GameControl()
{
}

void GameControl::registerPlayerIF(Player p, player_ptr interf)
{
  int idx = player2Index(p);

  // if a move is underway, stop it
  if (m_gameState.state == GameState::Moving &&
      p==m_board->getCurrentPlayer())
    {
      m_gameState.state = GameState::Idle;
      m_moveID++;

      m_player[idx]->cancelMove();
      m_signal_endMove(getCurrentPlayerInterface());
    }

  // replace the player
  m_player[idx] = interf;
  interf->setPlayer(p);
  interf->setRuleSpec( m_ruleSpec );
}

void GameControl::registerRuleSpec(rulespec_ptr r)
{
  m_ruleSpec=r;

  if (m_gameState.state == GameState::Moving && getCurrentPlayerInterface())
    {
      getCurrentPlayerInterface()->cancelMove();
      m_signal_endMove(getCurrentPlayerInterface());
    }

  for (int i=0;i<2;i++)
    if (m_player[i]!=NULL) m_player[i]->setRuleSpec(r);

  m_gameState.state = GameState::Idle;
}

int GameControl::doMove(Move m)
{
  assert(m_gameState.state == GameState::Moving);


  // check if there are still 'takes' to be added to the move

  int nTakes = m_ruleSpec->nPotentialMills(*m_board, m);
  if (nTakes > 0 && m_ruleSpec->mayTakeMultiple==false)
    nTakes=1;

  nTakes -= m.takes.size(); // subtract the number of takes we have already entered
  if (nTakes>0)
    {
      m_partialMoveActive= true;
      m_partialMoveBoard = *m_board;
      m_partialMoveBoard.doMove(m);
      m_partialMoveBoard.togglePlayer(); // stay with current player (toggle back)
      return nTakes;
    }


  // --- move is complete, carry out the move ---

  m_partialMoveActive=false;
  m_signal_endMove(getCurrentPlayerInterface());

  // add copy of current board to history end
  m_history.resize(m_currentHistoryPos+1); // delete future history
  m_movelog.resize(m_currentHistoryPos);   // delete future history
  m_history.push_back( boost::shared_ptr<Board>(new Board) );
  m_movelog.push_back(m);

  m_currentHistoryPos++;
  *(m_history.back()) = *m_board;
  m_history.back()->setPrevBoard(*(m_history.end()-2));
  m_board = m_history.back().get();

  m_board->doMove(m);


  // check for end of game

  if (m_ruleSpec->currentPlayerHasLost(*m_board))
    {
      m_gameState.state = GameState::Ended;
      m_gameState.SUBSTATE_Winner = m_board->getOpponentPlayer();
    }
  /* We do not have to check for the current player having won,
     because in that case, the opponent would have been detected
     as loser in the previous move. */
  else if (m_ruleSpec->tieBetweenBothPlayers(*m_board))
    {
      m_gameState.state = GameState::Ended;
      m_gameState.SUBSTATE_Winner = PL_None;
    }
  else
    {
      m_gameState.state = GameState::Idle;
    }


  // announce winner

  if (m_gameState.state == GameState::Ended)
    {
      m_gameHasEnded=true;
      m_winner=m_gameState.SUBSTATE_Winner;

      for (int i=0;i<2;i++)
	{ m_player[i]->notifyWinner(m_winner); }

      m_signal_gameOver(m_winner);
      m_signal_changeState(m_gameState);
    }

  m_signal_changeBoard();

  return 0; // no more takes, move is complete
}

void GameControl::startNextMove()
{
  assert(m_gameState.state == GameState::Idle);
  assert(getCurrentPlayerInterface() != NULL);


  m_gameState.state = GameState::Moving;

  // update game state
      
  const Player p = m_board->getCurrentPlayer();
  m_gameState.SUBSTATE_PlayerSet  = (m_board->getNPiecesToSet(p) > 0);
  m_gameState.SUBSTATE_PlayerMove = (m_board->getNPiecesToSet(p)==0 || m_ruleSpec->laskerVariant);


  // initiate next player's move

  m_moveID++;
  getCurrentPlayerInterface()->startMove(getCurrentBoard(), m_moveID);

  m_signal_startMove( getCurrentPlayerInterface() );
}


void GameControl::undoMove()
{
  if (m_currentHistoryPos>0)
    {
      m_moveID++;

      if (m_gameState.state == GameState::Moving)
	{
	  getCurrentPlayerInterface()->cancelMove();
	  m_signal_endMove(getCurrentPlayerInterface());
	}

      m_gameState.state = GameState::Idle;

      m_partialMoveActive=false;
      m_currentHistoryPos--;
      m_board = m_history[m_currentHistoryPos].get();

      m_signal_changeState(m_gameState);
      m_signal_changeBoard();
    }
}


void GameControl::redoMove()
{
  if (m_currentHistoryPos<m_history.size()-1)
    {
      m_moveID++;

      if (m_gameState.state == GameState::Moving)
	{
	  getCurrentPlayerInterface()->cancelMove();
	  m_signal_endMove(getCurrentPlayerInterface());
	}

      m_partialMoveActive=false;
      m_currentHistoryPos++;
      m_board = m_history[m_currentHistoryPos].get();

      Player winner;
      if (m_ruleSpec->isGameOver(*m_board, &winner))
	{
	  m_gameState.state = GameState::Ended;
	  m_gameState.SUBSTATE_Winner = winner;
	}
      else
	{
	  m_gameState.state = GameState::Idle;
	}

      m_signal_changeState(m_gameState);
      m_signal_changeBoard();
    }
}


void GameControl::resetGame()
{
  if (m_gameState.state==GameState::Moving)
    {
      getCurrentPlayerInterface()->cancelMove();
      m_signal_endMove(getCurrentPlayerInterface());
    }

  for (int i=0;i<2;i++)
    if (m_player[i])
      { m_player[i]->resetGame(); }

  m_moveID++;

  m_partialMoveActive = false;

  m_history.clear();
  m_movelog.clear();
  m_history.push_back( boost::shared_ptr<Board>(new Board) );
  m_currentHistoryPos=0;
  m_board = m_history[0].get();

  m_board->reset( m_ruleSpec->nPieces );

  m_gameHasEnded=false;
  m_gameState.state = GameState::Idle;

  m_signal_changeState(m_gameState);
  m_signal_changeBoard();
}


void GameControl::stopThreads()
{
  for (int i=0;i<2;i++)
    if (m_player[i])
      {
	m_player[i]->cancelMove();
      }
}
