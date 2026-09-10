#include <iostream>
#include <random>
#include <set>
#include <algorithm>
#include <bitset>
#include <vector>
#include "chess.h"

using namespace std;

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

uint64_t singlePushWhitePawns(uint64_t emptySquares, uint64_t pawns) {
  return pawns << 8 & emptySquares;
}

uint64_t doublePushWhitePawns(uint64_t emptySquares, uint64_t pawns) {
  return (((pawns & 0x000000000000FF00) & (emptySquares >> 8)) << 16) & emptySquares;
}

uint64_t singlePushBlackPawns(uint64_t emptySquares, uint64_t pawns) {
  return pawns >> 8 & emptySquares;
}

uint64_t doublePushBlackPawns(uint64_t emptySquares, uint64_t pawns) {
  return (((pawns & 0x00FF000000000000) & (emptySquares << 8)) >> 16) & emptySquares;
}

inline uint64_t wpRightAttacks(const uint64_t whitePawns) {
  uint64_t notHfile = 0x7F7F7F7F7F7F7F7F;
  return ((whitePawns & notHfile) << 9);
}

inline uint64_t wpLeftAttacks(const uint64_t whitePawns) {
  uint64_t notAfile = 0xFEFEFEFEFEFEFEFE;
  return ((whitePawns & notAfile) << 7);
}

inline uint64_t bpRightAttacks(const uint64_t blackPawns) {
  uint64_t notAfile = 0xFEFEFEFEFEFEFEFE;
  return ((blackPawns & notAfile) >> 9);
}

inline uint64_t bpLeftAttacks(const uint64_t blackPawns) {
  uint64_t notHfile = 0x7F7F7F7F7F7F7F7F;
  return ((blackPawns & notHfile) >> 7);
}

uint64_t moveBoardKnight(int square) {
  uint64_t moves = 0;
  int rank = square / 8;
  int file = square % 8;
  if (rank + 2 < 8) {
    if (file - 1 >= 0) moves |= 1ULL << square + 15;
    if (file + 1 < 8) moves |= 1ULL << square + 17;
  }
  if (rank + 1 < 8) {
    if (file - 2 >= 0) moves |= 1ULL << square + 6;
    if (file + 2 < 8) moves |= 1ULL << square + 10;
  }
  if (rank - 1 >= 0) {
    if (file + 2 < 8) moves |= 1ULL << square - 6;
    if (file - 2 >= 0) moves |= 1ULL << square - 10;
  }
  if (rank - 2 >= 0) {
    if (file + 1 < 8) moves |= 1ULL << square -15;
    if (file - 1 >= 0) moves |= 1ULL << square - 17;
  }
  return moves;
}

std::array<uint64_t, 64> genKnightAttacks() {
  std::array<uint64_t, 64> attacks;
  constexpr uint64_t NOT_A = 0xfefefefefefefefe;
  constexpr uint64_t NOT_AB = 0xfcfcfcfcfcfcfcfc;
  constexpr uint64_t NOT_H = 0x7f7f7f7f7f7f7f7f;
  constexpr uint64_t NOT_GH = 0x3f3f3f3f3f3f3f3f;
  for (int square = 0; square < 64; ++square) {
    uint64_t moveBoard = 0;
    moveBoard |= 1ULL << square;
  }
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

uint64_t moveBoardKing(int square) {
  uint64_t moves = 0;
  int rank = square / 8;
  int file = square % 8;
  if (rank + 1 < 8) moves |= 1ULL << square + 8;
  if (rank - 1 >= 0) moves |= 1ULL << square - 8;
  if (file + 1 < 8) moves |= 1ULL << square + 1;
  if (file - 1 >= 0) moves |= 1ULL << square - 1;
  moves |= (moves >> 1 & moves << 8) | (moves << 1 & moves << 8);
  moves |= (moves >> 1 & moves >> 8) | (moves << 1 & moves >> 8);
  return moves;
}

template<typename Fn>
void initMoveTable(Fn moveFunction, std::array<uint64_t, 64> & moveTable) {
  for (size_t i = 0; i < moveTable.size(); ++i) {
    moveTable[i] = moveFunction(i);
  }
}

inline int popLSB(uint64_t& bitBoard) {
  int index = __builtin_ctzll(bitBoard);
  bitBoard &= (bitBoard - 1);
  return index;
}

uint64_t randomU64() {
  static std::mt19937_64 gen(1337);
  return gen();
}

uint64_t generateCandidate() {
  return randomU64() & randomU64() & randomU64();
}

std::vector<uint64_t> setOccupancies(uint64_t mask) {
  uint64_t occupancy = 0;
  int variations = 1 << __builtin_popcountll(mask);
  std::vector<uint64_t> result;
  result.reserve(variations);
  do {
    result.emplace_back(occupancy);
    occupancy = (occupancy - mask) & mask;
  } while (occupancy);
  return result;
}

template <auto Fn_moveBoard>
uint64_t findMagic(uint64_t mask, int square) {
  size_t count = 0;
  int shift = __builtin_popcountll(mask);
  int variations = 1 << shift;
  std::vector<uint64_t> occupancies = setOccupancies(mask);
  std::vector<uint64_t> references(variations, 0);
  for (int i = 0; i < occupancies.size(); ++i) {
    references[i] = Fn_moveBoard(occupancies[i], square);
  }
  std::vector<uint64_t> result(variations, 0);
  while (true) {
    bool failed = false;
    std::fill(result.begin(), result.end(), 0);
    uint64_t magic = generateCandidate();
    for (int i = 0; i < variations; ++i) {
      unsigned int index = (occupancies[i] * magic) >> (64 - shift);
      if (result[index] == 0) result[index] = references[i];
      else if (result[index] != Fn_moveBoard(occupancies[i], square)) {
        failed = true;
        ++count;
        break;
      }
    }
    if (!failed) {
      cout << "  0x" << hex << magic << dec << "," << endl;
      return magic;
    }
  }
}

int main() {
  int bishopTableSize = 0;
  int rookTableSize = 0;
  std::vector<uint64_t> bishopMagics(64);
  cout << "#include <array>\n#include <cstdint>\n";
  cout << "std::array<uint64_t, 64> bishopMagics {\n";
  for (int square = 0; square < 64; ++square) {
    uint64_t mask = blockmaskBishop(square);
    bishopTableSize += (1 << __builtin_popcountll(mask));
    bishopMagics[square] = findMagic<moveBoardBishop>(mask, square);
  }
  cout << "};\n\nstd::array<uint64_t, 64> rookMagics {\n";
  std::vector<uint64_t> rookMagics(64);
  for (int square = 0; square < 64; ++square) {
    uint64_t mask = blockmaskRook(square);
    rookTableSize += (1 << __builtin_popcountll(mask));
    rookMagics[square] = findMagic<moveBoardRook>(mask, square);
  }
  cout << "};";
}
