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

#ifndef BOARDGUI_HH
#define BOARDGUI_HH

#include <gtk/gtk.h>

#include "control.hh"
#include "board.hh"
#include "player.hh"


enum MouseButton
  {
    MB_Left,
    MB_Middle,
    MB_Right
  };

enum GfxState
  {
    GS_Inactive,            // no game in progress
    GS_NonInteractiveMove,  // AI is computing its move
    GS_Waiting,        // interactive move: waiting for move input
    GS_DraggingPiece,  // interactive move: dragging of piece in progress
    GS_Take            // interactive move: taking of piece to progres
  };

enum HoverMode
  {
    Hover_None,
    Hover_Set,
    Hover_MoveStart,
    Hover_MoveEnd,
    Hover_Take
  };

struct HoverState
{
  HoverState();

  HoverMode hoverMode;
  Position  hoverPos;
  bool      hoverPosValid;
};


/* This is an abstraction layer that helps implement the interactive board GUI.
   This layer implements the user interaction process, independent of the actual
   graphics output. This class has to be derived and the missing methods be
   filled in to obtain a fully functional implementation.
 */
class BoardGUI_Base
{
public:
  BoardGUI_Base();
  virtual ~BoardGUI_Base() { }

  // recompute everything (e.g. layout for a changed board and new piece sizes)
  virtual void resetDisplay() { }

  // redraw everything
  virtual void redrawBoard() = 0;

  void startInteractiveMove();
  void cancelInteractiveMove();

  void startNonInteractiveMove();
  void endNonInteractiveMove();

  virtual void visualizeMove(const Move& move, int gameID);
  virtual void checkHover() { }

  // hints

  virtual void showHint(const Move&) { }
  virtual void removeHint() { }

  // preferences
  virtual void preferencesDialog_Display() { }

protected:
  GfxState          getGfxState() const { return m_gfxState; }
  const HoverState& getHoverState() const { return m_hoverState; }
  const Move&       getPlayerMove() const { return m_playermove; }

  virtual void startPieceDrag(Position p, Point2D pixelPos) { }
  virtual void stopPieceDrag() { }
  virtual void doPieceDrag(Point2D pixelPos, bool valid) { }

  virtual void invalidateHoverAtPos(Position) { }
  virtual void changeGfxState(GfxState) { }

  // callback that you have to connect to

  void cb_buttonPress  (Position p, Point2D pixelPos, MouseButton b);
  void cb_buttonRelease(Position p, MouseButton b);
  void cb_mouseMotion  (Position p, Point2D pixelPos);

private:
  void setState(GfxState s);
  void setHover(HoverMode mode, Position=-1, bool valid=true);
  void doMove();

  GfxState   m_gfxState;
  HoverState m_hoverState;

  Move m_playermove;  // the successively built, interactive player move
};

typedef boost::shared_ptr<BoardGUI_Base> boardgui_ptr;




/* The human player object. This closely interacts with the board-GUI.
   In fact, it is very simple and simply switches the interaction mode
   on and off.
 */
class PlayerIF_Human : public PlayerIF
{
public:
  PlayerIF_Human() { }

  bool isInteractivePlayer() const { return true; }

  void startMove(const Board& current, int moveID);
  void forceMove() { }
  void cancelMove();
};


#endif
