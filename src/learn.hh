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

#ifndef LEARN_HH
#define LEARN_HH

#include "board.hh"
#include <boost/shared_ptr.hpp>


class Experience
{
public:
  enum { SIGNIFICANT_BITS=8 };

  void reset()
  {
    for (int i=0;i< (1<<SIGNIFICANT_BITS) ;i++)
      memory[i].clear();
  }

  void addBoard(BoardID b, Player winner)
  {
    int list = b & ((1<<SIGNIFICANT_BITS)-1);
    float offset = (winner==PL_White ? 1.0 : -1.0);

    for (int i=0;i<memory[list].size();i++)
      {
	if (memory[list][i].id == b)
	  {
	    memory[list][i].offset += offset;
	    return;
	  }
      }

    MemEntry mem;
    mem.id = b;
    mem.offset = offset;
    memory[list].push_back(mem);
    nEntries++;
  }

  float getOffset(BoardID b, Player self) const
  {
    int list = b & ((1<<SIGNIFICANT_BITS)-1);
    for (int i=0;i<memory[list].size();i++)
      {
	if (memory[list][i].id == b)
	  {
	    return memory[list][i].offset * (self==PL_White ? 1 : -1);
	  }
      }

    return 0.0;
  }

private:
  struct MemEntry
  {
    BoardHash id;
    float offset;
  };

  std::vector<MemEntry> memory[1<<SIGNIFICANT_BITS];
  int nEntries;
};

typedef boost::shared_ptr<Experience> experience_ptr;


// ---------------------------------------------------------------------------

class PositionMemory
{
public:
  enum { SIGNIFICANT_BITS=12 };

  PositionMemory() { nEntries=0; }

  void storeBoard(const Board& b, float e)
  {
    return;

    int list = b.getHash() & ((1<<SIGNIFICANT_BITS)-1);
    for (int i=0;i<memory[list].size();i++)
      {
	if (memory[list][i].hash == b.getHash())
	  {
	    return;
	  }
      }

    MemEntry mem;
    mem.hash = b.getHash();
    mem.e = e;
    memory[list].push_back(mem);
    nEntries++;

    //std::cout << "position memory entries: " << nEntries << "\n";
  }

  float lookupHash(const Board& b) const
  {
    return 0.0;

    BoardHash hash=b.getHash();

    int list = b.getHash() & ((1<<SIGNIFICANT_BITS)-1);
    for (int i=0;i<memory[list].size();i++)
      {
	if (memory[list][i].hash == b.getHash())
	  {
	    return memory[list][i].e;
	  }
      }

    return 0.0;
  }

private:
  struct MemEntry
  {
    BoardHash hash;
    float e;
  };

  std::vector<MemEntry> memory[1<<SIGNIFICANT_BITS];
  int nEntries;
};

#endif
