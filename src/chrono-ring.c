#include <pebble.h>

#include "util.h"

#define CENTER_GPOINT GPoint(90, 90)

static Window *s_main_window;
static TextLayer *s_hour_layer;
static TextLayer *s_minute_layer;
static Layer *s_canvas_layer;

static int s_minute_angle = 0;

static void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

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
}

static void update_minute_position() {
  GPoint new_point = gpoint_from_point(CENTER_GPOINT, 60, s_minute_angle);
  GRect frame = GRect(new_point.x - 19, new_point.y - 14, 40, 30);
  layer_set_frame(text_layer_get_layer(s_minute_layer), frame);
  layer_mark_dirty(text_layer_get_layer(s_minute_layer));
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  update_minute_position();
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  GRect window_rect = layer_get_bounds(window_get_root_layer(s_main_window));

  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  s_minute_angle = fraction_to_angle(tick_time->tm_min, 60);

  int minutes_ring_cutout_angle = DEG_TO_TRIGANGLE(40);
  int angle_start = s_minute_angle + (minutes_ring_cutout_angle / 2);
  int angle_end = (s_minute_angle - (minutes_ring_cutout_angle / 2)) + TRIG_MAX_ANGLE;

  graphics_context_set_fill_color(ctx, GColorVividCerulean);
  graphics_fill_radial(
      ctx,
      window_rect,
      GOvalScaleModeFitCircle,
      8, // inset thickness
      angle_start,
      angle_end
  );
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(s_main_window);
  GRect bounds = layer_get_bounds(window_layer);

  s_hour_layer = text_layer_create(GRect(0, 58, bounds.size.w, 50));
  text_layer_set_background_color(s_hour_layer, GColorClear);
  text_layer_set_text_color(s_hour_layer, GColorBlack);
  text_layer_set_font(s_hour_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_hour_layer, GTextAlignmentCenter);
  text_layer_set_text(s_hour_layer, "00:00");

  s_minute_layer = text_layer_create(GRect(0, 100, bounds.size.w, 30));
  text_layer_set_background_color(s_minute_layer, GColorClear);
  text_layer_set_text_color(s_minute_layer, GColorBlack);
  text_layer_set_font(s_minute_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  text_layer_set_text_alignment(s_minute_layer, GTextAlignmentCenter);
  text_layer_set_text(s_minute_layer, "00:00");

  s_canvas_layer = layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);

  update_time();

  layer_add_child(window_layer, text_layer_get_layer(s_hour_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_minute_layer));
  layer_add_child(window_layer, s_canvas_layer);
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_hour_layer);
}

static void init() {
  s_main_window = window_create();

  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  window_stack_push(s_main_window, true);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
