import time
import serial
from pynput.mouse import Controller, Button
from pynput import keyboard

PORT = "/dev/ttyUSB0"   # <-- CHANGE THIS if needed
BAUD = 115200

mouse = Controller()
enabled = True
last_click_time = 0.0

def on_press(key):
    global enabled
    if key == keyboard.Key.space:
        enabled = not enabled
        print("ENABLED" if enabled else "PAUSED")

keyboard.Listener(on_press=on_press, daemon=True).start()

print("Opening serial:", PORT)
ser = serial.Serial(PORT, BAUD, timeout=0.2)
time.sleep(2)

print("Space = pause/resume. Ctrl+C = quit.")
print("Waiting for START...")

# Flush until START appears
while True:
    line = ser.readline().decode(errors="ignore").strip()
    if line == "START":
        break

print("START received. Moving mouse now.")

while True:
    line = ser.readline().decode(errors="ignore").strip()
    if not line:
        continue

    try:
        dx_s, dy_s, click_s = line.split(",")
        dx = int(dx_s)
        dy = int(dy_s)
        click = int(click_s)
    except Exception:
        continue

    if not enabled:
        continue

    if dx != 0 or dy != 0:
        mouse.move(dx, dy)

    if click == 1:
        now = time.time()
        if now - last_click_time > 0.2:
            mouse.click(Button.left, 1)
            last_click_time = now
