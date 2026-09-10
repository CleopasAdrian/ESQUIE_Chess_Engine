#include "chess.h"

#ifndef EVALUATE_H_INCLUDED
#define EVALUATE_H_INCLUDED

namespace ESQUIE {

// Value is an alias for int. Used to distinguish search values
// from any other integer values.
using Value = int;

constexpr Value PawnValue = 100;
constexpr Value KnightValue = 300;
constexpr Value BishopValue = 300;
constexpr Value RookValue = 500;
constexpr Value QueenValue = 900;

constexpr Value VALUE_ZERO = 0;
constexpr Value VALUE_DRAW = 0;
constexpr Value VALUE_MATE = 32000;
constexpr Value VALUE_INFINITE = 32001;
constexpr Value VALUE_NONE = 32002;

Value SearchEngine::materialEval() {

  Color Us = board.colorToMove == white ? white : black;
  Color Them = Color(Us ^ 1);

  Value material = 0;

  material += popcnt(board.bitboards[Us][pawn]) * PawnValue;
  material += popcnt(board.bitboards[Us][knight]) * KnightValue;
  material += popcnt(board.bitboards[Us][bishop]) * BishopValue;
  material += popcnt(board.bitboards[Us][rook]) * RookValue;
  material += popcnt(board.bitboards[Us][queen]) * QueenValue;

  material -= popcnt(board.bitboards[Them][pawn]) * PawnValue;
  material -= popcnt(board.bitboards[Them][knight]) * KnightValue;
  material -= popcnt(board.bitboards[Them][bishop]) * BishopValue;
  material -= popcnt(board.bitboards[Them][rook]) * RookValue;
  material -= popcnt(board.bitboards[Them][queen]) * QueenValue;

  return material;
}

} // namespace ESQUIE

#endif // #ifndef EVALUATE_H_INCLUDED
