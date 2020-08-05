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

#include "rules.hh"


std::string writeMove(const Move& m, boardspec_ptr spec)
{
  std::string str;

  if (m.mode == Move::Mode_Move)
    {
      str = spec->getPositionName(m.oldPos);
      str += '-';
    }

  str += spec->getPositionName(m.newPos);

  for (int i=0;i<m.takes.size();i++)
    {
      str += 'x';
      str += spec->getPositionName(m.takes[i]);
    }

  return str;
}


RuleSpec::RuleSpec()
{
  laskerVariant=false;
  mayJump=true;
  mayTakeMultiple=false;
  mayTakeFromMillsAlways=false;
  tieAfterNRepeats=3;
  nPieces=9;
}


int RuleSpec::nPotentialMills(const Board& currentBoard, const Move& move) const
{
  const Player pl = currentBoard.getCurrentPlayer();
  int cnt=0;

  // get set of all mills through this position

  const std::vector<MillPosVector>& mills = boardSpec->getMillsThroughPos(move.newPos);

  // iterate through all mills

  for (size_t i=0;i<mills.size();i++)
    {
      // if all positions (except 'newPos') are the player's pieces, it is a potential mill

      bool isMill=true;

      for (int k=0;k<mills[i].size();k++)
	{
	  if (mills[i][k]==move.oldPos ||
	      currentBoard.getPosition(mills[i][k]) != pl)
	    { isMill = false; break; }
	}

      if (isMill) { cnt++; }
    }

  return cnt;
}


bool RuleSpec::isInMill(const Board& currBoard, Position pos) const
{
  const std::vector<MillPosVector>& mills = boardSpec->getMillsThroughPos(pos);

  const Player pl = currBoard.getPosition(pos);

  for (size_t m=0;m<mills.size();m++)
    {
      assert(mills[m].size()==2);

      bool isMill=true;
      for (int i=0;i<mills[m].size();i++)
	if (currBoard.getPosition( mills[m][i] )!=pl)
	  {
	    isMill=false; break;
	  }

      if (isMill) { return true; }
    }

  return false;
}


bool RuleSpec::mayTake(const Board& currBoard, Position pos) const
{
  assert(currBoard.getPosition(pos) == opponent(currBoard.getCurrentPlayer()));

  if (mayTakeFromMillsAlways) return true;
  if (!isInMill(currBoard,pos)) return true;

  bool onlyMills = true;
  for (int i=0;i<boardSpec->nPositions();i++)
    if (currBoard.getPosition(i) == opponent(currBoard.getCurrentPlayer()))
      if (!isInMill(currBoard,i))
	{ onlyMills = false; break; }

  if (onlyMills) return true;
  else return false;
}


// generate all possible takes for the set/move-part of the partial move 'm'
void RuleSpec::generateTakes(std::vector<Move>& output, const Move& m, const Board& currBoard, int n) const
{
  assert(n>0);

  const Player other = currBoard.getOpponentPlayer();


  // First try to generate takes assuming that not all opponent pieces are within mills
  // (or that it is irrelevant, because we may take any piece).

  bool takesGenerated = false;

  for (int i=0;i<boardSpec->nPositions();i++)
    if (currBoard.getPosition(i) == other)
      {
	if (mayTakeFromMillsAlways ||
	    isInMill(currBoard, i) == false)
	  {
	    Move move = m;
	    move.addTake(i);

	    takesGenerated=true;

	    if (n==1)
	      {
		// move complete, add to set
		output.push_back(move);
	      }
	    else
	      {
		// we may add more takes, continue recursively
		Board tmpBoard = currBoard;
		tmpBoard.setPosition_noHash(i, PL_None);
		generateTakes(output, move, tmpBoard, n-1);
	      }
	  }
      }

  // If we could not take any opponent pieces (because all are in mills),
  // we are allowed to take any opponent piece we want (from mills).

  if (!takesGenerated)
    {
      for (int i=0;i<boardSpec->nPositions();i++)
	if (currBoard.getPosition(i) == other)
	  {
	    Move move = m;
	    move.addTake(i);

	    if (n==1)
	      {
		output.push_back(move);
	      }
	    else
	      {
		Board tmpBoard = currBoard;
		tmpBoard.setPosition_noHash(i, PL_None);
		generateTakes(output, move, tmpBoard, n-1);
	      }
	  }
    }
}


void RuleSpec::addTakesToMoveIfMillClosed(std::vector<Move>& output,
					  const Move& m,
					  const class Board& currBoard) const
{
  int nMills = nPotentialMills(currBoard, m);

  if (nMills==0)
    {
      // no mills closed, do not add takes

      output.push_back(m);
    }
  else
    {
      // add takes

      if (mayTakeMultiple==false)
	{
	  nMills=1;
	}

      generateTakes(output, m, currBoard, nMills);
    }
}


