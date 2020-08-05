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

#include "config.h"
#include "control.hh"
#include "gtkcairo_boardgui.hh"
#include "mainapp.hh"

#include <assert.h>
#include <cmath>
#include <iostream>

#include <cairo/cairo.h>

extern gint animation_timer(gpointer obj);


void BoardGUI_GtkCairo::expose_area_rect(int x0,int y0,int w,int h)
{
  if (offscreenBuffer==NULL)
    { return; }

  if (boardGfxInvalid)
    {
      draw_board(x0,y0,w,h);
      boardGfxInvalid=false;
    }

  gdk_draw_drawable( drawingArea->window,
		     drawingArea->style->fg_gc[GTK_WIDGET_STATE(drawingArea)], 
		     offscreenBuffer,
		     x0,y0,
		     x0,y0,
		     w,h );
}



void BoardGUI_GtkCairo::redrawBoard()
{
  boardGfxInvalid=true;

  gdk_window_invalidate_rect(drawingArea->window, NULL, false);
}


// this func is used when an area of the window is uncovered
void BoardGUI_GtkCairo::cb_expose_area(GtkWidget *widget, GdkEventExpose *event)
{
  expose_area_rect(event->area.x, event->area.y, 
		   event->area.width, event->area.height);
}


// this func is used when an area of the window is reconfigured
void BoardGUI_GtkCairo::cb_configure_area(GtkWidget *widget, GdkEventConfigure *event)
{
  cb_realize_area(widget);
}


void BoardGUI_GtkCairo::cb_realize_area(GtkWidget *widget)
{
  if (offscreenBuffer)
    { gdk_pixmap_unref(offscreenBuffer); }
  
  offscreenBuffer = gdk_pixmap_new(drawingArea->window,
				   drawingArea->allocation.width,
				   drawingArea->allocation.height, -1);

  resetDisplay();

  draw_board(0,0, drawingArea->allocation.width, drawingArea->allocation.height);
}


void BoardGUI_GtkCairo::resetDisplay()
{
  geom.fitInto(drawingArea->allocation.width,
	       drawingArea->allocation.height,
	       options);

  changeGfxState(getGfxState());
  redrawBoard();
}


void BoardGUI_GtkCairo::invalidatePiecePos(Point2D pos_inUnits, float extraRadius)
{
  GdkRectangle rect;
  Point2D pos = geom.board2Pixel(pos_inUnits);
  float radius= geom.getPieceRadiusInPixels() + extraRadius*geom.scale_units2pixels;

  rect.x = pos.x-radius -2;
  rect.y = pos.y-radius -2;
  rect.width  = 2*radius+5;
  rect.height = 2*radius+5;

  /*
  std::cout << "invalidate "
	    << rect.x << ";" << rect.y << ";"
	    << rect.width << ";" << rect.height << "\n";
  */

  gdk_window_invalidate_rect(drawingArea->window, &rect, false);
  boardGfxInvalid=true;
}


void BoardGUI_GtkCairo::invalidateHoverAtPos(Position p)
{
  GameControl& control = MainApp::app().getControl();
  boardspec_ptr board = control.getBoardSpec();

  Point2D pt = board->getPositionLocation(p);
  invalidatePiecePos(pt, 1.0/7/2);
}


void BoardGUI_GtkCairo::showHint(const Move& hint)
{
  m_hint = hint;
  m_showHint = true;

  redrawBoard();
}


void BoardGUI_GtkCairo::removeHint()
{
  m_showHint = false;

  redrawBoard();
}

// ===== drawing the board ====

int Animation::elapsedTimeMS() const
{
  struct timeval tv;
  gettimeofday(&tv,NULL);

  int diffMS = (tv.tv_sec-startTime.tv_sec)*1000 + (tv.tv_usec-startTime.tv_usec)/1000;
  return diffMS;
}

bool Animation_RemovePiece::action(BoardGUI_GtkCairo& gui)
{
  if (elapsedTimeMS() >= durationMS)
    {
      gui.hidePos.push_back(pos);
      gui.boardGfxInvalid=true;
      gdk_window_invalidate_rect(gui.drawingArea->window, NULL, false);
      return true;
    }
  else
    return false;
}

bool Animation_RemoveStack::action(BoardGUI_GtkCairo& gui)
{
  const int piecesOnStack = MainApp::app().getControl().getCurrentBoard().getNPiecesToSet(player);

  Point2D piecePos = gui.geom.getStackPiecePos(player, piecesOnStack-1);

  gui.invalidatePiecePos( piecePos );
  gui.removeStackPiece = player;

  return true;
}

