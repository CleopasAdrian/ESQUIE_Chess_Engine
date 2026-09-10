#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <bitset>
#include "chess.h"

using namespace std;

namespace ESQUIE {

uint64_t blockmaskRook(int square) {
  uint64_t mask = 0;
  for (int i = 1; i < 7; ++i) {
    int rank = square / 8;
    int file = square % 8;
    if (rank + i < 7) mask |= 1ULL << (square + i * 8);
    if (rank - i > 0) mask |= 1ULL << (square - i * 8);
    if (file + i < 7) mask |= 1ULL << (square + i);
    if (file - i > 0) mask |= 1ULL << (square - i);
  }
  return mask;
}

uint64_t blockmaskBishop(int square) {
  uint64_t mask = 0;
  for (int i = 1; i < 7; ++i) {
    int rank = square / 8;
    int file = square % 8;
    if (rank + i < 7 && file + i < 7) mask |= 1ULL << (square + (i * 8) + i);
    if (rank + i < 7 && file - i > 0) mask |= 1ULL << (square + (i * 8) - i);
    if (rank - i > 0 && file + i < 7) mask |= 1ULL << (square - (i * 8) + i);
    if (rank - i > 0 && file - i > 0) mask |= 1ULL << (square - (i * 8) - i);
  }
  return mask;
}

uint64_t blockmaskQueen(int square) {
  return blockmaskRook(square) | blockmaskBishop(square);
}

uint64_t moveBoardRook(uint64_t blockers, int square) {
  uint64_t moves = 0;
  uint64_t bit = 0;
  int rank = square / 8;
  int file = square % 8;
  for (int i = 1; file + i < 8; ++i) {
    bit = 1ULL << square + i;
    if (bit & ~blockers) moves |= bit;
    else {moves |= bit; break;}
  }
  for (int i = 1; file - i >= 0; ++i) {
    bit = 1ULL << square - i;
    if (bit & ~blockers) moves |= bit;
    else {moves |= bit; break;}
  }
  for (int i = 1; rank + i < 8; ++i) {
    bit = 1ULL << square + (i * 8);
    if (bit & ~blockers) moves |= bit;
    else {moves |= bit; break;}
  }
  for (int i = 1; rank - i >=0; ++i) {
    bit = 1ULL << square - (i * 8);
    if (bit & ~blockers) moves |= bit;
    else {moves |= bit; break;}
  }
  return moves;
}

uint64_t moveBoardBishop(uint64_t blockers, int square) {
  uint64_t moves = 0;
  uint64_t bit = 0;
  int rank = square / 8;
  int file = square % 8;
  for (int i = 1; file + i < 8 && rank + i < 8; ++i) {
    bit = 1ULL << square + (i * 8) + i;
    if (bit & ~blockers) moves |= bit;
    else {moves |= bit; break;}
  }
  for (int i = 1; file - i >= 0 && rank + i < 8; ++i) {
    bit = 1ULL << square + (i * 8) - i;
    if (bit & ~blockers) moves |= bit;
    else {moves |= bit; break;}
  }
  for (int i = 1; file + i < 8 && rank - i >= 0; ++i) {
    bit = 1ULL << square - (i * 8) + i;
    if (bit & ~blockers) moves |= bit;
    else {moves |= bit; break;}
  }
  for (int i = 1; file - i >= 0 && rank - i >= 0; ++i) {
    bit = 1ULL << square - (i * 8) - i;
    if (bit & ~blockers) moves |= bit;
    else {moves |= bit; break;}
  }
  return moves;
}

uint64_t moveBoardQueen(uint64_t blockers, int square) {
  return moveBoardRook(blockers, square) | moveBoardBishop(blockers, square);
}

inline void extractMoves(const Piece& pieceType, uint8_t from, uint64_t& moveBoard, uint64_t& enemies, std::array<Move, 255>& moves, uint8_t& count) {
  uint64_t captures = moveBoard & enemies;
  uint64_t quiets = moveBoard & ~enemies;
  while (captures) {
    moves[count++] = Move(pieceType, CAPTURE, popLSB(captures), from);
  }
  while (quiets) {
    moves[count++] = Move(pieceType, QUIET, popLSB(quiets), from);
  }
}

template <Color Us>
void generatePseudoLegal(Board& board, LookupTables& tables, std::array<Move, 255>& moves, uint8_t& count) {
  constexpr Color Them = (Us == white) ? black : white;
  uint64_t empty = board.occupancies[emptySquares];
  uint64_t enemies = board.occupancies[Them];
  uint64_t friends = board.occupancies[Us];
  uint64_t all = ~empty;

  // --- 1. PAWNS ---
  uint64_t pawns = board.bitboards[Us][pawn];
  constexpr uint64_t promoRank = (Us == white) ? 0xff00000000000000 : 0x00000000000000ff;
  uint64_t singlePushes = (Us == white) ? (pawns << 8) & empty : (pawns >> 8) & empty;
  uint64_t promoPushes = singlePushes & promoRank;
  uint64_t normalPushes = singlePushes & ~promoRank;
  uint64_t doublePushes = 0;
  if constexpr (Us == white) {
    doublePushes = ((singlePushes & 0x0000000000ff0000) << 8) & empty;
  } else {
    doublePushes = ((singlePushes & 0x0000ff0000000000) >> 8) & empty;
  }
  while (normalPushes) {
    int to = popLSB(normalPushes);
    moves[count++] = Move(pawn, QUIET, to, to + (Us == white ? -8 : 8));
  }
  while (promoPushes) {
    int to = popLSB(promoPushes);
    int from = to + (Us == white ? -8: 8);
    moves[count++] = Move(pawn, NPROMO, to, from);
    moves[count++] = Move(pawn, BPROMO, to, from);
    moves[count++] = Move(pawn, RPROMO, to, from);
    moves[count++] = Move(pawn, QPROMO, to, from);
  }
  while (doublePushes) {
    int to = popLSB(doublePushes);
    moves[count++] = Move(pawn, DOUBLEPP, to, to + (Us == white ? -16 : 16));
  }

  // Pawn captures & En Passant
  uint64_t pawnCaps = pawns;
  while (pawnCaps) {
    int from = popLSB(pawnCaps);
    uint64_t attacks = tables.pawn.getAttacks(Us, from);
    uint64_t validCaps = attacks & enemies;
    uint64_t normalCaps = validCaps & ~promoRank;
    uint64_t promoCaps = validCaps & promoRank;
    while (normalCaps) {
      moves[count++] = Move(pawn, CAPTURE, popLSB(normalCaps), from);
    }
    while (promoCaps) {
      int to = popLSB(promoCaps);
      moves[count++] = Move(pawn, NPROMOCAP, to, from);
      moves[count++] = Move(pawn, BPROMOCAP, to, from);
      moves[count++] = Move(pawn, RPROMOCAP, to, from);
      moves[count++] = Move(pawn, QPROMOCAP, to, from);
    }
    if (board.epSquare) {
      if (attacks & (1ULL << board.epSquare)) {
        moves[count++] = Move(pawn, ENPASSANT, board.epSquare, from);
      }
    }
  }

  // --- 2. KNIGHTS ---
  uint64_t knights = board.bitboards[Us][knight];
  while (knights) {
    int from = popLSB(knights);
    uint64_t attacks = tables.knight.getAttacks(from) & ~friends;
    extractMoves(knight, from, attacks, enemies, moves, count);
  }

  // --- 3. BISHOPS ---
  uint64_t bishops = board.bitboards[Us][bishop];
  while (bishops) {
    int from = popLSB(bishops);
    uint64_t attacks = tables.bishop.getAttacks(all, from) & ~friends;
    extractMoves(bishop, from, attacks, enemies, moves, count);
  }

  // --- 4. ROOKS ---
  uint64_t rooks = board.bitboards[Us][rook];
  while (rooks) {
    int from = popLSB(rooks);
    uint64_t attacks = tables.rook.getAttacks(all, from) & ~friends;
    extractMoves(rook, from, attacks, enemies, moves, count);
  }

  // --- 5. QUEENS ---
  uint64_t queens = board.bitboards[Us][queen];
  while (queens) {
    int from = popLSB(queens);
    uint64_t attacks = tables.queen.getAttacks(all, from) & ~friends;
    extractMoves(queen, from, attacks, enemies, moves, count);
  }

  // --- 6. KINGS ---
  uint64_t kings = board.bitboards[Us][king];
  int kingSquare = popLSB(kings);
  uint64_t attacks = tables.king.getAttacks(kingSquare) & ~friends;
  extractMoves(king, kingSquare, attacks, enemies, moves, count);

// --- 7. CASTLING ---
  if constexpr (Us == white) {
    if ((board.castlingRights & 1) and !(all & 0x0000000000000060)) {
      if (!board.isSquareAttacked(tables, e1, black) and !board.isSquareAttacked(tables, f1, black)) {
        moves[count++] = Move(king, CASTLING, g1, e1);
      }
    }
    if ((board.castlingRights & 2) and !(all & 0x000000000000000E)) {
      if (!board.isSquareAttacked(tables, e1, black) and !board.isSquareAttacked(tables, d1, black)) {
        moves[count++] = Move(king, CASTLING, c1, e1);
      }
    }
  } else {
    if ((board.castlingRights & 4) and !(all & 0x6000000000000000)) {
      if (!board.isSquareAttacked(tables, e8, white) and !board.isSquareAttacked(tables, f8, white)) {
        moves[count++] = Move(king, CASTLING, g8, e8);
      }
    }
    if ((board.castlingRights & 8) and !(all & 0x0E00000000000000)) {
      if (!board.isSquareAttacked(tables, e8, white) and !board.isSquareAttacked(tables, d8, white)) {
        moves[count++] = Move(king, CASTLING, c8, e8);
      }
    }
  }
}

void Board::moveGen(LookupTables& tables, std::array<Move, 255>& moves, uint8_t& length) {
  uint8_t count = 0;
  if (colorToMove == white) generatePseudoLegal<white>(*this, tables, moves, count);
  else generatePseudoLegal<black>(*this, tables, moves, count);
  length = count;
}

inline int inputMove() {
  int select = 0;
  std::cin >> select;
  return select - 1;
}

} // namespace ESQUIE
