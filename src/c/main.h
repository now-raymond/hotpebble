#include <pebble.h>

// Constants initialized in main.c.
#ifndef HEADER_ONCE
#define HEADER_ONCE
// Constants
extern const uint8_t Y_FACE_AWAY;
extern const uint8_t Y_FACE_TOWARDS;
extern const uint8_t Y_NEUTRAL;

// State Variables
extern uint8_t g_stateY;  // 1 = Away from body, -1 = Towards body
#endif

// Accelerometer.c
void init_accelerometer();

// Communication.c
#define COMMUNICATION_KEY_TIMESTAMP 100
#define COMMUNICATION_KEY_X 101
#define COMMUNICATION_KEY_Y 102
#define COMMUNICATION_KEY_Z 103
#define COMMUNICATION_KEY_TILTSPEED 200

void init_communication();
void send_accelerometer_data(uint64_t timestamp, int16_t x, int16_t y, int16_t z);

// Main Window
void window_update_orientation(int16_t x, int16_t y, int16_t z);
void window_update_status(const char* messageString);
void window_update_tilt_y(int16_t y);