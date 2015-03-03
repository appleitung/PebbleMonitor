#include "main_window.h"
#include <pebble.h>

// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window;
static GFont s_res_bitham_30_black;
static GFont s_res_gothic_14;
static InverterLayer *right_channel_peak;
static InverterLayer *sensitivity_layer;
static InverterLayer *left_channel_peak;
static ActionBarLayer *s_actionbarlayer;
static TextLayer *s_plusbuttonlayer;
static TextLayer *s_minusbuttonlayer;
static TextLayer *on_off_textlayer;

static void initialise_ui(void) {
  s_window = window_create();
  window_set_background_color(s_window, GColorBlack);
  window_set_fullscreen(s_window, false);
  
  s_res_bitham_30_black = fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK);
  s_res_gothic_14 = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  // right_channel_peak
  right_channel_peak = inverter_layer_create(GRect(70, 1, 35, 150));
  layer_add_child(window_get_root_layer(s_window), (Layer *)right_channel_peak);
  
  // sensitivity_layer
  sensitivity_layer = inverter_layer_create(GRect(0, 75, 144, 2));
  layer_add_child(window_get_root_layer(s_window), (Layer *)sensitivity_layer);
  
  // left_channel_peak
  left_channel_peak = inverter_layer_create(GRect(20, 1, 35, 150));
  layer_add_child(window_get_root_layer(s_window), (Layer *)left_channel_peak);
  
  // s_actionbarlayer
  s_actionbarlayer = action_bar_layer_create();
  action_bar_layer_add_to_window(s_actionbarlayer, s_window);
  action_bar_layer_set_background_color(s_actionbarlayer, GColorWhite);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_actionbarlayer);
  
  // s_plusbuttonlayer
  s_plusbuttonlayer = text_layer_create(GRect(124, 9, 20, 38));
  text_layer_set_text(s_plusbuttonlayer, "+");
  text_layer_set_text_alignment(s_plusbuttonlayer, GTextAlignmentCenter);
  text_layer_set_font(s_plusbuttonlayer, s_res_bitham_30_black);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_plusbuttonlayer);
  
  // s_minusbuttonlayer
  s_minusbuttonlayer = text_layer_create(GRect(124, 108, 20, 38));
  text_layer_set_text(s_minusbuttonlayer, "-");
  text_layer_set_text_alignment(s_minusbuttonlayer, GTextAlignmentCenter);
  text_layer_set_font(s_minusbuttonlayer, s_res_bitham_30_black);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_minusbuttonlayer);
  
  // on_off_textlayer
  on_off_textlayer = text_layer_create(GRect(125, 68, 18, 15));
  text_layer_set_text(on_off_textlayer, "On");
  text_layer_set_text_alignment(on_off_textlayer, GTextAlignmentCenter);
  text_layer_set_font(on_off_textlayer, s_res_gothic_14);
  layer_add_child(window_get_root_layer(s_window), (Layer *)on_off_textlayer);
}

static void destroy_ui(void) {
  window_destroy(s_window);
  inverter_layer_destroy(right_channel_peak);
  inverter_layer_destroy(sensitivity_layer);
  inverter_layer_destroy(left_channel_peak);
  action_bar_layer_destroy(s_actionbarlayer);
  text_layer_destroy(s_plusbuttonlayer);
  text_layer_destroy(s_minusbuttonlayer);
  text_layer_destroy(on_off_textlayer);
}
// END AUTO-GENERATED UI CODE

static const int DISPLAY_HEIGHT = 150;
static const int AVAILABLE_STEPS = 10;

static GRect LAST_SENSITIVITY_FRAME;

enum DataKeys {
  LEFT_CHANNEL_PEAK_KEY,
  RIGHT_CHANNEL_PEAK_KEY,
  LEFT_CHANNEL_AVG_KEY,
  RIGHT_CHANNEL_AVG_KEY,
  SENSITIVITY,
};

static AppSync sync;
static uint8_t sync_buffer[256];

static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) 
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

static void out_sent_handler(DictionaryIterator *sent, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Outgoing AppMessage successfully sent.");
}

static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Outgoing AppMessage failed");
}

