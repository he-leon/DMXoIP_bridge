import sacn
import serial
import time
import threading
import re

# ---------------- Configuration ----------------
SERIAL_PORT   = '/dev/ttyACM0'  # ESP32 serial port on Linux
BAUDRATE      = 115200
UNIVERSE      = 1
NUM_CHANNELS  = 512         # E1.31 universe size
PACKET_INTERVAL_HZ = 40        # in Hz
# -----------------------------------------------

# Open serial port
ser = serial.Serial(SERIAL_PORT, BAUDRATE, timeout=0.1)

# Setup sACN sender
sender = sacn.sACNsender(fps=PACKET_INTERVAL_HZ)
sender.activate_output(UNIVERSE)
#sender[UNIVERSE].multicast = False
#sender[UNIVERSE].destination = "192.168.178.196"
sender[UNIVERSE].multicast = True
sender.start()

# ---------------- Serial reader ----------------
latencies = []
send_times = {}
pattern = re.compile(r"SEQ:(\d+)")

# FPS + latency tracking
received_count = 0
second_latencies = []
last_fps_time = time.time()
fps_lock = threading.Lock()

def serial_reader():
    global received_count, last_fps_time, second_latencies
    while True:
        line = ser.readline().decode(errors='ignore').strip()
        if not line:
            continue
        m = pattern.search(line)
        if m:
            seq = int(m.group(1))
            now = time.time()

            if seq in send_times:
                latency_s = now - send_times[seq]
                with fps_lock:
                    received_count += 1
                    second_latencies.append(latency_s)

        # Once per second, print stats
        now = time.time()
        if now - last_fps_time >= 1.0:
            with fps_lock:
                elapsed = now - last_fps_time
                fps = received_count / elapsed if elapsed > 0 else 0

                if second_latencies:
                    mean_latency = sum(second_latencies) / len(second_latencies)
                    min_latency = min(second_latencies)
                    max_latency = max(second_latencies)
                else:
                    mean_latency = min_latency = max_latency = 0

                print(
                    f"Received FPS: {fps:.1f}, "
                    f"Mean latency: {mean_latency*1000:.2f} ms, "
                    f"Min: {min_latency*1000:.2f} ms, "
                    f"Max: {max_latency*1000:.2f} ms"
                )

                # reset counters
                received_count = 0
                second_latencies.clear()
                last_fps_time = now


reader_thread = threading.Thread(target=serial_reader, daemon=True)
reader_thread.start()

# ---------------- Send loop ----------------
sequence = 0
print("Starting sACN sender... Ctrl+C to stop.")
try:
    while True:
        data = [0] * NUM_CHANNELS
        # Will result in repating fade-in of red channel of first 4 LEDs 
        # Fade Up takes 256 packets (e.g. 40Hz = ~6.4s)
        data[0] = sequence & 0xFF
        data[3] = sequence & 0xFF
        data[6] = sequence & 0xFF
        data[9] = sequence & 0xFF
        


        sender[UNIVERSE].dmx_data = tuple(data)
        send_times[sequence] = time.time()

        sequence = (sequence + 1) % 256
        time.sleep(1 / PACKET_INTERVAL_HZ)

except KeyboardInterrupt:
    print("\nStopping sender.")
    sender.stop()
    if latencies:
        avg = sum(latencies)/len(latencies)
        print(f"Average latency: {avg*1000:.2f} ms")
        print(f"Min latency: {min(latencies)*1000:.2f} ms")
        print(f"Max latency: {max(latencies)*1000:.2f} ms")

