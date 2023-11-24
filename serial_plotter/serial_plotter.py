import serial
import matplotlib.pyplot as plt
import re

# Replace with actual values as needed
COM_PORT = '/dev/ttyUSB0'
BAUD_RATE = 115200

# Create a serial object
try:
    print('Opening serial connection...')
    ser = serial.Serial(COM_PORT, BAUD_RATE)
except serial.SerialException:
    print('Failed to open serial connection.')
    exit()

# Initialize an empty list to store the data and timestamps
data = []
timestamps = []

# Read and plot the data
plt.title('Pressure Readings over Time')
plt.xlabel('Timestamp (ms)')
plt.ylabel('Pressure (kPa)')
plt.ion()  # Turn on interactive mode
try:
    while True:
        line = ser.readline().decode('utf-8')  # Read a '\n' terminated line
        pattern = r'I \((\d+)\) Pressure_Sensor: Converted pressure: (-?[\d\.]+) kPa'
        match = re.search(pattern, line)

        if match:
            timestamp_ms = float(match.group(1))
            pressure_value = float(match.group(2))
            data.append(pressure_value)
            # timestamps.append(timestamp_ms)

            plt.clf()  # Clear the current figure
            plt.title('Pressure Readings over Time')
            plt.xlabel('Timestamp (ms)')
            plt.ylabel('Pressure (kPa)')
            # plt.plot(timestamps, data)  # TODO: Plot the data with timestamps
            plt.plot(data)  # plot the data without timestamps 
            plt.pause(0.01)  # Pause a bit so that the plot gets updated

except KeyboardInterrupt:
    print("Closing plot...")
    plt.close()
    print("Closing serial connection...")
    ser.close()
