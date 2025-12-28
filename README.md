# IMU AirMouse (Arduino Mega + MPU6050 → Ubuntu Cursor)

Control the Ubuntu cursor by tilting an MPU6050 connected to an Arduino Mega.
Mega streams `(dx,dy,click)` over USB serial, and a Python script moves the mouse.

✅ Tested on Ubuntu 22 (X11)

---

## Hardware
- Arduino Mega 2560
- MPU6050 (I2C @ 0x68)
- Breadboard + jumper wires
- Push button (optional)

### Wiring (Arduino Mega)
MPU6050 → Mega:
- VCC → 5V
- GND → GND
- SDA → 20 (SDA)
- SCL → 21 (SCL)

Button (optional):
- One side → D2
- Other side → GND (INPUT_PULLUP)

---

## Software

### Arduino libraries
Install in Arduino IDE:
- Adafruit MPU6050
- Adafruit Unified Sensor
- Adafruit BusIO

### Ubuntu (Python)
Works best on X11:


Quick Start (Ubuntu)
1) Create venv + install deps

python3 -m venv ~/imu_mouse_env
source ~/imu_mouse_env/bin/activate
python -m pip install --upgrade pip
python -m pip install -r pc/requirements.txt

2) Upload Arduino sketch

Upload:
arduino/mega_mpu6050_sender/mega_mpu6050_sender.ino

Close Arduino Serial Monitor before running the Python script.
3) Run receiver

source ~/imu_mouse_env/bin/activate
python pc/serial_mouse_rx.py

Controls:

    Space = pause/resume

    Ctrl+C = quit

Port is set to /dev/ttyUSB0 by default.
