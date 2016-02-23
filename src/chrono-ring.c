#include <pebble.h>

#include "util.h"
#include "persist.h"

#define DATE_ENABLED_DEFAULT_VALUE true

static Window *s_main_window;
static TextLayer *s_hour_layer;
static TextLayer *s_minute_layer;
static TextLayer *s_date_layer;
static Layer *s_canvas_layer;

static int s_minute_angle = 0;
static int s_hour_angle = 0;

static const int RING_CUTOUT_ANGLE = DEG_TO_TRIGANGLE(40);

// CONFIG ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
enum APP_MESSAGE_KEYS {
  KEY_HOUR_TEXT_COLOR,
  KEY_MINUTE_TEXT_COLOR,
  KEY_DATE_TEXT_COLOR,
  KEY_DATE_TEXT_ENABLED,

  KEY_HOUR_RING_COLOR,
  KEY_HOUR_CICLE_COLOR,
  KEY_MINUTE_RING_COLOR,
  KEY_BACKGROUND_COLOR,

  // The value of the last element in an enum is the number of items
  NUM_APP_MESSAGE_KEYS
};

static void update_config() {
  GColor color;

  color = persist_get_color(KEY_HOUR_TEXT_COLOR, GColorBlack);
  text_layer_set_text_color(s_hour_layer, color);

  color = persist_get_color(KEY_MINUTE_TEXT_COLOR, GColorBlack);
  text_layer_set_text_color(s_minute_layer, color);

  color = persist_get_color(KEY_BACKGROUND_COLOR, GColorWhite);
  window_set_background_color(s_main_window,  color);

  GRect bounds = layer_get_bounds(window_get_root_layer(s_main_window));
  GRect frame;

  bool date_enabled = persist_get_bool(KEY_DATE_TEXT_ENABLED,
      DATE_ENABLED_DEFAULT_VALUE);

  if (date_enabled) {
    frame = GRect(0, (bounds.size.h / 2) - 35, bounds.size.w, 50);
    layer_set_frame(text_layer_get_layer(s_hour_layer), frame);

    frame = GRect(0, (bounds.size.h / 2) + 8, bounds.size.w, 50);
    layer_set_frame(text_layer_get_layer(s_date_layer), frame);

    color = persist_get_color(KEY_DATE_TEXT_COLOR, GColorBlack);
    text_layer_set_text_color(s_date_layer, color);

  } else {
    frame = GRect(0, (bounds.size.h / 2) - 28, bounds.size.w, 50);
    layer_set_frame(text_layer_get_layer(s_hour_layer), frame);

    // Hide the text
    text_layer_set_text_color(s_date_layer, GColorClear);
  }

  layer_mark_dirty(text_layer_get_layer(s_hour_layer));
  layer_mark_dirty(text_layer_get_layer(s_date_layer));
  layer_mark_dirty(s_canvas_layer);
}

// UI ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static void update_text(struct tm *tick_time) {
  s_minute_angle = fraction_to_angle(tick_time->tm_min, 60);
  static char s_hour_buffer[sizeof("00")];
  strftime(
    s_hour_buffer,
    sizeof(s_hour_buffer),
    clock_is_24h_style() ? "%H" : "%I",
    tick_time
  );
  text_layer_set_text(s_hour_layer, s_hour_buffer);

  static char s_minute_buffer[sizeof("00")];
  strftime(s_minute_buffer, sizeof(s_minute_buffer), "%M", tick_time);
  text_layer_set_text(s_minute_layer, s_minute_buffer);

  bool date_enabled = persist_get_bool(KEY_DATE_TEXT_ENABLED, DATE_ENABLED_DEFAULT_VALUE);
  if (date_enabled) {
    static char s_date_buffer[sizeof("aaaa 00")];
    strftime(s_date_buffer, sizeof(s_date_buffer), "%b %d", tick_time);
    text_layer_set_text(s_date_layer, s_date_buffer);
  } else {
    text_layer_set_text(s_date_layer, "");
  }
}

static void update_minute_position() {
  const int x = 40;
  const int y = 40;
  const int length = 70;

  GRect window_bounds = layer_get_bounds(window_get_root_layer(s_main_window));
  GPoint center_gpoint = GPoint(window_bounds.size.w / 2, window_bounds.size.h / 2);
  GPoint new_point = gpoint_from_point(center_gpoint, length, s_minute_angle);
  GRect frame = GRect(new_point.x - (x / 2), new_point.y - (y / 2), x, y);
  layer_set_frame(text_layer_get_layer(s_minute_layer), frame);
  layer_mark_dirty(text_layer_get_layer(s_minute_layer));
}

