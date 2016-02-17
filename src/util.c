#include "util.h"

int fraction_to_angle(int numerator, int denominator) {
  return (numerator * TRIG_MAX_ANGLE) / denominator;
}
