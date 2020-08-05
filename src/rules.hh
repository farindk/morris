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

#ifndef RULES_HH
#define RULES_HH

#include "boardspec.hh"
#include "board.hh"


/* Write the move in human-readable notation.
 */
std::string writeMove(const Move& m, boardspec_ptr spec);


typedef unsigned long long BoardID; // a unique ID for a specific board-configuration


typedef boost::shared_ptr<class RuleSpec> rulespec_ptr;


/* The rule-specification class manages the various variants of the rules.
   It also carries out checks for rule-conforming moves, and it comprises
   the move-generator for AI players.
 */
class RuleSpec
{
public:
  RuleSpec();

  boardspec_ptr boardSpec;  // The board that is played on.

  bool laskerVariant;         // Player may either set a new piece or move an existing piece.
  bool mayJump;               // Player may jump if he is down to three pieces.
  bool mayTakeMultiple;       // When closing more than one mill at once, may also take several opponent pieces.
  bool mayTakeFromMillsAlways;// May take from mills even if there are other pieces available.
  int  tieAfterNRepeats;      // Number of board-repetitions to declare a tie. NOTE: set to zero to turn off.
  int  nPieces;               // Number of pieces each player has at the beginning.
  // int  movesBetweenReformingMill;  // number of turns before mills may reform


  // --- checking for valid moves ---

  // The number of mills that would be closed by the given move.
  int  nPotentialMills(const Board&, const Move&) const;

  /* Whether the specified piece may be taken. This depends on whether it is part of
     a mill and whether there are other pieces, not part of a mill. */
  bool mayTake(const Board&, Position) const;

  /* Check whether the given move is valid.
     NOTE: the takes in this move are ignored ! */
  bool isValidMove(const Board&, const Move&) const;

  /* The number of empty direct neighbors to the specified piece. */
  int  freedomAtPosition(const Board&, Position) const;

  // Whether the specified piece is part of a mill.
  bool isInMill(const Board&, Position) const;


  // --- game state ---

  // Whether the current situation is a game-over.
  bool isGameOver(const Board&, Player* winner=NULL) const;
  bool currentPlayerHasWon(const Board&) const;
  bool currentPlayerHasLost(const Board&) const;

  /* This checks for the current game history for board repetitions, if ties are enabled.
     For this to work, the chain of previous boards has to be set correctly.
   */
  bool tieBetweenBothPlayers(const Board&) const;

  // Get a unique ID for the current board.
  BoardID getBoardID(const Board& board) const;

  // Get a unique ID for the current board, considering symmetries. I.e. a similar situation,
  // in which the board is just rotated, mirrored, or otherwise permuted receives the same ID.
  BoardID getBoardID_Symmetric(const Board& board) const;


  // --- move generator ---

  // Generate a set of valid moves for the current board.
  // NOTE: the 'output' set is not cleared in this function.
  void generateMoves(std::vector<Move>& output, const class Board& currentBoard) const;

  // --- rule factory ---

  enum RulePreset
    {
      Preset_Standard,
      Preset_Lasker,
      Preset_Moebius,
      Preset_Morabaraba,
      Preset_Windmill,
      Preset_Sunmill,
      Preset_6MM,
      Preset_7MM,
      Preset_Tapatan,
      Preset_Achi,
      Preset_SmallTri,
      Preset_NineHoles,
      Preset_Polygon3,
      Preset_Polygon5,
      Preset_Polygon6
    };

  // Initialize the rules to one of the preset.
  static rulespec_ptr createPresetRule(enum RulePreset);

private:
  // Take the specified move as template and add all possible takes of 'n' opponent pieces.
  void generateTakes(std::vector<Move>& output, const Move&, const class Board& currentBoard, int n) const;

  /* Check is a mill will be closed by the specified move and if yes, extend the move with all
     possible takes and add to the set. Otherwise (if no mill was closed), simply add the move
     to the set (without takes).
  */
  void addTakesToMoveIfMillClosed(std::vector<Move>& output, const Move& m,const class Board& currentBoard) const;
};



/* NOTE: this function is defined inline, because it is used in the time critical
   board evaluation function.
 */
inline int RuleSpec::freedomAtPosition(const Board& b, Position p) const
{
  const NeighborVector& v = boardSpec->getNeighbors(p);
  int cnt=0;
  for (int i=0;i<v.size();i++)
    if (b.isEmpty(v[i]))
      cnt++;

  return cnt;    
}

#endif
