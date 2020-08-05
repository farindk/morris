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

#include "boardspec.hh"
#include <cmath>


BoardSpec_Polygon::BoardSpec_Polygon(int nCorners)
{
  m_nCorners = nCorners;

  assert(nCorners >= 3);
  assert(nCorners*2*3 <= MAXPOSITIONS);

  // --- generate positions ---

  double angleStep = 2*M_PI/nCorners;
  double angle = +M_PI/2 -angleStep/2;

  float maxX=0, minX=0;
  float maxY=0, minY=0;

  const float ringSpacing[] = { 0,0,0,
				/* 3 */ 2.25,
				/* 4 */ 1.57,
				/* 5 */ 1.35,
				/* 6 */ 1.30 };

  // compute bounding box

  for (int c=0;c<nCorners;c++)
    {
      float x = cos(angle);
      float y = sin(angle);
      angle += angleStep;

      if (x>maxX) { maxX=x; }
      if (x<minX) { minX=x; }

      if (y>maxY) { maxY=y; }
      if (y<minY) { minY=y; }
    }


  // compute positions

  float dist = (1.0-2*getPieceRadius()*ringSpacing[nCorners]) / std::max(maxX-minX, maxY-minY) ;
  minY *= dist;
  maxY *= dist;
  float shiftY = -(maxY+minY)/2 + 0.5;

  for (int ring=0;ring<3;ring++)
    {
      for (int c=0;c<nCorners;c++)
	{
	  const float d=dist-ring*2*getPieceRadius()*ringSpacing[nCorners];

	  float x = cos(angle)*d + 0.5;
	  float y = sin(angle)*d + shiftY;

	  positions.push_back( Point2D(x,y) );

	  float x2 = cos(angle+angleStep)*d + 0.5;
	  float y2 = sin(angle+angleStep)*d + shiftY;

	  positions.push_back( Point2D((x+x2)/2,(y+y2)/2) );

	  angle +=angleStep;
	}
    }


  // generate neighbors

  int p=0;
  for (int ring=0;ring<3;ring++)
    {
      for (int c=0;c<2*nCorners;c++)
	{
	  NeighborVector v;
	  v.push_back( (p+1)%(2*nCorners) + ring*2*nCorners);
	  v.push_back( (p+2*nCorners-1)%(2*nCorners) + ring*2*nCorners);

	  if (p % 2 == 1)
	    {
	      if (ring != 2)
		{
		  v.push_back( p+2*nCorners );
		}
	      if (ring != 0)
		{
		  v.push_back( p-2*nCorners );
		}
	    }

	  neighbors.push_back(v);
	  p++;
	}
    }


  // --- generate mill tables ---

  // mills along polygons

  for (int ring=0;ring<3;ring++)
    {
      for (int c=0;c<nCorners;c++)
	{
	  MillPosVector v;

	  v.push_back( 2*c                +2*nCorners*ring);
	  v.push_back( 2*c+1              +2*nCorners*ring);
	  v.push_back((2*c+2)%(2*nCorners)+2*nCorners*ring);

	  mills.push_back(v);
	}
    }

  // radial mills

  for (int c=0;c<nCorners;c++)
    {
      MillPosVector v;

      for (int ring=0;ring<3;ring++)
	v.push_back(2*c+1  + ring*nCorners*2);

      mills.push_back(v);
    }


  // position-indexed mill table

  millsAtPos.resize(nPositions());

  for (int i=0;i<mills.size();i++)
    {
      // insert mill to position-indexed tables

      for (int n=0;n<3;n++)
	{
	  MillPosVector v2;

	  for (int k=0;k<3;k++)
	    if (k!=n)
	      {
		v2.push_back(mills[i][k]);
	      }

	  millsAtPos[mills[i][n]].push_back(v2);
	}
    }

  initPermutations();
}

int BoardSpec_Polygon::nPositions() const
{
  return m_nCorners*2*3;
}

float BoardSpec_Polygon::getPieceRadius() const
{
  const float sizes[] = { 0,0,0, 0.04, 0.06, 0.05, 0.045 };

  return sizes[m_nCorners];
}

const NeighborVector& BoardSpec_Polygon::getNeighbors(Position p) const
{
  return neighbors[p];
}

Point2D BoardSpec_Polygon::getPositionLocation(Position pos) const
{
  return positions[pos];
}

