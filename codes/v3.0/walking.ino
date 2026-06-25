// Tweak some angles of the servos using serial commands via USB or Android Bluetooth.
// Wi-Fi and OTA have been removed to save memory space.
// This code also contains a walking sequence that can be triggered by sending the command '4' via serial or Bluetooth.
// The walking sequence is a simple gait that alternates the movement of the legs to simulate walking.

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "BluetoothSerial.h" // Required for Android Bluetooth Terminal

// Ensure Bluetooth is enabled in the compiler
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

Adafruit_PWMServoDriver pca = Adafruit_PWMServoDriver();
BluetoothSerial SerialBT; // Initialize Bluetooth Serial object

// Safe limits matched to Arduino Uno's Servo.h (544us to 2400us)
#define SERVOMIN 111   // Safe pulse length for 0°
#define SERVOMAX 491   // Safe pulse length for 180°

bool isStanding = false;

// --- HELPER FUNCTIONS FOR DUAL LOGGING ---
// These send text to BOTH the computer (USB) and the Android Phone (Bluetooth)
void logPrintln(String msg) {
  Serial.println(msg);
  SerialBT.println(msg);
}

void logPrint(String msg) {
  Serial.print(msg);
  SerialBT.print(msg);
}

void setup() {
  Serial.begin(115200);
  
  // Start Bluetooth Service
  SerialBT.begin("ESP32_Robodog"); // This is the name you will look for on your phone
  logPrintln("\nBluetooth Started! Pair your phone to 'ESP32_Robodog'");

  // Initialize I2C. ESP boards handle this automatically via Wire.begin()
  Wire.begin(21, 22); 
  pca.begin();
  pca.setPWMFreq(50);  // Standard analog servo frequency

  rest();

  logPrintln("=== ESP Robodog OS Initialized ===");
  logPrintln("COMMAND OPTIONS:");
  logPrintln("1. Type '1' to execute STAND sequence.");
  logPrintln("2. Type '<KEYWORD> <ANGLE>' to tweak a joint (e.g., 'FRK 90').");
  logPrintln("Keywords: FLS, FLH, FLK, FRS, FRH, FRK, RLS, RLH, RLK, RRS, RRH, RRK");
}

void loop() {
  String input = "";
  
  // Check if data is coming from USB Serial OR Bluetooth Serial
  if (Serial.available()) {
    input = Serial.readStringUntil('\n');
  } else if (SerialBT.available()) {
    input = SerialBT.readStringUntil('\n');
  }

  input.trim(); // Remove any invisible carriage returns or extra spaces

  if (input.length() > 0) {
      
    // === STUNT MODE ===
    if (input == "1") {
      standUp();
    } 
    // === RESTING MODE ===
    else if (input == "2") {
      rest();
    }
    // === HELLO MODE ===
    else if (input == "3") {
      hello();
    }
    // === WALKING MODE ===
    else if(input == "4"){
      standUp();
      for(int i=0; i<4; i++) {
        walk();
      }
      standUp();
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
            logPrint("SUCCESS: Moved ");
            logPrint(keyword);
            logPrint(" (Ch ");
            logPrint(String(channel));
            logPrint(") to ");
            logPrint(String(angle));
            logPrintln(" degrees.");
          } else {
            logPrintln("ERROR: Angle must be between 0 and 180.");
          }
        } else {
          logPrintln("ERROR: Unknown keyword. Check your spelling (e.g., FRK).");
        }
      } else {
        logPrintln("ERROR: Invalid format. Use '1' to stand, or 'FRK 90' to tweak.");
      }

      isStanding = false; // Reset standing state after any manual adjustment
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
    logPrintln("Already in a standing Position");
    return;
  }

  logPrintln("--- EXECUTING STUNT: STAND UP ---");
  
  // --- PHASE 1: HIPS ---
  logPrintln("Phase 1: Adjusting Hips...");
  setServoAngle(10, 145); // RLH
  setServoAngle(11, 40); // RRH
  setServoAngle(4, 85);  // FLH
  setServoAngle(5, 90);  // FRH
  delay(1000);

  // --- PHASE 2: SHOULDERS ---
  logPrintln("Phase 2: Adjusting Shoulders...");
  setServoAngle(2, 140);  // FLS
  setServoAngle(3, 30);  // FRS
  setServoAngle(12, 140); // RLS
  setServoAngle(13, 70); // RRS
  delay(1000);

  // --- PHASE 3: ALL KNEES (SLOW SYNC) ---
  logPrintln("Phase 3: Adjusting Knee...");
  setServoAngle(14, 110);   // RLK
  setServoAngle(15, 100); // RRK 
  delay(500);
  setServoAngle(1, 105);  // FRK 
  setServoAngle(0, 100);    // FLK
  delay(1000); // Brief pause to ensure they are at the starting position

  isStanding = true;

  logPrintln("--- STUNT COMPLETE: DOG IS STANDING ---");
  logPrintln("You can now enter '<KEYWORD> <ANGLE>' to tweak positions.");
}

