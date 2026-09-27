import serial
import numpy as np
import matplotlib.pyplot as plt

ser = serial.Serial("COM4", 115200, timeout=1)

plt.ion()

fig, ax = plt.subplots()
line, = ax.plot([])

ax.set_title("CSI Amplitude")
ax.set_xlabel("Subcarrier")
ax.set_ylabel("Amplitude")

while True:
    raw = ser.readline()

    if not raw:
        continue

    text = raw.decode("utf-8", errors="ignore").strip()

    if not text.startswith("CSI,"):
        continue

    values = text.split(",")[1:]

    try:
        csi = np.array([int(x) for x in values], dtype=np.int8)
    except ValueError:
        continue

    # I/Q pairs
    I = csi[0::2]
    Q = csi[1::2]

    complex_csi = I + 1j * Q

    amplitude = np.abs(complex_csi)

    line.set_data(np.arange(len(amplitude)), amplitude)

    ax.relim()
    ax.autoscale_view()

    plt.pause(0.001)