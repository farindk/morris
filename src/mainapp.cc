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
#include "algo_alphabeta.hh"
#include "algo_random.hh"
#include <assert.h>
#include <boost/bind.hpp>
#include "util.hh"

AppState::AppState()
{
  gamePaused       = false;
  networkConnected = false;
}


MainApp::Options::Options()
{
  alwaysPauseOnAIPlayer=false;
}


static boost::shared_ptr<MainApp> mainapp_singleton;

void MainApp::createMainAppSingleton()
{
  // init tables
  Board::initHashValues();

  // create main application
  mainapp_singleton = boost::shared_ptr<MainApp>(new MainApp);
}


MainApp& MainApp::app()
{
  assert(mainapp_singleton != NULL);
  return *(mainapp_singleton.get());
}


MainApp::MainApp()
  : threadTunnel(NULL),
    hintID(-100000) // set to a large negative number to avoid collision with gameID
{
  experience = experience_ptr(new Experience);

  // initialize the two AI players

  for (int c=0;c<2;c++)
    {
      TranspositionTable* tt = new TranspositionTable(TRANSPOSITION_TABLE_SIZE);
      ttable[c] = ttable_ptr(tt);

      PlayerIF_AlgoAB* algo = new PlayerIF_AlgoAB();
      player_computer[c] = player_ptr(algo);

      algo->registerExperience(experience);
    }

  setShareTTables(true);

  // initialize hint algorithm

  {
    PlayerIF_AlgoAB* hintAlgo = new PlayerIF_AlgoAB;
    hint_computer = player_ptr(hintAlgo);
    hint_computer->setRuleSpec( control.getRuleSpec() );
    hint_ttable = ttable_ptr(new TranspositionTable(TRANSPOSITION_TABLE_SIZE-1));
    hintAlgo->registerTTable(hint_ttable);
    hintAlgo->registerExperience(experience);
  }

  // set the default players

  control.registerPlayerIF(PL_White, player_ptr(new PlayerIF_Human));
  control.registerPlayerIF(PL_Black, player_computer[1]);

  control.resetGame();
}


void MainApp::init()
{
  control.getSignal_changeState().connect(boost::bind(&MainApp::setStatusbarText, this));
  control.getSignal_gameOver()   .connect(boost::bind(&MainApp::setStatusbarText, this));
  control.getSignal_startMove()  .connect(boost::bind(&MainApp::setStatusbarText, this));

  control.getSignal_startMove().connect(boost::bind(&MainApp::startMove, this, _1));
  control.getSignal_endMove()  .connect(boost::bind(&MainApp::endMove,   this, _1));
  control.getSignal_gameOver() .connect(boost::bind(&ApplicationGUI::showGameOverDialog, gui_application, _1));

  setStatusbarText();
}


void MainApp::setThreadTunnel(ThreadTunnel& tt)
{
  threadTunnel=&tt;

  for (int i=0;i<2;i++)
    player_computer[i]->registerThreadTunnel(tt);

  hint_computer->registerThreadTunnel(*threadTunnel);
}


void MainApp::setRules(rulespec_ptr rules, bool significantChange)
{
  getBoardGUI()->removeHint();
  hint_computer->cancelMove();
  hint_computer->setRuleSpec( rules );

  control.registerRuleSpec(rules); 
  experience->reset();

  if (significantChange)
    { control.resetGame(); }

  getBoardGUI()->resetDisplay();

  nextMove_butPauseIfAIPlayer();
}


void MainApp::setPlayer(Player pl, player_ptr player)
{
  control.registerPlayerIF(pl, player);

  if (control.getGameState().state == GameState::Idle &&
      pl == control.getCurrentPlayer())
    {
      nextMove_butPauseIfAIPlayer();
    }
}


void MainApp::resetGame()
{
  getBoardGUI()->removeHint();
  control.resetGame();
  hint_computer->resetGame();
  getBoardGUI()->redrawBoard();

  nextMove_butPauseIfAIPlayer();
}

void MainApp::undo()
{
  getBoardGUI()->removeHint();
  control.undoMove();
  getBoardGUI()->redrawBoard();

  nextMove_butPauseIfAIPlayer();
}

void MainApp::redo()
{
  getBoardGUI()->removeHint();
  control.redoMove();
  getBoardGUI()->redrawBoard();

  nextMove_butPauseIfAIPlayer();
}

void MainApp::startNextMove()
{
  assert(appState.gamePaused==false);


  /* NOTE: it is essential that the new move is started in an idle function
     at a later time, because this allows the initiator of the current move
     to clean up after the doMove() function returns. This cannot always be
     done beforehand, because the doMove() routine may also indicate that
     the move was not complete yet.
  */

  class IdleFunc_StartNextMove : public IdleFunc
  {
  public:
    void operator()() { MainApp::app().getControl().startNextMove(); }
  };

  IdleFunc::install(new IdleFunc_StartNextMove);
}