static int selected_sensitivity()
{
  Layer *layer = inverter_layer_get_layer(sensitivity_layer);
  GRect frame = layer_get_frame(layer);
  GPoint origin = frame.origin;
  
  return (origin.y - DISPLAY_HEIGHT) * -1;
}

static void trigger_alarm(int value)
{
  if (value > selected_sensitivity()) {
    vibes_short_pulse();
  }
}

static void send_sensitivity()
{
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  
  Tuplet value = TupletInteger(SENSITIVITY, selected_sensitivity());
  dict_write_tuplet(iter, &value);
  
  app_message_outbox_send();
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Sensitivity sent: %d.", selected_sensitivity());
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) 
{
  Layer *layer = NULL;
  
  switch (key) {
    case LEFT_CHANNEL_PEAK_KEY:
      layer = inverter_layer_get_layer(left_channel_peak);
      break;
    case RIGHT_CHANNEL_PEAK_KEY:
      layer = inverter_layer_get_layer(right_channel_peak);
      break;
    default:
      return;
  }
  
  GRect frame = layer_get_frame(layer);
  GPoint origin = frame.origin;
  GSize size = frame.size;
  
  int value = new_tuple->value->uint8 * 1.55;
  int new_h = DISPLAY_HEIGHT - (DISPLAY_HEIGHT - value);
  int new_y = DISPLAY_HEIGHT - value;
  
  GRect newFrame = GRect(origin.x, new_y, size.w, new_h);
  
  layer_set_frame(layer, newFrame);
  
  trigger_alarm(value);
}

static void window_load(Window *window) 
{
  // Prepare app_sync for sending data to phone.
  Tuplet initial_values[] = {
    TupletInteger(LEFT_CHANNEL_PEAK_KEY, (uint8_t) 0),
    TupletInteger(RIGHT_CHANNEL_PEAK_KEY, (uint8_t) 0),
    TupletInteger(LEFT_CHANNEL_AVG_KEY, (uint8_t) 0),
    TupletInteger(RIGHT_CHANNEL_AVG_KEY, (uint8_t) 0),
  };
  app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL);
  
  app_message_register_outbox_failed(out_failed_handler);
  app_message_register_outbox_sent(out_sent_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void handle_window_unload(Window* window) 
{
  destroy_ui();
}

void move_sensitivity_layer(int steps)
{
  Layer *layer = inverter_layer_get_layer(sensitivity_layer);
  GRect frame = layer_get_frame(layer);
  GPoint origin = frame.origin;
  GSize size = frame.size;
  int new_y = origin.y + steps;
  
  if (new_y > DISPLAY_HEIGHT || new_y < 0) {
    return;
  }
  
  GRect newFrame = GRect(origin.x, new_y, size.w, size.h);
  layer_set_frame(layer, newFrame);
  
  send_sensitivity();
}

void up_single_click_handler(ClickRecognizerRef recognizer, void *context) 
{
  move_sensitivity_layer(DISPLAY_HEIGHT / AVAILABLE_STEPS * -1);
}

void select_single_click_handler(ClickRecognizerRef recognizer, void *context) 
{
  Layer *layer = inverter_layer_get_layer(sensitivity_layer);
  GRect currentFrame = layer_get_frame(layer);
  GRect newFrame;
  
  if (currentFrame.origin.y >= 0 ) {
    LAST_SENSITIVITY_FRAME = currentFrame;
    newFrame = GRect(0, -1, 0, 0);
    text_layer_set_text(on_off_textlayer, "Off");
  } else {
    newFrame = LAST_SENSITIVITY_FRAME;
    text_layer_set_text(on_off_textlayer, "On");
  }
  
  layer_set_frame(layer, newFrame);
  
  send_sensitivity();
}

void down_single_click_handler(ClickRecognizerRef recognizer, void *context) 
{
  move_sensitivity_layer(DISPLAY_HEIGHT / AVAILABLE_STEPS);
}

void click_config_provider(Window *window) 
{
  window_single_click_subscribe(BUTTON_ID_UP, up_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_single_click_handler);
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
  
  window_set_click_config_provider(s_window, (ClickConfigProvider) click_config_provider);
  
  window_stack_push(s_window, true);
}

void hide_main_window(void) 
{
  window_stack_remove(s_window, true);
}