std::string BoardSpec_Polygon::getPositionName(Position p) const
{
  std::string name;
  name += (p%(2*m_nCorners))+'a';
  name += (p/(2*m_nCorners))+'1';

  return name;
}

int BoardSpec_Polygon::nMills() const
{
  return mills.size();
}

const MillPosVector&  BoardSpec_Polygon::getMill(int i) const
{
  return mills[i];
}

BoardSpec::BoardPreset BoardSpec_Polygon::getBoardPresetID() const
{
  switch (m_nCorners)
    {
    case 3:  return Board_Polygon3;
    case 5:  return Board_Polygon5;
    case 6:  return Board_Polygon6;
    default: return Board_Unknown;
    }
}

// ===========================================================================

static const short MM_9_neighbour[][MAXNEIGHBORS] =
  {
    {  1, 9,-1,-1 }, {  0, 2, 4,-1 }, {  1,14,-1,-1 },
    {  4,10,-1,-1 }, {  1, 3, 5, 7 }, {  4,13,-1,-1 },
    {  7,11,-1,-1 }, {  4, 6, 8,-1 }, {  7,12,-1,-1 },
    {  0,10,21,-1 }, {  3, 9,11,18 }, {  6,10,15,-1 },
    {  8,13,17,-1 }, {  5,12,14,20 }, {  2,13,23,-1 },
    { 11,16,-1,-1 }, { 15,17,19,-1 }, { 12,16,-1,-1 },
    { 10,19,-1,-1 }, { 16,18,20,22 }, { 13,19,-1,-1 },
    {  9,22,-1,-1 }, { 19,21,23,-1 }, { 14,22,-1,-1 }
  };

static const short MM_9_milltab_short[][MAXMILLSIZE] =
  {
    {  0, 1, 2 }, {  3, 4, 5 }, {  6, 7, 8 },
    {  9,10,11 }, { 12,13,14 }, { 15,16,17 },
    { 18,19,20 }, { 21,22,23 }, {  0, 9,21 },
    {  3,10,18 }, {  6,11,15 }, {  1, 4, 7 },
    { 16,19,22 }, {  8,12,17 }, {  5,13,20 },
    {  2,14,23 },
    { -1,-1,-1 }
  };

static const char MM_9_boardGeometry[][2] =
  {
    { 0,0 }, { 3,0 }, { 6,0 },
    { 1,1 }, { 3,1 }, { 5,1 },
    { 2,2 }, { 3,2 }, { 4,2 },
    { 0,3 }, { 1,3 }, { 2,3 },   { 4,3 }, { 5,3 }, { 6,3 },
    { 2,4 }, { 3,4 }, { 4,4 },
    { 1,5 }, { 3,5 }, { 5,5 },
    { 0,6 }, { 3,6 }, { 6,6 }
  };

static const short MM_9_coords[] = { 0,1,2,3,4,5,6, -1 };

struct boardTable
{
  BoardSpec::BoardPreset preset;

  int nPositions;
  int nMaxNeighbors;
  const short (*neighbors)[MAXNEIGHBORS];
  const short (*mills)[MAXMILLSIZE];
  const char (*geometry)[2];
  int geom_width, geom_height;  // if != 0, the positions are assumed to lie on a grid of this size
  const short *xcoords, *ycoords;
};

static const boardTable boardTable_Standard =
  {
    BoardSpec::Board_Standard9MM,
    24, 4,
    MM_9_neighbour,
    MM_9_milltab_short,
    MM_9_boardGeometry,
    7,7, MM_9_coords, MM_9_coords
  };

// ------------------------------------------------------------------------------

static const short MM_6_neighbour[][MAXNEIGHBORS] =
  {
    {  1, 6,-1,-1 }, {  0, 2, 4,-1 }, {  1, 9,-1,-1 },
    {  4, 7,-1,-1 }, {  1, 3, 5,-1 }, {  4, 8,-1,-1 },
    {  0, 7,13,-1 }, {  3, 6,10,-1 }, {  5, 9,12,-1 }, {  2, 8,15,-1 },
    {  7,11,-1,-1 }, { 10,12,14,-1 }, {  8,11,-1,-1 },
    {  6,14,-1,-1 }, { 11,13,15,-1 }, {  9,14,-1,-1 }
  };

