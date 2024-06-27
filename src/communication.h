#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <wiring_constants.h>

struct MotorKnobWriteCallbacks {
  void (*write_start_pos)(uint16_t start_pos);
  void (*write_end_pos)(uint16_t end_pos);
  void (*write_detents)(uint16_t detents);
};

struct MotorKnobReadCallbacks {
  uint16_t (*get_pos_func)(void);
  uint16_t (*get_start_pos_func)(void);
  uint16_t (*get_end_pos_func)(void);
  uint16_t (*get_detents_func)(void);
};


// function pointers to access MotorKnob
MotorKnobReadCallbacks motorknob_read_callbacks;
MotorKnobWriteCallbacks motorknob_write_callbacks;

#define DATA_START_POS 0b00000000
#define DATA_END_POS 0b00000001
#define DATA_DETENTS 0b00000010
#define DATA_POSITION 0b00000011 // special case, read only

#define MODE_BIT 7 // 0b10000000 // 0 = data, 1 = command
#define WRITE_COMMAND 0b10000000

#define WRITE_COMMAND_START_POS (WRITE_COMMAND | DATA_START_POS)
#define WRITE_COMMAND_END_POS (WRITE_COMMAND | DATA_END_POS)
#define WRITE_COMMAND_DETENTS (WRITE_COMMAND | DATA_DETENTS)

#define WRITE_STATE_IDLE 0
#define WRITE_STATE_DATA_UPPER 3
#define WRITE_STATE_DATA_LOWER 4

HardwareTimer *timer;
uint8_t write_state = WRITE_STATE_IDLE;
uint8_t requested_data = 0; 

int current_command = -1;
uint16_t current_data = 0;

/**
 * Restart Timer
 * Timer is used to reset the state machine if no data is received
 */
void restart_timer() {
  // reset timer and start it
  timer->setCount(0);
  timer->resume();
}

/**
 * Basic command validation
 */
void set_and_check_command(char data) {
  if (data == WRITE_COMMAND_START_POS || 
      data == WRITE_COMMAND_END_POS ||
      data == WRITE_COMMAND_DETENTS) {
    current_command = data;
  } else {
    timer->pause(); // timer stoppen
    write_state = WRITE_STATE_IDLE;
    Serial.print("Unkown Command: ");
    Serial.println(data, HEX);
  }
}

/*
 * Execute the current command
 */
void execute_command() {
  switch (current_command)
  {
  case WRITE_COMMAND_START_POS:
    motorknob_write_callbacks.write_start_pos(current_data);
    break;
  case WRITE_COMMAND_END_POS:
    motorknob_write_callbacks.write_end_pos(current_data);
    break;
  case WRITE_COMMAND_DETENTS:
    motorknob_write_callbacks.write_detents(current_data);
    break;
  }
}

/**
 * Interrupt Handler for I2C Write
 */
void write_handler(int byteCount) {
  while (Wire.available()) {
    char data = Wire.read();
    switch (write_state)
    {
    case WRITE_STATE_IDLE:
    {
      // check mode bit
      if (bitRead(data, 7) == 0) {
        requested_data = data;
      } else {
        write_state = WRITE_STATE_DATA_LOWER;
        restart_timer(); // state machine started, enable timer
        set_and_check_command(data);
      }
      break;
    }
    case WRITE_STATE_DATA_LOWER:
    {
      write_state = WRITE_STATE_DATA_UPPER;
      current_data = data;
      break;
    }
    case WRITE_STATE_DATA_UPPER:
    {
      timer->pause(); // done, stop timer
      current_data |= (((u_int16_t) data) << 8);
      write_state = WRITE_STATE_IDLE;
      execute_command();
      break;
    }
    default:
    {
      // just to have a default
      write_state = WRITE_STATE_IDLE;
      break;
    }
    }
  }
}

/*
 * Get the requested data
 */
uint16_t get_requested_data() {
  switch (requested_data)
  {
  case DATA_POSITION:
    return motorknob_read_callbacks.get_pos_func();
  case DATA_END_POS:
    return motorknob_read_callbacks.get_end_pos_func();
  case DATA_START_POS:
    return motorknob_read_callbacks.get_start_pos_func();
  case DATA_DETENTS:
    return motorknob_read_callbacks.get_detents_func();
  default:
    return 0;
  }
}

/**
 * Interrupt Handler für I2C Read
 */
void read_handler() {
  _delay(1); // RPi i2c can't handle fast responses (this took way to long to debug)

  uint16_t data = get_requested_data();
  // SMBus expexts low byte first, high byte second
  Wire.write(lowByte(data));
  Wire.write(highByte(data));

  requested_data = 0;
}

bool i2c_setup_done = false;

/**
 * Setup I2C
 * Attach Callbacks, can be called multiple times to overwrite callbacks
 * Begin Wire and setup Interrupt Timer. Multiple calls won't reinitialize Wire
 */
void setup_i2c(
  u_int8_t address,
  MotorKnobWriteCallbacks _motorknob_write_callbacks,
  MotorKnobReadCallbacks _motorknob_read_callbacks
) {
  // Changing Callbacks
  motorknob_write_callbacks = _motorknob_write_callbacks;
  motorknob_read_callbacks = _motorknob_read_callbacks;

  // Fuse Setup
  if (i2c_setup_done) return;
  i2c_setup_done = true;

  timer = new HardwareTimer(TIM1);
  // 5ms sollten reichen um eine Operation abzuschließen
  timer->setOverflow(5000, MICROSEC_FORMAT);
  timer->attachInterrupt([]() {
    write_state = WRITE_STATE_IDLE;
    Serial.println("Reset Command State Machine");
    timer->pause();
  });

  Wire.begin(address); // Initialize I2C (Slave Mode)
  Wire.onReceive(write_handler);
  Wire.onRequest(read_handler);
}