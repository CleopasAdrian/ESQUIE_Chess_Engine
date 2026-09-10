#include "chess.h"
#include <array>
#include <cstdint>

namespace ESQUIE {

SearchEngine::SearchEngine() : tables(), board(), moveLists{}, moveCounts{}, history{}, ply{}, bestMove{}, eval{} {
  board.setStartPosition();
}

void SearchEngine::moveGen() {
  board.moveGen(tables, moveLists[ply], moveCounts[ply]);
}

bool SearchEngine::isKingAttacked() {
  Color Us = board.colorToMove == white ? white : black;
  Color Them = Color(Us ^ 1);
  uint8_t kingsquare = countLSB(board.bitboards[Us][king]);
  return board.isSquareAttacked(tables, kingsquare, Them);
}

void SearchEngine::makeMove(const Move& move) {
  if (board.colorToMove == white) {
    board.makeMove<white>(move, history, ply);
  } else {
    board.makeMove<black>(move, history, ply);
  }
}

void SearchEngine::undoMove(const Move& move) {
  if (board.colorToMove == white) {
    board.undoMove<black>(move, history, ply);
  } else { 
    board.undoMove<white>(move, history, ply);
  }
}

} // namespace ESQUIE
