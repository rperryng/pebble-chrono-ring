#include "util.h"

int fraction_to_angle(int numerator, int denominator) {
  return (numerator * TRIG_MAX_ANGLE) / denominator;
}

GPoint gpoint_from_point(GPoint point, int length, int32_t angle) {
  return GPoint(
      (sin_lookup(angle) * length / TRIG_MAX_RATIO) + point.x,
      (-cos_lookup(angle) * length / TRIG_MAX_RATIO) + point.y
  );
}
