import serial
import psutil
import time
try:
    import pynvml  # Try to import NVIDIA GPU monitoring
    nvidia_available = True
except ImportError:
    nvidia_available = False
    print("NVIDIA GPU not found, or pynvml not installed.")

# Configure the serial port
SERIAL_PORT = 'COM5'  # Replace with your actual COM port
BAUD_RATE = 115200

# Initialize serial communication
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
ser.dtr = False  # Disable DTR
ser.rts = False  # Disable RTS

time.sleep(2)  # Allow ESP32 to stabilize after disabling DTR/RTS
print("Serial port initialized without reset.")

def get_system_stats():
    """Fetch CPU, RAM, and GPU statistics."""
    cpu_usage = psutil.cpu_percent(interval=1)
    memory = psutil.virtual_memory()

    # Check if NVIDIA GPU is available
    if nvidia_available:
        try:
            pynvml.nvmlInit()
            handle = pynvml.nvmlDeviceGetHandleByIndex(0)  # Assuming 0th GPU (first GPU)
            gpu_usage = pynvml.nvmlDeviceGetUsage(handle)
            pynvml.nvmlShutdown()
        except pynvml.NVMLError as e:
            print("Error querying NVIDIA GPU:", e)
            gpu_usage = 0  # Fallback if there's an error
    else:
        # If there's no NVIDIA GPU, fallback to a static value or another way to measure AMD GPU usage
        gpu_usage = 0  # You could attempt to use a different method for AMD if needed
    
    return {
        "cpu": cpu_usage,
        "ram": memory.percent,
        "gpu": gpu_usage
    }

try:
    while True:
        stats = get_system_stats()
        # Send statistics as a comma-separated string
        data = f"{stats['cpu']},{stats['ram']},{stats['gpu']}\n"
        ser.write(data.encode('utf-8'))
        print(f"Sent: {data.strip()}")
        time.sleep(1)
except KeyboardInterrupt:
    print("Exiting...")
finally:
    ser.close()