bool Animation_MoveOverlay::action(BoardGUI_GtkCairo& gui)
{
  if (overlayIdx<0)
    {
      overlayIdx = gui.overlay.size();

      BoardGUI_GtkCairo::OverlayPiece o;
      o.player = player;
      gui.overlay.push_back(o);
    }
  else
    {
      gui.invalidatePiecePos(gui.overlay[overlayIdx].coordinate);
    }

  int elapsed = elapsedTimeMS();
  if (elapsed>=durationMS)
    {
      gui.overlay[overlayIdx].coordinate = endPos;

      gui.invalidatePiecePos(endPos);
      gui.boardGfxInvalid=true;

      return true;
    }
  else
    {
      const float f = float(elapsed)/durationMS;

      Point2D c;
      c.x = (1.0-f)*startPos.x + f*endPos.x;
      c.y = (1.0-f)*startPos.y + f*endPos.y;
      gui.overlay[overlayIdx].coordinate = c;

      gui.invalidatePiecePos(c);
      gui.boardGfxInvalid=true;

      return false;
    }
}

bool Animation_DoMove::action(BoardGUI_GtkCairo& gui)
{
  /* We are in a g_idle callback, which does not lock.
     This will cause problems in recursive main-loops when, e.g.,
     gtk_dialog_run() is called from within doMove().

     gtk_dialog_run() will first call unlock() on a mutex, which is not locked,
     and at the end again call lock(). This will cause the main-loop to lock,
     because it assumes that the thread will come back in the unlocked state
     in which it left the main-loop.
  */
  gdk_threads_enter();

  gui.overlay.clear();
  gui.hidePos.clear();
  gui.removeStackPiece = PL_None;

  GameControl& control = MainApp::app().getControl();

  if (gameID == control.getCurrentMoveID() &&
      control.getGameState().state == GameState::Moving)
    {
      MainApp::app().doMove(move);
    }
  else
    {
      // If the move turned out to come too late (did not match the moveID),
      // the gfx-state at the end of the animation does not match the board state.
      // Hence, we have to redraw the board.
      gui.redrawBoard();
    }

  gdk_threads_leave();

  return true;
}

gint BoardGUI_GtkCairo::cb_animation_timer()
{
  while (!animation.empty())
    {
      bool stop = animation[0]->action(*this);
      if (stop)
	{
	  for (int i=1;i<animation.size();i++)
	    animation[i-1] = animation[i];

	  animation.pop_back();
	}
      else
	{ break; }
    }

  return !animation.empty();
}

void BoardGUI_GtkCairo::visualizeMove(const Move& m, int gameID)
{
  const bool set_moveIn   = options.set_moveIn;
  const bool take_moveOut = options.take_moveOut;
  const int  take_delay   = options.take_delay;
  const int  move_unitTime = options.animationSpeed; // 1sec for moving across whole board

  const GameControl& control = MainApp::app().getControl();
  boardspec_ptr board = control.getBoardSpec();


  if (options.animateComputerMoves)
    {
      if (m.mode == Move::Mode_Move)
	{
	  Animation_RemovePiece* anim0 = new Animation_RemovePiece;
	  anim0->pos = m.oldPos;
	  anim0->durationMS = 0;
	  animation.push_back( animation_ptr(anim0) );


	  Animation_MoveOverlay* anim = new Animation_MoveOverlay;
	  anim->player = control.getCurrentBoard().getPosition(m.oldPos);
	  anim->startPos = board->getPositionLocation(m.oldPos);
	  anim->endPos   = board->getPositionLocation(m.newPos);

	  float dist = hypot(anim->startPos.x-anim->endPos.x,
			     anim->startPos.y-anim->endPos.y);

	  anim->durationMS = dist*move_unitTime;
	  animation.push_back( animation_ptr(anim) );
	}
      else if (m.mode == Move::Mode_Set)
	{
	  if (set_moveIn)
	    {
	      Animation_RemoveStack* anim0 = new Animation_RemoveStack;
	      anim0->player = control.getCurrentPlayer();
	      anim0->durationMS = 0;
	      animation.push_back( animation_ptr(anim0) );
	    }

	  Animation_MoveOverlay* anim = new Animation_MoveOverlay;
	  anim->player = control.getCurrentPlayer();
	  anim->startPos = geom.getStackPiecePos(anim->player,
						 control.getCurrentBoard().getNPiecesToSet(anim->player)-1);
	  anim->endPos = board->getPositionLocation(m.newPos);

	  float dist = hypot(anim->startPos.x-anim->endPos.x,
			     anim->startPos.y-anim->endPos.y);

	  if (set_moveIn) anim->durationMS = dist*move_unitTime;
	  else            anim->durationMS = 0;

	  animation.push_back( animation_ptr(anim) );
	}

      for (int i=0;i<m.takes.size();i++)
	{
	  Animation_RemovePiece* anim0 = new Animation_RemovePiece;
	  anim0->pos = m.takes[i];
	  anim0->durationMS = take_delay;
	  animation.push_back( animation_ptr(anim0) );

	  if (take_moveOut)
	    {
	      Animation_MoveOverlay* anim = new Animation_MoveOverlay;
	      anim->player = control.getCurrentBoard().getPosition(m.takes[i]);
	      anim->startPos = board->getPositionLocation(m.takes[i]);

	      anim->endPos.x = ((anim->player==PL_White) ?
				geom.windowWidth_inUnits + geom.piece_radius_inUnits :
				-geom.piece_radius_inUnits);
	      anim->endPos.y = geom.windowHeight_inUnits/2;

	      anim->endPos.x -= geom.xoffset_board_inUnits;
	      anim->endPos.y -= geom.yoffset_board_inUnits;

	      float dist = hypot(anim->startPos.x-anim->endPos.x,
				 anim->startPos.y-anim->endPos.y);

	      anim->durationMS = dist*move_unitTime;
	      animation.push_back( animation_ptr(anim) );
	    }
	}
    }

  {
    Animation_DoMove* anim = new Animation_DoMove;
    anim->move = m;
    anim->gameID = gameID;
    anim->durationMS = 0;
    animation.push_back( animation_ptr(anim) );
  }


  // set start times

  struct timeval startTime;
  gettimeofday(&startTime,NULL);

  for (int i=0;i<animation.size();i++)
    {
      animation[i]->startTime = startTime;

      startTime.tv_usec += animation[i]->durationMS*1000;

      while (startTime.tv_usec > 1000000)
	{
	  startTime.tv_usec -= 1000000;
	  startTime.tv_sec++;
	}
    }


  // start animation

  g_timeout_add(20 /* ms */, animation_timer, this);
}

