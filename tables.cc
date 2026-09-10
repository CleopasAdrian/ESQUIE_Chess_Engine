#include "chess.h"
#include "magics.h"

using namespace ESQUIE;

uint64_t Knight::pregenAttacks(uint8_t square) {
    uint64_t knightBB = 1ULL << square;
    uint64_t attacks = 0;

    constexpr uint64_t NOT_A = 0xfefefefefefefefe;
    constexpr uint64_t NOT_AB = 0xfcfcfcfcfcfcfcfc;
    constexpr uint64_t NOT_H = 0x7f7f7f7f7f7f7f7f;
    constexpr uint64_t NOT_GH = 0x3f3f3f3f3f3f3f3f;

    attacks |= (knightBB & NOT_A) << 15;
    attacks |= (knightBB & NOT_AB) << 6;
    attacks |= (knightBB & NOT_AB) >> 10;
    attacks |= (knightBB & NOT_A) >> 17;
    attacks |= (knightBB & NOT_H) << 17;
    attacks |= (knightBB & NOT_GH) << 10;
    attacks |= (knightBB & NOT_GH) >> 6;
    attacks |= (knightBB & NOT_H) >> 15;

    return attacks;
}

Knight::Knight() {
  for (int square = 0; square < 64; ++ square) {
    knightAttacks[square] = pregenAttacks(square);
  }
}

uint64_t Knight::getAttacks(uint8_t square) {
  return knightAttacks[square];
}

uint64_t King::pregenAttacks(uint8_t square) {
  uint64_t kingBB = 1ULL << square;
  uint64_t attacks = 0;
  constexpr uint64_t NOT_A = 0xfefefefefefefefe;
  constexpr uint64_t NOT_H = 0x7f7f7f7f7f7f7f7f;
  attacks |= kingBB << 8 | kingBB >> 8;
  attacks |= (kingBB & NOT_A) << 7;
  attacks |= (kingBB & NOT_A) >> 1;
  attacks |= (kingBB & NOT_A) >> 9;
  attacks |= (kingBB & NOT_H) << 9;
  attacks |= (kingBB & NOT_H) << 1;
  attacks |= (kingBB & NOT_H) >> 7;
  return attacks;
}

King::King() {
  for (int square = 0; square < 64; ++square) {
    kingAttacks[square] = pregenAttacks(square);
  }
}

uint64_t King::getAttacks(uint8_t square) {
  return kingAttacks[square];
}

inline uint64_t Pawn::pregenWhiteAttacks(uint8_t square) {
  constexpr uint64_t NOT_A = 0xfefefefefefefefe;
  constexpr uint64_t NOT_H = 0x7f7f7f7f7f7f7f7f;
  return (1ULL << square & NOT_A) << 7 | (1ULL << square & NOT_H) << 9;
}

uint64_t Pawn::pregenBlackAttacks(uint8_t square) {
  constexpr uint64_t NOT_A = 0xfefefefefefefefe;
  constexpr uint64_t NOT_H = 0x7f7f7f7f7f7f7f7f;
  return (1ULL << square & NOT_H) >> 7 | (1ULL << square & NOT_A) >> 9;
}

Pawn::Pawn() {
  for (int square = 0; square < 64; ++square) {
    pawnAttacks[1][square] = pregenWhiteAttacks(square);
    pawnAttacks[0][square] = pregenBlackAttacks(square);
  }
}

uint64_t Pawn::getAttacks(uint8_t color, uint8_t square) {
  return pawnAttacks[color][square];
}

Bishop::Bishop() : magics{}, magicRedirect{}, lookup{} {
  size_t offset = 0;
  for (int square = 0; square < 64; ++square) {
    uint64_t occupancy = 0;
    uint64_t mask = blockmaskBishop(square);
    int shift = __builtin_popcountll(mask);
    std::vector<uint64_t> uniqueAttacks;
    do {
      uint8_t uniqueIndex;
      uint64_t rawAttack = moveBoardBishop(occupancy, square);
      bool unique = true;
      for (int i = 0; i < uniqueAttacks.size(); ++i) {
        if (uniqueAttacks[i] == rawAttack) {
          uniqueIndex = (uint8_t)i;
          unique = false; 
          break;
        }
      }
      if (unique) {
        uniqueIndex = (uint8_t)uniqueAttacks.size();
        uniqueAttacks.emplace_back(rawAttack);
        lookup[uniqueIndex + offset] = rawAttack;
      }
      int index = (occupancy * bishopMagics[square]) >> (64 - shift);
      magicRedirect[square][index] = uniqueIndex;
      occupancy = (occupancy - mask) & mask;
    } while (occupancy);
    magics[square].magic = bishopMagics[square];
    magics[square].mask = mask;
    magics[square].offset = offset;
    magics[square].shift = shift;
    offset += uniqueAttacks.size();
  }
}

uint64_t Bishop::getAttacks(uint64_t occupancy, uint8_t square) {
  occupancy &= magics[square].mask;
  occupancy *= magics[square].magic;
  occupancy >>= (64 - magics[square].shift);
  return lookup[ magics[square].offset + magicRedirect[square][occupancy] ];
}

Rook::Rook() {
  size_t offset = 0;
  for (int square = 0; square < 64; ++square) {
    uint64_t occupancy = 0;
    uint64_t mask = blockmaskRook(square);
    int shift = __builtin_popcountll(mask);
    std::vector<uint64_t> uniqueAttacks;
    do {
      uint8_t uniqueIndex;
      uint64_t rawAttack = moveBoardRook(occupancy, square);
      bool unique = true;
      for (int i = 0; i < uniqueAttacks.size(); ++i) {
        if (uniqueAttacks[i] == rawAttack) {
          uniqueIndex = (uint8_t)i;
          unique = false; 
          break;
        }
      }
      if (unique) {
        uniqueIndex = (uint8_t)uniqueAttacks.size();
        uniqueAttacks.emplace_back(rawAttack);
        lookup[uniqueIndex + offset] = rawAttack;
      }
      int index = (occupancy * rookMagics[square]) >> (64 - shift);
      magicRedirect[square][index] = uniqueIndex;
      occupancy = (occupancy - mask) & mask;
    } while (occupancy);
    magics[square].magic = rookMagics[square];
    magics[square].mask = mask;
    magics[square].offset = offset;
    magics[square].shift = shift;
    offset += uniqueAttacks.size();
  }
}

uint64_t Rook::getAttacks(uint64_t occupancy, uint8_t square) {
  occupancy &= magics[square].mask;
  occupancy *= magics[square].magic;
  occupancy >>= 64 - magics[square].shift;
  return lookup[ magics[square].offset + magicRedirect[square][occupancy] ];
}

Queen::Queen(Rook& rooks, Bishop& bishops) : rookLookup{rooks}, bishopLookup{bishops} {}

uint64_t Queen::getAttacks(uint64_t occupancy, uint8_t square) {
  return rookLookup.getAttacks(occupancy, square) | bishopLookup.getAttacks(occupancy, square);
}

LookupTables::LookupTables() : pawn(), knight(), bishop(), rook(), queen(rook, bishop), king() {}