static const short MM_6_milltab_short[][MAXMILLSIZE] =
  {
    {  0, 1, 2 }, {  3, 4, 5 },
    { 10,11,12 }, { 13,14,15 },
    {  0, 6,13 }, {  3, 7,10 },
    {  5, 8,12 }, {  2, 9,15 },
    { -1,-1,-1 }
  };

static const char MM_6_boardGeometry[][2] =
  {
    { 0,0 }, { 2,0 }, { 4,0 },
    { 1,1 }, { 2,1 }, { 3,1 },
    { 0,2 }, { 1,2 }, { 3,2 }, { 4,2 },
    { 1,3 }, { 2,3 }, { 3,3 },
    { 0,4 }, { 2,4 }, { 4,4 }
  };

static const short MM_6_coords[] = { 0,1,2,3,4, -1 };

static const boardTable boardTable_6MM =
  {
    BoardSpec::Board_6MM,
    16, 3,
    MM_6_neighbour,
    MM_6_milltab_short,
    MM_6_boardGeometry,
    5,5, MM_6_coords, MM_6_coords
  };

// ------------------------------------------------------------------------------

static const short MM_7_neighbour[][MAXNEIGHBORS] =
  {
    {  1, 6,-1,-1 }, {  0, 2, 4,-1 }, {  1,10,-1,-1 },
    {  4, 7,-1,-1 }, {  1, 3, 5, 8 }, {  4, 9,-1,-1 },
    {  0, 7,14,-1 }, {  3, 6,11, 8 }, {  4,7,9,12 }, {  5, 8,10,13 }, {  2, 9,16,-1 },
    {  7,12,-1,-1 }, {  8,11,13,15 }, {  9,12,-1,-1 },
    {  6,15,-1,-1 }, { 12,14,16,-1 }, { 10,15,-1,-1 }
  };

static const short MM_7_milltab_short[][MAXMILLSIZE] =
  {
    {  0, 1, 2 }, {  3, 4, 5 },
    { 11,12,13 }, { 14,15,16 },
    {  0, 6,14 }, {  3, 7,11 },
    {  5, 9,13 }, {  2,10,16 },
    { 1,4,8 }, { 4,8,12 }, { 8,12,15 },
    { 6,7,8 }, { 7,8,9 }, { 8,9,10 },
    { -1,-1,-1 }
  };

static const char MM_7_boardGeometry[][2] =
  {
    { 0,0 }, { 2,0 }, { 4,0 },
    { 1,1 }, { 2,1 }, { 3,1 },
    { 0,2 }, { 1,2 }, { 2,2 }, { 3,2 }, { 4,2 },
    { 1,3 }, { 2,3 }, { 3,3 },
    { 0,4 }, { 2,4 }, { 4,4 }
  };

static const short MM_7_coords[] = { 0,1,2,3,4, -1 };

static const boardTable boardTable_7MM =
  {
    BoardSpec::Board_7MM,
    17, 4,
    MM_7_neighbour,
    MM_7_milltab_short,
    MM_7_boardGeometry,
    5,5, MM_7_coords, MM_7_coords
  };

// ------------------------------------------------------------------------------

static const short MM_Morabaraba_neighbour[][MAXNEIGHBORS] =
  {
    {  1, 9, 3,-1 }, {  0, 2, 4,-1 }, {  1,14, 5,-1 },
    {  4,10, 0, 6 }, {  1, 3, 5, 7 }, {  4,13, 2, 8 },
    {  7,11, 3,-1 }, {  4, 6, 8,-1 }, {  7,12, 5,-1 },
    {  0,10,21,-1 }, {  3, 9,11,18 }, {  6,10,15,-1 },
    {  8,13,17,-1 }, {  5,12,14,20 }, {  2,13,23,-1 },
    { 11,16,18,-1 }, { 15,17,19,-1 }, { 12,16,20,-1 },
    { 10,19,15,21 }, { 16,18,20,22 }, { 13,19,17,23 },
    {  9,22,18,-1 }, { 19,21,23,-1 }, { 14,22,20,-1 }
  };

