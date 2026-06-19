import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque
import numpy as np

PORT = "COM4"  
BAUD = 115200

ser = serial.Serial(PORT, BAUD, timeout=0.1)
rssi_data = deque([0.0] * 300, maxlen=300)
time_data = deque([0.0] * 300, maxlen=300)

fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 6))
fig.suptitle("ESP32 WiFi Radar - Human Sensing")

line1, = ax1.plot([], [], 'cyan', lw=1)
ax1.set_ylim(-100, -20)
ax1.set_ylabel("RSSI (dBm)")
ax1.set_title("Raw Signal")
ax1.grid(True, alpha=0.3)

line2, = ax2.plot([], [], 'orange', lw=1)
ax2.set_ylim(-5, 5)
ax2.set_ylabel("Delta RSSI")
ax2.set_title("Motion Indicator (تغيّر الإشارة = حركة)")
ax2.axhline(y=0, color='white', lw=0.5)
ax2.grid(True, alpha=0.3)

plt.style.use('dark_background')

def update(frame):
    line = ser.readline().decode(errors='ignore').strip()

    if line:
        try:
            rssi = float(line)
            print(rssi)

            rssi_data.append(rssi)

            data = list(rssi_data)

            line1.set_data(range(len(data)), data)
            ax1.set_xlim(0, len(data))

            delta = np.diff(data)
            line2.set_data(range(len(delta)), delta)
            ax2.set_xlim(0, len(delta))

        except:
            print("bad line:", line)

    return line1, line2

ani = animation.FuncAnimation(fig, update, interval=30, blit=True)
plt.tight_layout()
plt.show()