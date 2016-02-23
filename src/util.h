#pragma once

#include <pebble.h>

int fraction_to_angle(int numerator, int denominator);
GPoint gpoint_from_point(GPoint point, int length, int32_t angle);