static const short MM_Morabaraba_milltab_short[][MAXMILLSIZE] =
  {
    {  0, 1, 2 }, {  3, 4, 5 }, {  6, 7, 8 },
    {  9,10,11 }, { 12,13,14 }, { 15,16,17 },
    { 18,19,20 }, { 21,22,23 }, {  0, 9,21 },
    {  3,10,18 }, {  6,11,15 }, {  1, 4, 7 },
    { 16,19,22 }, {  8,12,17 }, {  5,13,20 },
    {  2,14,23 },
    {  0, 3, 6 }, {  2, 5, 8 }, { 15,18,21 }, { 17,20,23 },
    { -1,-1,-1 }
  };

static const char MM_Morabaraba_boardGeometry[][2] =
  {
    { 0,0 }, { 3,0 }, { 6,0 },
    { 1,1 }, { 3,1 }, { 5,1 },
    { 2,2 }, { 3,2 }, { 4,2 },
    { 0,3 }, { 1,3 }, { 2,3 },   { 4,3 }, { 5,3 }, { 6,3 },
    { 2,4 }, { 3,4 }, { 4,4 },
    { 1,5 }, { 3,5 }, { 5,5 },
    { 0,6 }, { 3,6 }, { 6,6 }
  };

static const boardTable boardTable_Morabaraba =
  {
    BoardSpec::Board_Morabaraba,
    24, 4,
    MM_Morabaraba_neighbour,
    MM_Morabaraba_milltab_short,
    MM_Morabaraba_boardGeometry,
    7,7, MM_9_coords, MM_9_coords
  };

// ------------------------------------------------------------------------------

static const short MM_Moebius_neighbour[][MAXNEIGHBORS] =
  {
    {  1, 9,-1,-1 }, {  0, 2, 4,-1 }, {  1,12,-1,-1 },
    {  4,10,-1,-1 }, {  1, 3, 5, 7 }, {  4,13,-1,-1 },
    {  7,11,-1,-1 }, {  4, 6, 8,-1 }, {  7,14,-1,-1 },
    {  0,10,21,-1 }, {  3, 9,11,18 }, {  6,10,15,-1 },
    {  2,13,17,-1 }, {  5,12,14,20 }, {  8,13,23,-1 },
    { 11,16,-1,-1 }, { 15,17,19,-1 }, { 12,16,-1,-1 },
    { 10,19,-1,-1 }, { 16,18,20,22 }, { 13,19,-1,-1 },
    {  9,22,-1,-1 }, { 19,21,23,-1 }, { 14,22,-1,-1 }
  };

static const short MM_Moebius_milltab_short[][MAXMILLSIZE] =
  {
    {  0, 1, 2 }, {  3, 4, 5 }, {  6, 7, 8 },
    {  9,10,11 }, { 12,13,14 }, { 15,16,17 },
    { 18,19,20 }, { 21,22,23 }, {  0, 9,21 },
    {  3,10,18 }, {  6,11,15 }, {  1, 4, 7 },
    { 16,19,22 }, {  2,12,17 }, {  5,13,20 },
    {  8,14,23 },
    { -1,-1,-1 }
  };

static const char MM_Moebius_boardGeometry[][2] =
  {
    { 0,0 }, { 3,0 }, { 4,0 },
    { 1,1 }, { 3,1 }, { 5,1 },
    { 2,2 }, { 3,2 }, { 6,2 },
    { 0,3 }, { 1,3 }, { 2,3 },   { 4,3 }, { 5,3 }, { 6,3 },
    { 2,4 }, { 3,4 }, { 4,4 },
    { 1,5 }, { 3,5 }, { 5,5 },
    { 0,6 }, { 3,6 }, { 6,6 }
  };

static const boardTable boardTable_Moebius =
  {
    BoardSpec::Board_Moebius,
    24, 4,
    MM_Moebius_neighbour,
    MM_Moebius_milltab_short,
    MM_Moebius_boardGeometry,
    7,7, MM_9_coords, MM_9_coords
  };


// ------------------------------------------------------------------------------

