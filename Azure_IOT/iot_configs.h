// C99 libraries
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <stdio.h>
#include <cstdlib>

// Libraries for MQTT client, WiFi connection and SAS-token generation.
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <base64.h>
#include <bearssl/bearssl.h>
#include <bearssl/bearssl_hmac.h>
#include <libb64/cdecode.h>

// Azure IoT SDK for C includes
#include <az_core.h>
#include <az_iot.h>
#include <azure_ca.h>

//Others
#include <Ticker.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <DHT.h>
#include <ArduinoJson.h>

#define SIZE 20
#define LEDAZUL D0
#define DHTPIN D5
#define DHTTYPE DHT22          // DHT 11
#define DEBUG_SW 1
#define MAX_DISTANCE 400
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)

// When developing for your own Arduino-based platform,
// please follow the format '(ard;<platform>)'. 
#define AZURE_SDK_CLIENT_USER_AGENT "c%2F" AZ_SDK_VERSION_STRING "(ard;esp8266)"

// Utility macros and defines
#define LED_PIN 2
#define sizeofarray(a) (sizeof(a) / sizeof(a[0]))
#define ONE_HOUR_IN_SECS 3600
#define NTP_SERVERS "pool.ntp.org", "time.nist.gov"
#define MQTT_PACKET_SIZE 1024

// Wifi
#define IOT_CONFIG_WIFI_SSID "Your Wifi SSID"
#define IOT_CONFIG_WIFI_PASSWORD "*Your Password Wifi"

// Azure IoT
#define IOT_CONFIG_IOTHUB_FQDN "yourPage.azure-devices.net"
#define IOT_CONFIG_DEVICE_ID "MyDevice1"
#define IOT_CONFIG_DEVICE_KEY "YChfyGC6q2xOFDdrkneqiXXXXXXXXXXXXXXXXXXXXx"

// Publish 1 message every 2 seconds
#define TELEMETRY_FREQUENCY_MILLISECS 60000
