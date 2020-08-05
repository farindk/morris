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

#ifndef BOARDSPEC_HH
#define BOARDSPEC_HH

#include "constants.hh"
#include "util.hh"
#include <boost/shared_ptr.hpp>
#include <vector>
#include <string>


typedef SmallVec<Position,MAXNEIGHBORS> NeighborVector;
typedef SmallVec<Position,MAXMILLSIZE>  MillPosVector;

typedef boost::shared_ptr<class BoardSpec> boardspec_ptr;


/* The BoardSpec specifies the board geometry.
   This includes the graphical layout as well as logical topology.
   The base class comprises a complete list of board presets, which
   may be implemented in different derived classes. The main purpose
   of using a hierarchy of classes here is code reuse.
 */
class BoardSpec
{
public:
  virtual ~BoardSpec() { }

  // The number of positions on this board.
  virtual int                   nPositions() const = 0;

  // All neighbors to position p.
  virtual const NeighborVector& getNeighbors(Position p) const = 0;

  // The human-readable name of the position.
  virtual std::string           getPositionName(Position) const = 0;

  /* The geometrical location of the position. This assumes that the
     board spans the area [0;1]x[0;1]. */
  virtual Point2D               getPositionLocation(Position) const = 0;

  // The radius of a piece in the same units as for the board position.
  virtual float                 getPieceRadius() const = 0;

  // The total number of mill configurations on this board.
  virtual int                   nMills() const = 0;

  // The positions for mill 'i'.
  virtual const MillPosVector&  getMill(int i) const = 0;

  // All the mills through a fixed position.
  virtual const std::vector<MillPosVector>& getMillsThroughPos(Position) const = 0;

  /* Return a set of position-permutation vectors for this board configuration.
     Permuting the board positions according to these permutations results
     in symmetric situations.
  */
  typedef SmallVec<Position, MAXPOSITIONS> Permutation;
  const std::vector<Permutation>& getPermutations() const { return m_permutations; }


  enum BoardPreset
    {
      Board_Standard9MM,
      Board_Moebius,
      Board_Morabaraba,
      Board_Windmill,
      Board_Sunmill,
      Board_6MM,
      Board_7MM,
      Board_SmallSq,
      Board_SmallSqWithDiag,
      Board_SmallTri,
      Board_Polygon3,
      Board_Polygon5,
      Board_Polygon6,
      Board_Unknown
    };

  // The ID of this board configuration.
  virtual BoardPreset getBoardPresetID() const { return Board_Unknown; }


  /* A board factory that can create any of the preset board configurations.
   */
  static boardspec_ptr boardFactory(BoardPreset);

protected:
  /* This initializes the set of permutations for symmetric boards.
     Call this method once in the constructor.
  */
  void initPermutations();

private:
  typedef bool UsageVector[MAXPOSITIONS];
  void recursePermutation(Permutation&, UsageVector& used, int pos);

  std::vector<Permutation> m_permutations;
};


/* An implementation of the board-specification interface which makes it
   easy to generate polygonal boards (three rings of n-polygons).
   It is used currently for the triangular, pentagonal, and hexagonal boards.
 */
class BoardSpec_Polygon : public BoardSpec
{
public:
  virtual ~BoardSpec_Polygon() { }

  BoardSpec_Polygon(int nCorners);

  // --- standard interface ---

  virtual int                   nPositions() const;
  virtual const NeighborVector& getNeighbors(Position) const;
  virtual std::string           getPositionName(Position) const;
  virtual Point2D               getPositionLocation(Position) const;
  virtual float                 getPieceRadius() const;

  virtual int                   nMills() const;
  virtual const MillPosVector&  getMill(int i) const;
  virtual const std::vector<MillPosVector>& getMillsThroughPos(Position p) const { return millsAtPos[p]; }

  virtual BoardPreset getBoardPresetID() const;

  // --- class specific methods ---

  // (none)

private:
  int m_nCorners;

  std::vector<NeighborVector> neighbors;
  std::vector<MillPosVector>  mills;
  std::vector<std::vector<MillPosVector> > millsAtPos;
  std::vector<Point2D> positions;
};


/* An implementation of the board-specification interface which arranges the
   positions on a regular grid (however also with some positions remaining empty).
   The neighborhood relationsships and mill structure is defined via tables.
 */
class BoardSpec_Grid : public BoardSpec
{
public:
  virtual ~BoardSpec_Grid() { }

  BoardSpec_Grid(BoardPreset);

  // --- standard interface ---

  virtual int                   nPositions() const;
  virtual const NeighborVector& getNeighbors(Position) const;
  virtual std::string           getPositionName(Position) const;
  virtual Point2D               getPositionLocation(Position) const;
  virtual float                 getPieceRadius() const;

  virtual int                   nMills() const;
  virtual const MillPosVector&  getMill(int i) const;
  virtual const std::vector<MillPosVector>& getMillsThroughPos(Position p) const { return millsAtPos[p]; }

  virtual BoardPreset getBoardPresetID() const;

  // --- class specific methods ---

  /* A vector with the used geometric X- and Y-coordinates of all the positions.
     This can be used for displaying a coordinate frame around the board. */
  std::vector<float>   xCoords() const;
  std::vector<float>   yCoords() const;

private:
  const struct boardTable* spec;

  std::vector<NeighborVector> neighbors;
  std::vector<MillPosVector>  mills;
  std::vector<std::vector<MillPosVector> > millsAtPos;
};

#endif
