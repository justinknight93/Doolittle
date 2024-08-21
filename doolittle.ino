#define DUK_USE_GET_MONOTONIC_TIME

#include "M5Cardputer.h"
#include "M5GFX.h"
#include "stdio.h"
#include <SPI.h>
#include <SD.h>
#include <duktape.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <string.h>
#include <chrono>

#define SD_SPI_SCK_PIN  40
#define SD_SPI_MISO_PIN 39
#define SD_SPI_MOSI_PIN 14
#define SD_SPI_CS_PIN   12

String ssid = "";
String password = "";
String headers[20];
String script = "drawString('Something wrong.', 4, 4);";

HTTPClient http;

static duk_ret_t native_load(duk_context *ctx) {
  script = duk_to_string(ctx, 0);
  return 0;
}

static duk_ret_t native_print(duk_context *ctx) {
  Serial.print(duk_to_string(ctx, 0));
  return 0;
}

static duk_ret_t native_now(duk_context *ctx) {
    using namespace std::chrono;
    auto now = high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    auto millis = duration_cast<milliseconds>(duration).count();
    duk_push_number(ctx, static_cast<double>(millis));
    return 1; // Return 1 value (the timestamp) to JavaScript
}

static duk_ret_t native_delay(duk_context *ctx) {
  delay(duk_to_number(ctx, 0));
  return 0;
}

static duk_ret_t native_digitalWrite(duk_context *ctx) {
  digitalWrite(duk_to_number(ctx, 0),duk_to_boolean(ctx, 1));
  return 0;
}

static duk_ret_t native_pinMode(duk_context *ctx) {
  pinMode(duk_to_number(ctx, 0),duk_to_number(ctx, 1));
  return 0;
}

static duk_ret_t native_get(duk_context *ctx) {
  duk_idx_t obj_idx;
  if(WiFi.status()== WL_CONNECTED){
      // Your Domain name with URL path or IP address with path
      http.begin(duk_to_string(ctx, 0));

      // Add Headers if headers are included.
      if (duk_is_array(ctx, 1)) {
         // Get the length of the array
        duk_uint_t len = duk_get_length(ctx, 1);
        for (duk_uint_t i = 0; i < len; i++) {
            // Get each element in the array
            duk_get_prop_index(ctx, 1, i);
            
            // Ensure it's a string
            if (!duk_is_string(ctx, -1)) {
                duk_pop(ctx);
                duk_error(ctx, DUK_ERR_TYPE_ERROR, "Header array elements must be strings.");
            }

            // Get the string
            const char *headerKey = duk_get_string(ctx, -1);
            duk_pop(ctx);
            i++;
            duk_get_prop_index(ctx, 1, i);
            
            // Ensure it's a string
            if (!duk_is_string(ctx, -1)) {
                duk_pop(ctx);
                duk_error(ctx, DUK_ERR_TYPE_ERROR, "Header array elements must be strings.");
            }

            // Get the string
            const char *headerValue = duk_get_string(ctx, -1);
            duk_pop(ctx);
            http.addHeader(headerKey, headerValue);
        }
      }
      
      // Send HTTP GET request
      int httpResponseCode = http.GET();
      
      if (httpResponseCode>0) {
        String payload = http.getString();

        obj_idx = duk_push_object(ctx);
        duk_push_int(ctx, httpResponseCode);
        duk_put_prop_string(ctx, obj_idx, "response");
        duk_push_string(ctx, payload.c_str());
        duk_put_prop_string(ctx, obj_idx, "body");

      }
      else {
        String errorMessage = "Error Response";
        obj_idx = duk_push_object(ctx);
        duk_push_int(ctx, 0);
        duk_put_prop_string(ctx, obj_idx, "response");
        duk_push_string(ctx, errorMessage.c_str());
        duk_put_prop_string(ctx, obj_idx, "body");
      }
      // Free resources
      http.end();
    }
    else {
      String noWifiMessage = "WIFI Not Connected";
      obj_idx = duk_push_object(ctx);
      duk_push_int(ctx, 0);
      duk_put_prop_string(ctx, obj_idx, "response");
      duk_push_string(ctx, noWifiMessage.c_str());
      duk_put_prop_string(ctx, obj_idx, "body");
    }
  return 1;
}

static duk_ret_t native_color(duk_context *ctx) {
  int color = M5Cardputer.Display.color565(duk_to_int(ctx, 0),duk_to_int(ctx, 1),duk_to_int(ctx, 2));
  duk_push_int(ctx, color);
  return 1;
}

