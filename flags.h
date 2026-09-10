#include <cstdint>

enum Flag : uint8_t { 
  QUIET = 0, 
  DOUBLEPP = 1,
  CASTLING = 2, 
  CAPTURE = 4,
  ENPASSANT = 5,
  NPROMO = 8,
  BPROMO = 9,
  RPROMO = 10,
  QPROMO = 11,
  NPROMOCAP = 12,
  BPROMOCAP = 13,
  RPROMOCAP = 14,
  QPROMOCAP = 15
};
