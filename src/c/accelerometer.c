#include <pebble.h>
#include "main.h"

// Config
uint32_t num_samples = 3;  // Number of samples per batch/callback

int16_t modelX = 0, modelY = 0, modelZ = 0;
int16_t tiltValue = 0;         // The calculated tilt value taking into account tolerance.
int16_t balancePoint_Y = 0;    // Where the "level" surface is.
int16_t toleranceLevel = 175;  // Distance away from the balance point to be regarded as a tilt.
int16_t boundsLevel = 500;     // Distance away from the balance point to obtain maximum tilt. (TODO)
float easingValue = 1;  // 0.05  Now is essentially off.
float jitterValue = 0;  // 5     Now is essentially off.

int16_t cutoff_x = 500;        // If abs(modelX) is > than this value then scrolling stops.

int16_t tiltJitter = 0;
float tiltEasing = 0.05;

// Determins which way the pebble is facing.
void update_state() {
  // Update display
  window_update_orientation(modelX, modelY, modelZ);
  
  bool shouldCutoff = abs(modelX) > cutoff_x;
  
  int16_t deltaTilt = 0;
  
  if (!shouldCutoff && modelY > balancePoint_Y + toleranceLevel) {
    // Facing away from body
    if (g_stateY != Y_FACE_AWAY) {
      g_stateY = Y_FACE_AWAY;
      window_update_status("Scroll down.");
      tiltEasing = 0.05;
    }
    deltaTilt = modelY - (balancePoint_Y + toleranceLevel) - tiltValue;
    //tiltValue = modelY - (balancePoint_Y + toleranceLevel);
  } else if (!shouldCutoff && modelY < balancePoint_Y - toleranceLevel) {
    // Facing towards body
    if (g_stateY != Y_FACE_TOWARDS) {
      g_stateY = Y_FACE_TOWARDS;
      window_update_status("Scroll up.");
      tiltEasing = 0.05;
    }
    deltaTilt = modelY - (balancePoint_Y - toleranceLevel) - tiltValue;
    //tiltValue = modelY - (balancePoint_Y - toleranceLevel);
  } else {
    // Neutral position
    if (!shouldCutoff && g_stateY != Y_NEUTRAL) {
      g_stateY = Y_NEUTRAL;
      window_update_status("Neutral.");
      tiltEasing = 0.80;
    }
    
    if (shouldCutoff && g_stateY != Y_INACTIVE) {
      // Cutoff.
      g_stateY = Y_INACTIVE;
      window_update_status("Inactive.");
      tiltEasing = 0.80;
    }
    
    deltaTilt = 0 - tiltValue;
    //tiltValue = 0;
  }
  
  // Move towards target value.
  if (abs(deltaTilt) > tiltJitter) {
    tiltValue += deltaTilt * tiltEasing;
  } else {
    tiltValue += deltaTilt;
  }
  
  // Update tilt value on interface
  window_update_tilt_y(tiltValue);
  
  // Send tilt value to server
  send_tilt_data(tiltValue);
}

// Sets balancePoint_Y to the current Y axis orientation.
void accelerometer_calibrate_zero() {
  balancePoint_Y = modelY;
}

static void accel_data_handler(AccelData *data, uint32_t num_samples) {
  // Determine if the sample occured during vibration, and when it occured
  bool did_vibrate = data[0].did_vibrate;

  if(!did_vibrate) {
    // Read sample 0's x, y, and z values
    uint64_t timestamp = data[0].timestamp;
    int16_t x = data[0].x;
    int16_t y = data[0].y;
    int16_t z = data[0].z;
    
    // Print it out
    //APP_LOG(APP_LOG_LEVEL_INFO, "t: %llu, x: %d, y: %d, z: %d", timestamp, x, y, z);
    
    // Update our internal state values
    int16_t dx = x - modelX;
    int16_t dy = y - modelY;
    int16_t dz = z - modelZ;
    
    if (abs(dx) > jitterValue || abs(dy) > jitterValue || abs(dz) > jitterValue) {
      modelX += dx * easingValue;
      modelY += dy * easingValue;
      modelZ += dz * easingValue;
    }
    
    update_state();
    
    // Send it to the application.
    //send_accelerometer_data(timestamp, x, y, z);
    
  } else {
    // Discard with a warning
    APP_LOG(APP_LOG_LEVEL_WARNING, "Vibration occured during collection - discarding.");
  }
}

void init_accelerometer() {
  APP_LOG(APP_LOG_LEVEL_INFO, "Initializing accelerometer.");
  // Wait 5 seconds before starting to send data.
  // NOTE: This causes the main thread to hold for 5 seconds. Remove later.
  //psleep(5000);
  accel_data_service_subscribe(num_samples, accel_data_handler);
}

void deinit_accelerometer() {
  accel_data_service_unsubscribe();
}