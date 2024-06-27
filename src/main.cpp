#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <SimpleFOC.h>
#include <SimpleFOCDrivers.h>
#include <encoders/mt6701/MagneticSensorMT6701SSI.h>
#include <haptic.h>

#include "communication.h"
#include "motor.h"
#include "profiles.h"

#define MODE_BUTTON PA4
#define ROTARY_ENCODER_SSI_nCS PB0

// Main HapticInterface used to drive the motor and haptic feedback
HapticInterface haptic(&motor); // motor exported from motor.h

void UserHapticEventCallback(HapticEvt event, float currentAngle, uint16_t currentPos) {
      Serial.print("Event: ");
      Serial.print(event);
      Serial.print(" Angle: ");
      Serial.print(currentAngle);
      Serial.print(" Pos: ");
      Serial.println(currentPos);
}

DetentProfile defaultProfile = {
  .mode = HapticMode::REGULAR, // not used by library
  .start_pos = 80,
  .end_pos = 160,
  .detent_count = 80,
  .vernier = 0, // only multiplies detent_count by vernier and changes position counting
  .kxForce = false // not used by library
};

void setup() {
  Serial.begin(115200);
  _delay(1000); // wait for serial monitor to open

  Serial.println("Starting motor...");
  // SimpleFOCDebug::enable(&Serial);

  setup_simple_foc();
  haptic.setEventCallback(UserHapticEventCallback);
  haptic.haptic_state.load_profile(defaultProfile);
  haptic.init();

  setup_i2c(
    0x55,
    {
      .write_start_pos = [](uint16_t pos) -> void {
        defaultProfile.start_pos = pos;
        haptic.haptic_state.load_profile(defaultProfile);
        Serial.print("Wrote: start pos ");
        Serial.println(pos);
      },
      .write_end_pos = [](uint16_t pos) {
        defaultProfile.end_pos = pos;
        haptic.haptic_state.load_profile(defaultProfile);
        Serial.print("Wrote: end pos ");
        Serial.println(pos);
      },
      .write_detents = [](uint16_t detents) {
        defaultProfile.detent_count = detents;
        haptic.haptic_state.load_profile(defaultProfile);
        Serial.print("Wrote: detents ");
        Serial.println(detents);
      }
    },
    {
      .get_pos_func = []() -> uint16_t {
        return haptic.haptic_state.current_pos;
      },
      .get_start_pos_func = []() -> uint16_t {
        Serial.print("Read: start pos ");
        Serial.println(haptic.haptic_state.detent_profile.start_pos);
        return haptic.haptic_state.detent_profile.start_pos;
      },
      .get_end_pos_func = []() -> uint16_t {
        Serial.print("Read: end pos ");
        Serial.println(haptic.haptic_state.detent_profile.end_pos);
        return haptic.haptic_state.detent_profile.end_pos;
      },
      .get_detents_func = []() -> uint16_t {
        Serial.print("Read: detents ");
        Serial.println(haptic.haptic_state.detent_profile.detent_count);
        return haptic.haptic_state.detent_profile.detent_count;
      }
    }
  );
}

void loop() {
  // Runs haptic loop
  haptic.haptic_loop();
}