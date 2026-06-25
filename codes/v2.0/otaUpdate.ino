// tweek some angles of the servos using serial commands and also support OTA updates for the ESP32.

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <WiFi.h>          // Core ESP32 Wi-Fi library
#include <ESPmDNS.h>        // Required for mDNS (network discovery)
#include <WiFiUdp.h>        // Required for OTA data transport
#include <ArduinoOTA.h>     // Core Over-The-Air library

// --- YOUR WI-FI CREDENTIALS ---
const char* ssid = "akshat";         // Replace with your Wi-Fi name
const char* password = "82189166"; // Replace with your Wi-Fi password

Adafruit_PWMServoDriver pca = Adafruit_PWMServoDriver();

// Safe limits matched to Arduino Uno's Servo.h (544us to 2400us)
#define SERVOMIN 111   // Safe pulse length for 0°
#define SERVOMAX 491   // Safe pulse length for 180°

bool isStanding = false;

void setup() {
  Serial.begin(115200);

   // --- CONNECT TO WI-FI ---
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  // Wi-Fi connection timeout (10 seconds max) to prevent bricking the boots
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi Connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWi-Fi Connection Failed. Continuing offline...");
  }

   // --- ARDUINO OTA SETUP ---
  ArduinoOTA.setHostname("esp32-robodog");
  ArduinoOTA.begin();

  // Initialize I2C. ESP boards handle this automatically via Wire.begin()
  // ESP32 defaults: SDA = 21, SCL = 22
  // ESP8266 defaults: SDA = D2, SCL = D1
  Wire.begin(21, 22); 
  pca.begin();
  pca.setPWMFreq(50);  // Standard analog servo frequency

  rest();

  Serial.println("=== ESP Robodog OS Initialized ===");
  Serial.println("COMMAND OPTIONS:");
  Serial.println("1. Type '1' to execute STAND sequence.");
  Serial.println("2. Type '<KEYWORD> <ANGLE>' to tweak a joint (e.g., 'FRK 90').");
  Serial.println("Keywords: FLS, FLH, FLK, FRS, FRH, FRK, RLS, RLH, RLK, RRS, RRH, RRK");
}

void loop() {
  // Check for OTA updates continuously
  ArduinoOTA.handle();
  
  if (Serial.available()) {
    // Read the entire line until the user presses Enter
    String input = Serial.readStringUntil('\n');
    input.trim(); // Remove any invisible carriage returns or extra spaces

    if (input.length() > 0) {
      
      // === STUNT MODE ===
      if (input == "1") {
        standUp();
      } 
      // ===RESTING MODE===
      else if (input == "2") {
        rest();
      }
      // ===HELLO MODE===
      else if (input == "3") {
        hello();
      }
      // === CALIBRATION MODE ===
      else {
        int spaceIndex = input.indexOf(' ');

        if (spaceIndex > 0) {
          String keyword = input.substring(0, spaceIndex);
          keyword.toUpperCase(); 
          
          int angle = input.substring(spaceIndex + 1).toInt();
          int channel = getChannelFromKeyword(keyword);

          if (channel != -1) {
            if (angle >= 0 && angle <= 180) {
              setServoAngle(channel, angle);
              Serial.print("SUCCESS: Moved ");
              Serial.print(keyword);
              Serial.print(" (Ch ");
              Serial.print(channel);
              Serial.print(") to ");
              Serial.print(angle);
              Serial.println(" degrees.");
            } else {
              Serial.println("ERROR: Angle must be between 0 and 180.");
            }
          } else {
            Serial.println("ERROR: Unknown keyword. Check your spelling (e.g., FRK).");
          }
        } else {
          Serial.println("ERROR: Invalid format. Use '1' to stand, or 'FRK 90' to tweak.");
        }
      }
    }
  }
}

// Helper function to map keywords to PCA9685 channels
int getChannelFromKeyword(String keyword) {
  // Front Left
  if (keyword == "FLS") return 2;
  if (keyword == "FLH") return 4;
  if (keyword == "FLK") return 0;
  
  // Front Right
  if (keyword == "FRS") return 3;
  if (keyword == "FRH") return 5;
  if (keyword == "FRK") return 1;

  // Rear Left
  if (keyword == "RLS") return 12;
  if (keyword == "RLH") return 10;
  if (keyword == "RLK") return 14;

  // Rear Right
  if (keyword == "RRS") return 13;
  if (keyword == "RRH") return 11;
  if (keyword == "RRK") return 15;

  return -1; 
}

// Helper function to send the PWM pulse to the servo safely
void setServoAngle(uint8_t channel, int angle) {
  angle = constrain(angle, 0, 180); 
  int pulse = map(angle, 0, 180, SERVOMIN, SERVOMAX);
  pca.setPWM(channel, 0, pulse);
}

