/**
 * Send dice rolls as JSON data to an MQTT broker.
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
 * This example depends on https://github.com/256dpi/arduino-mqtt which can be
 * installed through the Arduino library as "MQTT".
 * 
 * For an example of how to build an application that uses the dices' MQTT
 * output to log rolls to a CSV file, see `dice_logger.py`
 */

#include <Arduino.h>
#include <WiFi.h>

#include <MQTT.h>
#include <pixels_dice_interface.h>

// SET THESE VALUES TO THE WIFI CREDENTIALS TO BE USED!
static constexpr const char *SSID = "SSID";
static constexpr const char *PASSWORD = "PASSWORD";
// SET THE SERVER BELOW TO THE IP OF THE MQTT BROKER TO USE!
static constexpr const char *SERVER_ADDR = "SERVER";

WiFiClient net;
MQTTClient client;

// The vectors to hold results queried from the library
// Since vectors allocate data, it's more efficient to keep reusing objects
// instead of declaring them on the stack
std::vector<pixels::PixelsDieID> dice_list;
pixels::RollUpdates roll_updates;
pixels::BatteryUpdates battery_updates;

void connect()
{
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nconnecting...");
  while (!client.connect("arduino", "public", "public"))
  {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nconnected!");
}

void setup()
{
  Serial.begin(9600);
  // Start BLE scans for 2 seconds then waiting 5 seconds for the next scan
  // On completion the discovered dice are connected to
  pixels::ScanForDice(2, 5);

  WiFi.begin(SSID, PASSWORD);

  // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported
  // by Arduino. You need to set the IP address directly.
  client.begin(SERVER_ADDR, net);

  connect();
}

void loop()
{
  client.loop();
  delay(10); // <- fixes some issues with WiFi stability

  if (!client.connected())
  {
    connect();
  }

  // Update dice_list with the connected dice
  pixels::ListDice(dice_list);
  // Get all the roll/battery updates since the last loop
  pixels::GetDieRollUpdates(roll_updates);
  pixels::GetDieBatteryUpdates(battery_updates);

  for (const auto &roll : roll_updates)
  {
    const char *name = pixels::GetDieDescription(roll.first).name.c_str();
    Serial.printf("Roll %s: %s value: %u\n",
                  name,
                  pixels::ToString(roll.second.state),
                  roll.second.current_face + 1);
    char json_buffer[128];
    sprintf(json_buffer,
            "{\"name\":\"%s\",\"state\":%d,\"val\":%d,\"time\":%d}",
            name,
            int(roll.second.state),
            roll.second.current_face + 1,
            roll.second.timestamp);
    client.publish("/dice/roll", json_buffer);
  }
}
