#include <pebble.h>
#include "main.h"
#include "main_window.h"

// Global variables initialization
const uint8_t Y_FACE_AWAY    = 1;
const uint8_t Y_FACE_TOWARDS = -1;
const uint8_t Y_NEUTRAL = 0;
const uint8_t Y_INACTIVE = -1;
uint8_t g_stateY = 0;
uint8_t g_currentContext = CONTEXT_SCROLL;

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

// Context switching
void switch_context(uint8_t new_context) {
  send_change_context(new_context);
}
void next_context() {
  uint8_t nextContext = g_currentContext + 1;
  if (nextContext > NUM_CONTEXTS) {
    // Back to first context.
    g_currentContext = nextContext - NUM_CONTEXTS;
  } else {
    g_currentContext = nextContext;
  }
  switch_context(g_currentContext);
}
void previous_context() {
  uint8_t prevContext = g_currentContext - 1;
  if (prevContext < 1) {
    // Back to last context.
    g_currentContext = NUM_CONTEXTS;
  } else {
    g_currentContext = prevContext;
  }
  switch_context(g_currentContext);
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