#include <iostream>
#include "chess.h"
using namespace std;

namespace ESQUIE {

Board::Board() :
  bitboards{0},
  occupancies{ 0, 0, 0xffffffffffffffff },
  piecePlacement{},
  colorToMove{white},
  castlingRights{0},
  castlingMask{
    13, 15, 15, 15, 12, 15, 15, 14,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    7,  15, 15, 15, 3,  15, 15, 11
  },
  epSquare{0} {}

Board::Board(const Board& other) = default;

Board::Board(Board&& other) = default;

Board& Board::operator=(const Board& other) = default;

Board& Board::operator=(Board&& other) = default;

Board::~Board() = default;

void Board::setStartPosition() {
  bitboards[white][pawn] = 0x000000000000FF00;
  bitboards[white][rook] = 0x0000000000000081;
  bitboards[white][knight] = 0x0000000000000042;
  bitboards[white][bishop] = 0x0000000000000024;
  bitboards[white][queen] = 0x0000000000000008;
  bitboards[white][king] = 0x0000000000000010;
  bitboards[black][pawn] = 0x00FF000000000000;
  bitboards[black][rook] = 0x8100000000000000;
  bitboards[black][knight] = 0x4200000000000000;
  bitboards[black][bishop] = 0x2400000000000000;
  bitboards[black][queen] = 0x0800000000000000;
  bitboards[black][king] = 0x1000000000000000;
  occupancies[whitePieces] = 0x000000000000FFFF;
  occupancies[blackPieces] = 0xFFFF000000000000;
  occupancies[emptySquares] = 0x0000FFFFFFFF0000;
  piecePlacement[white] = { 
    rook,  knight, bishop, queen, king,  bishop, knight, rook,
    pawn,  pawn,   pawn,   pawn,  pawn,  pawn,   pawn,   pawn,
    none, none,  none,  none, none, none,  none,  none,
    none, none,  none,  none, none, none,  none,  none,
    none, none,  none,  none, none, none,  none,  none,
    none, none,  none,  none, none, none,  none,  none,
    none, none,  none,  none, none, none,  none,  none,
    none, none,  none,  none, none, none,  none,  none,
  };
  piecePlacement[black] = {
    none, none,  none,  none, none, none,  none,  none,
    none, none,  none,  none, none, none,  none,  none,
    none, none,  none,  none, none, none,  none,  none,
    none, none,  none,  none, none, none,  none,  none,
    none, none,  none,  none, none, none,  none,  none,
    none, none,  none,  none, none, none,  none,  none,
    pawn,  pawn,   pawn,   pawn,  pawn,  pawn,   pawn,   pawn,
    rook,  knight, bishop, queen, king,  bishop, knight, rook
  };                  
  colorToMove = white;
  castlingRights = 15;
  epSquare = 0;
}

uint64_t Board::getWhite() const {
  return occupancies[whitePieces];
}

uint64_t Board::getBlack() const {
  return occupancies[blackPieces];
}

uint64_t Board::getEmptySquares() const {
  return occupancies[emptySquares];
}

} // namespace ESQUIE