// Stunt 2: Resting sequence
void rest() {
  logPrintln("--- EXECUTING STUNT: RESTING ---");
  
  // --- PHASE 1: ALL KNEES (SLOW SYNC) ---
  logPrintln("Phase 1: Adjusting Knee...");
  setServoAngle(14, 35);   // RLK 
  setServoAngle(15, 175); // RRK 
  setServoAngle(1, 180);  // FRK 
  setServoAngle(0, 25);    // FLK
  delay(1000);
  
  // --- PHASE 2: SHOULDERS ---
  logPrintln("Phase 2: Adjusting Shoulders...");
  setServoAngle(2, 180);  // FLS
  setServoAngle(3, 0);  // FRS
  setServoAngle(12, 180); // RLS
  setServoAngle(13, 35); // RRS
  delay(1000);

  // --- PHASE 3: HIPS ---
  logPrintln("Phase 3: Adjusting Hips...");
  setServoAngle(4, 25);  // FLH
  setServoAngle(5, 155);  // FRH
  setServoAngle(10, 85); // RLH
  setServoAngle(11, 100); // RRH
  delay(1000);

  isStanding = false;

  logPrintln("--- STUNT COMPLETE: DOG IS RESTING ---");
  logPrintln("You can now enter '<KEYWORD> <ANGLE>' to tweak positions.");
}

// Stunt 3: Hello sequence
void hello() {
  logPrintln("--- EXECUTING STUNT: HELLO ---");
  
  // --- PHASE 1: STANDING UP ---
  standUp();
  delay(1000);

  // --- PHASE 1: Front Setup ---
  logPrintln("Adjusting the Front.");
  setServoAngle(2, 145);  // FLS
  setServoAngle(3, 35);  // FRS
  setServoAngle(1, 110);  // FRK 
  setServoAngle(0, 100);    // FLK
  setServoAngle(4, 95);  // FLH
  setServoAngle(5, 95);  // FRH
  delay(1000);

  // --- PHASE 2: Back Setup ---
  logPrintln("Adjusting the Back.");
  setServoAngle(12, 150); // RLS
  setServoAngle(13, 60); // RRS  
  setServoAngle(14, 30);   // RLK
  setServoAngle(15, 180); // RRK
  setServoAngle(10, 150); // RLH
  setServoAngle(11, 30); // RRH
  delay(1000);

  // --- PHASE 3: Hello Emote ---
  logPrintln("Doing High-five action.");
  setServoAngle(3, 150);  // FRS
  delay(2000);
  setServoAngle(3, 20);  // FRS
  delay(1000);

  logPrintln("Adjusting the Front.");
  setServoAngle(0, 25);   // FLK
  setServoAngle(1, 180);  // FRK

  isStanding = false;

  standUp(); 
  delay(1000);
}

// Stunt 4: Walking sequence
void walk() {
  // Pair 1: Knee Up
  setServoAngle(0, 70);    // FLK
  setServoAngle(15, 130);   // RRK

  delay(200);

  //Pair 2: Sholder Drag
  setServoAngle(3, 10);  // FRS
  setServoAngle(12, 160); // RLS

  //Pair 1: Sholder & Knee Standing position
  setServoAngle(0, 100);    // FLK
  setServoAngle(15, 100); // RRK 
  setServoAngle(2, 140);  // FLS
  setServoAngle(13, 70); // RRS

  delay(200);

  //Pair 2: Knee Up
  setServoAngle(14, 80);   // RLK
  setServoAngle(1, 135);    // FRK 

  delay(200);

  //Pair 1: Sholder Drag
  setServoAngle(2, 160);  // FLS
  setServoAngle(13, 50); // RRS

  //Pair 2: Sholder & Knee Standing position
  setServoAngle(3, 30);  // FRS
  setServoAngle(12, 140); // RLS
  setServoAngle(1, 105);  // FRK 
  setServoAngle(14, 110);   // RLK

  delay(200);

  isStanding = false;
}