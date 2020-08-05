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

#ifndef CONSTANTS_HH
#define CONSTANTS_HH

enum { MAXPOSITIONS  =36 };  // maximum number of positions on a board
enum { MAXPIECES     =15 };  // maximum number of pieces for a player
enum { MAXNEIGHBORS  =8  };  // maximum number of neighbors of a board position
enum { MAXMILLSIZE   =3  };  // maximum mill-size
enum { MAXSEARCHDEPTH=50 };

enum { TRANSPOSITION_TABLE_SIZE=20 }; // significant bits for the transposition-table hash

#endif
