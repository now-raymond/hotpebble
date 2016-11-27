#include <pebble.h>
#include "swaypattern.h"
#include "main.h"
#include "main_window.h"

// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window;
static GFont s_res_gothic_28_bold;
static TextLayer *s_textlayer_1;

static void initialise_ui(void) {
  s_window = window_create();
  window_set_background_color(s_window, GColorBlack);
  #ifndef PBL_SDK_3
    window_set_fullscreen(s_window, true);
  #endif
  
  s_res_gothic_28_bold = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
  // s_textlayer_1
  s_textlayer_1 = text_layer_create(GRect(24, 55, 100, 28));
  text_layer_set_background_color(s_textlayer_1, GColorClear);
  text_layer_set_text_color(s_textlayer_1, GColorWhite);
  text_layer_set_text(s_textlayer_1, "SWAY");
  text_layer_set_text_alignment(s_textlayer_1, GTextAlignmentCenter);
  text_layer_set_font(s_textlayer_1, s_res_gothic_28_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_1);
}

static void destroy_ui(void) {
  window_destroy(s_window);
  text_layer_destroy(s_textlayer_1);
}
// END AUTO-GENERATED UI CODE


time_t history_s[4];
static uint16_t history_ms[4];
time_t difference[3];
int percentage[3];
int16_t diff_head;
int16_t head = 0;
#define matches_size 7
#define sample_size 3
int matches[matches_size][3];
int error_margin = 7;
int total = 0;
bool button = false;
int avg_history[5];
int avg_head;
int avg_sample[sample_size];
bool calibrating = true;
int pos = 0;
int changes = -1;
int fixed_avg = 0;
AppTimer *calibrate_timer; 
AppTimer *button_timer; 
AppTimer *success_timer; 


static void button_pressed(void);


static void init_avg_history () {
  for (int i=0; i<9; i++){ 
    avg_history[i] = 0;
  }
}

static void init_matches () {  
  matches[0][0] = 55;
  matches[0][1] = 22;
  matches[0][2] = 22;
  
  matches[1][0] = 33;
  matches[1][1] = 33;
  matches[1][2] = 33;
  
  matches[2][0] = 22;
  matches[2][1] = 22;
  matches[2][2] = 55;
  
  matches[3][0] = 22;
  matches[3][1] = 55;
  matches[3][2] = 22;
  
  matches[4][0] = 40;
  matches[4][1] = 20;
  matches[4][2] = 40;
  
  matches[5][0] = 40;
  matches[5][1] = 40;
  matches[5][2] = 20;
  
  matches[6][0] = 20;
  matches[6][1] = 40;
  matches[6][2] = 40;
}

static int match () {
  int count;
  int i;
  bool matched = false;
  
  for(count=0; count<matches_size; count++) {
//     APP_LOG(APP_LOG_LEVEL_INFO, "entered match()");
    matched = true;
    for(i=0; i<3 && matched; i++){
      int error = abs(percentage[i] - matches[count][i]);
//       APP_LOG(APP_LOG_LEVEL_INFO, "error is: %d", error);
      if (error > error_margin) {
        matched = false;
      }
    }
    if (matched) {
      return count;
    }
  }
  return -1;
}

// static void logtime() {
//   int count;
//   for (count=0;count<3;count++) {
//     APP_LOG(APP_LOG_LEVEL_INFO, "%ld", (history_s[count+1]*1000+history_ms[count+1])-(history_s[count]*1000+history_ms[count]));

//   }
  
//   for (count=0;count<3;count++) {
//     APP_LOG(APP_LOG_LEVEL_INFO, "%d",percentage[count]);
//   }
  
//   for (count=0;count<3;count++) {
//     APP_LOG(APP_LOG_LEVEL_INFO, "%ld",difference[count]);

//   }
  
// }

