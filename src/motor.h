#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <SimpleFOC.h>
#include <SimpleFOCDrivers.h>
#include <encoders/mt6701/MagneticSensorMT6701SSI.h>
#include <haptic.h>

#define ROTARY_ENCODER_SSI_nCS PB0

MagneticSensorMT6701SSI sensor(ROTARY_ENCODER_SSI_nCS);

//  BLDCMotor(int pp, (optional R, KV))
//  - pp  - pole pair number
//  - R   - phase resistance value - optional
//  - KV  - motor KV rating [rpm/V] - optional
// BLDCMotor motor = BLDCMotor(7, 3.6, 300);
BLDCMotor motor(7, 3.6);

InlineCurrentSense current_sense = InlineCurrentSense(0.01f, 50.0f, PA2, PA3);

BLDCDriver3PWM driver(PB3, PB4, PB8, PB5);

void setup_simple_foc() {
  sensor.init();

  // power supply voltage [V]
  driver.voltage_power_supply = 12;
  // Max DC voltage allowed - default voltage_power_supply
  driver.voltage_limit = 7;
  driver.init();

  // current_sense.linkDriver(&driver);
  // motor.linkCurrentSense(&current_sense);
  // current_sense.init();

  motor.linkSensor(&sensor);
  motor.linkDriver(&driver);
  motor.current_limit = 0.6;
  // motor.velocity_limit = 10;

  // overriden anyways
  motor.torque_controller = TorqueControlType::voltage;
  // motor.controller = MotionControlType::torque;

  // controller configuration based on the control type 
  motor.PID_velocity.P = 0.1;
  motor.PID_velocity.I = 3.0;
  motor.PID_velocity.D = 0.001;
  motor.LPF_velocity.Tf = 0.03;

  // angle loop controller
  motor.P_angle.P = 15;
  motor.P_angle.I = 2;
  motor.P_angle.D = 0.001;

  // initialize motor
  motor.init();

  // pre tested values for this exact setup
  // motor.sensor_direction = Direction::CW;
  // motor.zero_electric_angle = 5.3;

  // align sensor and start FOC
  // can be skipped, but currently mount is not super rigid 
  motor.initFOC();
}