static const short MM_Windmill_neighbour[][MAXNEIGHBORS] =
  {
    {  3,12,-1,-1 }, {  2, 4,-1,-1 }, {  1, 3, 5,-1 },
    {  0, 2, 6,11 }, {  1, 5, 7,15 }, {  2, 4, 6, 8 },
    {  3, 5,10,-1 }, {  4, 8,16,-1 }, {  5, 7, 9,-1 },
    {  8,13,14,-1 }, {  6,11,14,-1 }, {  3,10,12,20 },
    {  0,11,23,-1 }, {  9,17,18,-1 }, {  9,10,18,-1 },
    {  4,16,27,-1 }, {  7,15,17,24 }, { 13,16,21,-1 },
    { 13,14,19,-1 }, { 18,20,22,-1 }, { 11,19,23,-1 },
    { 17,22,24,-1 }, { 19,21,23,25 }, { 12,20,22,26 },
    { 16,21,25,27 }, { 22,24,26,-1 }, { 23,25,-1,-1 },
    { 15,24,-1,-1 }
  };

static const short MM_Windmill_milltab_short[][MAXMILLSIZE] =
  {
    // H
    {  1, 2, 3 }, {  4, 5, 6 }, {  7, 8, 9 },
    { 10,11,12 }, { 15,16,17 }, { 18,19,20 },
    { 21,22,23 }, { 24,25,26 },
    // V
    {  0,12,23 }, {  3,11,20 }, {  6,10,14 },
    { 19,22,25 }, {  2, 5, 8 }, { 13,17,21 },
    {  7,16,24 }, {  4,15,27 },
    // D
    {  1, 4, 7 }, {  0, 3, 6 },
    { 21,24,27 }, { 20,23,26 },
    { -1,-1,-1 }
  };

static const char MM_Windmill_boardGeometry[][2] =
  {
    { 7,0 }, { 0,1 }, { 3,1 }, { 6,1 }, { 1,2 },
    { 3,2 }, { 5,2 }, { 2,3 }, { 3,3 }, { 4,3 },
    { 5,3 }, { 6,3 }, { 7,3 }, { 3,4 }, { 5,4 },
    { 1,5 }, { 2,5 }, { 3,5 }, { 4,5 }, { 5,5 },
    { 6,5 }, { 3,6 }, { 5,6 }, { 7,6 }, { 2,7 },
    { 5,7 }, { 8,7 }, { 1,8 }
  };

static const short MM_coords9[] = { 0,1,2,3,4,5,6,7,8, -1 };

static const boardTable boardTable_Windmill =
  {
    BoardSpec::Board_Windmill,
    28, 4,
    MM_Windmill_neighbour,
    MM_Windmill_milltab_short,
    MM_Windmill_boardGeometry,
    9,9, MM_coords9, MM_coords9
  };


// ------------------------------------------------------------------------------

static const short MM_Sunmill_neighbour[][MAXNEIGHBORS] =
  {
    {  1, 3,-1,-1 }, {  0, 2, 4,-1 }, {  1, 5,-1,-1 },
    {  0, 4, 6,29 }, {  1, 3, 5, 7 }, {  2, 4,11,-1 },
    {  3, 7,31,-1 }, {  4, 6,14,-1 },
    {  9,11,-1,-1 }, {  8,10,12,-1 }, {  9,13,-1,-1 },
    {  8,12,14, 5 }, {  9,11,13,15 }, { 10,12,19,-1 },
    { 11,15, 7,-1 }, { 12,14,22,-1 },
    { 17,19,-1,-1 }, { 16,18,20,-1 }, { 17,21,-1,-1 },
    { 16,20,22,13 }, { 17,19,21,23 }, { 18,20,27,-1 },
    { 19,23,15,-1 }, { 20,22,30,-1 },
    { 25,27,-1,-1 }, { 24,26,28,-1 }, { 25,29,-1,-1 },
    { 24,28,30,21 }, { 25,27,29,31 }, { 26,28, 3,-1 },
    { 27,31,23,-1 }, { 28,30, 6,-1 }
  };

static const short MM_Sunmill_milltab_short[][MAXMILLSIZE] =
  {
    {  0, 1, 2 }, {  3, 4, 5 }, { 31, 6, 7 },
    {  0, 3, 6 }, {  1, 4, 7 }, {  2, 5,11 },

    {  8, 9,10 }, { 11,12,13 }, {  7,14,15 },
    {  8,11,14 }, {  9,12,15 }, { 10,13,19 },

    { 16,17,18 }, { 19,20,21 }, { 15,22,23 },
    { 16,19,22 }, { 17,20,23 }, { 18,21,27 },

    { 24,25,26 }, { 27,28,29 }, { 23,30,31 },
    { 24,27,30 }, { 25,28,31 }, { 26,29, 3 },

    { -1,-1,-1 }
  };

