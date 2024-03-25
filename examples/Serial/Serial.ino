/**
 * Print dice events out over serial.
 */

#include <Arduino.h>

#include "pixels_dice_interface.h"

void setup() {
  Serial.begin(9600);
  // Start BLE scans for 2 seconds then waiting 5 seconds for the next scan
  // On completion the discovered dice are connected to
  pixels::ScanForDice(2, 5);
}

// The vectors to hold results queried from the library
// Since vectors allocate data, it's more efficient to keep reusing objects
// instead of declaring them on the stack
std::vector<pixels::PixelsDieID> dice_list;
pixels::RollUpdates roll_updates;
pixels::BatteryUpdates battery_updates;

// Blink red once for 1 second on all faces before fading out slowly
pixels::BlinkData blink{1, 1000, 0xFF0000, 0xFFFFFFFF, 0xFF, 0};

void loop() {
  delay(5000);
  // Update dice_list with the connected dice
  pixels::ListDice(dice_list);
  // Get all the roll/battery updates since the last loop
  pixels::GetDieRollUpdates(roll_updates);
  pixels::GetDieBatteryUpdates(battery_updates);
  Serial.println("###########################");
  for (auto id : dice_list) {
    auto description = pixels::GetDieDescription(id);
    Serial.printf(">>>> %s (0x%08X)\n", description.name.c_str(), id);
    for (const auto& roll : roll_updates) {
      if (roll.first == id) {
        Serial.printf("   Roll state: %s value: %u\n",
                      pixels::ToString(roll.second.state),
                      roll.second.current_face + 1);
      }
    }
    for (const auto& battery : battery_updates) {
      if (battery.first == id) {
        Serial.printf("   Battery level: %u%% is_charging: %u \n",
                      battery.second.battery_level, battery.second.is_charging);
      }
    }
    Serial.println("<<<<");
    pixels::SendDieBlink(id, blink);
  }
}
