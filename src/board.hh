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

#ifndef BOARD_HH
#define BOARD_HH

#include <assert.h>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <iostream>

#include "util.hh"
#include "constants.hh"


/* This class encodes a half-move of a player. The mode can either be setting
   a new piece onto the board, or moving an existing piece.
   For setting, the 'oldPos' remains undefined.
 */
class Move
{
public:
  enum { MAXTAKES=3 };

  enum MoveMode { Mode_Set, Mode_Move } mode;

  Position oldPos;   // old position of the piece (undefined in set-mode)
  Position newPos;   // new position of the piece
  SmallVec<Position,MAXTAKES> takes;  // the set of opponent pieces to remove from the board


  // methods for easier manipulation

  void reset()                          { takes.clear(); oldPos=newPos=-1; }
  void setMove_Set(Position p)                  { mode=Mode_Set; newPos=p; oldPos=-1; }
  void setMove_Move(Position from, Position to) { mode=Mode_Move; oldPos=from; newPos=to; }
  void addTake(Position p)              { takes.push_back(p); }

  bool operator==(const Move& m) const;
};


// move logging for debugging
std::ostream& operator<<(std::ostream& ostr, const Move&);



typedef unsigned long long BoardHash;

/* The board class hold the current configuation of the players' pieces, as well as
   additional status information like the current player and the number of pieces
   each player can still set. The Board class also maintains a hash-code for the
   board using the Zobrist hashing algorithm.

   Additionally, the board can include a pointer to the previous board (in a running game).
   This is used to detect ties by repeated board positions.

   NOTE: you have to call reset() before the board is in a playable state.
 */
class Board
{
public:
  void reset(int nPiecesToSet);

  void doMove(const Move&);
  void undoMove(const Move&);

  // --- current player ---

  void   togglePlayer() { ::togglePlayer(currentPlayer); hash ^= hash_playerToggle; }

  Player getCurrentPlayer() const { return currentPlayer; }
  Player getOpponentPlayer() const { return opponent(currentPlayer); }


  // --- querying the board ---

  Player getPosition(int p) const { return Player(boardPos[p]); }

  bool   isPlayer  (Position p) const { return boardPos[p]==currentPlayer; }
  bool   isOpponent(Position p) const { return boardPos[p]==opponent(currentPlayer); }
  bool   isEmpty   (Position p) const { return boardPos[p]==PL_None; }


  // --- counting pieces ---

  short  getNPiecesToSet(Player p) const { return nPiecesToSet[ player2Index(p) ]; }
  short  getNPiecesToSet() const { return nPiecesToSet[ player2Index(currentPlayer) ]; }

  short  getNPiecesOnBoard(Player p) const { return nPiecesOnBoard[ player2Index(p) ]; }
  short  getNPiecesOnBoard() const { return nPiecesOnBoard[ player2Index(currentPlayer) ]; }

  short  getNPiecesLeft(Player p) const { return getNPiecesToSet(p) + getNPiecesOnBoard(p); }
  short  getNPiecesLeft() const { return getNPiecesLeft(currentPlayer); }


  // --- chaining ---

  void   setPrevBoard(boost::shared_ptr<Board> b) { prev=b; }
  boost::shared_ptr<Board> getPrevBoard() const { return prev; }


  // --- hashes ---

  BoardHash getHash() const { return hash; }
  static void initHashValues(); // fill the hash tables with random values


  // --- hard board modification, not considering the hash value ---

  void   setPosition_noHash(int p, Player pl) { boardPos[p]=pl; }


  // --- standard operators ---

  bool operator==(const Board& b) const;


  // --- debugging ---

  //void   setNPiecesToSet(Player pl, int n) { nPiecesToSet[player2Index(pl)]=n; }
  //void   setCurrentPlayer(Player pl) { currentPlayer=pl; }
  void   displayOnConsole() const;

private:
  signed char boardPos[MAXPOSITIONS];
  Player      currentPlayer;
  signed char nPiecesToSet[2];
  signed char nPiecesOnBoard[2];

  boost::shared_ptr<Board> prev;


  // --- hash ---

  BoardHash hash;

  /* The arrays are organized as follows: index [1] is unused, [0] and [2] is mapped
     to the two players. This makes it possible to access the arrays with simply
     'player+1', since player is {-1;1}.
  */
  static BoardHash hash_pos[3][MAXPOSITIONS]; // player, board-position
  static BoardHash hash_nToSet[3][MAXPIECES]; // player, number of pieces in stack
  static BoardHash hash_playerToggle; // xor'ed to hash if player is PL_Black

  BoardHash hashFromScratch() const; // for debugging only
};

#endif
