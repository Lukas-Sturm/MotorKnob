#pragma once

#include <Arduino.h>
#include <haptic.h>

struct NamedProfile {
    const char* name;
    const DetentProfile profile;
};

// const NamedProfile regular_mode { 
//   .name = "Regular",
//   .profile = {
//     .mode = HapticMode::REGULAR,
//     .start_pos = 60,
//     .end_pos = 120,
//     .detent_count = 30,
//     .vernier = 0,
//     .kxForce = false
//   }
// };

// const NamedProfile vernier_mode { 
//   .name = "Vernier",
//   .profile = {
//     .mode = HapticMode::VERNIER,
//     .start_pos = 60,
//     .end_pos = 120,
//     .detent_count = 20,
//     .vernier = 5,
//     .kxForce = false
//   }
// };

// const NamedProfile smooth_mode { 
//   .name = "Vernier",
//   .profile = {
//     .mode = HapticMode::VERNIER,
//     .start_pos = 160,
//     .end_pos = 360,
//     .detent_count = 60,
//     .vernier = 6,
//     .kxForce = false
//   }
// };