static duk_ret_t native_setTextColor(duk_context *ctx) {
  M5Cardputer.Display.setTextColor(duk_to_int(ctx, 0));
  return 0;
}

static duk_ret_t native_setTextSize(duk_context *ctx) {
  M5Cardputer.Display.setTextSize(duk_to_number(ctx, 0));
  return 0;
}

static duk_ret_t native_drawRect(duk_context *ctx) {
  M5Cardputer.Display.drawRect(duk_to_int(ctx, 0),duk_to_int(ctx, 1),duk_to_int(ctx, 2),duk_to_int(ctx, 3),duk_to_int(ctx, 4));
  return 0;
}

static duk_ret_t native_drawFillRect(duk_context *ctx) {
  M5Cardputer.Display.fillRect(duk_to_int(ctx, 0),duk_to_int(ctx, 1),duk_to_int(ctx, 2),duk_to_int(ctx, 3),duk_to_int(ctx, 4));
  return 0;
}

static duk_ret_t native_drawString(duk_context *ctx) {
  M5Cardputer.Display.drawString(duk_to_string(ctx, 0),duk_to_int(ctx, 1),duk_to_int(ctx, 2));
  return 0;
}

static duk_ret_t native_width(duk_context *ctx) {
  int width = M5Cardputer.Display.width();
  duk_push_int(ctx, width);
  return 1;
}

static duk_ret_t native_height(duk_context *ctx) {
  int height = M5Cardputer.Display.height();
  duk_push_int(ctx, height);
  return 1;
}

static duk_ret_t native_getKeysPressed(duk_context *ctx) {
  // Create a new array on the stack
  duk_push_array(ctx);

  M5Cardputer.update();
    if (M5Cardputer.Keyboard.isChange()) {
        if (M5Cardputer.Keyboard.isPressed()) {
            Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
            int arrayIndex = 0;
            for (auto i : status.word) {
                char str[2] = { i, '\0' };
                duk_push_string(ctx, str);
                duk_put_prop_index(ctx, -2, arrayIndex);
                arrayIndex++;
            }

            if (status.del) {
                duk_push_string(ctx, "Delete");
                duk_put_prop_index(ctx, -2, arrayIndex);
                arrayIndex++;
            }

            if (status.enter) {
                duk_push_string(ctx, "Enter");
                duk_put_prop_index(ctx, -2, arrayIndex);
                arrayIndex++;
            }

            if (status.alt) {
                duk_push_string(ctx, "Alt");
                duk_put_prop_index(ctx, -2, arrayIndex);
                arrayIndex++;
            }

            if (status.tab) {
                duk_push_string(ctx, "Tab");
                duk_put_prop_index(ctx, -2, arrayIndex);
                arrayIndex++;
            }

            if (status.fn) {
                duk_push_string(ctx, "Function");
                duk_put_prop_index(ctx, -2, arrayIndex);
                arrayIndex++;
            }

            if (status.opt) {
                duk_push_string(ctx, "Option");
                duk_put_prop_index(ctx, -2, arrayIndex);
                arrayIndex++;
            }
        }
    }
  return 1;
}

void setup() {
  Serial.begin(9600);

  // SD Card Initialization
  SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);

  // Setup M5 Cardputer
  auto cfg = M5.config();
  M5Cardputer.begin(cfg, true);

  loadConfigFile(ssid,password);
  // Connect to Wifi
  WiFi.begin(ssid, password);

  script = readBootFile();
  
}

