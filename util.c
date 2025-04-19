#include "util.h"

Uint32 max(Uint32 a, Uint32 b) {
    return a > b ? a : b;
}

Uint32 min(Uint32 a, Uint32 b) {
    return a < b ? a : b;
}

double temperDouble(double z) {
    return z > UINT16_MAX ? 0 : z;
}
