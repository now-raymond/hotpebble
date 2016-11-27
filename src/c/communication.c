#include <pebble.h>
#include "main.h"

// Specify max size of a single message.
const uint32_t inbox_size = 256;
const uint32_t outbox_size = 256;

// Success callbacks
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Message was received.");
}

static void outbox_sent_callback(DictionaryIterator *iter, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Message was successfully sent.");
}

// Failure callbacks
static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_WARNING, "The received message was dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iter, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_WARNING, "Message delivery failed! Reason: %d", reason);
}

void init_communication() {
  APP_LOG(APP_LOG_LEVEL_INFO, "Starting to initialize communication.");
  
  // Register all callbacks. Success callbacks.
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  // Failure callbacks.
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  
  
  app_message_open(inbox_size, outbox_size);
}

// NOT CURRENTLY USED.
// Call this function to send data to the server application.
void send_accelerometer_data(uint64_t timestamp, int16_t x, int16_t y, int16_t z) {
  DictionaryIterator *out_iter;
  
  // Prepare outbox buffer
  AppMessageResult result = app_message_outbox_begin(&out_iter);
  if (result == APP_MSG_OK) {
    dict_write_uint16(out_iter, COMMUNICATION_KEY_TIMESTAMP, timestamp);
    dict_write_int16(out_iter, COMMUNICATION_KEY_X, x);
    dict_write_int16(out_iter, COMMUNICATION_KEY_Y, y);
    dict_write_int16(out_iter, COMMUNICATION_KEY_Z, z);
    
    result = app_message_outbox_send();
    if(result != APP_MSG_OK) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the outbox: %d", (int)result);
    }
  } else if (result == APP_MSG_BUSY) {
    //APP_LOG(APP_LOG_LEVEL_INFO, "Busy: Target is still processing the message.");
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox: %d", (int)result);
  }
}

// Call this function to send tilt data to the server application.
// Negative values = scroll up.
void send_tilt_data(int16_t speed) {
  DictionaryIterator *out_iter;
  
  // Prepare outbox buffer
  AppMessageResult result = app_message_outbox_begin(&out_iter);
  if (result == APP_MSG_OK) {
    dict_write_int16(out_iter, COMMUNICATION_KEY_TILTSPEED, speed);
    
    result = app_message_outbox_send();
    if(result != APP_MSG_OK) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the outbox: %d", (int)result);
    }
  } else if (result == APP_MSG_BUSY) {
    //APP_LOG(APP_LOG_LEVEL_INFO, "Busy: Target is still processing the message.");
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox: %d", (int)result);
  }
}

// Call this function to tell the server to switch contexts.
void send_change_context(uint8_t new_context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Attempting to switch to context %d.", new_context);
  DictionaryIterator *out_iter;
  
  // Prepare outbox buffer
  AppMessageResult result = app_message_outbox_begin(&out_iter);
  if (result == APP_MSG_OK) {
    dict_write_int16(out_iter, COMMUNICATION_KEY_CHANGE_CONTEXT, new_context);
    
    result = app_message_outbox_send();
    if (result != APP_MSG_OK) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the context change message: %d", (int)result);
    } else {
      APP_LOG(APP_LOG_LEVEL_INFO, "Context change message sent successfully.");
    }
  } else if (result == APP_MSG_BUSY) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Busy: Target is still processing the message.");
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox: %d", (int)result);
  }
}