void BoardGUI_GtkCairo::checkHover()
{
  // emit a dummy mouse-motion event

  gint mx,my;
  gdk_window_get_pointer(drawingArea->window,&mx,&my,NULL);

  Position pos = geom.pixel2Pos(mx,my);
  cb_mouseMotion( pos, Point2D(mx,my) );
}

void BoardGUI_GtkCairo::draw_piece(cairo_t* cr, Point2D pos_inUnits, Player player, bool transparent)
{
  draw_piece_inPixels(cr, geom.board2Pixel(pos_inUnits), player, transparent);
}

void BoardGUI_GtkCairo::draw_piece_inPixels(cairo_t* cr, Point2D pos_inPixels, Player player, bool transparent)
{
  float col_main = 0.0;
  float col_med  = 0.5;
  float col_high = 1.0;

  float radius = geom.getPieceRadiusInPixels();

  if (player==PL_White) { std::swap(col_main,col_high); }

  cairo_save(cr);
  cairo_translate(cr, pos_inPixels.x, pos_inPixels.y);

  const float alpha = (transparent ? 0.5 : 1.0);

  cairo_set_source_rgba(cr, col_main,col_main,col_main, alpha);
  cairo_arc (cr, 0,0, radius, 0., 2 * M_PI);
  cairo_fill(cr);

  cairo_set_line_width (cr, 0.75);
  cairo_set_source_rgba(cr, col_high,col_high,col_high, alpha);
  cairo_arc (cr, 0,0, radius, 0., 2 * M_PI);
  cairo_stroke (cr);
  cairo_set_source_rgba(cr, col_med,col_med,col_med, alpha);
  cairo_arc (cr, 0,0, 0.8*radius, 0., 2 * M_PI);
  cairo_stroke (cr);
  cairo_arc (cr, 0,0, 0.37*radius, 0., 2 * M_PI);
  cairo_stroke (cr);

  cairo_restore(cr);
}


Point2D BoardGUI_GtkCairo::Geometry::getStackPiecePos(Player p, int n) const
{
  int maxPieces = MainApp::app().getControl().getRuleSpec()->nPieces;

  Point2D pt;

  float ySpacing = 0.8/(maxPieces-1);
  if (ySpacing>1.5*piece_radius_inUnits)
    {
      ySpacing = 1.5*piece_radius_inUnits;
      pt.y = 0.5 -((maxPieces-1)/2.0-n)*ySpacing;
    }
  else
    {
      pt.y = ySpacing*n;
      pt.y += 0.1;
    }

  if (p==PL_White) pt.x = stackX_white_inUnits;
  else             pt.x = stackX_black_inUnits;

  return pt;
}


BoardGUI_GtkCairo::Options::Options()
{
  coloredCrossingWhileDragging = true;
  animateComputerMoves = true;
  animationSpeed = 1000; // 1sec
  set_moveIn = true;
  take_moveOut = true;
  take_delay = 250;
  showCoordinates = true;
}


