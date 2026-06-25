# 🐕 ESP32 RoboDog Firmware

12-DOF Quadruped Robot powered by **ESP32** and **PCA9685 16-Channel Servo Driver**.

Supports:

* 🦴 Stand posture
* 💤 Rest posture
* 👋 Hello / High-Five emote
* 🚶 Walking gait
* 🎮 Live joint calibration
* 📱 Android Bluetooth control
* 🔌 USB Serial control

---

# 🔌 Hardware Configuration

## ESP32 Connections

| Device      | ESP32 Pin |
| ----------- | --------- |
| SDA         | GPIO21    |
| SCL         | GPIO22    |
| PCA9685 VCC | 5V        |
| PCA9685 GND | GND       |

---

## PCA9685 Channel Mapping

| Joint | Description          | Channel |
| ----- | -------------------- | ------- |
| FLK   | Front Left Knee      | 0       |
| FRK   | Front Right Knee     | 1       |
| FLS   | Front Left Shoulder  | 2       |
| FRS   | Front Right Shoulder | 3       |
| FLH   | Front Left Hip       | 4       |
| FRH   | Front Right Hip      | 5       |
| RLH   | Rear Left Hip        | 10      |
| RRH   | Rear Right Hip       | 11      |
| RLS   | Rear Left Shoulder   | 12      |
| RRS   | Rear Right Shoulder  | 13      |
| RLK   | Rear Left Knee       | 14      |
| RRK   | Rear Right Knee      | 15      |

---

# ⚙ Servo Configuration

Servo pulse limits are configured to match Arduino Servo library safe limits.

```cpp
SERVOMIN = 111
SERVOMAX = 491
```

Equivalent to:

```text
544 μs → 0°
2400 μs → 180°
```

Servo frequency:

```cpp
50 Hz
```

---

# 📐 Motion Profile Matrix

## 1️⃣ Stand Up (`standUp()`)

| Joint | Angle |
| ----- | ----- |
| FLK   | 100° |
| FRK   | 105° |
| FLS   | 140° |
| FRS   | 30°  |
| FLH   | 85°  |
| FRH   | 90°  |
| RLH   | 145° |
| RRH   | 40°  |
| RLS   | 140° |
| RRS   | 70°  |
| RLK   | 110° |
| RRK   | 100° |

---

## 2️⃣ Rest (`rest()`)

| Joint | Angle |
| ----- | ----- |
| FLK   | 25°  |
| FRK   | 180° |
| FLS   | 180° |
| FRS   | 0°   |
| FLH   | 25°  |
| FRH   | 155° |
| RLH   | 85°  |
| RRH   | 100° |
| RLS   | 180° |
| RRS   | 35°  |
| RLK   | 35°  |
| RRK   | 175° |

---

## 3️⃣ Hello / High-Five (`hello()`)

### Front Setup

| Joint | Angle |
| ----- | ----- |
| FLS   | 145° |
| FRS   | 35°  |
| FLK   | 100° |
| FRK   | 110° |
| FLH   | 95°  |
| FRH   | 95°  |

### Rear Setup

| Joint | Angle |
| ----- | ----- |
| RLS   | 150° |
| RRS   | 60°  |
| RLK   | 30°  |
| RRK   | 180° |
| RLH   | 150° |
| RRH   | 30°  |

### High-Five Motion

FRS performs a waving motion:

```
35°
↓
150°
↓
20°
↓
Stand Position
```

Wave duration:

```text
2 Seconds
```

---

# 🚶 Walking Gait

Command:

```text
4
```

Sequence:

The gait alternates diagonal leg pairs.

### Pair 1

Front Left Knee

```text
100° → 70°
```

Rear Right Knee

```text
100° → 130°
```

Shoulder Drag

```text
FRS = 10°
RLS = 160°
```

---

### Pair 2

Rear Left Knee

```text
110° → 80°
```

Front Right Knee

```text
105° → 135°
```

Shoulder Drag

```text
FLS = 160°

RRS = 50°
```

Walking command performs:

```text
Stand
↓

4 gait cycles

↓

Stand
```

---

# 🚶 Walking Cycle (`walk()`)

The RoboDog implements a **simple diagonal gait** that alternates movement between opposite leg pairs to simulate forward walking while maintaining stability.

Each execution of `walk()` performs **one complete gait cycle**.

Command `4` performs:

```text
Stand Up
    ↓
Walk Cycle × 4
    ↓
Stand Up
```

---

## Walking Sequence Breakdown

### 🦿 Phase 1 – Pair 1 Knee Lift

The robot lifts the **Front Left** and **Rear Right** legs.