static const char MM_Sunmill_boardGeometry[][2] =
  {
    { 1,0 }, { 2,0 }, { 3,0 },
    { 2,1 }, { 3,1 }, { 4,1 },
    { 3,2 }, { 4,2 },
    { 6,1 }, { 6,2 }, { 6,3 },
    { 5,2 }, { 5,3 }, { 5,4 },
    { 4,3 }, { 4,4 },
    { 5,6 }, { 4,6 }, { 3,6 },
    { 4,5 }, { 3,5 }, { 2,5 },
    { 3,4 }, { 2,4 },
    { 0,5 }, { 0,4 }, { 0,3 },
    { 1,4 }, { 1,3 }, { 1,2 },
    { 2,3 }, { 2,2 }
  };

static const short MM_coords7[] = { 0,1,2,3,4,5,6, -1 };

static const boardTable boardTable_Sunmill =
  {
    BoardSpec::Board_Sunmill,
    32, 4,
    MM_Sunmill_neighbour,
    MM_Sunmill_milltab_short,
    MM_Sunmill_boardGeometry,
    7,7, MM_coords7, MM_coords7
  };


// ------------------------------------------------------------------------------

static const short MM_SmallSq_neighbour[][MAXNEIGHBORS] =
  {
    {  1, 3, -1 }, {  0, 2, 4, -1 }, {  1, 5,-1 },
    {  0, 4, 6,-1 }, {  1, 3, 5, 7, -1 }, {  2, 4, 8, -1 },
    {  3, 7,-1 }, {  4, 6, 8,-1 }, {  5, 7,-1 }
  };

static const short MM_SmallSq_milltab_short[][MAXMILLSIZE] =
  {
    { 0,1,2 }, { 3,4,5 }, { 6,7,8 },
    { 0,3,6 }, { 1,4,7 }, { 2,5,8 },
    { -1,-1,-1 }
  };

static const char MM_SmallSq_boardGeometry[][2] =
  {
    { 1,1 }, { 3,1 }, { 5,1 },
    { 1,3 }, { 3,3 }, { 5,3 },
    { 1,5 }, { 3,5 }, { 5,5 }
  };

static const short MM_SmallSq_coords[] = { 1,3,5, -1 };

static const boardTable boardTable_SmallSq =
  {
    BoardSpec::Board_SmallSq,
    9, 8,
    MM_SmallSq_neighbour,
    MM_SmallSq_milltab_short,
    MM_SmallSq_boardGeometry,
    7,7, MM_SmallSq_coords, MM_SmallSq_coords
  };

// ------------------------------------------------------------------------------

static const short MM_SmallSqWithDiag_neighbour[][MAXNEIGHBORS] =
  {
    {  1, 3, 4,-1 }, {  0, 2, 4,-1 }, {  1, 4, 5,-1 },
    {  0, 4, 6,-1 }, {  0, 1, 2, 3, 5, 6, 7, 8 }, {  2, 4, 8, -1 },
    {  3, 4, 7,-1 }, {  4, 6, 8,-1 }, {  4, 5, 7,-1 }
  };

static const short MM_SmallSqWithDiag_milltab_short[][MAXMILLSIZE] =
  {
    { 0,1,2 }, { 3,4,5 }, { 6,7,8 },
    { 0,3,6 }, { 1,4,7 }, { 2,5,8 },
    { 0,4,8 }, { 2,4,6 },
    { -1,-1,-1 }
  };

static const char MM_SmallSqWithDiag_boardGeometry[][2] =
  {
    { 1,1 }, { 3,1 }, { 5,1 },
    { 1,3 }, { 3,3 }, { 5,3 },
    { 1,5 }, { 3,5 }, { 5,5 }
  };

static const short MM_SmallSqWithDiag_coords[] = { 1,3,5, -1 };

static const boardTable boardTable_SmallSqWithDiag =
  {
    BoardSpec::Board_SmallSqWithDiag,
    9, 8,
    MM_SmallSqWithDiag_neighbour,
    MM_SmallSqWithDiag_milltab_short,
    MM_SmallSqWithDiag_boardGeometry,
    7,7, MM_SmallSqWithDiag_coords, MM_SmallSqWithDiag_coords
  };