void BoardGUI_GtkCairo::drawRect(cairo_t* cr, Point2D tl, Point2D br, bool fill)
{
  tl = geom.board2Pixel(tl);
  br = geom.board2Pixel(br);

  cairo_move_to (cr, tl.x, tl.y);
  cairo_line_to (cr, tl.x, br.y);
  cairo_line_to (cr, br.x, br.y);
  cairo_line_to (cr, br.x, tl.y);
  cairo_line_to (cr, tl.x, tl.y);

  if (fill) cairo_fill(cr);
  else      cairo_stroke (cr);
}

void BoardGUI_GtkCairo::drawCoordinates(cairo_t* cr)
{
  GameControl& control = MainApp::app().getControl();
  boardspec_ptr board = control.getBoardSpec();
  BoardSpec_Grid* boardGrid = dynamic_cast<BoardSpec_Grid*>(board.get());

  {
    const float bkggrey=0.7;

    cairo_set_line_width (cr, 1);

    Point2D tl;
    tl.x = -geom.boardBorderLeft_inUnits;
    tl.y = -geom.boardBorderTop_inUnits;

    Point2D br;
    br.x = 1+ geom.boardBorderRight_inUnits;
    br.y = 1+ geom.boardBorderBottom_inUnits;

    cairo_set_source_rgb (cr, 0.9, 0.9, 0.9);
    drawRect(cr,tl,br,true);

    tl.x = 0;
    tl.y = 0;
    br.x = tl.x+1;
    br.y = tl.y+1;

    cairo_set_source_rgb (cr, bkggrey+0.05, bkggrey+0.05, bkggrey+0.05);
    drawRect(cr,tl,br,true);
  }

  if (boardGrid)
    {
      cairo_text_extents_t extents;

      cairo_select_font_face (cr, "Sans",
			      CAIRO_FONT_SLANT_NORMAL,
			      CAIRO_FONT_WEIGHT_NORMAL);
    
      cairo_set_font_size (cr, geom.scale_units2pixels*geom.textSize_inUnits);

      cairo_set_source_rgb (cr, 0, 0, 0);

      double maxUp=0;
      double maxDown=0;

      std::vector<float> coords = boardGrid->xCoords();
      for (int i=0; i<coords.size(); i++)
	{
	  char coord[2];
	  coord[0]='a'+i;
	  coord[1]=0;

	  cairo_text_extents (cr, coord, &extents);
	  maxUp   = std::max(maxUp,  -extents.y_bearing);
	  maxDown = std::max(maxDown, extents.height+extents.y_bearing);
	}

      for (int b=0; b<2; b++)
	for (int i=0; i<coords.size(); i++)
	  {
	    char coord[2];
	    coord[0]='a'+i;
	    coord[1]=0;

	    cairo_text_extents (cr, coord, &extents);

	    Point2D p;
	    p.x = coords[i];
	    if (b==0) p.y = 1.0+0*geom.boardBorderBottom_inUnits/2;
	    else      p.y = 0.0-0*geom.boardBorderTop_inUnits/2;
	    p = geom.board2Pixel(p);

	    int borderHeight = geom.boardBorderBottom_inUnits * geom.scale_units2pixels;
	    int offset = (borderHeight-maxUp-maxDown)/2;

	    p.x -= extents.width/2 + extents.x_bearing;
	    if (b==0) p.y += maxUp+offset;
	    else      p.y -= offset+maxDown;

	    cairo_move_to (cr, p.x, p.y);
	    cairo_show_text (cr, coord);
	  }

      coords = boardGrid->yCoords();
      for (int b=0; b<2; b++)
	for (int i=0; i<coords.size(); i++)
	  {
	    char coord[2];
	    coord[0]='1'+i;
	    coord[1]=0;

	    cairo_text_extents (cr, coord, &extents);

	    Point2D p;
	    if (b==0) p.x = 1.0+geom.boardBorderRight_inUnits/2;
	    else      p.x = -geom.boardBorderLeft_inUnits/2;
	    p.y = coords[i];
	    p = geom.board2Pixel(p);
	   
	    p.x -= extents.width/2 + extents.x_bearing;
	    p.y -= extents.height/2+ extents.y_bearing;

	    cairo_move_to (cr, p.x, p.y);
	    cairo_show_text (cr, coord);
	  }
    }
}

// draw board with vector-graphics
void BoardGUI_GtkCairo::draw_board_complete()
{
  gint width,height;
  gdk_drawable_get_size(offscreenBuffer, &width,&height);

  draw_board(0,0,width,height);
}


