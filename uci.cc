#include "chess.h"
#include "evaluate.h"
#include <string>
#include <sstream>
#include <iostream>
#include <thread>
#include <atomic>
#include <random>

using namespace std;

namespace ESQUIE {

atomic<bool> isThinking(false);
atomic<bool> stopSearch(false);

thread searchThread;

void fenMove(SearchEngine* engine, uint8_t fromSquare, uint8_t toSquare, uint8_t promoType) {
  engine->board.moveGen(engine->tables, engine->moveLists[engine->ply], engine->moveCounts[engine->ply]);
  for (uint8_t i = 0; i < engine->moveCounts[engine->ply]; ++i) {
    Move& move = engine->moveLists[engine->ply][i];
    uint8_t from = move.from();
    uint8_t to = move.to();
    uint8_t promo = 0;
    if (move.flag() >= NPROMO) {
      promo = (move.flag() & 3) + 2;
    }
    if (fromSquare == from and toSquare == to and promoType == promo) {
      engine->makeMove(move);
      if (engine->isLegal()) return;
      engine->undoMove(move);
      return;
    }
  }
  engine->ply = 0;
}

Value negamax(SearchEngine* engine, uint8_t depth) {
  engine->moveGen();

  // Base case. Needs to ackwardly check for checkmate/stalemate 
  // because of pseudo-legal move generation.
  if (depth == 0) {
    bool legalExists = false;
    for (uint8_t moveCount = 0; moveCount < engine->moveCounts[engine->ply]; ++moveCount) {
      const Move& move = engine->moveLists[engine->ply][moveCount];
      engine->makeMove(move);
      if (engine->isLegal()) {
        legalExists = true;
        engine->undoMove(move);
        break;
      }
      engine->undoMove(move);
    }
    if (!legalExists && engine->isKingAttacked())
      return -VALUE_MATE + engine->ply;
    else if (!legalExists)
      return VALUE_DRAW;
    else 
      return engine->materialEval();
  }

  Value maxEval = -VALUE_INFINITE;
  uint8_t legal = engine->moveCounts[engine->ply];
  for (uint8_t moveCount = 0; moveCount < engine->moveCounts[engine->ply]; ++moveCount) {
    const Move& move = engine->moveLists[engine->ply][moveCount];
    engine->makeMove(move);
    if (engine->isLegal()) 
    {
      Value currentEval = -negamax(engine, depth - 1);
      engine->undoMove(move);
      if (currentEval > maxEval) 
        maxEval = currentEval;
    }
    else {
      --legal;
      engine->undoMove(move);
    }
  }
  if (legal == 0 && engine->isKingAttacked())
    return -VALUE_MATE + engine->ply;
  else if (legal == 0)
    return VALUE_DRAW;
  return maxEval;
}

void search(SearchEngine* engine, bool infinite) {
  isThinking = true;
  engine->ply = 0;
  int depth = 4;
  engine->eval = -VALUE_INFINITE;
  engine->moveGen();
  uint8_t legal = engine->moveCounts[engine->ply];
  for (uint8_t moveCount = 0; moveCount < engine->moveCounts[engine->ply]; ++moveCount) {
    const Move& move = engine->moveLists[engine->ply][moveCount];
    engine->makeMove(move);
    if (engine->isLegal()) {
      Value eval = -negamax(engine, depth - 1);
      if (eval > engine->eval) {
        engine->eval = eval;
        engine->bestMove = move;
      }
      std::cout << "info score cp " << engine->eval << "\n" << std::flush;
      engine->undoMove(move);
    } 
    else {
      --legal;
      engine->undoMove(move);
    }
  }
  if (legal == 0 && engine->isKingAttacked())
    engine->eval = -VALUE_MATE;
  else if (legal == 0)
    engine->eval = VALUE_DRAW;
  if (engine->board.colorToMove == black)
    engine->eval = -engine->eval;
  std::cout 
    << "info score cp " << engine->eval
    << " depth "        << depth 
    << "\n"             << std::flush;
  if (!infinite)
    std::cout << "bestmove " << engine->bestMove << "\n" << std::flush;
  isThinking = false;
}


void randomMoveGenerator(SearchEngine* engine, bool infinite) {
  isThinking = true;
  engine->ply = 0;
  uint8_t& moveCount = engine->moveCounts[engine->ply];
  bool legal = false;
  engine->board.moveGen(engine->tables, engine->moveLists[engine->ply], moveCount);
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_int_distribution<uint8_t> distr(0, moveCount - 1);
  uint8_t randomMoveIndex = distr(gen);
  Move move = engine->moveLists[engine->ply][randomMoveIndex];
  engine->makeMove(move);
  if (engine->isLegal()) {
    engine->bestMove = move;
    engine->eval = engine->materialEval();
    engine->undoMove(move);
  }
  else {
    engine->undoMove(move);
    for (uint8_t i = 0; i < moveCount; ++i) {
      move = engine->moveLists[engine->ply][i];
      engine->makeMove(move);
      if (engine->isLegal()) {
        engine->bestMove = move;
        engine->eval = engine->materialEval();
        engine->undoMove(move);
        legal = true;
        break;
      }
      else {
        engine->undoMove(move);
      }
    }
    if (!legal)
      engine->eval = -VALUE_MATE;
  }
  std::cout << "info score cp " << engine->eval << "\n" << std::flush;
  if (!infinite)
    std::cout << "bestmove " << engine->bestMove << "\n" << std::flush;
  isThinking = false;
}

void uciInfo() {
  std::cout 
    << "id name ESQUIE 0.1\n"
    << "id author Cameron S. Ratte\n" 
    << std::flush;
}

void uciLoop(std::stringstream& stream) {
  bool initialized = false;
  bool running = true;
  SearchEngine* engine = NULL;
  std::string buffer;
  while (running && std::getline(std::cin, buffer)) {
    if (buffer.empty()) 
      continue;
    std::stringstream stream(buffer);
    std::string command;
    while (stream >> command) {
      if (command == "isready") {
        if (!initialized) {
          static SearchEngine se;
          engine = &se;
          initialized = true;
        }
        cout << "readyok\n" << flush;
      }
      else if (command == "position") {
        std::string postype;
        stream >> postype;
        if (postype == "startpos") engine->board.setStartPosition();
        else if (postype == "fen") fenString(engine->board, stream);
      }
      else if (command == "moves") {
        std::string moveString;
        while (stream >> moveString) {
          char fromFile = moveString[0];
          char fromRank = moveString[1];
          char toFile = moveString[2];
          char toRank = moveString[3];
          char promo = (moveString.length() == 5) ? moveString[4] : none;
          uint8_t fromSquare = ((fromFile - 'a') % 8) + ((fromRank - '1') * 8);
          uint8_t toSquare = ((toFile - 'a') % 8) + ((toRank - '1') * 8);
          if (promo == 'n') promo = knight;
          else if (promo == 'b') promo = bishop;
          else if (promo == 'r') promo = rook;
          else if (promo == 'q') promo = queen;
          fenMove(engine, fromSquare, toSquare, promo);
        }
      }
      else if (command == "go") {
        int wtime, btime, winc, binc = 0;
        bool infinite = false;
        while (stream >> command) {
          if (command == "wtime")
            stream >> wtime;
          else if (command == "btime")
            stream >> btime;
          else if (command == "winc")
            stream >> winc;
          else if (command == "binc")
            stream >> binc;
          else if (command == "infinite")
            infinite = true;
        }
        if (searchThread.joinable()) {
          searchThread.join();
        }
        stopSearch = false;
        searchThread = thread(search, engine, infinite);
        //search(engine, infinite);
        //randomMoveGenerator(engine, infinite);
      }
      else if (command == "perft") {
        int depth;
        stream >> depth;
        if (engine->board.colorToMove) std::cout << engine->perft<white>(depth);
        else std::cout << engine->perft<black>(depth);
        std::cout << std::endl;
      }
      else if (command == "showboard") {
        engine->board.printBoard();
      }
      else if (command == "stop" && isThinking) {
        stopSearch = true;
        if (searchThread.joinable()) {
          searchThread.join();
        }
        std::cout << "bestmove " << engine->bestMove << "\n" << std::flush;
      }
      else if (command == "quit") {
        stopSearch = true;
        if (searchThread.joinable()) {
          searchThread.join();
        }
        return;
      }
      else stream.ignore();
    }
  }
  if (searchThread.joinable()) {
    searchThread.detach();
  }
}

} // namespace ESQUIE

int main() {
  string buffer = "";
  getline(cin, buffer);
  stringstream stream(buffer);
  string command = "";
  while (stream >> command) {
    if (command == "uci") {
      ESQUIE::uciInfo();
      std::cout << "uciok\n" << std::flush;
      ESQUIE::uciLoop(stream);
      return 0;
    }
  }
}
