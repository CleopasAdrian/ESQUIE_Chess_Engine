#include "chess.h"

namespace ESQUIE {

using U8 = uint8_t;
using U16 = uint16_t;

Move::Move() : data{} {}

Move::Move(Piece piece, Flag flag, U8 to, U8 from) : 
  data{ piece << 16 | flag << 12 | to << 6 | from } {}

bool Move::operator==(const Move& other) const {
  return data == other.data;
}

U8 Move::from() const { return data & 0x3F; }

U8 Move::to() const { return (data >> 6) & 0x3F; }

U8 Move::flag() const { return (data >> 12) & 0xF; }

U8 Move::piece() const { return data >> 16; }

std::ostream& operator<<(std::ostream& out, const Move& move) {
  uint8_t from = move.from();
  uint8_t to = move.to();
  char promo = ' ';
  constexpr char promoType[] = {'n', 'b', 'r', 'q'};
  if (move.flag() >= NPROMO) {
    promo = promoType[move.flag() & 3];
  }
  out 
    << SQUARES[FILES][from] 
    << SQUARES[RANKS][from] 
    << SQUARES[FILES][to] 
    << SQUARES[RANKS][to] 
    << promo;
  return out;
}

} // namespace ESQUIE