void BoardGUI_GtkCairo::draw_board(int x0,int y0,int w,int h)
{
  GameControl& control = MainApp::app().getControl();

  const HoverState& hoverState = getHoverState();

  cairo_t* cr = gdk_cairo_create(offscreenBuffer);

  //std::cout << "redraw(" << x0 << "," << y0 << ", " << w << "," << h << ")\n";


  // set clip-path to expose-rectangle

  GdkRectangle rect;
  rect.x = x0;
  rect.y = y0;
  rect.width = w;
  rect.height = h;

  gdk_cairo_rectangle(cr,&rect);
  cairo_clip(cr);

  // draw background

  const float bkggrey=0.7;

  {
    gint width,height;
    gdk_drawable_get_size(offscreenBuffer, &width,&height);
    cairo_set_source_rgb(cr, bkggrey,bkggrey,bkggrey);
    cairo_rectangle(cr,x0,y0,w,h);
    cairo_fill(cr);
  }

  boardspec_ptr board = control.getBoardSpec();
  const float pieceSize = geom.getPieceRadiusInPixels();

  // draw coordinate-grid

  if (options.showCoordinates)
    {
      drawCoordinates(cr);
    }

  // draw board

  for (int i=0;i<board->nPositions(); i++)
    {
      const NeighborVector& neighbors = board->getNeighbors(i);
      for (int n=0;n<neighbors.size();n++)
	{
	  Position p = neighbors[n];
	  if (p>i) { continue; }

	  Point2D p1 = board->getPositionLocation(i);
	  Point2D p2 = board->getPositionLocation(p);

	  p1 = geom.board2Pixel(p1);
	  p2 = geom.board2Pixel(p2);

	  cairo_set_source_rgb (cr, 0, 0, 0);
	  cairo_set_line_width (cr, geom.scale_units2pixels*1/64);
	  cairo_move_to (cr, p1.x, p1.y);
	  cairo_line_to (cr, p2.x, p2.y);
	  cairo_stroke (cr);
	}
    }

  // draw crossings

  for (int i=0;i<board->nPositions(); i++)
    {
      Point2D p = board->getPositionLocation(i);
      p = geom.board2Pixel(p);

      cairo_set_source_rgb (cr, 0, 0, 0);
      cairo_arc (cr, p.x,p.y, geom.scale_units2pixels*1/64, 0., 2 * M_PI);
      cairo_fill(cr);


      if (getGfxState()==GS_DraggingPiece && options.coloredCrossingWhileDragging)
	{
	  // colored crossings

	  Move m = getPlayerMove();
	  m.newPos = i;
	  if (control.getRuleSpec()->isValidMove( control.getCurrentBoard(), m))
	    { cairo_set_source_rgb (cr, 0, 1, 0); }
	  else
	    { cairo_set_source_rgb (cr, 1, 0, 0); }
	}
      else
	{ cairo_set_source_rgb (cr, 1, 1, 1); }

      cairo_arc (cr, p.x,p.y, geom.scale_units2pixels*5/(7*64), 0., 2 * M_PI);
      cairo_fill(cr);
    }


  // draw pieces stacks

  int maxPieces = control.getRuleSpec()->nPieces;
  int nPieces;
  const Board& b = control.getCurrentBoard();
  nPieces = b.getNPiecesToSet(PL_White);
  if (removeStackPiece==PL_White) nPieces--;
  for (int i=0;i<nPieces;i++)
    {
      Point2D pt = geom.getStackPiecePos(PL_White, i);
      draw_piece(cr, pt, PL_White);
    }

  nPieces = b.getNPiecesToSet(PL_Black);
  if (removeStackPiece==PL_Black) nPieces--;
  for (int i=0;i<nPieces;i++)
    {
      Point2D pt = geom.getStackPiecePos(PL_Black, i);
      draw_piece(cr, pt, PL_Black);
    }


  // draw pieces

  Player pl;

  for (int p=0;p<board->nPositions();p++)
    if ((pl = control.getCurrentBoard().getPosition(p)) != PL_None)
      {
	bool hide=false;
	for (int i=0;!hide && i<hidePos.size();i++)
	  if (hidePos[i] == p)
	    hide=true;

	if (hide) continue;

	Point2D pt = board->getPositionLocation(p);
	draw_piece(cr, pt, pl);
      }

  // draw overlays

  for (int i=0;i<overlay.size();i++)
    {
      Point2D pt = overlay[i].coordinate;
      draw_piece(cr, pt, overlay[i].player);
    }

  // hover set

  switch (hoverState.hoverMode)
    {
    case Hover_Set:
      {
	Point2D pt = board->getPositionLocation(hoverState.hoverPos);
	pt=geom.board2Pixel(pt);
      
	cairo_save(cr);
	cairo_translate(cr, pt.x,pt.y);

	cairo_set_source_rgb(cr, 0.0, 0.75, 0.0);
	cairo_set_line_width (cr, 3);
	cairo_arc (cr, 0,0, pieceSize, 0., 2 * M_PI);
	cairo_stroke(cr);

	cairo_restore(cr);
      }
      break;

      // hover move

    case Hover_MoveStart:
      {
	Point2D pt = board->getPositionLocation(hoverState.hoverPos);
	pt=geom.board2Pixel(pt);
      
	cairo_save(cr);
	cairo_translate(cr, pt.x,pt.y);

	cairo_set_source_rgb (cr, 0.0, 0.8, 0.0);
	cairo_set_line_width (cr, 5.0);
	cairo_arc (cr, 0,0, pieceSize, 0., 2 * M_PI);
	cairo_stroke(cr);

	cairo_restore(cr);
      }
      break;

      // hover dest

    case Hover_MoveEnd:
      {
	Point2D pt = board->getPositionLocation(hoverState.hoverPos);
	pt=geom.board2Pixel(pt);
      
	cairo_save(cr);
	cairo_translate(cr, pt.x,pt.y);

	if (hoverState.hoverPosValid)
	  cairo_set_source_rgb (cr, 0.0, 0.8, 0.0);
	else
	  cairo_set_source_rgb (cr, 0.8, 0.0, 0.0);

	cairo_set_line_width (cr, 5.0);
	cairo_arc (cr, 0,0, pieceSize, 0., 2 * M_PI);
	cairo_stroke(cr);

	cairo_restore(cr);
      }
      break;

      // hover take

    case Hover_Take:
      {
	Point2D pt = board->getPositionLocation(hoverState.hoverPos);
	pt=geom.board2Pixel(pt);
      
	cairo_save(cr);
	cairo_translate(cr, pt.x,pt.y);

	if (hoverState.hoverPosValid)
	  cairo_set_source_rgb (cr, 0.0, 0.8, 0.0);
	else
	  cairo_set_source_rgb (cr, 0.8, 0.0, 0.0);

	cairo_set_line_width (cr, 5.0);

	const float crossSize = pieceSize*0.8;
	cairo_move_to (cr, -crossSize, -crossSize);
	cairo_line_to (cr,  crossSize,  crossSize);
	cairo_move_to (cr,  crossSize, -crossSize);
	cairo_line_to (cr, -crossSize,  crossSize);

	cairo_stroke(cr);

	cairo_restore(cr);
      }
      break;
    }


  // show hint

  if (m_showHint)
    {
      Player pl = control.getCurrentPlayer();

      //if (m_hint.mode == Move::Mode_Set)
      {
	Point2D pt = board->getPositionLocation(m_hint.newPos);
	draw_piece(cr, pt, pl, true);
      }

      if (m_hint.mode == Move::Mode_Move)
	{
	  Point2D p1 = board->getPositionLocation(m_hint.oldPos);
	  Point2D p2 = board->getPositionLocation(m_hint.newPos);
	  p1=geom.board2Pixel(p1);
	  p2=geom.board2Pixel(p2);
      
	  const float width  = pieceSize*0.1;
	  const float arrowW = 3*width;
	  const float arrowL = 6*width;

	  const float len = hypot(p1.x-p2.x, p1.y-p2.y);
	  const float angle = atan2(p2.y-p1.y, p2.x-p1.x);

	  cairo_save(cr);
	  cairo_set_line_width (cr, 1.0);
	  cairo_translate(cr, p1.x,p1.y);
	  cairo_rotate(cr, angle);

	  cairo_set_source_rgba(cr, 0.8, 0.6, 0.2, 1.0);

	  cairo_move_to (cr,   0,       -width);
	  cairo_line_to (cr, len-arrowL,-width);
	  cairo_line_to (cr, len-arrowL,-arrowW);
	  cairo_line_to (cr, len,       0);
	  cairo_line_to (cr, len-arrowL, arrowW);
	  cairo_line_to (cr, len-arrowL, width);
	  cairo_line_to (cr,   0,        width);

	  cairo_fill(cr);

	  cairo_restore(cr);
	}

      for (int i=0;i<m_hint.takes.size();i++)
	{
	  Point2D pt = board->getPositionLocation(m_hint.takes[i]);
	  pt=geom.board2Pixel(pt);

	  cairo_save(cr);
	  cairo_translate(cr, pt.x,pt.y);

	  cairo_set_source_rgba(cr, 0.0, 0.8, 0.0, 0.75);
	  cairo_set_line_width (cr, 5.0);

	  const float crossSize = pieceSize*0.8;
	  cairo_move_to (cr, -crossSize, -crossSize);
	  cairo_line_to (cr,  crossSize,  crossSize);
	  cairo_move_to (cr,  crossSize, -crossSize);
	  cairo_line_to (cr, -crossSize,  crossSize);

	  cairo_stroke(cr);
	  cairo_restore(cr);
	}
    }

  // draw dragged piece

  if (drag_active)
    {
      draw_piece_inPixels(cr, Point2D(drag_position.x + drag_offset_x,
				      drag_position.y + drag_offset_y), drag_player);
    }

  cairo_destroy(cr);
}


