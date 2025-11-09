import sacn
import serial
import time
import threading
import re

# ---------------- Configuration ----------------
SERIAL_PORT   = '/dev/ttyACM1'  # ESP32 serial port on Linux
BAUDRATE      = 115200
UNIVERSE      = 1
NUM_CHANNELS  = 512         # E1.31 universe size
PACKET_INTERVAL_HZ = 100        # in Hz
# -----------------------------------------------

def log(msg):
    """Simple logger with timestamp in ms resolution."""
    print(f"[{time.strftime('%H:%M:%S')}.{int((time.time()%1)*1000):03d}] {msg}")

# Open serial port
ser = serial.Serial(SERIAL_PORT, BAUDRATE, timeout=0.1)

# Setup sACN sender
sender = sacn.sACNsender(fps=PACKET_INTERVAL_HZ)
sender.activate_output(UNIVERSE)
#sender[UNIVERSE].multicast = False
#sender[UNIVERSE].destination = "192.168.178.196"
sender[UNIVERSE].multicast = True
sender[UNIVERSE].manual_flush = True
sender.start()

# ---------------- Serial reader ----------------
latencies = []
send_times = {}
pattern = re.compile(r"CH1:(\d+)")

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
        #log(f"Serial line: {line}")
        m = pattern.search(line)
        if m:
            seq = int(m.group(1))
            now = time.time()
            #log(f"  -> parsed seq: {seq} at {now:.3f}")
            if seq in send_times:
                latency_s = now - send_times[seq]
                #log(f"  -> matched seq: {seq}, latency: {latency_s*1000:.2f} ms")
                with fps_lock:
                    received_count += 1
                    second_latencies.append(latency_s)

        # Once per second, log stats
        now = time.time()
        if now - last_fps_time >= 0.1:
            with fps_lock:
                elapsed = now - last_fps_time
                fps = received_count / elapsed if elapsed > 0 else 0

                if second_latencies:
                    mean_latency = sum(second_latencies) / len(second_latencies)
                    min_latency = min(second_latencies)
                    max_latency = max(second_latencies)
                else:
                    mean_latency = min_latency = max_latency = 0

                log(
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
last_update_time = 0
log("Starting sACN sender... Ctrl+C to stop.")
try:
    while True:
        if time.time() - last_update_time < (1 / PACKET_INTERVAL_HZ):
            time.sleep(0.001)
            continue
        data = [0] * NUM_CHANNELS
        # Will result in repating fade-in of red channel of first 4 LEDs 
        # Fade Up takes 256 packets (e.g. 40Hz = ~6.4s)
        data[0] = sequence & 0xFF
        data[3] = sequence & 0xFF
        data[6] = sequence & 0xFF
        data[9] = sequence & 0xFF
        


        sender[UNIVERSE].dmx_data = tuple(data)
        last_update_time = time.time()
        send_times[sequence] = last_update_time 

        #log(f"Sent seq: {sequence} at {send_times[sequence]:.3f}")

        sequence = (sequence + 1) % 256
        sender.flush()


except KeyboardInterrupt:
    log("\nStopping sender.")
    sender.stop()
    if latencies:
        avg = sum(latencies)/len(latencies)
        log(f"Average latency: {avg*1000:.2f} ms")
        log(f"Min latency: {min(latencies)*1000:.2f} ms")
        log(f"Max latency: {max(latencies)*1000:.2f} ms")

