#include <pebble.h>
#include "main.h"
#include "main_window.h"

// Global variables initialization
const uint8_t Y_FACE_AWAY    = 1;
const uint8_t Y_FACE_TOWARDS = -1;
const uint8_t Y_NEUTRAL = 0;
const uint8_t Y_INACTIVE = -1;
uint8_t g_stateY = 0;

// App initialisation
static void init(void) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Now doing init.");
  show_main_window();
  init_communication();
  init_accelerometer();
}

// Destruction
static void deinit(void) {
  
}

//**************************************************************************************************
//*************************************   APP STARTS HERE   ****************************************
//**************************************************************************************************
int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_INFO, "Finished init.");

  app_event_loop();
  deinit();
}