void BoardGUI_GtkCairo::startPieceDrag(Position p, Point2D pixelPos)
{
  GameControl& control = MainApp::app().getControl();
  const Board& board = control.getCurrentBoard();

  OverlayPiece o;
  o.player = board.getCurrentPlayer();
  o.coordinate = control.getBoardSpec()->getPositionLocation(p);

  Point2D piecePos_inPixels = geom.board2Pixel(o.coordinate);

  drag_active = true;
  drag_player = board.getCurrentPlayer();
  drag_offset_x = (piecePos_inPixels.x - pixelPos.x);
  drag_offset_y = (piecePos_inPixels.y - pixelPos.y);
  drag_position = pixelPos;
  hidePos.push_back(p);

  if (options.coloredCrossingWhileDragging) redrawBoard();
}

void BoardGUI_GtkCairo::stopPieceDrag()
{
  drag_active = false;
  hidePos.clear();
}

void BoardGUI_GtkCairo::doPieceDrag(Point2D pixelPos, bool valid)
{
  Point2D oldpos = drag_position;
  oldpos.x += drag_offset_x;
  oldpos.y += drag_offset_y;

  invalidatePiecePos(geom.pixel2Board(oldpos));

  drag_position = pixelPos;

  boardGfxInvalid=true;

  Point2D newpos = pixelPos;
  newpos.x += drag_offset_x;
  newpos.y += drag_offset_y;
  invalidatePiecePos(geom.pixel2Board(newpos));
}


