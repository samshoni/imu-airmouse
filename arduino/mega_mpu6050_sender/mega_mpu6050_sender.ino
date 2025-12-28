#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;

const int BTN_PIN = 2;

// Filter state
float pitch = 0.0f, roll = 0.0f;
unsigned long lastMicros = 0;

// Tuning
float alpha = 0.98f;
float sensitivity = 23.0f;
float deadzone = 1.5f;
int clampStep = 8;

float pitch0 = 0.0f, roll0 = 0.0f;

// Debounce state
bool lastStableBtn = true;           // INPUT_PULLUP: true=not pressed
bool lastReadBtn   = true;
unsigned long lastBounceMs = 0;
const unsigned long debounceMs = 50;

unsigned long lastClickMs = 0;

void calibrateOffsets() {
  const int N = 200;
  float pSum = 0, rSum = 0;

  Serial.println("Calibrating... keep still (2s)");
  for (int i = 0; i < N; i++) {
    sensors_event_t a, g, t;
    mpu.getEvent(&a, &g, &t);

    float accPitch = atan2(a.acceleration.y, a.acceleration.z) * 180.0f / PI;
    float accRoll  = atan2(-a.acceleration.x, a.acceleration.z) * 180.0f / PI;

    pSum += accPitch;
    rSum += accRoll;
    delay(10);
  }

  pitch0 = pSum / N;
  roll0  = rSum / N;

  Serial.print("pitch0="); Serial.print(pitch0);
  Serial.print(" roll0="); Serial.println(roll0);
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  pinMode(BTN_PIN, INPUT_PULLUP);

  if (!mpu.begin(0x68)) {
    Serial.println("MPU6050 begin failed");
    while (1) delay(100);
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_10_HZ);

  delay(500);
  calibrateOffsets();

  lastMicros = micros();
  Serial.println("START");
}

void loop() {
  // --- Button debounce ---
  bool readBtn = digitalRead(BTN_PIN); // true = not pressed
  if (readBtn != lastReadBtn) {
    lastBounceMs = millis();
    lastReadBtn = readBtn;
  }
  if (millis() - lastBounceMs > debounceMs) {
    lastStableBtn = readBtn;
  }
  bool pressed = (lastStableBtn == LOW);

  // --- IMU ---
  sensors_event_t a, g, t;
  mpu.getEvent(&a, &g, &t);

  unsigned long now = micros();
  float dt = (now - lastMicros) / 1000000.0f;
  lastMicros = now;
  if (dt <= 0) return;

  float accPitch = atan2(a.acceleration.y, a.acceleration.z) * 180.0f / PI;
  float accRoll  = atan2(-a.acceleration.x, a.acceleration.z) * 180.0f / PI;

  float gyroPitchRate = g.gyro.x * 180.0f / PI;
  float gyroRollRate  = g.gyro.y * 180.0f / PI;

  pitch = alpha * (pitch + gyroPitchRate * dt) + (1 - alpha) * accPitch;
  roll  = alpha * (roll  + gyroRollRate  * dt) + (1 - alpha) * accRoll;

  float p = pitch - pitch0;
  float r = roll  - roll0;

  float xMove = r * sensitivity * dt;
  float yMove = -p * sensitivity * dt;

  if (fabs(r) < deadzone) xMove = 0;
  if (fabs(p) < deadzone) yMove = 0;

  int dx = (int)constrain(xMove, -clampStep, clampStep);
  int dy = (int)constrain(yMove, -clampStep, clampStep);

  // click event only on press (edge), debounced, rate-limited
  static bool prevPressed = false;
  int click = 0;
  if (pressed && !prevPressed) {
    if (millis() - lastClickMs > 250) {
      click = 1;
      lastClickMs = millis();
    }
  }
  prevPressed = pressed;

  Serial.print(dx);
  Serial.print(",");
  Serial.print(dy);
  Serial.print(",");
  Serial.println(click);

  delay(10);
}
