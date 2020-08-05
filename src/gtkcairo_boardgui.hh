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

#ifndef GTKCAIRO_BOARDGUI_HH
#define GTKCAIRO_BOARDGUI_HH

#include <gtk/gtk.h>
#include "board.hh"
#include "boardgui.hh"
#include <sys/time.h>


struct Animation
{
  virtual ~Animation() { }
  virtual bool action(class BoardGUI_GtkCairo&) = 0; // return true when effect ended

  int elapsedTimeMS() const;

  struct timeval startTime;
  int durationMS;
};

struct Animation_RemovePiece : public Animation
{
  virtual bool action(class BoardGUI_GtkCairo&);

  Position pos;
};

struct Animation_RemoveStack : public Animation
{
  virtual bool action(class BoardGUI_GtkCairo&);

  Player player;
};

struct Animation_MoveOverlay : public Animation
{
  Animation_MoveOverlay() : overlayIdx(-1) { }

  virtual bool action(class BoardGUI_GtkCairo&);

  Player  player;
  Point2D startPos;
  Point2D endPos;

private:
  int     overlayIdx;
};

// Carry out the move and remove the overlays and hidden pieces.
struct Animation_DoMove : public Animation
{
  virtual bool action(class BoardGUI_GtkCairo&);

  Move move;
  int  gameID;
};

typedef boost::shared_ptr<Animation> animation_ptr;


class BoardGUI_GtkCairo : public BoardGUI_Base
{
public:
  BoardGUI_GtkCairo();
  ~BoardGUI_GtkCairo();

  void resetDisplay();

  void redrawBoard();
  void visualizeMove(const Move&, int gameID);
  void checkHover();

  void showHint(const Move&);
  void removeHint();

  GtkWidget* getBoardWidget() { return drawingArea; }

  struct Options
  {
    Options();

    bool  coloredCrossingWhileDragging;
    bool  animateComputerMoves;
    float animationSpeed;  // time in [ms] for moving across whole board
    bool  set_moveIn;
    bool  take_moveOut;
    float take_delay;
    bool  showCoordinates;
  };

  Options& getOptions() { return options; }

private:
  GtkWidget* drawingArea;
  GdkPixmap* offscreenBuffer;
  bool       boardGfxInvalid;

  struct OverlayPiece
  {
    Player   player;
    Point2D  coordinate;
  };

  SmallVec<OverlayPiece,Move::MAXTAKES+1> overlay;
  SmallVec<Position,Move::MAXTAKES+1> hidePos;
  Player removeStackPiece;

  std::vector<animation_ptr> animation;

  void draw_board_complete();
  void draw_board(int x0,int y0,int w,int h);
  void expose_area_rect(int x0,int y0,int w,int h);

  void invalidatePiecePos(Point2D pos_inUnits, float extraRadius=0);
  virtual void invalidateHoverAtPos(Position);
  virtual void changeGfxState(GfxState);

  void draw_piece         (cairo_t* cr, Point2D pos_inUnits,  Player player, bool halfTransparent=false);
  void draw_piece_inPixels(cairo_t* cr, Point2D pos_inPixels, Player player, bool halfTransparent=false);

  // geometric layout

  struct Geometry
  {
    Geometry();

    void     fitInto(int w,int h, const Options&);
    Point2D  board2Pixel(Point2D boardCoord) const;
    Point2D  pixel2Board(Point2D boardCoord) const;
    Point2D  getStackPiecePos(Player, int n) const;
    float    getPieceRadiusInPixels() const { return piece_radius_inUnits*scale_units2pixels; }
    Position pixel2Pos(int mx,int my) const;

    float width_inUnits;
    float height_inUnits;

    float windowWidth_inUnits;
    float windowHeight_inUnits;

    float scale_units2pixels;

    float xoffset_board_inUnits;
    float yoffset_board_inUnits;

    float boardBorderLeft_inUnits;
    float boardBorderRight_inUnits;
    float boardBorderTop_inUnits;
    float boardBorderBottom_inUnits;

    float stackX_white_inUnits;
    float stackX_black_inUnits;

    float piece_radius_inUnits;
    float textSize_inUnits;
  } geom;

  // display options

  Options options;

  // manual piece dragging

  virtual void startPieceDrag(Position p, Point2D pixelPos);
  virtual void stopPieceDrag();
  virtual void doPieceDrag(Point2D pixelPos, bool valid);

  bool    drag_active;
  Player  drag_player;
  Point2D drag_position;
  float   drag_offset_x;
  float   drag_offset_y;

  // hint display

  Move m_hint;
  bool m_showHint;

  // utility functions

  void drawRect(cairo_t*, Point2D topleft, Point2D bottomright, bool fill);
  void drawCoordinates(cairo_t*);

  // additional Gtk callbacks

  void cb_expose_area(GtkWidget *widget, GdkEventExpose *event);
  void cb_configure_area(GtkWidget *widget, GdkEventConfigure *event);
  void cb_realize_area(GtkWidget *widget);
  gint cb_animation_timer();

  // kicker functions

  friend gint button_press_area(GtkWidget *widget, GdkEventButton *event, gpointer obj);
  friend gint button_release_area(GtkWidget *widget, GdkEventButton *event, gpointer obj);
  friend gint mouse_motion_area(GtkWidget *widget, GdkEventButton *event, gpointer obj);

  friend gint expose_area(GtkWidget *widget, GdkEventExpose *event, gpointer obj);
  friend gint configure_area(GtkWidget *widget, GdkEventConfigure *event, gpointer obj);
  friend gint realize_area(GtkWidget *widget, gpointer obj);
  friend gint animation_timer(gpointer obj);

  // animation classes are friends

  friend class Animation_RemovePiece;
  friend class Animation_RemoveStack;
  friend class Animation_MoveOverlay;
  friend class Animation_DoMove;
};

#endif
