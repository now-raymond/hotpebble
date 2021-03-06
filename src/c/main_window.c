#include <pebble.h>
#include "main.h"
#include "main_window.h"

int16_t windowHeight;
int16_t windowWidth;

bool movePebble = false;

// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window;
static GFont s_res_gothic_28_bold;
static GFont s_res_gothic_18_bold;
static GBitmap *s_res_red_pebble;
static TextLayer *s_textlayer_1;
static TextLayer *model_x_txt;
static TextLayer *model_y_txt;
static TextLayer *model_z_txt;
static TextLayer *s_statusmessage;
static BitmapLayer *s_bitmaplayer_1;
static TextLayer *s_text_tilt;

static void initialise_ui(void) {
  s_window = window_create();
  #ifndef PBL_SDK_3
    window_set_fullscreen(s_window, true);
  #endif
  
  s_res_gothic_28_bold = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
  s_res_gothic_18_bold = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  s_res_red_pebble = gbitmap_create_with_resource(RESOURCE_ID_Red_Pebble);
  // s_textlayer_1
  s_textlayer_1 = text_layer_create(GRect(11, 34, 100, 28));
  text_layer_set_text(s_textlayer_1, "HotPebble");
  text_layer_set_text_alignment(s_textlayer_1, GTextAlignmentCenter);
  text_layer_set_font(s_textlayer_1, s_res_gothic_28_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_1);
  
  // model_x_txt
  model_x_txt = text_layer_create(GRect(4, 75, 42, 21));
  text_layer_set_text(model_x_txt, "-");
  text_layer_set_text_alignment(model_x_txt, GTextAlignmentCenter);
  text_layer_set_font(model_x_txt, s_res_gothic_18_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)model_x_txt);
  
  // model_y_txt
  model_y_txt = text_layer_create(GRect(47, 75, 42, 20));
  text_layer_set_text(model_y_txt, "-");
  text_layer_set_text_alignment(model_y_txt, GTextAlignmentCenter);
  text_layer_set_font(model_y_txt, s_res_gothic_18_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)model_y_txt);
  
  // model_z_txt
  model_z_txt = text_layer_create(GRect(90, 75, 46, 24));
  text_layer_set_text(model_z_txt, "-");
  text_layer_set_text_alignment(model_z_txt, GTextAlignmentCenter);
  text_layer_set_font(model_z_txt, s_res_gothic_18_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)model_z_txt);
  
  // s_statusmessage
  s_statusmessage = text_layer_create(GRect(7, 146, 127, 20));
  text_layer_set_text(s_statusmessage, "Status");
  text_layer_set_text_alignment(s_statusmessage, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_statusmessage);
  
  // s_bitmaplayer_1
  s_bitmaplayer_1 = bitmap_layer_create(GRect(109, 43, 18, 19));
  bitmap_layer_set_bitmap(s_bitmaplayer_1, s_res_red_pebble);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_bitmaplayer_1);
  
  // s_text_tilt
  s_text_tilt = text_layer_create(GRect(45, 109, 49, 21));
  text_layer_set_text(s_text_tilt, "250");
  text_layer_set_text_alignment(s_text_tilt, GTextAlignmentCenter);
  text_layer_set_font(s_text_tilt, s_res_gothic_18_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_text_tilt);
}

static void destroy_ui(void) {
  window_destroy(s_window);
  text_layer_destroy(s_textlayer_1);
  text_layer_destroy(model_x_txt);
  text_layer_destroy(model_y_txt);
  text_layer_destroy(model_z_txt);
  text_layer_destroy(s_statusmessage);
  bitmap_layer_destroy(s_bitmaplayer_1);
  text_layer_destroy(s_text_tilt);
  gbitmap_destroy(s_res_red_pebble);
}
// END AUTO-GENERATED UI CODE

static void handle_window_unload(Window* window) {
  destroy_ui();
}