| Joint | Angle |
| ----- | ----- |
| FLK   | 70°  |
| RRK   | 130° |

Delay:

```text
200 ms
```

---

### 🦿 Phase 2 – Pair 1 Shoulder Drag

The lifted legs are moved forward.

| Joint | Angle |
| ----- | ----- |
| FRS   | 10°  |
| RLS   | 160° |

---

### 🦿 Phase 3 – Pair 1 Recovery

The robot returns Pair 1 to the neutral standing posture.

| Joint | Angle |
| ----- | ----- |
| FLK   | 100° |
| RRK   | 100° |
| FLS   | 140° |
| RRS   | 70°  |

Delay:

```text
200 ms
```

---

### 🦿 Phase 4 – Pair 2 Knee Lift

The robot lifts the opposite diagonal pair.

| Joint | Angle |
| ----- | ----- |
| RLK   | 80°  |
| FRK   | 135° |

Delay:

```text
200 ms
```

---

### 🦿 Phase 5 – Pair 2 Shoulder Drag

The lifted legs are swung forward.

| Joint | Angle |
| ----- | ----- |
| FLS   | 160° |
| RRS   | 50°  |

---

### 🦿 Phase 6 – Pair 2 Recovery

Return Pair 2 to the standard standing pose.

| Joint | Angle |
| ----- | ----- |
| FRS   | 30°  |
| RLS   | 140° |
| FRK   | 105° |
| RLK   | 110° |

Delay:

```text
200 ms
```

---

## Walking Gait Visualization

```text
              Pair 1
      (FL Leg + RR Leg)

          Lift Knees
               ↓
         Swing Forward
               ↓
       Return to Stand
               ↓

────────────────────────

              Pair 2
      (FR Leg + RL Leg)

          Lift Knees
               ↓
         Swing Forward
               ↓
       Return to Stand
```

---

## Walking Characteristics

| Property       | Value                     |
| -------------- | ------------------------- |
| Gait Type      | Diagonal Alternating Gait |
| Stability      | Medium                    |
| Step Delay     | 200 ms                    |
| Cycle Duration | ~0.8 s                    |
| Command        | `4`                     |
| Repetitions    | 4 Cycles                  |
| Initial Pose   | `standUp()`             |
| Final Pose     | `standUp()`             |

### Motion Flow

```text
Stand
   ↓
Cycle 1
   ↓
Cycle 2
   ↓
Cycle 3
   ↓
Cycle 4
   ↓
Stand
```

---

# 🎬 Stunt Sequences

## 1 — Stand

Raises the robot from resting posture into a stable standing pose.

Phases:

1. Hips
2. Shoulders
3. Knees

---

## 2 — Rest

Folds legs underneath the chassis.

Phases:

1. Knees
2. Shoulders
3. Hips

---

## 3 — Hello

Procedure:

```text
Stand
↓

Front Adjustment

↓

Rear Adjustment

↓

High-Five

↓

Return to Stand
```

---

## 4 — Walk

Procedure:

```text
Stand

↓

4 Walking Cycles

↓

Stand
```

---

# 📱 Bluetooth Control

ESP32 creates a Bluetooth device:

```text
ESP32_Robodog
```

Connect using:

* Android Bluetooth Terminal
* Serial Bluetooth Terminal
* Bluetooth Electronics

All commands available on USB Serial also work over Bluetooth.

---

# 💻 Command Interface

Baud Rate

```text
115200
```

Line Ending

```text
Newline (\n)
```

---

## Stunt Commands

| Command | Action |
| ------- | ------ |
| 1       | Stand  |
| 2       | Rest   |
| 3       | Hello  |
| 4       | Walk   |

---

## Live Calibration

Format:

```text
<KEYWORD> <ANGLE>
```

Examples:

```text
FRK 90

FLS 140

RLH 120

RRK 175
```

Supported Keywords:

```text
FLS
FLH
FLK

FRS
FRH
FRK

RLS
RLH
RLK

RRS
RRH
RRK
```

Angles accepted:

```text
0° – 180°
```

Example response:

```text
SUCCESS: Moved FRK (Ch 1) to 90 degrees.
```

Errors:

```text
ERROR: Angle must be between 0 and 180.


ERROR: Unknown keyword.


ERROR: Invalid format.
```

---

# 🐾 RoboDog Features

✔ 12-DOF Quadruped

✔ ESP32 Based

✔ PCA9685 Servo Expansion

✔ Bluetooth Control

✔ USB Serial Control

✔ Real-Time Calibration

✔ High-Five Emote

✔ Walking Gait

✔ Safe Servo Pulse Limits

✔ Modular Motion Sequences
