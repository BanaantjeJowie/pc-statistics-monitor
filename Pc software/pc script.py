import psutil
import time
import serial
import pynvml  # NVIDIA GPU monitoring library

# Initialize GPU monitoring
pynvml.nvmlInit()

# Configure the serial port
SERIAL_PORT = 'COM5'  # Replace with your actual COM port
BAUD_RATE = 115200

# Initialize serial communication
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
ser.dtr = False  # Disable DTR
ser.rts = False  # Disable RTS

time.sleep(2)  # Allow ESP32 to stabilize after disabling DTR/RTS
print("Serial port initialized without reset.")

def get_gpu_stats():
    """Fetch GPU stats using pynvml"""
    try:
        handle = pynvml.nvmlDeviceGetHandleByIndex(0)  # For the first GPU
        gpu_usage = pynvml.nvmlDeviceGetUtilizationRates(handle).gpu
        gpu_temp = pynvml.nvmlDeviceGetTemperature(handle, pynvml.NVML_TEMPERATURE_GPU)
        return gpu_usage, gpu_temp
    except pynvml.NVMLError as err:
        print("No GPU detected or error occurred:", err)
        return None, None

def get_system_stats():
    """Fetch CPU, RAM, and Disk statistics"""
    cpu_usage = psutil.cpu_percent(interval=1)
    cpu_temp = psutil.sensors_temperatures().get('coretemp', [])[0].current  # CPU temperature (Windows specific)
    memory = psutil.virtual_memory()

    # Fetch GPU stats if available, otherwise use CPU values for GPU
    gpu_usage, gpu_temp = get_gpu_stats()
    if gpu_usage is None:
        gpu_usage = cpu_usage
        gpu_temp = cpu_temp

    return {
        "cpu_usage": cpu_usage,
        "cpu_temp": cpu_temp,
        "ram_usage": memory.percent,
        "ram_temp": 40.0,  # Placeholder value for RAM temperature
        "gpu_usage": gpu_usage,
        "gpu_temp": gpu_temp
    }

try:
    while True:
        stats = get_system_stats()
        # Send statistics as a comma-separated string
        data = f"{stats['cpu_usage']},{stats['cpu_temp']},{stats['ram_usage']},{stats['ram_temp']},{stats['gpu_usage']},{stats['gpu_temp']}\n"
        ser.write(data.encode('utf-8'))
        print(f"Sent: {data.strip()}")
        time.sleep(1)
except KeyboardInterrupt:
    print("Exiting...")
finally:
    ser.close()
