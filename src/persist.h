#pragma once

#include <pebble.h>

bool persist_get_bool(const uint32_t key, bool default_value);

int32_t persist_get_int(const uint32_t key, int32_t default_value);

GColor persist_get_color(const uint32_t key, GColor default_color);

bool persist_get_data(const uint32_t key, void *buffer, const size_t buffer_size, void *default_value);

bool persist_get_string(const uint32_t key, char *buffer, const size_t buffer_size, char *default_value);
