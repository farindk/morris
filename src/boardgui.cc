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

#include "boardgui.hh"
#include "mainapp.hh"

#include <iostream>


// this is usually overridden by the derived class
void BoardGUI_Base::visualizeMove(const Move& move, int gameID)
{
  GameControl& control = MainApp::app().getControl();
  if (gameID == control.getCurrentMoveID() &&
      control.getGameState().state == GameState::Moving)
    {
      MainApp::app().doMove(move);
    }

  redrawBoard();
}


static std::ostream& operator<<(std::ostream& ostr, GfxState s)
{
  switch (s)
    {
    case GS_Waiting: ostr << "GS_Waiting"; break;
    case GS_Inactive: ostr << "GS_Inactive"; break;
    case GS_DraggingPiece: ostr << "GS_DraggingPiece"; break;
    case GS_Take: ostr << "GS_Take"; break;
    case GS_NonInteractiveMove: ostr << "GS_NonInteractiveMove"; break;
    default: ostr << "GS_UNKNOWN"; break;
    }
  return ostr;
}


HoverState::HoverState()
  : hoverPosValid(false),
    hoverMode(Hover_None),
    hoverPos(-1)
{
}


BoardGUI_Base::BoardGUI_Base()
  : m_gfxState(GS_Inactive)
{
}


void BoardGUI_Base::setState(GfxState s)
{
  m_gfxState=s;
  changeGfxState(m_gfxState);
}

void BoardGUI_Base::startInteractiveMove()
{
  assert(m_gfxState==GS_Inactive);
  setState(GS_Waiting);
}

void BoardGUI_Base::cancelInteractiveMove()
{
  setState(GS_Inactive);
}

void BoardGUI_Base::startNonInteractiveMove()
{
  assert(m_gfxState==GS_Inactive);
  setState(GS_NonInteractiveMove);
}

void BoardGUI_Base::endNonInteractiveMove()
{
  assert(m_gfxState==GS_NonInteractiveMove);
  setState(GS_Inactive);
}


void BoardGUI_Base::doMove()
{
  int nTakes = MainApp::app().doMove(m_playermove);
  if (nTakes>0)
    {
      char buf[100];
      sprintf(buf,_("You make take %d opponent piece(s)."),nTakes);
      MainApp::app().getApplicationGUI()->setStatusbar(buf);

      setState(GS_Take);
    }
  else
    {
      setState(GS_Inactive);

      m_playermove.reset();
    }

  redrawBoard();
}


void BoardGUI_Base::cb_buttonPress(Position p, Point2D pixelPos, MouseButton button)
{
  GameControl& control = MainApp::app().getControl();

  // if the left mouse button is pressed
  if (button == MB_Left)
    {
      if (p==-1)
	{ return; }

      switch (m_gfxState)
	{
	case GS_Waiting:
	  if (control.getCurrentBoard().getPosition(p) == PL_None &&
	      control.getGameState().SUBSTATE_PlayerSet)
	    {
	      m_playermove.setMove_Set(p);
	      setHover(Hover_None);
	      doMove();
	    }
	  else if (control.getCurrentBoard().getPosition(p) == control.getCurrentPlayer() &&
		   control.getGameState().SUBSTATE_PlayerMove)
	    {
	      m_playermove.setMove_Move(p,-1);
	      setHover(Hover_None);
	      m_gfxState = GS_DraggingPiece;

	      changeGfxState(m_gfxState);
	      startPieceDrag(p,pixelPos);
	    }
	  break;

	case GS_Take:
	  if (control.getCurrentBoard().isOpponent(p))
	    {
	      bool mayTake = control.getRuleSpec()->mayTake( control.getCurrentBoard(), p );

	      if (mayTake)
		{
		  m_playermove.addTake(p);
		  setHover(Hover_None);
		  doMove();
		}
	    }
	  break;

	case GS_Inactive:
	  break;
	}
    }
}


void BoardGUI_Base::cb_buttonRelease(Position p, MouseButton button)
{
  const Board&    board =  MainApp::app().getControl().getCurrentBoard();
  const RuleSpec& rules = *MainApp::app().getControl().getRuleSpec();

  // if the left mouse button is pressed
  if (button == MB_Left)
  {
    if (m_gfxState==GS_DraggingPiece)
      {
	setHover(Hover_None);
	m_playermove.newPos = p;

	stopPieceDrag();

	if (p == -1 || rules.isValidMove(board, m_playermove)==false)
	  {
	    // release at invalid position

	    m_gfxState=GS_Waiting;
	    changeGfxState(m_gfxState);
	    redrawBoard();
	  }
	else
	  {
	    // check if position is a valid move

	    if (rules.isValidMove(board, m_playermove))
	      {
		// release at valid position

		doMove();
	      }
	  }
      }
  }
}


void BoardGUI_Base::cb_mouseMotion(Position pos, Point2D pixelPos)
{
  const GameControl& control = MainApp::app().getControl();
  const Board& board    =  MainApp::app().getControl().getCurrentBoard();
  const RuleSpec& rules = *MainApp::app().getControl().getRuleSpec();

  HoverMode hoverMode = Hover_None;
  bool      hoverValid;

  switch (m_gfxState)
    {
    case GS_Waiting:
      if (pos != -1 &&
	  control.getGameState().SUBSTATE_PlayerSet &&
	  board.getPosition(pos) == PL_None)
	{
	  hoverMode = Hover_Set;
	}
      else if (pos != -1 &&
	       control.getGameState().SUBSTATE_PlayerMove &&
	       board.getPosition(pos) == board.getCurrentPlayer())
	{
	  hoverMode = Hover_MoveStart;
	}
      break;

    case GS_DraggingPiece:
      if (pos != -1 &&
	  control.getGameState().SUBSTATE_PlayerMove &&
	  board.getPosition(pos) == PL_None)
	{
	  // check if this is really a valid move
	      
	  m_playermove.newPos = pos;
	      
	  hoverMode  = Hover_MoveEnd;
	  hoverValid = rules.isValidMove(board, m_playermove);

	  doPieceDrag(pixelPos, hoverValid);
	}
      else
	{ doPieceDrag(pixelPos, false); }
      break;

    case GS_Take:
      if (pos != -1 &&
	  board.getPosition(pos) == opponent(board.getCurrentPlayer()))
	{
	  // check for opponent mills
	      
	  hoverMode = Hover_Take;
	  hoverValid = rules.mayTake( board, pos );
	}
      break;
    }

  setHover(hoverMode, pos, hoverValid);
}



void BoardGUI_Base::setHover(HoverMode mode, Position pos, bool valid)
{
  const HoverMode oldHoverMode = m_hoverState.hoverMode;
  const Position  oldHoverPos  = m_hoverState.hoverPos;

  m_hoverState.hoverMode = mode;
  m_hoverState.hoverPos  = pos;
  m_hoverState.hoverPosValid = valid;

  if (m_hoverState.hoverMode != oldHoverMode ||
      m_hoverState.hoverPos  != oldHoverPos)
    {
      if (oldHoverPos>=0) invalidateHoverAtPos(oldHoverPos);
      if (pos>=0)         invalidateHoverAtPos(pos);
    }
}


// ---------------------------------------------------------------------------

void PlayerIF_Human::startMove(const Board& current, int moveID)
{
  MainApp::app().getBoardGUI()->startInteractiveMove();
}

void PlayerIF_Human::cancelMove()
{
  MainApp::app().getBoardGUI()->cancelInteractiveMove();
}
