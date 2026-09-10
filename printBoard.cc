#include <iostream>
#include <cstdint>
#include <string>
#include "chess.h"
using namespace ESQUIE;
const std::string PIECE_SYMBOLS[2][7] = {
  {"   ", " ♟ ", " ♞ ", " ♝ ", " ♜ ", " ♛ ", " ♚ "},  // black pieces
  {"   ", " ♙ ", " ♘ ", " ♗ ", " ♖ ", " ♕ ", " ♔ "}  // white pieces
};

const std::string BG_LIGHT = "\033[48;5;223m"; // light biege
const std::string BG_DARK = "\033[48;5;94m";   // dark brown
const std::string BG_RESET = "\033[0m";

const std::string TXT_WHITE = "\033[38;5;255m";
const std::string TXT_BLACK = "\033[38;5;0m";

void Board::printBoard() {
  char boardArray[2][64] = {};
  for (int pieceType = pawn; pieceType <= king; ++pieceType) {
    uint64_t whiteMask = bitboards[white][pieceType];
    uint64_t blackMask = bitboards[black][pieceType];
    while (whiteMask) {
      int square = popLSB(whiteMask);
      boardArray[white][square] = pieceType;
    }
    while (blackMask) {
      int square = popLSB(blackMask);
      boardArray[black][square] = pieceType;
    }
  }
  std::cout << std::endl;
  for (int rank = 7; rank >= 0; --rank) {
    std::cout << rank + 1 << " ";
    for (int file = 0; file < 8; ++file) {
      bool isLight = (rank + file) % 2 == 1;
      std::string bgColor = isLight ? BG_LIGHT : BG_DARK;
      int squareIndex = rank * 8 + file; 
      int color = boardArray[white][squareIndex] ? white : black;
      int piece = boardArray[white][squareIndex] ? boardArray[white][squareIndex] : 
        boardArray[black][squareIndex];

      //std::string txtColor = (piece <= P && piece >= K) ? TXT_WHITE : TXT_BLACK;
      std::cout << bgColor << TXT_BLACK << PIECE_SYMBOLS[color][piece] << BG_RESET;
    }
    std::cout << std::endl;
  }
  std::cout << "   a  b  c  d  e  f  g  h\n\n";
  std::string color = colorToMove ? "White" : "Black";
  std::cout << "       " << color << " to move.\n" << std::endl;
}