static void matched_action (int pattern) {
  if (g_currentContext == CONTEXT_MEDIA) {
    switch (pattern) {
      case 1:
        g_currentContext = CONTEXT_SCROLL;
        show_main_window();
        break;
      case 0:
        send_action_message(COMMUNICATION_KEY_TRACK_CHANGE, 1);
       break;
      case 2:
        send_action_message(COMMUNICATION_KEY_TRACK_CHANGE, -1);  
        break;
      case 3:
        send_action_message(COMMUNICATION_KEY_PLAY_PAUSE, 1);
        break;
      case 4:
        g_currentContext = CONTEXT_PRESENTATION;
        show_main_window();
        break;
      default:
        break;
    }
  }
  else if (g_currentContext == CONTEXT_SCROLL) {
    switch (pattern) {
      case 1:
        g_currentContext = CONTEXT_MEDIA;
        hide_main_window();
        break;
      case 4:
        g_currentContext = CONTEXT_PRESENTATION;

        break;
      default:
      break;
    }

  }
  else if (g_currentContext == CONTEXT_PRESENTATION) {
    switch (pattern) {
      case 0:
        send_action_message(COMMUNICATION_KEY_CHANGE_SLIDE, 1);
        break;
      case 1:
        g_currentContext = CONTEXT_MEDIA;
        hide_main_window();
        break;
      case 2:
        send_action_message(COMMUNICATION_KEY_CHANGE_SLIDE, -1);
        break;
      default:
      break;
    }

  }
}

static void findDifference() {
  total = 0;
  for (diff_head=0; diff_head<3;diff_head++) {
    difference[diff_head] = (history_s[(head+diff_head+1)%4]*1000+history_ms[(diff_head+head+1)%4])-(history_s[(head+diff_head)%4]*1000+history_ms[(head+diff_head)%4]);
    total = total + difference[diff_head];
  }
  
//   APP_LOG(APP_LOG_LEVEL_INFO, " diff %lu, %lu, %lu", difference[0], difference[1], difference[2]);

  int count;
//   APP_LOG(APP_LOG_LEVEL_INFO, "%d",total);

  for (count=0; count<3; count++) {
    percentage[count] = (difference[count]*100)/total;
  }
  
//   APP_LOG(APP_LOG_LEVEL_INFO, "perc %d, %d, %d", percentage[0], percentage[1], percentage[2]);

}

static void recordTime() {
  time_ms(&history_s[head%4], &history_ms[head%4]);
//   APP_LOG(APP_LOG_LEVEL_INFO, " s %lu, %lu, %lu, %lu", history_s[0], history_s[1], history_s[2], history_s[3]);
//   APP_LOG(APP_LOG_LEVEL_INFO, " ms %u, %u, %u, %u", history_ms[0], history_ms[1], history_ms[2], history_ms[3]);

  head++;
  if (head>3) {
    findDifference();
    if (total < 10000) {
      int result = match();
      if (result != -1) {
        static char test[64] = "";
        snprintf(test,sizeof(test),"%i", result);
        text_layer_set_text(s_textlayer_1, test);
        APP_LOG(APP_LOG_LEVEL_INFO, "MATCHED!! patern %d",result);
        vibes_short_pulse();
        matched_action(result);
        head = 0;
      }
    }
    else {
      head = 0;
    }
//     logtime();
  }
}

// static void clear() {
//   head = -1;
// } 

static void handle_window_unload(Window* window) {
  destroy_ui();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  // A single click has just occured
  button_pressed();
  recordTime();
}

static void reset_calibration () {
  head = 0;
  calibrating = true;
  changes = -1;
  pos = 0;
}


static void button_timer_callback () {
  button = false;  
}

static int extend_button_timer () {
  return app_timer_reschedule(button_timer, 2000);
}

static void  cancel_button_timer () {
  app_timer_cancel(button_timer);
}


static void button_pressed() {
  button = true;
  if (!extend_button_timer()) {
    button_timer = app_timer_register(2000, (AppTimerCallback) button_timer_callback, NULL);
  }
}




