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

#include "board.hh"

#include <iostream>
#include <iomanip>
#include <algorithm>


bool Move::operator==(const Move& m) const
{
  // note: the comparisons are ordered somehow in the order of decreasing probability

  if (newPos!=m.newPos) return false;
  if (mode==Mode_Move && oldPos!=m.oldPos) return false;
  if (mode != m.mode) return false;

  if (takes.size() != m.takes.size()) return false;
  for (int i=0;i<takes.size();i++)
    if (takes[i]!=m.takes[i])
      return false;

  return true;
}


std::ostream& operator<<(std::ostream& ostr, const Move& m)
{
  ostr << "[";
  if (m.oldPos<0) ostr << "NA";
  else ostr << m.oldPos;

  ostr << "->" << m.newPos;
  if (m.takes.size()>0)
    {
      ostr << " (";
      for (int i=0;i<m.takes.size();i++)
	{
	  if (i>0) ostr << ",";
	  ostr << m.takes[i];
	}
      ostr << ")";
    }

  ostr << "]";

  return ostr;
}



// generate a random hash value
static BoardHash randomHash()
{
  return BoardHash(rand()) ^ (BoardHash(rand())<<16) ^ (BoardHash(rand())<<32) ^ (BoardHash(rand())<<48);
}

BoardHash Board::hash_pos[3][MAXPOSITIONS];
BoardHash Board::hash_nToSet[3][MAXPIECES];
BoardHash Board::hash_playerToggle;

void Board::initHashValues()
{
  for (int p=0 ; p<=2 ; p+=2 /* skip [1] */)
    {
      for (int i=0;i<MAXPOSITIONS;i++)
	hash_pos[p][i] = randomHash();

      for (int i=0;i<MAXPIECES;i++)
	hash_nToSet[p][i] = randomHash();
    }

  hash_playerToggle = randomHash();
}


void Board::reset(int p_nPiecesToSet)
{
  for (int i=0;i<MAXPOSITIONS;i++)
    { boardPos[i]=PL_None; }

  currentPlayer = PL_White;

  nPiecesToSet  [0]=nPiecesToSet  [1]=p_nPiecesToSet;
  nPiecesOnBoard[0]=nPiecesOnBoard[1]=0;

  hash = hash_nToSet[0][p_nPiecesToSet] ^ hash_nToSet[2][p_nPiecesToSet];

  prev.reset();
}


void Board::doMove(const Move& m)
{
  assert(boardPos[m.newPos] == PL_None);

  // set new piece or move existing piece

  switch (m.mode)
    {
    case Move::Mode_Set:
      {
	const int playerIndex = player2Index(currentPlayer);

	assert(nPiecesToSet[playerIndex]>0);

	boardPos[m.newPos] = currentPlayer;
	hash ^= hash_pos [currentPlayer+1][m.newPos];
	hash ^= hash_nToSet[currentPlayer+1][ nPiecesToSet[playerIndex]  ];
	hash ^= hash_nToSet[currentPlayer+1][ nPiecesToSet[playerIndex]-1];

	nPiecesOnBoard[ playerIndex ]++;
	nPiecesToSet  [ playerIndex ]--;
      }
      break;

    case Move::Mode_Move:
      {
	boardPos[m.oldPos] = PL_None;
	boardPos[m.newPos] = currentPlayer;
	hash ^= hash_pos[currentPlayer+1][m.oldPos];
	hash ^= hash_pos[currentPlayer+1][m.newPos];
      }
      break;
    }

  // optionally take away opponent piece(s)

  for (int i=0; i<m.takes.size(); i++)
    {
      assert(boardPos[m.takes[i]] == opponent(currentPlayer));

      boardPos[m.takes[i]] = PL_None;
      nPiecesOnBoard[ player2Index(opponent(currentPlayer)) ]--;

      hash ^= hash_pos[opponent(currentPlayer)+1][m.takes[i]];
    }
					  
  // now, it's the next player's turn

  togglePlayer();

  //assert(hash == hashFromScratch());
}


