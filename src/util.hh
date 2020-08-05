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

#ifndef UTIL_HH
#define UTIL_HH

#include <assert.h>

#include "config.h"
#include "gettext.h"

#define _(String) gettext (String)
#define N_(String) (String)


/* Simple, computationally efficient vector-class with a fixed, maximum size.
 */
template <class T, int N> class SmallVec
{
public:
  SmallVec() { nElements=0; }

  void  push_back(const T& t) { assert(nElements<N); elem[nElements++]=t; }
  void  clear() { nElements=0; }
  short size() const { return nElements; }
  bool  empty() const { return nElements==0; }
  void  resize(int s) { nElements=s; }

  /* */ T& operator[](int i)       { return elem[i]; }
  const T& operator[](int i) const { return elem[i]; }

  void append(const SmallVec<T,N>& v)
  {
    for (int i=0;i<v.size();i++)
      { push_back(v[i]); }
  }

private:
  T elem[N];
  short nElements;
};


/* Simple 2D-vector for specifying graphics coordinates.
 */
class Point2D
{
public:
  Point2D() { }
  Point2D(float _x, float _y) :x(_x),y(_y) { }

  float x,y;

  void operator*=(float factor) { x*=factor; y*=factor; }
};


/* A position for pieces on the boards. */
typedef short Position;


/* The player-identifier enum. */
enum Player { PL_None=0, PL_Black=-1, PL_White=1 };

inline void togglePlayer(Player& p) { p = Player(-p); }
inline Player opponent(Player p) { return Player(-p); }

// Map players to [0;1]. Undefined for PL_None.
inline int player2Index(Player p) { return (p+1)>>1; }

#endif