void RuleSpec::generateMoves(std::vector<Move>& output, const Board& currBoard) const
{
  const bool maySet  = (currBoard.getNPiecesToSet()>0);
  const bool mayMove = (currBoard.getNPiecesToSet()==0) || laskerVariant;
  const bool mayFly  = mayJump && (currBoard.getNPiecesLeft()==3);


  // generate set-moves

  if (maySet)
    {
      for (int i=0;i<boardSpec->nPositions();i++)
	if (currBoard.getPosition(i) == PL_None)
	  {
	    Move m;
	    m.setMove_Set(i);
	    addTakesToMoveIfMillClosed(output,m,currBoard);
	  }
    }


  // generate moves

  if (mayMove)
    {
      for (int i=0;i<boardSpec->nPositions();i++)
	if (currBoard.getPosition(i) == currBoard.getCurrentPlayer())
	  {
	    if (mayFly)
	      {
		for (int k=0;k<boardSpec->nPositions();k++)
		  {
		    if (k!=i && currBoard.getPosition(k) == PL_None)
		      {
			Move m;
			m.setMove_Move(i,k);
			addTakesToMoveIfMillClosed(output,m,currBoard);
		      }
		  }
	      }
	    else
	      {
		const NeighborVector& v = boardSpec->getNeighbors(i);
		for (int n=0;n<v.size();n++)
		  {
		    if (currBoard.getPosition(v[n]) == PL_None)
		      {
			Move m;
			m.setMove_Move(i,v[n]);
			addTakesToMoveIfMillClosed(output,m,currBoard);
		      }
		  }
	      }
	  }
    }
}


bool RuleSpec::isValidMove(const Board& b, const Move& m) const
{
  switch (m.mode)
    {
    case Move::Mode_Set:
      return (b.getPosition(m.newPos)==PL_None);
      break;

    case Move::Mode_Move:
      if (b.getPosition(m.newPos)!=PL_None) { return false; }
      if (b.getPosition(m.oldPos)!=b.getCurrentPlayer()) { return false; }

      if (mayJump && b.getNPiecesLeft() <= 3)
	{
	  return true;
	}
      else
	{
	  const NeighborVector& n = boardSpec->getNeighbors(m.oldPos);
	  for (int i=0;i<n.size();i++)
	    if (n[i]==m.newPos)
	      return true;

	  return false;
	}

      break;
    }

  assert(0);
}


bool RuleSpec::isGameOver(const Board& b, Player* winner) const
{
  if (currentPlayerHasWon(b))
    {
      if (winner) *winner = b.getCurrentPlayer();
      return true;
    }

  if (currentPlayerHasLost(b))
    {
      if (winner) *winner = opponent(b.getCurrentPlayer());
      return true;
    }

  if (tieBetweenBothPlayers(b))
    {
      if (winner) *winner = PL_None;
      return true;
    }

  return false;
}


bool RuleSpec::currentPlayerHasWon(const Board& b) const
{
  if (b.getNPiecesLeft( b.getOpponentPlayer() )<=2)
    return true;

  return false;
}


bool RuleSpec::currentPlayerHasLost(const Board& b) const
{
  // less than three pieces
  if (b.getNPiecesLeft()<=2)
    return true;

  // have to move (no piece to set anymore), but no freedom
  int freedom=0;
  for (int i=0;i<boardSpec->nPositions();i++)
    if (b.isPlayer(i))
      {
	freedom=freedomAtPosition(b,i);

	if (freedom>0)
	  break;
      }

  if (freedom==0 &&
      b.getNPiecesToSet()==0 &&
      (b.getNPiecesOnBoard()>3 || !mayJump))
    { return true; }

  // have to set a piece, but the board is completely filled
  if (b.getNPiecesToSet()>0 &&
      b.getNPiecesOnBoard(PL_White) + b.getNPiecesOnBoard(PL_Black) == boardSpec->nPositions())
    { return true; }

  return false;
}


bool RuleSpec::tieBetweenBothPlayers(const Board& b) const
{
  if (tieAfterNRepeats==0) { return false; }

  int repeatCnt = 0;

  const Board* curr = &b;
  const Board* prev;

  for (;;)
    {
      prev = curr->getPrevBoard().get();

      if (prev==NULL)
	break;

      if (*prev==b)
	{ repeatCnt++; }

      if (repeatCnt==tieAfterNRepeats)
	return true;

      curr=prev;
    }

  return false;
}