BoardGUI_GtkCairo::Geometry::Geometry()
{
  textSize_inUnits = 0.25/7;
}

void BoardGUI_GtkCairo::Geometry::fitInto(int w,int h, const BoardGUI_GtkCairo::Options& options)
{
  piece_radius_inUnits = MainApp::app().getControl().getBoardSpec()->getPieceRadius();

  float coordBorder = 0;
  if (options.showCoordinates) coordBorder = piece_radius_inUnits*0.8;
 
  boardBorderLeft_inUnits   = coordBorder;
  boardBorderRight_inUnits  = coordBorder;
  boardBorderTop_inUnits    = coordBorder;
  boardBorderBottom_inUnits = coordBorder;

  float borderLR = boardBorderLeft_inUnits + boardBorderRight_inUnits;
  float borderTB = boardBorderTop_inUnits + boardBorderBottom_inUnits;

  width_inUnits =(7+1.5+1.5)/7 + borderLR;
  height_inUnits=1.0 + borderTB;

  float scaleH = w/width_inUnits;
  float scaleV = h/height_inUnits;
  scale_units2pixels = std::min(scaleH,scaleV);

  windowWidth_inUnits  = w/scale_units2pixels;
  windowHeight_inUnits = h/scale_units2pixels;

  float boardWidth_inPixels  = (1.0+borderLR)*scale_units2pixels;
  float boardHeight_inPixels = (1.0+borderTB)*scale_units2pixels;
  xoffset_board_inUnits = (w-boardWidth_inPixels )/ scale_units2pixels/2 + boardBorderLeft_inUnits;
  yoffset_board_inUnits = (h-boardHeight_inPixels)/ scale_units2pixels/2 + boardBorderTop_inUnits;

  stackX_white_inUnits = (   -0.75/7) - boardBorderLeft_inUnits;
  stackX_black_inUnits = (1.0+0.75/7) + boardBorderRight_inUnits;
}

Point2D  BoardGUI_GtkCairo::Geometry::board2Pixel(Point2D boardCoord) const
{
  Point2D p;
  p.x = (boardCoord.x + xoffset_board_inUnits) * scale_units2pixels;
  p.y = (boardCoord.y + yoffset_board_inUnits) * scale_units2pixels;
  return p;
}

Point2D  BoardGUI_GtkCairo::Geometry::pixel2Board(Point2D pixelCoord) const
{
  Point2D p;
  p.x = pixelCoord.x / scale_units2pixels - xoffset_board_inUnits;
  p.y = pixelCoord.y / scale_units2pixels - yoffset_board_inUnits;
  return p;
}

Position BoardGUI_GtkCairo::Geometry::pixel2Pos(int mx,int my) const
{
  const float distThreshold = 0.055;

  float x = mx/scale_units2pixels - xoffset_board_inUnits;
  float y = my/scale_units2pixels - yoffset_board_inUnits;

  boardspec_ptr spec = MainApp::app().getControl().getBoardSpec();
  float dist=100;
  Position pos;
  for (int i=0;i<spec->nPositions();i++)
    {
      Point2D p = spec->getPositionLocation(i);
      float d = hypot(p.x-x, p.y-y);
      if (d<dist) { dist=d; pos=i; }
    }

  if (dist>distThreshold) { return -1; }

  return pos;
}


static MouseButton convertGtkMouseButton(guint button)
{
  switch (button)
    {
    case 1: return MB_Left;
    case 2: return MB_Middle;
    case 3: return MB_Right;
    }

  assert(false);
}