//**************************************************************************************************
//*************************************   CLICK HANDLERS   *****************************************
//**************************************************************************************************
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  switch (g_currentContext) {
    case CONTEXT_SCROLL:
      accelerometer_calibrate_zero();
      window_update_status("Zero calibrated.");
      break;
    case CONTEXT_MEDIA:
      window_update_status("Play/Pause.");
      send_action_message(COMMUNICATION_KEY_PLAY_PAUSE, 1);
      break;
    case CONTEXT_PRESENTATION:
      window_update_status("Fullscreen.");
      send_action_message(COMMUNICATION_KEY_FULLSCREEN, 1);
      break;
  }
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  switch (g_currentContext) {
    case CONTEXT_SCROLL:
      previous_context();
      break;
    case CONTEXT_MEDIA:
      window_update_status("Volume up.");
      send_action_message(COMMUNICATION_KEY_VOLUME, 1);
      break;
    case CONTEXT_PRESENTATION:
      window_update_status("Next slide.");
      send_action_message(COMMUNICATION_KEY_CHANGE_SLIDE, 1);
      break;
  }
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  switch (g_currentContext) {
    case CONTEXT_SCROLL:
      next_context();
      break;
    case CONTEXT_MEDIA:
      window_update_status("Volume down.");
      send_action_message(COMMUNICATION_KEY_VOLUME, -1);
      break;
    case CONTEXT_PRESENTATION:
      window_update_status("Previous slide.");
      send_action_message(COMMUNICATION_KEY_CHANGE_SLIDE, -1);
      break;
  }
}

static void up_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  previous_context();
}

static void down_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  next_context();
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  
}

void up_multi_click_handler(ClickRecognizerRef recognizer, void *context) {
  switch (g_currentContext) {
    case CONTEXT_SCROLL:
      window_update_status("Scroll to top.");
      send_action_message(COMMUNICATION_KEY_SCROLL_TO_TOP, 1);
      break;
    case CONTEXT_MEDIA:
      window_update_status("Next track.");
      send_action_message(COMMUNICATION_KEY_TRACK_CHANGE, 1);
      break;
  }
}

void down_multi_click_handler(ClickRecognizerRef recognizer, void *context) {
  switch (g_currentContext) {
    case CONTEXT_MEDIA:
      window_update_status("Previous track.");
      send_action_message(COMMUNICATION_KEY_TRACK_CHANGE, -1);
      break;
  }
}

void select_multi_click_handler(ClickRecognizerRef recognizer, void *context) {
  switch (g_currentContext) {
    case CONTEXT_PRESENTATION:
      window_update_status("Exit fullscreen.");
      send_action_message(COMMUNICATION_KEY_FULLSCREEN, 0);
      break;
  }
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  
  window_long_click_subscribe(BUTTON_ID_SELECT, 700, select_long_click_handler, NULL);
  window_long_click_subscribe(BUTTON_ID_UP, 700, up_long_click_handler, NULL);
  window_long_click_subscribe(BUTTON_ID_DOWN, 700, down_long_click_handler, NULL);
  
  window_multi_click_subscribe(BUTTON_ID_SELECT, 2, 0, 0, true, select_multi_click_handler);
  window_multi_click_subscribe(BUTTON_ID_UP, 2, 0, 0, true, up_multi_click_handler);
  window_multi_click_subscribe(BUTTON_ID_DOWN, 2, 0, 0, true, down_multi_click_handler);
}

void show_main_window(void) {
  initialise_ui();
  window_set_click_config_provider(s_window, click_config_provider);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_stack_push(s_window, true);
  
  // Ray
  GRect windowRect = layer_get_frame(window_get_root_layer(s_window));
  windowHeight = windowRect.size.h;
  windowWidth = windowRect.size.w;
  APP_LOG(APP_LOG_LEVEL_INFO, "Window height is %d.", windowHeight);
}

void hide_main_window(void) {
  window_stack_remove(s_window, true);
}

// Update on screen X Y Z labels
void window_update_orientation(int16_t x, int16_t y, int16_t z) {
  static char bufX[] = "00000000000";
  snprintf(bufX, sizeof(bufX), "%d", x);
  static char bufY[] = "00000000000";
  snprintf(bufY, sizeof(bufY), "%d", y);
  static char bufZ[] = "00000000000";
  snprintf(bufZ, sizeof(bufZ), "%d", z);
  
  //text_layer_set_text(model_x_txt, bufX);
  //text_layer_set_text(model_y_txt, bufY);
  //text_layer_set_text(model_z_txt, bufZ);
}

void window_update_tilt_y(int16_t y) {
  static char bufY[] = "00000000000";
  snprintf(bufY, sizeof(bufY), "%d", y);
  text_layer_set_text(s_text_tilt, bufY);
  
  if (movePebble) {
    Layer *bitmapLayer = bitmap_layer_get_layer(s_bitmaplayer_1);
    GRect frame = layer_get_frame(bitmapLayer);
    frame.origin.y = (y+500) * windowHeight / 1000;
    //APP_LOG(APP_LOG_LEVEL_INFO, "Putting at y: %d.", (y+500) * windowHeight / 1000);
    //frame.origin.y = 50;
    layer_set_frame(bitmapLayer, frame);
  }
}

void window_update_status(const char* messageString) {
  text_layer_set_text(s_statusmessage, messageString);
}