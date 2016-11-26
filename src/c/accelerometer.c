#include <pebble.h>
#include "main.h"

// Config
uint32_t num_samples = 3;  // Number of samples per batch/callback

static void accel_data_handler(AccelData *data, uint32_t num_samples) {
  // Read sample 0's x, y, and z values
  int16_t x = data[0].x;
  int16_t y = data[0].y;
  int16_t z = data[0].z;

  // Determine if the sample occured during vibration, and when it occured
  bool did_vibrate = data[0].did_vibrate;
  uint64_t timestamp = data[0].timestamp;

  if(!did_vibrate) {
    // Print it out
    APP_LOG(APP_LOG_LEVEL_INFO, "t: %llu, x: %d, y: %d, z: %d", timestamp, x, y, z);
    
    // Send it to the application.
    send_accelerometer_data(timestamp, x, y, z);
    
  } else {
    // Discard with a warning
    APP_LOG(APP_LOG_LEVEL_WARNING, "Vibration occured during collection");
  }
}

void init_accelerometer() {
  APP_LOG(APP_LOG_LEVEL_INFO, "Initializing accelerometer.");
  // Wait 5 seconds before starting to send data.
  // NOTE: This causes the main thread to hold for 5 seconds. Remove later.
  psleep(5000);
  accel_data_service_subscribe(num_samples, accel_data_handler);
}

void deinit_accelerometer() {
  accel_data_service_unsubscribe();
}