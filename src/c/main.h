#include <pebble.h>

// Constants initialized in main.c.
#ifndef HEADER_ONCE
#define HEADER_ONCE
// Constants
extern const uint8_t Y_FACE_AWAY;
extern const uint8_t Y_FACE_TOWARDS;
extern const uint8_t Y_NEUTRAL;
extern const uint8_t Y_INACTIVE;

// State Variables
extern uint8_t g_stateY;  // 1 = Away from body, -1 = Towards body
extern uint8_t g_currentContext;
#endif

// Contexts
#define NUM_CONTEXTS 3
#define CONTEXT_MEDIA 1
#define CONTEXT_SCROLL 2
#define CONTEXT_PRESENTATION 3

// Accelerometer.c
void init_accelerometer();

// Communication.c
#define COMMUNICATION_KEY_TIMESTAMP 100
#define COMMUNICATION_KEY_X 101
#define COMMUNICATION_KEY_Y 102
#define COMMUNICATION_KEY_Z 103

// Scroll - 20X
#define COMMUNICATION_KEY_TILTSPEED 200
#define COMMUNICATION_KEY_SCROLL_TO_TOP 201

// Contexts - 30X
#define COMMUNICATION_KEY_CHANGE_CONTEXT 300

// Media - 50X:
#define COMMUNICATION_KEY_PLAY_PAUSE 500
#define COMMUNICATION_KEY_VOLUME 501
#define COMMUNICATION_KEY_TRACK_CHANGE 502

// Presentation - 60X:
#define COMMUNICATION_KEY_CHANGE_SLIDE 600
#define COMMUNICATION_KEY_FULLSCREEN 601

void init_communication();
void send_accelerometer_data(uint64_t timestamp, int16_t x, int16_t y, int16_t z);
void send_tilt_data(int16_t speed);
void accelerometer_calibrate_zero();

void send_change_context(int8_t new_context);
void send_action_message(int action, int8_t parameter);

// Main Window
void window_update_orientation(int16_t x, int16_t y, int16_t z);
void window_update_status(const char* messageString);
void window_update_tilt_y(int16_t y);

// Main.c
void switch_context(uint8_t new_context);
void next_context();
void previous_context();

// Sway.c
void init_sway();
void sway_accel_handler(AccelData *data, uint32_t num_samples);