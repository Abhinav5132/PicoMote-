#pragma once 

//IMU
#include <stdint.h>
#define SCL 16
#define SDA 17

//IR RECEIVER 
#define IR_TRANSMITTER 18

//CONTROL BUTTONS 
#define BUTTON_1 19
#define BUTTON_2 20
#define BUTTON_3 21
#define BUTTON_4 14
#define BUTTON_5 15

//joystick 
#define JOYSTICK_X 0
#define JOYSTICK_Y 1
#define JOYSTICK_BUTTON 4
#define JOYSTICK_POWER 3 //turn this off when the remote is in a sleep mode. 

static uint16_t deadzone_x = 150; 
static uint16_t deadzone_y = 150;