static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  // A single click has just occured
  button_pressed();
  reset_calibration();
}


static void click_config_provider(void *context) {
  // Subcribe to button click events here
  ButtonId select = BUTTON_ID_SELECT;  // The Select button
  ButtonId down = BUTTON_ID_DOWN;  // The Select button


  window_single_click_subscribe(select, select_click_handler);
  window_single_click_subscribe(down, down_click_handler);

}


static void push_avg(int avg) {
  avg_history[avg_head] = avg;
  avg_head++;
  if (avg_head > 9) {
    avg_head = 0;
  }
}

static int get_avg_history () {
  int avg_total = 0;
  for (int count=0; count<9; count++) {
    avg_total += avg_history[count];
  }
  return avg_total/10;
}


static void calibrate_timer_callback () {
  reset_calibration();
  
}

static int extend_calibrate_timer () {
  return app_timer_reschedule(calibrate_timer, 5000);
}

static void  cancel_calibrate_timer () {
  app_timer_cancel(calibrate_timer);
}



void sway_accel_handler(AccelData *data, uint32_t num_samples) {
  if(!button) {
    
    int sample_total = 0;
    int max = -10000;
    int min = 10000;
    for (int i=0; i<3; i++){
      avg_sample[i] = data[i].y;
      sample_total += avg_sample[i];
      if (avg_sample[i] > max) {
        max = avg_sample[i];
      }
      if (avg_sample[i] < min) {
        min = avg_sample[i];
      }
    }
    
    int avg = sample_total/3;
    push_avg(avg);
    int history_avg = get_avg_history();
    max = max - history_avg;
    min =  history_avg - min;
    int range = max - min;
    
    if (calibrating) {
      
      bool same = true;
      for (int i=0; i<10 && same; i++ ) {
        if (abs(avg_history[i] - history_avg)>150) {
          same = false;
        }
      }
      
      if (same) {
        calibrating = false;
        text_layer_set_text(s_textlayer_1, "Callibrated");
//         window_set_background_color(s_window, GColorBlack);
//         text_layer_set_text_color(s_textlayer_1, GColorWhite);
        APP_LOG(APP_LOG_LEVEL_INFO, "calibratied");    
        fixed_avg = history_avg;
        calibrate_timer = app_timer_register(3000, (AppTimerCallback) calibrate_timer_callback, NULL); 
      }
      else {
//         window_set_background_color(s_window, GColorWhite);
        //text_layer_set_text_color(s_textlayer_1, GColorBlack);
        //text_layer_set_text(s_textlayer_1, "Hold Still!!");
      }
      
      
    }
    
    if (!calibrating) {
      int real_avg = avg - fixed_avg;
//       APP_LOG(APP_LOG_LEVEL_INFO, "%d", real_avg);    

      
      if (changes < 3) {
        if (real_avg>300) {
          if (pos<1) {
//             APP_LOG(APP_LOG_LEVEL_INFO, "pos");    
            pos = 1;
            changes++;
            recordTime();
            extend_calibrate_timer();
          }
        }
        
        if (real_avg < -300) {
          if (pos > -1) {
//             APP_LOG(APP_LOG_LEVEL_INFO, "neg");    
            pos = -1;
            changes++;
            recordTime();
            extend_calibrate_timer();
          }
        }
      }
      else {
        head = 0;
        calibrating = true;
        changes = -1;
        pos = 0;
        cancel_calibrate_timer();
      }
    }
    
    
   
    
  }
}

void init_sway() {
  button_timer = app_timer_register(0, (AppTimerCallback) button_timer_callback, NULL);
  
  init_avg_history();
  init_matches();
}




// UI CODE

void show_swaypattern(void) {
  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_stack_push(s_window, true);
  // Use this provider to add button click subscriptions
  window_set_click_config_provider(s_window, click_config_provider);
}



void hide_swaypattern(void) {
  window_stack_remove(s_window, true);
}
