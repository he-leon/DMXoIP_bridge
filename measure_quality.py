import sacn
import serial
import time
import threading
import re
import argparse
import sys

# ---------------- Constants ----------------
BAUDRATE = 115200  # Fixed for the ESP32 debug port
UNIVERSE = 1
NUM_CHANNELS = 512
# -------------------------------------------


def log(msg):
    """Simple logger with timestamp in ms resolution."""
    # Check if sys.stdout is interactive (not redirected) before using fancy logging
    if sys.stdout.isatty():
        print(
            f"\r[{time.strftime('%H:%M:%S')}.{int((time.time()%1)*1000):03d}] {msg}",
            end="",
        )
    else:
        print(f"[{time.strftime('%H:%M:%S')}.{int((time.time()%1)*1000):03d}] {msg}")


def main():
    # --- 1. Argument Parsing ---
    parser = argparse.ArgumentParser(
        description="sACN to Serial DMX Latency Tester.",
        formatter_class=argparse.RawTextHelpFormatter,
    )
    parser.add_argument(
        "-p",
        "--port",
        type=str,
        default="/dev/ttyACM1",
        help="Serial port connected to the ESP32 debug output.\n(e.g., /dev/ttyACM0 on Linux, COM3 on Windows)",
    )
    parser.add_argument(
        "-f",
        "--freq",
        type=int,
        default=50,
        help="sACN packet sending frequency in Hz (FPS).\nMax recommended: 80 Hz.",
    )
    args = parser.parse_args()

    SERIAL_PORT = args.port
    PACKET_INTERVAL_HZ = args.freq

    log(
        f"Configuration: Port={SERIAL_PORT}, Frequency={PACKET_INTERVAL_HZ} Hz, Universe={UNIVERSE}"
    )

    # --- 2. Setup Serial Port ---
    try:
        # Open serial port (Note: This is the ESP32's DEBUG port for latency feedback)
        ser = serial.Serial(SERIAL_PORT, BAUDRATE, timeout=0.1)
    except serial.SerialException as e:
        log(f"Error opening serial port {SERIAL_PORT}: {e}")
        sys.exit(1)

    # --- 3. Setup sACN Sender ---
    sender = sacn.sACNsender(fps=PACKET_INTERVAL_HZ)
    sender.activate_output(UNIVERSE)
    sender[UNIVERSE].multicast = True
    sender[UNIVERSE].manual_flush = True
    sender.start()

    # ---------------- Serial reader state ----------------
    global latencies, send_times
    latencies = []
    send_times = {}
    pattern = re.compile(r"CH1:(\d+)")

    # FPS + latency tracking
    received_count = 0
    second_latencies = []
    last_fps_time = time.time()
    fps_lock = threading.Lock()

    def serial_reader():
        nonlocal received_count, last_fps_time, second_latencies
        while True:
            try:
                line = ser.readline().decode(errors="ignore").strip()
            except serial.SerialException:
                time.sleep(0.001)
                continue

            if not line:
                continue

            m = pattern.search(line)
            if m:
                seq = int(m.group(1))
                now = time.time()

                if seq in send_times:
                    latency_s = now - send_times[seq]
                    # Filter out packets that take longer than 1 second (likely old/delayed)
                    if latency_s > 1:
                        del send_times[seq]
                        continue

                    with fps_lock:
                        received_count += 1
                        second_latencies.append(latency_s)
                        latencies.append(latency_s)

            # Log stats every 0.5 seconds
            now = time.time()
            if now - last_fps_time >= 0.5:
                with fps_lock:
                    elapsed = now - last_fps_time
                    fps = received_count / elapsed if elapsed > 0 else 0

                    if second_latencies:
                        mean_latency = sum(second_latencies) / len(second_latencies)
                        min_latency = min(second_latencies)
                        max_latency = max(second_latencies)
                    else:
                        mean_latency = min_latency = max_latency = 0

                    # Print without log timestamp for clean stats output
                    stats_msg = (
                        f"Received FPS: {fps:.1f}, "
                        f"Mean latency: {mean_latency*1000:.2f} ms, "
                        f"Min: {min_latency*1000:.2f} ms, "
                        f"Max: {max_latency*1000:.2f} ms"
                    )
                    # Use carriage return only if interactive
                    if sys.stdout.isatty():
                        print("\r" + stats_msg.ljust(80), end="")
                    else:
                        log(stats_msg)

                    # reset counters
                    received_count = 0
                    second_latencies.clear()
                    last_fps_time = now

    # Start serial reader thread
    reader_thread = threading.Thread(target=serial_reader, daemon=True)
    reader_thread.start()

    # ---------------- Send loop ----------------
    sequence = 0
    last_update_time = time.time()
    log("Starting sACN sender... Ctrl+C to stop.")

    # Calculate target interval in seconds
    TARGET_INTERVAL_S = 1.0 / PACKET_INTERVAL_HZ

    try:
        while True:
            now = time.time()
            # Only send if enough time has elapsed
            if now - last_update_time < TARGET_INTERVAL_S:
                time.sleep(0.001)  # Small sleep to avoid hogging CPU
                continue

            data = [0] * NUM_CHANNELS

            # Embed sequence in DMX channel 1 (index 0) and other channels
            seq_val = sequence & 0xFF
            data[0] = seq_val
            data[3] = seq_val
            data[6] = seq_val
            data[9] = seq_val

            sender[UNIVERSE].dmx_data = tuple(data)

            last_update_time = now
            send_times[sequence] = now

            sequence = (sequence + 1) % 256
            sender.flush()

    except KeyboardInterrupt:
        log("\nStopping sender and cleaning up.")
        sender.stop()
        ser.close()

        # --- Final Stats ---
        if latencies:
            avg = sum(latencies) / len(latencies)
            print("\n" + "=" * 50)
            log(f"--- Full Run Stats ({len(latencies)} samples) ---")
            log(f"Average latency: {avg*1000:.2f} ms")
            log(f"Min latency: {min(latencies)*1000:.2f} ms")
            log(f"Max latency: {max(latencies)*1000:.2f} ms")
            print("=" * 50)


if __name__ == "__main__":
    main()