static void update() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  s_minute_angle = fraction_to_angle(tick_time->tm_sec, 60);
  s_hour_angle = fraction_to_angle(tick_time->tm_hour, clock_is_24h_style() ? 24 : 12);

  update_text(tick_time);
  update_minute_position();
  layer_mark_dirty(s_canvas_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update();
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  GRect window_bounds = layer_get_bounds(window_get_root_layer(s_main_window));
  GColor color;

  // Hour ring
  const int offset = 1; // Fix pixel offset
  const int radius = 49;
  int angle_start = s_hour_angle + (RING_CUTOUT_ANGLE / 2);
  int angle_end = (s_hour_angle - (RING_CUTOUT_ANGLE / 2)) + TRIG_MAX_ANGLE;
  GRect rect = GRect(
      (window_bounds.size.w / 2) - radius,
      (window_bounds.size.h / 2) - radius,
      radius * 2 + offset,
      radius * 2 + offset
  );
  color = persist_get_color(KEY_HOUR_RING_COLOR, GColorJazzberryJam);
  graphics_context_set_fill_color(ctx, color);
  graphics_fill_radial(ctx, rect, GOvalScaleModeFitCircle, 10, // Inset thickness
      angle_start, angle_end);

  // Hour circle
  color = persist_get_color(KEY_HOUR_CICLE_COLOR, GColorSunsetOrange);
  graphics_context_set_fill_color(ctx, color);
  graphics_fill_circle(ctx,
      GPoint(window_bounds.size.w / 2, window_bounds.size.h / 2), 40);

  // Minute ring
  angle_start = s_minute_angle + (RING_CUTOUT_ANGLE / 2);
  angle_end = (s_minute_angle - (RING_CUTOUT_ANGLE / 2)) + TRIG_MAX_ANGLE;
  rect = layer_get_bounds(window_get_root_layer(s_main_window));
  color = persist_get_color(KEY_MINUTE_RING_COLOR, GColorVividCerulean);
  graphics_context_set_fill_color(ctx, color);
  graphics_fill_radial(ctx, rect, GOvalScaleModeFitCircle, 15, // inset thickness
      angle_start, angle_end);
}

static void in_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Incoming message dropped: %d", reason);
}

static void in_received_handler(DictionaryIterator *received, void *context) {
  Tuple *tuple;

  // APP_LOG(APP_LOG_LEVEL_DEBUG, "size: %d", (int)received->end - (int)received->dictionary);

  for (int key = KEY_HOUR_TEXT_COLOR; key < NUM_APP_MESSAGE_KEYS; key++) {
    tuple = dict_find(received, key);
    if (!tuple) continue;

    if (key == KEY_DATE_TEXT_ENABLED) {
      persist_write_bool(key, tuple->value->int32);
    } else {
      persist_write_int(key, tuple->value->int32);
    }
  }

  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  update_config();
  update_text(tick_time);
}

static void main_window_load(Window *window) {
  // App message
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_inbox_received(in_received_handler);
  const int inbox_size = 89; // calculated in the in_received_handler
  const int outbox_size = inbox_size;
  app_message_open(inbox_size, outbox_size);

  // UI
  Layer *window_layer = window_get_root_layer(s_main_window);
  GRect bounds = layer_get_bounds(window_layer);

  s_hour_layer = text_layer_create(GRect(
        0, (bounds.size.h / 2) - 35, bounds.size.w, 50));
  text_layer_set_background_color(s_hour_layer, GColorClear);
  text_layer_set_font(s_hour_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_hour_layer, GTextAlignmentCenter);
  text_layer_set_text(s_hour_layer, "00:00");

  s_minute_layer = text_layer_create(GRect(0, 100, bounds.size.w, 30));
  text_layer_set_background_color(s_minute_layer, GColorClear);
  text_layer_set_font(s_minute_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  text_layer_set_text_alignment(s_minute_layer, GTextAlignmentCenter);
  text_layer_set_text(s_minute_layer, "00:00");

  s_date_layer = text_layer_create(GRect(
        0, (bounds.size.h / 2) + 8, bounds.size.w, 50));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_text(s_date_layer, "00:00");

  s_canvas_layer = layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);

  layer_add_child(window_layer, s_canvas_layer);
  layer_add_child(window_layer, text_layer_get_layer(s_hour_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_minute_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  update();
  update_config();
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_hour_layer);
  text_layer_destroy(s_minute_layer);
  text_layer_destroy(s_date_layer);
  layer_destroy(s_canvas_layer);
}

static void init() {
  s_main_window = window_create();

  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  window_stack_push(s_main_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
