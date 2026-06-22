# 🐕 ESP32 RoboDog Angles

Servo angles for a 12-DOF Quadruped Robot using the **PCA9685 I2C PWM Driver** in this project.

---

## 🔌 Hardware Configuration & Pin Map

### PCA9685 Channel Mapping

| Joint Keyword | Description          | PCA9685 Channel |
| :------------ | :------------------- | :-------------: |
| **FLK** | Front Left Knee      |        0        |
| **FRK** | Front Right Knee     |        1        |
| **FLS** | Front Left Shoulder  |        2        |
| **FRS** | Front Right Shoulder |        3        |
| **FLH** | Front Left Hip       |        4        |
| **FRH** | Front Right Hip      |        5        |
| **RLH** | Rear Left Hip        |       10       |
| **RRH** | Rear Right Hip       |       11       |
| **RLS** | Rear Left Shoulder   |       12       |
| **RRS** | Rear Right Shoulder  |       13       |
| **RLK** | Rear Left Knee       |       14       |
| **RRK** | Rear Right Knee      |       15       |

---

## 📐 Motion Profile Reference Matrix

The following table tracks the exact targeted servo angles (0°–180°) across all programmed operating states:

| Joint Keyword | 1️⃣ Stand Up (`standUp`) | 2️⃣ Resting (`rest`) |         3️⃣ Hello Setup (`hello`)         |
| :-----------: | :--------------------------: | :----------------------: | :--------------------------------------------: |
| **FLK** |            100°            |           25°           |      100°*(Resets to 25° after-wave)*      |
| **FRK** |            110°            |          180°          |     110°*(Resets to 180° after-wave)*     |
| **FLS** |            140°            |          180°          |                     135°                     |
| **FRS** |             35°             |           0°           | 35°*(Waves up to 150° then back to 35°)* |
| **FLH** |             90°             |           30°           |                      95°                      |
| **FRH** |            100°            |          160°          |                      95°                      |
| **RLH** |            110°            |           40°           |                     110°                     |
| **RRH** |             95°             |          160°          |                      95°                      |
| **RLS** |            135°            |          180°          |                     160°                     |
| **RRS** |             65°             |           0°           |                      45°                      |
| **RLK** |            110°            |           35°           |                      30°                      |
| **RRK** |            100°            |          175°          |                     170°                     |

---

## 🎬 Stunt Sequence Profiles

### 1. Stand Up Sequence (`1`)

* **Hips Matrix:** Rear Left to 110°, Rear Right to 95°, Front Left to 90°, Front Right to 100°.
* **Shoulders Matrix:** Front Left to 140°, Front Right to 35°, Rear Left to 135°, Rear Right to 65°.
* **Knees Sync:** Rear knees unlock immediately to stand, followed by front knees 500ms later.

### 2. Resting Sequence (`2`)

* **Knees Fold:** Deep compression shift (RLK: 35°, RRK: 175°, FRK: 180°, FLK: 25°).
* **Shoulders Drop:** Rotates flat against chassis base boundaries.
* **Hips Flatten:** Completes total mechanical grounding layout.

### 3. Hello/Hi Emote Sequence (`3`)

1. **Initial Pose:** Triggers standard `standUp()` framework.
2. **Shift Weight:** Lowers rear chassis height and stabilizes forward footprint balance.
3. **The Wave:** Extends **FRS (Front Right Shoulder)** up to **150°** for a 2-second high-five gesture, resets, and returns cleanly back to standing posture.

---

## 💻 Serial Monitor Interface Commands

Set your baud rate to **`115200`** with **Newline (`\n`)** enabled.

### Operational Sequences

* `1` : Triggers the high-posture standing loop.
* `2` : Triggers the low-profile storage/resting loop.
* `3` : Executes the high-five gesture routine.

### Live Calibration Tuning Syntax

To tweak an individual joint angle dynamically, input the uppercase three-letter keyword followed by a space and target angle:

```text
<KEYWORD> <ANGLE>
```

* *Example:* `FRK 90` *(Commands Front Right Knee instantly to 90°)*
* *Example:* `FLS 140` *(Commands Front Left Shoulder instantly to 140°)*