// ==========================================
// STUNT FUNCTIONS
// ==========================================

// Stunt 1: Stand Up sequence
void standUp() {

  if(isStanding) {
    Serial.println("Already in a standing Position");
    return;
  }

  Serial.println("--- EXECUTING STUNT: STAND UP ---");
  
  // --- PHASE 1: HIPS ---
  Serial.println("Phase 1: Adjusting Hips...");
  setServoAngle(10, 110); // RLH
  // delay(1000);
  setServoAngle(11, 95); // RRH
  // delay(1000);
  setServoAngle(4, 90);  // FLH
  // delay(1000);
  setServoAngle(5, 100);  // FRH
  // delay(1000);
  delay(2000);

  // --- PHASE 2: SHOULDERS ---
  Serial.println("Phase 2: Adjusting Shoulders...");
  setServoAngle(2, 140);  // FLS
  // delay(1000);
  setServoAngle(3, 35);  // FRS
  // delay(1000);
  setServoAngle(12, 135); // RLS
  // delay(1000);
  setServoAngle(13, 80); // RRS
  delay(2000);

  // --- PHASE 3: ALL KNEES (SLOW SYNC) ---
  Serial.println("Phase 3: Adjusting Knee...");
  setServoAngle(14, 110);   // RLK
  setServoAngle(15, 100); // RRK 
  delay(500);
  setServoAngle(1, 110);  // FRK 
  setServoAngle(0, 100);    // FLK
  delay(1000); // Brief pause to ensure they are at the starting position

  isStanding = true;

  Serial.println("--- STUNT COMPLETE: DOG IS STANDING ---");
  Serial.println("You can now enter '<KEYWORD> <ANGLE>' to tweak positions.");
}

// Stunt 2: Resting sequence
void rest() {
  Serial.println("--- EXECUTING STUNT: RESTING ---");
  
  // --- PHASE 1: ALL KNEES (SLOW SYNC) ---
  Serial.println("Phase 1: Adjusting Knee...");
  setServoAngle(14, 35);   // RLK 
  setServoAngle(15, 175); // RRK 
  // delay(500);
  setServoAngle(1, 180);  // FRK 
  setServoAngle(0, 25);    // FLK
  delay(1000);
  
  // --- PHASE 2: SHOULDERS ---
  Serial.println("Phase 2: Adjusting Shoulders...");
  setServoAngle(2, 180);  // FLS
  // delay(1000);
  setServoAngle(3, 0);  // FRS
  // delay(1000);
  setServoAngle(12, 180); // RLS
  // delay(1000);
  setServoAngle(13, 25); // RRS
  delay(1000);

  // --- PHASE 3: HIPS ---
  Serial.println("Phase 3: Adjusting Hips...");
  setServoAngle(4, 30);  // FLH
  // delay(1000);
  setServoAngle(5, 160);  // FRH
  // delay(1000);
  setServoAngle(10, 40); // RLH
  // delay(1000);
  setServoAngle(11, 160); // RRH
  delay(1000);

  isStanding = false;

  Serial.println("--- STUNT COMPLETE: DOG IS RESTING ---");
  Serial.println("You can now enter '<KEYWORD> <ANGLE>' to tweak positions.");
}

// Stunt 3: Hello sequence
void hello() {
  Serial.println("--- EXECUTING STUNT: HELLO ---");
  
  // --- PHASE 1: STANDING UP ---
  standUp();
  delay(1000);

  // --- PHASE 1: Front Setup ---
  Serial.println("Adjusting the Front.");
  setServoAngle(2, 135);  // FLS
  setServoAngle(3, 35);  // FRS
  setServoAngle(1, 110);  // FRK 
  setServoAngle(0, 100);    // FLK
  setServoAngle(4, 95);  // FLH
  setServoAngle(5, 95);  // FRH
  delay(1000);

  // --- PHASE 2: Back Setup ---
  Serial.println("Adjusting the Back.");
  setServoAngle(12, 150); // RLS
  setServoAngle(13, 60); // RRS  
  setServoAngle(14, 30);   // RLK
  setServoAngle(15, 180); // RRK
  setServoAngle(10, 110); // RLH
  setServoAngle(11, 95); // RRH
  delay(1000);

  // --- PHASE 3: Hello Emote ---
  Serial.println("Doing High-five action.");
  setServoAngle(3, 150);  // FRS
  delay(2000);
  setServoAngle(3, 35);  // FRS
  delay(1000);

  Serial.println("Adjusting the Front.");
  setServoAngle(0, 25);   // FLK
  setServoAngle(1, 180);  // FRK

  isStanding = false;

  standUp(); 
  delay(1000);
}