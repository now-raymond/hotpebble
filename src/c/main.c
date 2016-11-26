#include <pebble.h>
#include "main.h"
#include "main_window.h"

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