void loop() {
  M5Cardputer.Display.fillRect(0,0,M5Cardputer.Display.width(),M5Cardputer.Display.height(),BLACK);
  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setTextSize(2);
  M5Cardputer.Display.setTextColor(WHITE);
  // Create context.
  duk_context *ctx = duk_create_heap_default();

  // Add native functions to context.
  duk_push_c_function(ctx, native_load, 1);
  duk_put_global_string(ctx, "load");
  duk_push_c_function(ctx, native_print, 1);
  duk_put_global_string(ctx, "print");
  duk_push_c_function(ctx, native_now, 0);
  duk_put_global_string(ctx, "now");
  duk_push_c_function(ctx, native_delay, 1);
  duk_put_global_string(ctx, "delay");
  duk_push_c_function(ctx, native_digitalWrite, 2);
  duk_put_global_string(ctx, "digitalWrite");
  duk_push_c_function(ctx, native_pinMode, 2);
  duk_put_global_string(ctx, "pinMode");

  // Networking
  duk_push_c_function(ctx, native_get, 2);
  duk_put_global_string(ctx, "httpGet");

  // Graphics
  duk_push_c_function(ctx, native_color, 3);
  duk_put_global_string(ctx, "color");
  duk_push_c_function(ctx, native_setTextColor, 1);
  duk_put_global_string(ctx, "setTextColor");
  duk_push_c_function(ctx, native_setTextSize, 1);
  duk_put_global_string(ctx, "setTextSize");
  duk_push_c_function(ctx, native_drawRect, 5);
  duk_put_global_string(ctx, "drawRect");
  duk_push_c_function(ctx, native_drawFillRect, 5);
  duk_put_global_string(ctx, "drawFillRect");
  duk_push_c_function(ctx, native_drawString, 3);
  duk_put_global_string(ctx, "drawString");
  duk_push_c_function(ctx, native_width, 0);
  duk_put_global_string(ctx, "width");
  duk_push_c_function(ctx, native_height, 0);
  duk_put_global_string(ctx, "height");

  // Input
  duk_push_c_function(ctx, native_getKeysPressed, 0);
  duk_put_global_string(ctx, "getKeysPressed");

  duk_push_string(ctx, script.c_str());
  if (duk_peval(ctx) != 0) {
      printf("eval failed: %s\n", duk_safe_to_string(ctx, -1));
  } else {
      printf("result is: %s\n", duk_safe_to_string(ctx, -1));
  }
  duk_pop(ctx);

  // Clean up.
  duk_destroy_heap(ctx);

  delay(1000);
}

String readBootFile() {
    String mountError = "drawString('Can't mount card.', 4, 4);";
    String cardError = "drawString('No SD card.', 4, 4);";
    String fileError = "drawString('No boot.js file.', 4, 4);";
    
    if (!SD.begin(SD_SPI_CS_PIN, SPI, 25000000)) {
        Serial.println("Failed to mount");
        return mountError;
    }

    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE) {
        Serial.println("No Card");
        return cardError;
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);

    File file = SD.open("/doolittle/boot.js");
    if (!file) {
        return fileError;
    }

    String s;
    Serial.println("Read from file");
    while (file.available()) {
        s += (char)file.read();
    }
    file.close();
    Serial.println("loaded file:");
    Serial.println(s);
    return s;
}

// Function to load SSID and password from a config file
void loadConfigFile(String &ssid, String &password) {
    String mountError = "Can't mount card.";
    String cardError = "No SD card.";
    String fileError = "No config.txt file.";
    String formatError = "Invalid config file format.";

    // Initialize SD card
    if (!SD.begin(SD_SPI_CS_PIN, SPI, 25000000)) {
        Serial.println(mountError);
        return;
    }

    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        Serial.println(cardError);
        return;
    }

    // Open the config file
    File file = SD.open("/doolittle/config.txt");
    if (!file) {
        Serial.println(fileError);
        return;
    }

    // Read the file line by line and extract SSID and passkey
    String line;
    bool ssidFound = false;
    bool passwordFound = false;
    while (file.available()) {
        line = file.readStringUntil('\n');
        line.trim(); // Remove any leading or trailing whitespace

        if (line.startsWith("SSID:")) {
            String extractedSSID = line.substring(5);
            extractedSSID.trim(); // Trim the extracted SSID
            ssid = extractedSSID;
            ssidFound = true;
        } else if (line.startsWith("PASSWORD:")) {
            String extractedPassword = line.substring(9);
            extractedPassword.trim(); // Trim the extracted password
            password = extractedPassword;
            passwordFound = true;
        }
    }
    file.close();

    // Check if both SSID and passkey were found
    if (!ssidFound || !passwordFound) {
        Serial.println(formatError);
        return;
    }

    Serial.println("Config loaded:");
    Serial.print("SSID: ");
    Serial.println(ssid);
    Serial.print("Passkey: ");
    Serial.println(password);
}

void writeFile(fs::FS &fs, const char *path, const char *message) {
    Serial.print("Writing file");

    File file = fs.open(path, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }
    if (file.print(message)) {
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message) {
    Serial.print("Appending to file");

    File file = fs.open(path, FILE_APPEND);
    if (!file) {
        Serial.println("Failed to open file for appending");
        return;
    }
    if (file.print(message)) {
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}

void renameFile(fs::FS &fs, const char *path1, const char *path2) {
    Serial.print("Renaming file");
    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}

void deleteFile(fs::FS &fs, const char *path) {
    Serial.print("Deleting file");
    if (fs.remove(path)) {
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}