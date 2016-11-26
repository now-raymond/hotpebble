#include <pebble.h>

// Accelerometer.c
void init_accelerometer();

// Communication.c
#define COMMUNICATION_KEY_TIMESTAMP 100
#define COMMUNICATION_KEY_X 101
#define COMMUNICATION_KEY_Y 102
#define COMMUNICATION_KEY_Z 103

void init_communication();
void send_accelerometer_data(uint64_t timestamp, int16_t x, int16_t y, int16_t z);