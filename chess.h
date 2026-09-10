#ifndef CHESS_H_INCLUDED
#define CHESS_H_INCLUDED
#include "squares.h"
#include "flags.h"
#include <cstdint>
#include <vector>
#include <array>
#include <chrono>
#include <iostream>
#include <string>

namespace ESQUIE {

// Value is a term used to distinguish a search 
// value from any other integer value.
using Value = int;

enum Color { black = 0, white = 1};
enum Piece { none = 0, pawn, knight, bishop, rook, queen, king };
enum Occupancies { blackPieces = 0, whitePieces = 1, emptySquares = 2 };
enum { FILES = 0, RANKS };

constexpr char SQUARES[2][64] = {
  {
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'
  },
  {
    '1', '1', '1', '1', '1', '1', '1', '1',
    '2', '2', '2', '2', '2', '2', '2', '2',
    '3', '3', '3', '3', '3', '3', '3', '3',
    '4', '4', '4', '4', '4', '4', '4', '4',
    '5', '5', '5', '5', '5', '5', '5', '5',
    '6', '6', '6', '6', '6', '6', '6', '6',
    '7', '7', '7', '7', '7', '7', '7', '7',
    '8', '8', '8', '8', '8', '8', '8', '8',
  }
};

struct Move {
  uint32_t data; // 3-bit piece code / 4-bit flag code / 6-bit 'to' square / 6-bit 'from' square
  Move();
  Move(Piece piece, Flag flag, uint8_t to, uint8_t from);
  bool operator==(const Move& other) const;
  uint8_t from() const;
  uint8_t to() const;
  uint8_t flag() const;
  uint8_t piece() const;
};

std::ostream& operator<<(std::ostream& out, const Move& move);

uint64_t blockmaskRook(int square);

uint64_t blockmaskBishop(int square);

uint64_t blockmaskQueen(int square);

uint64_t moveBoardRook(uint64_t blockers, int square);

uint64_t moveBoardBishop(uint64_t blockers, int square);

uint64_t moveBoardQueen(uint64_t blockers, int square);

void show(uint64_t board);

inline int popLSB(uint64_t& bitBoard) {
  int index = __builtin_ctzll(bitBoard);
  bitBoard &= (bitBoard - 1);
  return index;
}

inline int countLSB(uint64_t& bitBoard) {
  return __builtin_ctzll(bitBoard);
}

inline int popcnt(uint64_t& bitBoard) {
  return __builtin_popcountll(bitBoard);
}

class Knight {
  private:
    std::array<uint64_t, 64> knightAttacks; 
    uint64_t pregenAttacks(uint8_t square);
  public:
   Knight();
   uint64_t getAttacks(uint8_t square);
}; 

class King {
  private:
    std::array<uint64_t, 64> kingAttacks;
    uint64_t pregenAttacks(uint8_t square);
  public:
    King();
    uint64_t getAttacks(uint8_t square);
};

class Pawn {
  private:
    std::array<std::array<uint64_t, 64>, 2> pawnAttacks;
    uint64_t pregenWhiteAttacks(uint8_t square);
    uint64_t pregenBlackAttacks(uint8_t square);
  public:
    Pawn();
    uint64_t getAttacks(uint8_t color, uint8_t square);
};

struct Magic {
  uint64_t mask;  // mask for relevant blockers
  uint64_t magic; // magic 64-bit factor
  int shift;      // shift for fancy magics - necessary since the magics were generated that way
  int offset;     // offset into main lookup array
};

class Bishop {
  public:
    std::array<Magic, 64> magics;
    std::array<std::array<uint8_t, 512>, 64> magicRedirect; // 32 K
    std::array<uint64_t, 1428> lookup; // 11 K
  public:
    Bishop();
    uint64_t getAttacks(uint64_t occupancy, uint8_t square);
};

class Rook {
  public:
    std::array<Magic, 64> magics;
    std::array<std::array<uint8_t, 4096>, 64> magicRedirect; // 256 K
    std::array<uint64_t, 4900> lookup; // 39 K
  public:
    Rook();
    uint64_t getAttacks(uint64_t occupancy, uint8_t square);
};

class Queen {
  private:
    Rook& rookLookup;
    Bishop& bishopLookup;
  public:
    Queen() = delete;
    Queen(Rook& rooks, Bishop& bishops);
    uint64_t getAttacks(uint64_t occupancy, uint8_t square);
};

struct LookupTables {
  Pawn pawn;
  Knight knight;
  Bishop bishop;
  Rook rook;
  Queen queen;
  King king;
  LookupTables();
};

struct PrevState {
  uint8_t capturedPiece;
  uint8_t castlingRights;
  uint8_t epSquare;
  // uint8_t halfMoveClock;
};

class Board {
  public:
  std::array<std::array<uint64_t, 7>, 2> bitboards; // white & black bitboards (includes two dummy bitboards)
  std::array<uint64_t, 3> occupancies; // collective bitboards, e.g. all white or all black pieces
  std::array<std::array<uint8_t, 64>, 2> piecePlacement;
  Color colorToMove;
  uint8_t castlingRights;
  std::array<uint8_t, 64> castlingMask;
  int8_t epSquare; // 0 if no en-passant square exists
  Board();
  Board(const Board& other);
  Board(Board&& other);
  Board& operator=(const Board& other);
  Board& operator=(Board&& other);
  ~Board();
  uint64_t getWhite() const;
  uint64_t getBlack() const;
  uint64_t getEmptySquares() const;
  void setStartPosition();
  template <Color Us>
    void makeMove(const Move& move, std::array<PrevState, 100>& history, uint8_t& ply);
  template <Color Us>
    void undoMove(const Move& move, const std::array<PrevState, 100>& history, uint8_t& ply);
  void moveGen(LookupTables& tables, std::array<Move, 255>& moves, uint8_t& length);
  void printBoard();
  bool isSquareAttacked(LookupTables& tables, int square, Color attackerColor);
  void findPinnedToKing(LookupTables& tables, uint8_t& square, Color Us, uint64_t& diagonals, uint64_t& straights);
};

inline bool Board::isSquareAttacked(LookupTables& tables, int square, Color attackerColor) {
  uint64_t all = ~occupancies[emptySquares];
  if (tables.knight.getAttacks(square) & bitboards[attackerColor][knight]) return true;
  if (tables.king.getAttacks(square) & bitboards[attackerColor][king]) return true;
  if (tables.pawn.getAttacks(!attackerColor, square) & bitboards[attackerColor][pawn]) return true;
  if (tables.bishop.getAttacks(all, square) &
      (bitboards[attackerColor][bishop] | bitboards[attackerColor][queen])) return true;
  if (tables.rook.getAttacks(all, square) &
      (bitboards[attackerColor][rook] | bitboards[attackerColor][queen])) return true;
  return false;
}

inline void Board::findPinnedToKing(LookupTables& tables, uint8_t& square, Color Us, uint64_t& diagonalPins, uint64_t& straightPins) {
  Color Them = (Us == white) ? black : white;
  uint64_t diagonalFriends = tables.bishop.getAttacks(occupancies[emptySquares], square) & occupancies[Us];
  uint64_t diagonalEnemies = tables.bishop.magics[square].mask & (bitboards[Them][bishop] | bitboards[Them][queen]);
}

template <Color Us>
inline void Board::makeMove(const Move& move, std::array<PrevState, 100>& history, uint8_t& ply) {
  constexpr Color Them = (Us == white) ? black : white;
  const uint8_t from = move.from();
  const uint8_t to = move.to();
  const uint64_t fromMask = 1ULL << from;
  const uint64_t toMask = 1ULL << to;
  const uint64_t moveMask = fromMask | toMask;
  const uint8_t flag = move.flag();
  const uint8_t movedPiece = move.piece();
  const uint8_t capturedPiece = toMask & bitboards[Them][pawn]   ? pawn
                              : toMask & bitboards[Them][knight] ? knight
                              : toMask & bitboards[Them][bishop] ? bishop
                              : toMask & bitboards[Them][rook]   ? rook
                              : toMask & bitboards[Them][queen]  ? queen
                              : none;
  history[ply].capturedPiece = capturedPiece;
  history[ply].castlingRights = castlingRights;
  history[ply].epSquare = epSquare;
  ++ply;
  bitboards[Us][movedPiece] ^= moveMask;
  bitboards[Them][capturedPiece] ^= toMask;
  occupancies[Us] ^= moveMask;
  occupancies[Them] &= ~moveMask;
  occupancies[emptySquares] ^= moveMask;
  occupancies[emptySquares] &= ~toMask;
  castlingRights &= castlingMask[from] & castlingMask[to];
  epSquare = 0;
  if (flag != QUIET and flag != CAPTURE) {
    if (flag == DOUBLEPP) {
      epSquare = (from + to) / 2;
    }
    else if (flag == ENPASSANT) {
      int capSquare = (Us == white ? to - 8 : to + 8);
      uint64_t capMask = 1ULL << capSquare;
      bitboards[Them][pawn] &= ~capMask;
      occupancies[Them] &= ~capMask;
      occupancies[emptySquares] |= capMask;
    }
    else if (flag == CASTLING) {
      int rookFrom = (to == 6) ? 7 : (to == 2) ? 0 : (to == 62) ? 63 : 56;
      int rookTo = (to == 6) ? 5 : (to == 2) ? 3 : (to == 62) ? 61 : 59;
      uint64_t rookMask = (1ULL << rookFrom) | (1ULL << rookTo);
      bitboards[Us][rook] ^= rookMask;
      occupancies[Us] ^= rookMask;
      occupancies[emptySquares] ^= rookMask;
    }
    else if (flag & 8) {
      bitboards[Us][pawn] &= ~toMask;
      uint8_t promoPiece = (flag & 3) + 2;
      bitboards[Us][promoPiece] |= toMask;
    }
  }
  colorToMove = Them;
}

template <Color Us>
inline void Board::undoMove(const Move& move, const std::array<PrevState, 100>& history, uint8_t& ply) {
  constexpr Color Them = (Us == white) ? black : white;
  --ply;
  const uint8_t from = move.from();
  const uint8_t to = move.to();
  const uint8_t flag = move.flag();
  const uint64_t fromMask = 1ULL << from;
  const uint64_t toMask = 1ULL << to;
  const uint64_t moveMask = fromMask | toMask;
  const uint8_t capturedPiece = history[ply].capturedPiece;
  castlingRights = history[ply].castlingRights;
  epSquare = history[ply].epSquare;
  const uint8_t movedPiece = move.piece();
  if (flag < NPROMO) {
    bitboards[Us][movedPiece] ^= moveMask;
  } else {
    uint8_t promoted = (flag & 3) + 2;
    bitboards[Us][promoted] ^= toMask;
    bitboards[Us][pawn] ^= fromMask;
  }
  bitboards[Them][capturedPiece] |= toMask;
  occupancies[Us] ^= moveMask;
  if (capturedPiece) occupancies[Them] |= toMask;
  occupancies[emptySquares] = ~(occupancies[Us] | occupancies[Them]);
  if (flag != QUIET and flag != CAPTURE) {
    if (flag == ENPASSANT) {
      int capSquare = to + (Us == white ? -8 : 8);
      uint64_t capMask = 1ULL << capSquare;
      bitboards[Them][pawn] |= capMask;
      occupancies[Them] |= capMask;
      occupancies[emptySquares] &= ~capMask;
    }
    else if (flag == CASTLING) {
      int rookFrom = (to == 6) ? 7 : (to == 2) ? 0 : (to == 62) ? 63 : 56;
      int rookTo = (to == 6) ? 5 : (to == 2) ? 3 : (to == 62) ? 61 : 59;
      uint64_t rookMask = (1ULL << rookFrom) | (1ULL << rookTo);
      bitboards[Us][rook] ^= rookMask;
      occupancies[Us] ^= rookMask;
      occupancies[emptySquares] ^= rookMask;
    }
  }
  colorToMove = Us;
}

class SearchEngine {
  public:
  LookupTables tables;
  Board board;
  std::array<std::array<Move, 255>, 100> moveLists;
  std::array<uint8_t, 100> moveCounts;
  std::array<PrevState, 100> history;
  std::array<Board, 100> boardStates;
  uint8_t ply;
  Move bestMove;
  Value eval;
  SearchEngine();
  template <Color Us>
    uint64_t perft(int depth);
  template <Color Us>
    void perftDetailed(int depth, uint64_t& captures, uint64_t& promotions, uint64_t& enpassant, uint64_t& castles);
  template <Color Us>
    void perftDivide(int depth);
  void makeMove(const Move& move);
  void undoMove(const Move& move);
  bool isLegal();
  void moveGen();
  bool isKingAttacked();
  Value materialEval();
};

inline bool SearchEngine::isLegal() {
  auto& bb = board.bitboards;
  Color Us = board.colorToMove ? black : white;
  Color Them = Us ? black : white;
  uint64_t kingSquare = __builtin_ctzll(bb[Us][king]);
  if (!board.isSquareAttacked(tables, kingSquare, Them)) return true;
  return false;
}

template <Color Us>
uint64_t SearchEngine::perft(int depth) {
  if (depth == 0) return 1ULL;
  constexpr Color Them = (Us == white) ? black : white;
  uint8_t& count = moveCounts[ply];
  count = 0;
  board.moveGen(tables, moveLists[ply], count);
  uint64_t nodes = 0;
  for (int i = 0; i < count; ++i) {
    Move move = moveLists[ply][i];
    board.makeMove<Us>(move, history, ply);
    uint8_t kingSquare = __builtin_ctzll(board.bitboards[Us][king]);
    if (!board.isSquareAttacked(tables, kingSquare, Them)) {
      nodes += perft<Them>(depth - 1);
    }
    board.undoMove<Us>(move, history, ply);
  }
  return nodes;
}

template <Color Us>
void SearchEngine::perftDetailed(int depth, uint64_t& captures, uint64_t& promotions, uint64_t& enpassant, uint64_t& castles) {
  if (depth == 0) return;
  constexpr Color Them = (Us == white) ? black : white;
  uint8_t& count = moveCounts[ply];
  count = 0;
  board.moveGen(tables, moveLists[ply], count);
  for (int i = 0; i < count; ++i) {
    Move move = moveLists[ply][i];
    board.makeMove<Us>(move, history, ply);
    uint8_t kingSquare = __builtin_ctzll(board.bitboards[Us][king]);
    if (!board.isSquareAttacked(tables, kingSquare, Them)) {
      perftDetailed<Them>(depth - 1, captures, promotions, enpassant, castles);
      if (move.flag() == ENPASSANT) ++enpassant;
      if (move.flag() & 8) ++promotions;
      if (move.flag() & CAPTURE) ++captures;
      else if (move.flag() == CASTLING) ++castles;
    }
    board.undoMove<Us>(move, history, ply);
  }
}

template <Color Us>
void SearchEngine::perftDivide(int depth) {
  constexpr Color Them = (Us == white) ? black : white;
  uint8_t count = 0;
  board.moveGen(tables, moveLists[ply], count);
  for (int i = 0; i < count; ++i) {
    bool legal = false;
    uint64_t nodes = 0;
    Move move = moveLists[ply][i];
    board.makeMove<Us>(move, history, ply);
    uint8_t kingSquare = __builtin_ctzll(board.bitboards[Us][king]);
    if (!board.isSquareAttacked(tables, kingSquare, Them)) {
      legal = true;
      nodes = perft<Them>(depth - 1);
    }
    board.undoMove<Us>(move, history, ply);
    if (legal) {
      std::cout << move << " : " << nodes << std::endl;
    }
  }
}

void fenString(Board& board, std::istream& stream);

}; // namespace ESQUIE

#endif // #ifndef CHESS_H_INCLUDED