// ------------------------------------------------------------------------------

static const short MM_SmallTri_neighbour[][MAXNEIGHBORS] =
  {
    { 1,2,3,-1 },
    { 0,2,4,-1 }, { 0,1,3,5 }, { 0,2,6,-1 },
    { 1,5,-1}, { 2,4,6,-1 }, { 3,5,-1 }
  };

static const short MM_SmallTri_milltab_short[][MAXMILLSIZE] =
  {
    { 0,1,4 }, { 0,2,5 }, { 0,3,6 },
    { 1,2,3 }, { 4,5,6 },
    { -1,-1,-1 }
  };

static const char MM_SmallTri_boardGeometry[][2] =
  {
    { 3,2 },
    { 2,3 }, { 3,3 }, { 4,3 },
    { 1,4 }, { 3,4 }, { 5,4 }
  };

static const short MM_SmallTri_coordsX[] = { 1,2,3,4,5, -1 };
static const short MM_SmallTri_coordsY[] = { 2,3,4, -1 };

static const boardTable boardTable_SmallTri =
  {
    BoardSpec::Board_SmallTri,
    7, 4,
    MM_SmallTri_neighbour,
    MM_SmallTri_milltab_short,
    MM_SmallTri_boardGeometry,
    7,7, MM_SmallTri_coordsX, MM_SmallTri_coordsY
  };


BoardSpec_Grid::BoardSpec_Grid(BoardPreset t)
{
  switch (t)
    {
    case Board_Standard9MM: spec = &boardTable_Standard;   break;
    case Board_Morabaraba:  spec = &boardTable_Morabaraba; break;
    case Board_Moebius:     spec = &boardTable_Moebius;    break;
    case Board_Windmill:    spec = &boardTable_Windmill;   break;
    case Board_Sunmill:     spec = &boardTable_Sunmill;    break;
    case Board_6MM:         spec = &boardTable_6MM;   break;
    case Board_7MM:         spec = &boardTable_7MM;   break;
    case Board_SmallSq:     spec = &boardTable_SmallSq;     break;
    case Board_SmallSqWithDiag: spec = &boardTable_SmallSqWithDiag;     break;
    case Board_SmallTri:    spec = &boardTable_SmallTri;   break;
    default: assert(false); break;
    }


  // convert neighbor-table

  neighbors.resize(nPositions());
  for (int i=0;i<nPositions();i++)
    {
      NeighborVector v;

      for (int n=0;n<spec->nMaxNeighbors;n++)
	{
	  if (spec->neighbors[i][n]<0)
	    break;

	  v.push_back(spec->neighbors[i][n]);
	}

      neighbors[i] = v;
    }


  // convert mills-table

  int nMills=0;
  while (spec->mills[nMills][0] >= 0) { nMills++; }

  mills.resize(nMills);
  millsAtPos.resize(nPositions());
  for (int i=0;i<nMills;i++)
    {
      MillPosVector v;

      int millSize=MAXMILLSIZE;
      for (int n=0;n<MAXMILLSIZE;n++)
	{
	  if (spec->mills[i][n]<0)
	    { millSize=n; break; }

	  v.push_back(spec->mills[i][n]);
	}

      mills[i] = v;

      // insert mill to position-indexed tables

      for (int n=0;n<millSize;n++)
	{
	  MillPosVector v2;

	  for (int k=0;k<millSize;k++)
	    if (k!=n)
	      {
		v2.push_back(v[k]);
	      }

	  millsAtPos[v[n]].push_back(v2);
	}
    }

  initPermutations();
}


float BoardSpec_Grid::getPieceRadius() const
{
  if (spec == &boardTable_Sunmill)
    {
      return 0.36 / spec->geom_width;
    }

  switch (spec->geom_width)
    {
    case 5:  return 0.32 / spec->geom_width;
    default: return 0.42 / spec->geom_width;
    }
}

int BoardSpec_Grid::nPositions() const
{
  return spec->nPositions;
}

const NeighborVector& BoardSpec_Grid::getNeighbors(Position p) const
{
  return neighbors[p];
}

