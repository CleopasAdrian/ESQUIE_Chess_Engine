#include "chess.h"
#include <iostream>
#include <cstdint>

using namespace std;

namespace ESQUIE {

void fenString(Board& board, istream& stream) {
  Board copy = board;
  board = Board();
  char c{};
  bool invalid = false;
  bool hasBlackKing = false;
  bool hasWhiteKing = false;
  int rank = 7;
  int file = 0;
  stream >> ws >> noskipws;
  while (!invalid and stream >> c) {
    int square = rank * 8 + file++;
    if (c == 'p') {
      board.bitboards[black][pawn] |= 1ULL << square;
      board.occupancies[black] |= 1ULL << square;
      board.piecePlacement[black][square] = pawn;
    }
    else if (c == 'n') {
      board.bitboards[black][knight] |= 1ULL << square;
      board.occupancies[black] |= 1ULL << square;
      board.piecePlacement[black][square] = knight;
    }
    else if (c == 'b') {
      board.bitboards[black][bishop] |= 1ULL << square;
      board.occupancies[black] |= 1ULL << square;
      board.piecePlacement[black][square] = bishop;
    }
    else if (c == 'r') {
      board.bitboards[black][rook] |= 1ULL << square;
      board.occupancies[black] |= 1ULL << square;
      board.piecePlacement[black][square] = rook;
    }
    else if (c == 'q') {
      board.bitboards[black][queen] |= 1ULL << square;
      board.occupancies[black] |= 1ULL << square;
      board.piecePlacement[black][square] = queen;
    }
    else if (c == 'k') { 
      board.bitboards[black][king] = 1ULL << square; 
      board.occupancies[black] |= 1ULL << square;
      board.piecePlacement[black][square] = king;
      hasBlackKing = true; 
    }
    else if (c == 'P') {
      board.bitboards[white][pawn] |= 1ULL << square;
      board.occupancies[white] |= 1ULL << square;
      board.piecePlacement[white][square] = pawn;
    }
    else if (c == 'N') {
      board.bitboards[white][knight] |= 1ULL << square;
      board.occupancies[white] |= 1ULL << square;
      board.piecePlacement[white][square] = knight;
    }
    else if (c == 'B') {
      board.bitboards[white][bishop] |= 1ULL << square;
      board.occupancies[white] |= 1ULL << square;
      board.piecePlacement[white][square] = bishop;
    }
    else if (c == 'R') {
      board.bitboards[white][rook] |= 1ULL << square;
      board.occupancies[white] |= 1ULL << square;
      board.piecePlacement[white][square] = rook;
    }
    else if (c == 'Q') {
      board.bitboards[white][queen] |= 1ULL << square;
      board.occupancies[white] |= 1ULL << square;
      board.piecePlacement[white][square] = queen;
    }
    else if (c == 'K') { 
      board.bitboards[white][king] = 1ULL << square; 
      board.occupancies[white] |= 1ULL << square;
      board.piecePlacement[white][square] = king;
      hasWhiteKing = true; 
    }
    else if (c >= '1' and c <= '8') file += c - '1';
    else if (c == '/') { --rank; file = 0; }
    else if (c == ' ') break; 
    else { invalid = true; cout << "pieces failed\n";}
  }
  board.occupancies[emptySquares] = ~(board.occupancies[white] | board.occupancies[black]);
  if (!hasWhiteKing or !hasBlackKing) { invalid = true; }
  while (!invalid and stream >> c) {
    if (c == 'w') board.colorToMove = white;
    else if (c == 'b') board.colorToMove = black;
    else if (c == ' ') break;
    else { invalid = true; cout << "color to move failed\n"; }
  }
  board.castlingRights = 0;
  while (!invalid and stream >> c) {
    if (c == 'K') board.castlingRights |= 1;
    else if (c == 'Q') board.castlingRights |= 2;
    else if (c == 'k') board.castlingRights |= 4;
    else if (c == 'q') board.castlingRights |= 8;
    else if (c == '-') board.castlingRights = 0;
    else if (c == ' ') break;
    else { invalid = true; cout << "castling rights failed\n"; }
  }
  while (!invalid and stream >> c) {
    if (c == '-') board.epSquare = 0;
    else if (c >= 'a' and c <= 'h') board.epSquare = c -'a';
    else if (c >= '1' and c <= '8') board.epSquare += (c - '1') * 8;
    else if (c == ' ') break;
    else { invalid = true; cout << "ep square failed"; }
  }
  uint8_t halfMoveClock = 0;
  while (!invalid and stream >> c) {
    if (c >= '0' and c <= '9') halfMoveClock = (halfMoveClock * 10) + (c - '0');
    else break;
  }
  uint16_t fullMoves = 0;
  while (!invalid and stream >> c) {
    if (c >= '0' and c <= '9') fullMoves = (fullMoves * 10) + (c - '0');
    else break;
  }
  if (invalid) {
    board = copy;
    cout << "Invalid FEN string\n\n";
  }
  stream.clear();
  stream >> skipws;
}

} // namespace ESQUIE