void MainApp::visualizeMove(const Move& move, int gameID)
{
  // Only carry out the move, if we are still in the same game.
  // (The user could have started a new game, while the thread was still computing on the old one.)
  if (gameID == control.getCurrentMoveID() &&
      control.getGameState().state == GameState::Moving)
    {
      getBoardGUI()->visualizeMove(move, gameID);
    }
  else if (gameID == hintID &&
	   hint_gameID == control.getCurrentMoveID())
    {
      getBoardGUI()->showHint(move);
    }
  else
    {
      // std::cout << "did not consider move, because it was too old...\n";
    }
}


int MainApp::doMove(Move m)
{
  // TODO: here, we could add more things, like logging

  getBoardGUI()->removeHint();
  int nTakes = control.doMove(m);

  if (nTakes==0)
    {
      if (control.getGameState().state != GameState::Ended)
	{
	  if (options.alwaysPauseOnAIPlayer)
	    nextMove_butPauseIfAIPlayer();
	  else
	    startNextMove();
	}
    }

  // TODO: it is not quite logical to do this here
  getBoardGUI()->checkHover();

  return nTakes;
}

void MainApp::computeHint()
{
  hintID++;

  hint_gameID = control.getCurrentMoveID();
  hint_computer->startMove(control.getCurrentBoard_noTemporary(), hintID);
}


// ------------------------- pause -------------------------

void MainApp::setPause(bool flag)
{
  bool changed = (appState.gamePaused!=flag);

  appState.gamePaused=flag;

  if (changed) m_signal_pauseChanged(appState.gamePaused);
}

void MainApp::continueGame()
{
  assert(appState.gamePaused);
  assert(control.getGameState().state != GameState::Ended);

  setPause(false);
  startNextMove();
}

void MainApp::nextMove_butPauseIfAIPlayer()
{
  bool ended = (control.getGameState().state == GameState::Ended);

  bool nonInteractive = (control.getCurrentPlayerInterface()->isInteractivePlayer() == false);
  setPause(nonInteractive && !ended);

  if (appState.gamePaused==false && !ended) // && control.getGameState().state==GameState::Idle)
    {
      assert(control.getGameState().state==GameState::Idle);

      startNextMove();
    }
}

void MainApp::setShareTTables(bool share)
{
  share_TT = share;

  for (int c=0;c<2;c++)
    {
      PlayerIF_AlgoAB* algo = dynamic_cast<PlayerIF_AlgoAB*>(player_computer[c].get());
      algo->registerTTable(share_TT ? ttable[0] : ttable[c]);
    }
}


void MainApp::setThinkingInfo(const std::string& thinking)
{
  setStatusbarText_withThinking(thinking);
}

void MainApp::setStatusbarText()
{
  setStatusbarText_withThinking("");
}

void MainApp::setStatusbarText_withThinking(const std::string& thinkingSuffix)
{
  GameState state = control.getGameState();

  switch (state.state)
    {
    case GameState::Ended:
      if (state.SUBSTATE_Winner==PL_White)
	{
	  gui_application->setStatusbar(_("Game over: white has won!"));
	}
      else if (state.SUBSTATE_Winner==PL_Black)
	{
	  gui_application->setStatusbar(_("Game over: black has won!"));
	}
      else
	{
	  gui_application->setStatusbar(_("Game ended in a tie between both players."));
	}
      break;
	
      // TODO: this has to be handled in another way
    case GameState::Idle:
      gui_application->setStatusbar(_("Game paused. Click continue to proceed."));
      break;

    case GameState::Moving:
	{
	  bool set  = state.SUBSTATE_PlayerSet;
	  bool move = state.SUBSTATE_PlayerMove;

	  std::string str;
	  if (control.getCurrentPlayer()==PL_White)
	    { str = _("White player"); }
	  else
	    { str = _("Black player"); }

	  str += ": ";

	  if (control.getCurrentPlayerInterface()->isInteractivePlayer())
	    {
	      if (set && move) { str += _("set or move a piece."); }
	      else if (set)    { str += _("set a piece."); }
	      else if (move)   { str += _("move a piece."); }
	    }
	  else
	    {
	      str += _("computer is taking his turn...   ");
	      str += thinkingSuffix;
	    }

	  gui_application->setStatusbar(str.c_str());
	}
      break;
    }
}


void MainApp::startMove(player_ptr p)
{
  if (!p->isInteractivePlayer()) gui_application->getBoardGUI()->startNonInteractiveMove();
}

void MainApp::endMove(player_ptr p)
{
  if (!p->isInteractivePlayer()) gui_application->getBoardGUI()->endNonInteractiveMove();
}

