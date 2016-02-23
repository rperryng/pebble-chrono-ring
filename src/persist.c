#include "persist.h"

bool persist_get_bool(const uint32_t key, bool default_value) {
  return persist_exists(key)
    ? persist_read_bool(key)
    : default_value;
}

int32_t persist_get_int(const uint32_t key, int32_t default_value) {
  return persist_exists(key)
    ? persist_read_int(key)
    : default_value;
}

GColor persist_get_color(const uint32_t key, GColor default_color) {
  return persist_exists(key)
    ? GColorFromHEX(persist_read_int(key))
    : default_color;
}

bool persist_get_data(const uint32_t key, void *buffer, const size_t buffer_size, void *default_value) {
  int result = persist_read_data(key, buffer, buffer_size);

  if (result == E_DOES_NOT_EXIST) {
    buffer = default_value;
    return true;
  } else {
    return false;
  }
}

bool persist_get_string(const uint32_t key, char *buffer, const size_t buffer_size, char *default_value) {
  int result = persist_read_data(key, buffer, buffer_size);

  if (result == E_DOES_NOT_EXIST) {
    buffer = default_value;
    return true;
  } else {
    return false;
  }
}