rulespec_ptr RuleSpec::createPresetRule(enum RulePreset p)
{
  rulespec_ptr rule = rulespec_ptr(new RuleSpec);

  switch (p)
    {
    case Preset_Standard:
      rule->boardSpec = boardspec_ptr(new BoardSpec_Grid(BoardSpec::Board_Standard9MM));
      rule->nPieces = 9;
      break;

    case Preset_Lasker:
      rule->boardSpec = boardspec_ptr(new BoardSpec_Grid(BoardSpec::Board_Standard9MM));
      rule->nPieces = 10;
      rule->laskerVariant=true;
      rule->mayJump=false;
      break;

    case Preset_Morabaraba:
      rule->boardSpec = boardspec_ptr(new BoardSpec_Grid(BoardSpec::Board_Morabaraba));
      rule->nPieces = 12;
      break;

    case Preset_Moebius:
      rule->boardSpec = boardspec_ptr(new BoardSpec_Grid(BoardSpec::Board_Moebius));
      rule->nPieces = 9;
      break;

    case Preset_Windmill:
      rule->boardSpec = boardspec_ptr(new BoardSpec_Grid(BoardSpec::Board_Windmill));
      rule->nPieces = 12;
      rule->mayTakeMultiple=true;
      break;

    case Preset_Sunmill:
      rule->boardSpec = boardspec_ptr(new BoardSpec_Grid(BoardSpec::Board_Sunmill));
      rule->nPieces = 12;
      rule->mayTakeMultiple=true;
      break;

    case Preset_6MM:
      rule->boardSpec = boardspec_ptr(new BoardSpec_Grid(BoardSpec::Board_6MM));
      rule->nPieces = 6;
      rule->mayJump = false;
      rule->mayTakeMultiple=false;
      break;

    case Preset_7MM:
      rule->boardSpec = boardspec_ptr(new BoardSpec_Grid(BoardSpec::Board_7MM));
      rule->nPieces = 7;
      rule->mayJump = false;
      rule->mayTakeMultiple=false;
      break;

    case Preset_Tapatan:
      rule->boardSpec = boardspec_ptr(new BoardSpec_Grid(BoardSpec::Board_SmallSqWithDiag));
      rule->nPieces = 3;
      rule->mayJump = false;
      break;

    case Preset_NineHoles:
      rule->boardSpec = boardspec_ptr(new BoardSpec_Grid(BoardSpec::Board_SmallSq));
      rule->nPieces = 3;
      rule->mayJump = true;
      break;

    case Preset_Achi:
      rule->boardSpec = boardspec_ptr(new BoardSpec_Grid(BoardSpec::Board_SmallSqWithDiag));
      rule->nPieces = 4;
      rule->mayJump = false;
      break;

    case Preset_SmallTri:
      rule->boardSpec = boardspec_ptr(new BoardSpec_Grid(BoardSpec::Board_SmallTri));
      rule->nPieces = 3;
      rule->mayJump = false;
      break;

    case Preset_Polygon3:
      rule->boardSpec = boardspec_ptr(new BoardSpec_Polygon(3));
      rule->nPieces = 7;
      break;

    case Preset_Polygon5:
      rule->boardSpec = boardspec_ptr(new BoardSpec_Polygon(5));
      rule->nPieces = 11;
      break;

    case Preset_Polygon6:
      rule->boardSpec = boardspec_ptr(new BoardSpec_Polygon(6));
      rule->nPieces = 12;
      break;
    }

  return rule;
}


BoardID RuleSpec::getBoardID(const Board& board) const
{
  BoardID id=0;
  id += board.getNPiecesToSet(PL_White); id *= nPieces+1;
  id += board.getNPiecesToSet(PL_Black);

  const int nPos = boardSpec->nPositions();
  for (int p=0;p<nPos;p++)
    {
      id *= 3;

      int pl = board.getPosition(p);
      if (pl<0) pl=2;

      id += pl;
    }

  id *= 2;
  id += player2Index( board.getCurrentPlayer() );

  return id;
}


BoardID RuleSpec::getBoardID_Symmetric(const Board& board) const
{
  const std::vector<BoardSpec::Permutation>& permutations = boardSpec->getPermutations();

  // Compute all symmetric permutations of the board position and determine the board ID for each.
  // Return the lowest ID.

  Board b = board;  // TODO: setting only the meta-information would be faster (next player, pieces to set)
  BoardID id;
  for (int i=0;i<permutations.size();i++)
    {
      for (int p=0;p<boardSpec->nPositions();p++)
	b.setPosition_noHash( permutations[i][p], board.getPosition(p) );

      BoardID newid = getBoardID(b);
      if (i==0 || newid<id)
	id=newid;
    }

  return id;
}

