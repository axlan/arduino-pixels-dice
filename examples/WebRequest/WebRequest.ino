/**
 * Send dice rolls as JSON data in HTTP POST requests to a web server.
 *
 * NOTE: THIS SKETCH IS TOO BIG FOR THE DEFAULT 1MB ESP32 PARTITION!
 *
 * For Arduino:
 *   In the Arduino IDE in the "Tools" bar on top, set the "Partition Scheme"
 *   to "No OTA (2MB APP/2MB SPIFFS)", or any partition with an APP size of 2MB
 *   or more.
 *
 * For PlatformIO:
 *   See `platformio.ini` and README for how to set the `board_build.partitions`
 *   to a custom partition CSV.
 *   See
 * https://docs.platformio.org/en/latest/platforms/espressif32.html#partition-tables
 *   and included examples/WebRequest/no_factory_4MB.csv
 *
 * The included examples/WebRequest/test_server.py Python server can be used to
 * test this functionality. You may need to set a port 8080 (or whatever port
 * you use) inbound firewall exception on Windows.
 */

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiMulti.h>

#include <unordered_map>

#include "pixels_dice_interface.h"

#define USE_SERIAL Serial

WiFiMulti wifiMulti;

static constexpr const char *SSID = "SSID";
static constexpr const char *PASSWORD = "PASSWORD";
// The host server to send data to. This example uses port 8080, but leaving out
// the port defaults to the normal HTTP port 80.
static constexpr const char *REQUEST_URL = "http://host_address:8080/test";

void setup() {
  USE_SERIAL.begin(9600);
  USE_SERIAL.print("\n\n\n");

  wifiMulti.addAP(SSID, PASSWORD);

  // Start BLE scans for 2 seconds then waiting 5 seconds for the next scan
  // On completion the discovered dice are connected to
  pixels::ScanForDice(2, 5);
}

// The vectors to hold results queried from the library
// Since vectors allocate data, it's more efficient to keep reusing objects
// instead of declaring them on the stack
pixels::RollUpdates roll_updates;
pixels::BatteryUpdates battery_updates;

void loop() {
  // Always drain events from library to avoid getting old data.
  pixels::GetDieRollUpdates(roll_updates);
  pixels::GetDieBatteryUpdates(battery_updates);

  // Send out web requests if WiFi connected.
  if ((wifiMulti.run() == WL_CONNECTED)) {
    // Find the last reported roll values from each die. This is done to rate
    // limit if a bunch of rolls are reported at the same time.
    std::unordered_map<pixels::PixelsDieID, uint8_t> last_rolls;
    for (const auto &pair : roll_updates) {
      if (pair.second.state == pixels::RollState::ON_FACE) {
        last_rolls[pair.first] = pair.second.current_face;
      }
    }

    for (const auto &pair : last_rolls) {
      HTTPClient http;
      auto name = pixels::GetDieDescription(pair.first).name.c_str();
      String jsondata = String("{\"pixelName\":\"") + name +
                        "\",\"faceValue\":" + (pair.second + 1) + "}";
      USE_SERIAL.print("Sending: ");
      USE_SERIAL.println(jsondata);

      http.begin(REQUEST_URL);
      http.addHeader("Content-Type", "Content-Type: application/json");

      int httpResponseCode =
          http.POST(jsondata);  // Send the actual POST request

      if (httpResponseCode > 0) {
        USE_SERIAL.println(httpResponseCode);  // Print return code
        USE_SERIAL.println(http.getString());  // Print request answer
      } else {
        USE_SERIAL.print("Error on sending POST: ");
        USE_SERIAL.println(httpResponseCode);
      }

      http.end();
    }
  }

  delay(100);
}