void Board::undoMove(const Move& m)
{
  togglePlayer();

  // undo takes

  for (int i=0; i<m.takes.size(); i++)
    {
      boardPos[m.takes[i]] = opponent(currentPlayer);
      nPiecesOnBoard[ player2Index(opponent(currentPlayer)) ]++;

      hash ^= hash_pos[opponent(currentPlayer)+1][m.takes[i]];
    }

  // undo move

  switch (m.mode)
    {
    case Move::Mode_Set:
      {
	const int playerIndex = player2Index(currentPlayer);

	boardPos[m.newPos] = PL_None;
	hash ^= hash_pos   [currentPlayer+1][m.newPos];
	hash ^= hash_nToSet[currentPlayer+1][ nPiecesToSet[playerIndex]  ];
	hash ^= hash_nToSet[currentPlayer+1][ nPiecesToSet[playerIndex]+1];
	nPiecesOnBoard[ playerIndex ]--;
	nPiecesToSet  [ playerIndex ]++;
      }
      break;

    case Move::Mode_Move:
      boardPos[m.oldPos] = currentPlayer;
      boardPos[m.newPos] = PL_None;
      hash ^= hash_pos[currentPlayer+1][m.oldPos];
      hash ^= hash_pos[currentPlayer+1][m.newPos];
      break;
    }
}


BoardHash Board::hashFromScratch() const
{
  BoardHash h = 0;

  for (int i=0;i<MAXPOSITIONS;i++)
    if (boardPos[i] != PL_None)
      {
	h ^= hash_pos[ boardPos[i]+1 ][i];
      }

  if (currentPlayer == PL_Black) { h ^= hash_playerToggle; }

  h ^= hash_nToSet[PL_White+1][ nPiecesToSet[ player2Index(PL_White) ] ];
  h ^= hash_nToSet[PL_Black+1][ nPiecesToSet[ player2Index(PL_Black) ] ];

  return h;
}


bool Board::operator==(const Board& b) const
{
  if (hash != b.hash) { return false; }

  if (currentPlayer != b.currentPlayer) return false;

  for (int i=0;i<2;i++)
    {
      if (nPiecesToSet  [i] != b.nPiecesToSet  [i]) return false;
      if (nPiecesOnBoard[i] != b.nPiecesOnBoard[i]) return false;
    }

  for (int i=0;i<MAXPOSITIONS;i++)
    {
      if (boardPos[i] != b.boardPos[i]) return false;
    }

  return true;
}


void Board::displayOnConsole() const
{
  char p[3];
  char* pc=&p[1];
  pc[PL_White] = 'W';
  pc[PL_Black] = 'B';
  pc[PL_None ] = ' ';

  std::cout << " 7 " << pc[boardPos[0]] << " --------- " << pc[boardPos[1]] << " --------- " << pc[boardPos[2]] << "\n";
  std::cout << "   |           |           |\n";
  std::cout << " 6 |   " << pc[boardPos[3]] << " ----- " << pc[boardPos[4]] << " ----- " << pc[boardPos[5]] <<  "   |\n";
  std::cout << "   |   |       |       |   |\n";
  std::cout << " 5 |   |   " << pc[boardPos[6]] << " - " << pc[boardPos[7]] << " - " << pc[boardPos[8]] << "   |   |\n";
  std::cout << "   |   |   |       |   |   |\n";
  std::cout << " 4 " << pc[boardPos[9]] << " - " << pc[boardPos[10]] << " - " << pc[boardPos[11]] << "       " << pc[boardPos[12]] << " - " << pc[boardPos[13]] << " - " << pc[boardPos[14]] << "\n";
  std::cout << "   |   |   |       |   |   |\n";
  std::cout << " 3 |   |   " << pc[boardPos[15]] << " - " << pc[boardPos[16]] << " - " << pc[boardPos[17]] << "   |   |\n";
  std::cout << "   |   |       |       |   |\n";
  std::cout << " 2 |   " << pc[boardPos[18]] << " ----- " << pc[boardPos[19]] << " ----- " << pc[boardPos[20]] << "   |\n";
  std::cout << "   |           |           |\n";
  std::cout << " 1 " << pc[boardPos[21]] << " --------- " << pc[boardPos[22]] << " --------- " << pc[boardPos[23]] << "\n";
  std::cout << "   a   b   c   d   e   f   g\n";

  int wIdx = player2Index(PL_White);
  int bIdx = player2Index(PL_Black);

  std::cout 
    << "   white(X): " << nPiecesToSet[wIdx] + nPiecesOnBoard[wIdx] << "\n"
    << "   black(O): " << nPiecesToSet[bIdx] + nPiecesOnBoard[bIdx] << "\n";
}