Point2D BoardSpec_Grid::getPositionLocation(Position pos) const
{
  Point2D p;
  p.x = spec->geometry[pos][0];
  p.y = spec->geometry[pos][1];

  if (spec->geom_width != 0)
    {
      assert(spec->geom_height != 0);

      p.x = (p.x+0.5) / spec->geom_width;
      p.y = (p.y+0.5) / spec->geom_height;
    }

  return p;
}

std::string BoardSpec_Grid::getPositionName(Position p) const
{
  short x = spec->geometry[p][0];
  short y = spec->geometry[p][1];

  std::string name;
  int i; for (i=0; spec->xcoords[i] != x; i++) { }
  int k; for (k=0; spec->ycoords[k] != y; k++) { }

  name += i+'a';
  name += k+'1';

  return name;
}

int BoardSpec_Grid::nMills() const
{
  return mills.size();
}

const MillPosVector&  BoardSpec_Grid::getMill(int i) const
{
  return mills[i];
}

BoardSpec::BoardPreset BoardSpec_Grid::getBoardPresetID() const
{
  return spec->preset;
}

std::vector<float>  BoardSpec_Grid::xCoords() const
{
  std::vector<float> coords;
  for (int i=0; spec->xcoords[i]>=0 ; i++)
    {
      double p = spec->xcoords[i];
      p = (p+0.5) / spec->geom_width;
      coords.push_back(p);
    }

  return coords;
}

std::vector<float>  BoardSpec_Grid::yCoords() const
{
  std::vector<float> coords;
  for (int i=0; spec->ycoords[i]>=0 ; i++)
    {
      double p = spec->ycoords[i];
      p = (p+0.5) / spec->geom_height;
      coords.push_back(p);
    }

  return coords;
}

boardspec_ptr BoardSpec::boardFactory(BoardSpec::BoardPreset preset)
{
  BoardSpec* spec = NULL;

  switch (preset)
    {
    case Board_Standard9MM:
    case Board_Moebius:
    case Board_Morabaraba:
    case Board_Windmill:
    case Board_Sunmill:
    case Board_6MM:
    case Board_7MM:
    case Board_SmallSq:
    case Board_SmallSqWithDiag:
    case Board_SmallTri:
      spec = new BoardSpec_Grid(preset);
      break;

    case Board_Polygon3:    spec = new BoardSpec_Polygon(3); break;
    case Board_Polygon5:    spec = new BoardSpec_Polygon(5); break;
    case Board_Polygon6:    spec = new BoardSpec_Polygon(6); break;
    case Board_Unknown:  assert(0);
    }

  assert(spec != NULL);

  return boardspec_ptr(spec);
}


void BoardSpec::recursePermutation(Permutation& p, UsageVector& used, int pos)
{
  // If permutation vector is complete, store it into the set.

  if (pos==nPositions())
    {
      /*
      for (int i=0;i<pos;i++)
	{
	  std::cout << int(p[i]) << " ";
	}
      std::cout << "\n";
      */

      m_permutations.push_back(p);

      return;
    }


  for (int i=0;i<nPositions();i++)
    if (used[i]==false)
      {
	const NeighborVector& srcNeigh = getNeighbors(pos);
	const NeighborVector& dstNeigh = getNeighbors(i);

	// check for compatible node types (same number of neighbors)

	if (srcNeigh.size() == dstNeigh.size())
	  {
	    bool compatible=true;

	    // check for compatible local topology (already assigned neighbors must be equal)

	    for (int k=0;k<srcNeigh.size();k++)
	      {
		if (srcNeigh[k]<pos) // check the previous neighbors for neighborhood with 'i'
		  {
		    bool isNeighbor=false;
		    for (int n=0;n<dstNeigh.size();n++)
		      if (dstNeigh[n]==p[srcNeigh[k]])
			isNeighbor=true;

		    if (!isNeighbor) compatible=false;
		  }
	      }

	    // if the assignment is compatible (up to 'pos'), we continue with the next position

	    if (compatible)
	      {
		used[i]=true;
		p[pos]=i;

		recursePermutation(p,used, pos+1);

		used[i]=false;
	      }
	  }
      }
}

void BoardSpec::initPermutations()
{
  Permutation p;
  UsageVector used;

  for (int i=0;i<nPositions();i++)
    used[i]=false;

  p.resize(nPositions());

  //std::cout << "--- BEGIN ---\n";
  recursePermutation(p, used, 0);
  //std::cout << "--- END ---\n";
}
