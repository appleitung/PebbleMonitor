#include "main_window.h"
#include <pebble.h>

// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window;
static InverterLayer *right_channel_layer;
static InverterLayer *left_channel_layer;
static InverterLayer *sensitivity_layer;

static void initialise_ui(void) {
  s_window = window_create();
  window_set_background_color(s_window, GColorBlack);
  window_set_fullscreen(s_window, false);
  
  // right_channel_layer
  right_channel_layer = inverter_layer_create(GRect(80, 1, 20, 150));
  layer_add_child(window_get_root_layer(s_window), (Layer *)right_channel_layer);
  
  // left_channel_layer
  left_channel_layer = inverter_layer_create(GRect(45, 1, 20, 150));
  layer_add_child(window_get_root_layer(s_window), (Layer *)left_channel_layer);
  
  // sensitivity_layer
  sensitivity_layer = inverter_layer_create(GRect(0, 75, 144, 2));
  layer_add_child(window_get_root_layer(s_window), (Layer *)sensitivity_layer);
}

static void destroy_ui(void) {
  window_destroy(s_window);
  inverter_layer_destroy(right_channel_layer);
  inverter_layer_destroy(left_channel_layer);
  inverter_layer_destroy(sensitivity_layer);
}
// END AUTO-GENERATED UI CODE

enum DataKeys {
  CHANNEL_0_KEY,
  CHANNEL_1_KEY,
};

static AppSync sync;
static uint8_t sync_buffer[256];

static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) 
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) 
{
  Layer *layer = NULL;
  
  switch (key) {
    case CHANNEL_0_KEY:
      layer = inverter_layer_get_layer(left_channel_layer);
      break;
    case CHANNEL_1_KEY:
      layer = inverter_layer_get_layer(right_channel_layer);
      break;
  }
  
  GRect frame = layer_get_frame(layer);
  GPoint origin = frame.origin;
  GSize size = frame.size;
  
  int value = new_tuple->value->uint8;
  int new_h = 150 - (150 - value);
  int new_y = 150 - value;
  
  GRect newFrame = GRect(origin.x, new_y, size.w, new_h);
  
  layer_set_frame(layer, newFrame);
}

static void window_load(Window *window) 
{
  Tuplet initial_values[] = {
    TupletInteger(CHANNEL_0_KEY, (uint8_t) 0),
    TupletInteger(CHANNEL_1_KEY, (uint8_t) 0),
  };
  app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL);  
}

static void handle_window_unload(Window* window) 
{
  destroy_ui();
}

void show_main_window(void) 
{
  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = handle_window_unload,
  });
  
  const int inbound_size = app_message_inbox_size_maximum();
  const int outbound_size = app_message_inbox_size_maximum();
  app_message_open(inbound_size, outbound_size);
  
  //window_set_click_config_provider(s_window, click_config_provider);
  
  window_stack_push(s_window, true);
}

void hide_main_window(void) 
{
  window_stack_remove(s_window, true);
}
