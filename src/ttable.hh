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

#ifndef TTABLE_HH
#define TTABLE_HH

#include "board.hh"

#define SAFE_HASH 0

class TranspositionTable
{
public:
  TranspositionTable(int nBits);
  ~TranspositionTable();

  void clear();

  enum BoundType { LowerBound, AccurateValue, UpperBound };

  struct Entry
  {
    BoardHash   hash;
    float       eval;
    signed char depth; // depth to which this node was calculated
    signed char bound;
    Move        bestMove;

#if SAFE_HASH
    Board board; // TMP
#endif

    BoundType getBoundType() const { return (BoundType)(bound); }
  };

  const Entry* lookup(BoardHash h, const Board&) const;
  void insert(BoardHash h, float eval, BoundType, int depth, const Move& bestMove, const Board&);

  static inline BoundType boundType(float eval, float alpha, float beta)
  {
    if (eval<=alpha) return UpperBound;
    if (eval>=beta)  return LowerBound;
    return AccurateValue;
  }

  void resetStats() { lookups=hits=collisions=misses=0; }
  int  nHits() const { return hits; }
  int  nCollisions() const { return collisions; }
  int  nLookups() const { return lookups; }

  float getFillStatus() const;

private:
  Entry* table;
  int tableSize;
  BoardHash mask;

  mutable int lookups,hits,collisions,misses;
};

typedef boost::shared_ptr<TranspositionTable> ttable_ptr;

#include <iostream>
#include <iomanip>

inline std::ostream& operator<<(std::ostream& ostr, const TranspositionTable::BoundType& t)
{
  switch (t)
    {
    case TranspositionTable::LowerBound: ostr << "lower"; break;
    case TranspositionTable::UpperBound: ostr << "upper"; break;
    case TranspositionTable::AccurateValue: ostr << "accurate"; break;
    }

  return ostr;
}

inline std::ostream& operator<<(std::ostream& ostr, const TranspositionTable::Entry& e)
{
  ostr << "hash=" << std::hex << e.hash
       << " eval=" << e.eval
       << " depth=" << ((int)e.depth)
       << " bound=" << e.getBoundType()
       << " bestMove=" << e.bestMove;
  return ostr;
}

#endif
