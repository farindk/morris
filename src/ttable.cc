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

#include "ttable.hh"

#include <iostream>


// 16 -   65536
// 17 -  131072
// 18 -  262144
// 19 -  524288
// 20 - 1048576
TranspositionTable::TranspositionTable(int nBits)
{
  tableSize = 1<<nBits;
  table = new Entry[tableSize];

  mask=0;
  for (int i=0;i<nBits;i++)
    {
      mask <<= 1;
      mask |= 1;
    }

  hits=collisions=misses=0;

  clear();
}

void TranspositionTable::clear()
{
  for (int i=0;i<tableSize;i++)
    {
      table[i].hash = 0;
    }
}

TranspositionTable::~TranspositionTable()
{
  delete[] table;
}

const TranspositionTable::Entry* TranspositionTable::lookup(BoardHash hash, const Board& b) const
{
  BoardHash h = hash & mask;

  lookups++;
  if (table[h].hash != 0) { hits++; }
  if (table[h].hash != hash) { misses++; }

#if SAFE_HASH
  if (table[h].hash == hash && !(b==table[h].board))
    {
      std::cout << "HASH COLLISION\n";
      assert(0);
    }
#endif

  if (table[h].hash == hash)
    {
      return &table[h];
    }
  else
    {
      return NULL;
    }
}

void TranspositionTable::insert(BoardHash hash, float eval, BoundType bound, int depth, const Move& bestMove,
				const Board& b)
{
  BoardHash h = hash & mask;

  bool replaceEntry = false;
  /**/ if (table[h].hash==0)
    {
      replaceEntry=true;
    }
  else if (table[h].hash != hash) // collision
    {
      replaceEntry=true; /* Always replace such that irrelevant moves do not block the table. */
      collisions++;
    }
  else
    {
      if (depth > table[h].depth)
	{
	  replaceEntry=true;
	}
      else
	{
	  /* NOP */
	}
    }

  if (replaceEntry)
    {
      table[h].hash  = hash;
      table[h].eval  = eval;
      table[h].depth = depth;
      table[h].bound = bound;
      table[h].bestMove = bestMove;

#if SAFE_HASH
      table[h].board = b;
#endif
    }
}

float TranspositionTable::getFillStatus() const
{
  int nFilled=0;
  for (int i=0;i<tableSize;i++)
    {
      if (table[i].hash) { nFilled++; }
    }

  return float(nFilled)/tableSize;
}