gint button_press_area(GtkWidget *widget, GdkEventButton *event, gpointer obj)
{
  BoardGUI_GtkCairo* self = (BoardGUI_GtkCairo*)(obj);

  Position p = self->geom.pixel2Pos(event->x, event->y);
  self->cb_buttonPress( p, Point2D(event->x, event->y), convertGtkMouseButton(event->button) );
  return FALSE;
}

gint button_release_area(GtkWidget *widget, GdkEventButton *event, gpointer obj)
{
  BoardGUI_GtkCairo* self = (BoardGUI_GtkCairo*)(obj);

  Position p = self->geom.pixel2Pos(event->x, event->y);
  self->cb_buttonRelease( p, convertGtkMouseButton(event->button) );
  return FALSE;
}

gint mouse_motion_area(GtkWidget *widget, GdkEventButton *event, gpointer obj)
{
  BoardGUI_GtkCairo* self = (BoardGUI_GtkCairo*)(obj);

  gint mx,my;
  gdk_window_get_pointer(self->drawingArea->window,&mx,&my,NULL);

  Position pos = self->geom.pixel2Pos(mx,my);
  self->cb_mouseMotion( pos, Point2D(mx,my) );

  return FALSE;
}

gint expose_area(GtkWidget *widget, GdkEventExpose *event, gpointer obj)
{
  BoardGUI_GtkCairo* self = (BoardGUI_GtkCairo*)(obj);
  self->cb_expose_area(widget, event);

  return FALSE; 
}


gint configure_area(GtkWidget *widget, GdkEventConfigure *event, gpointer obj)
{
  BoardGUI_GtkCairo* self = (BoardGUI_GtkCairo*)(obj);
  self->cb_configure_area(widget,event);

  return TRUE;
}

gint realize_area(GtkWidget *widget, gpointer obj)
{
  BoardGUI_GtkCairo* self = (BoardGUI_GtkCairo*)(obj);
  self->cb_realize_area(widget);

  return TRUE;
}

gint animation_timer(gpointer obj)
{
  BoardGUI_GtkCairo* self = (BoardGUI_GtkCairo*)(obj);
  return self->cb_animation_timer();
}



BoardGUI_GtkCairo::BoardGUI_GtkCairo()
  : drawingArea(NULL),
    offscreenBuffer(NULL),
    boardGfxInvalid(true),
    removeStackPiece(PL_None),
    drag_active(false),
    m_showHint(false)
{
  drawingArea = gtk_drawing_area_new();
  gtk_drawing_area_size(GTK_DRAWING_AREA(drawingArea),
			10*64, 7*64);

  gtk_widget_set_events(drawingArea,
			GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK |
			GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK |
			GDK_BUTTON_RELEASE_MASK);

  g_signal_connect(GTK_OBJECT(drawingArea),"expose_event",    G_CALLBACK(expose_area),    this);
  g_signal_connect(GTK_OBJECT(drawingArea),"configure_event", G_CALLBACK(configure_area), this);
  g_signal_connect_after(GTK_OBJECT(drawingArea),"realize",G_CALLBACK(realize_area),this);

  g_signal_connect(GTK_OBJECT(drawingArea),"button_press_event",   G_CALLBACK(button_press_area), this);
  g_signal_connect(GTK_OBJECT(drawingArea),"button_release_event", G_CALLBACK(button_release_area),this);
  g_signal_connect(GTK_OBJECT(drawingArea),"motion_notify_event",  G_CALLBACK(mouse_motion_area), this);

  ////boardpic  = gdk_pixbuf_new_from_file(GTK_ICONDIR"/morris/board.png", NULL); // TODO: proper error handling
  ////piecespic = gdk_pixbuf_new_from_file(GTK_ICONDIR"/morris/pieces.png",NULL); // TODO: proper error handling
}


BoardGUI_GtkCairo::~BoardGUI_GtkCairo()
{
  if (offscreenBuffer)
    { gdk_pixmap_unref(offscreenBuffer); offscreenBuffer=NULL; }
}


void BoardGUI_GtkCairo::changeGfxState(GfxState gfx)
{
  if (!drawingArea->window) return;

  GdkCursor* cursor = NULL;

  switch (gfx)
    {
    case GS_Inactive:           cursor = NULL; break;
    case GS_NonInteractiveMove: cursor = gdk_cursor_new(GDK_WATCH);    break;
    case GS_Waiting:            cursor = gdk_cursor_new(GDK_TCROSS);   break;
    case GS_DraggingPiece:      cursor = gdk_cursor_new(GDK_CIRCLE);   break;
    case GS_Take:               cursor = gdk_cursor_new(GDK_X_CURSOR); break;
    }

  gdk_window_set_cursor(drawingArea->window, cursor);
  if (cursor) gdk_cursor_destroy(cursor);
}
