import matplotlib
import serial
import matplotlib.pyplot as plt
import re

# Replace with actual values as needed
COM_PORT = "/dev/ttyUSB0"
BAUD_RATE = 115200

# Create a serial object
try:
    print("Opening serial connection...")
    ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)
except serial.SerialException:
    print("Failed to open serial connection.")
    exit()

# Initialize an empty list to store the data and timestamps
ps_data = []
target_data = []
# timestamps = []

# Read and plot the data
plt.title("Pressure Readings over Time")
plt.xlabel("Timestamp (ms)")
plt.ylabel("Pressure (kPa)")
plt.ion()  # Turn on interactive mode
try:
    while True:
        line = ser.readline().decode("utf-8")  # Read a '\n' terminated line
        ps_data_pattern = r"I \((\d+)\) Main: Cuff Pressure: (-?[\d]+) mmHg"
        target_data_pattern = r"I \((\d+)\) Main: Target Pressure: (-?[\d]+) mmHg"
        match_ps = re.search(ps_data_pattern, line)
        match_target = re.search(target_data_pattern, line)

        if match_ps:
            # timestamp_ms = float(match.group(1))
            pressure_value = float(match_ps.group(2))
            ps_data.append(pressure_value)
            # timestamps.append(timestamp_ms)

            plt.clf()  # Clear the current figure
            plt.title("Pressure Readings over Time")
            plt.xlabel("Timestamp (ms)")
            plt.ylabel("Pressure (kPa)")
            # plt.plot(timestamps, data)  # TODO: Plot the data with timestamps
            plt.plot(ps_data)  # plot the data without timestamps
            plt.plot(target_data)  # plot the data without timestamps
            plt.pause(0.01)  # Pause a bit so that the plot gets updated

        if match_target:
            # timestamp_ms = float(match.group(1))
            target_pressure_value = float(match_target.group(2))
            target_data.append(target_pressure_value)
            # timestamps.append(timestamp_ms)

            plt.clf()  # Clear the current figure
            plt.title("Pressure Readings over Time")
            plt.xlabel("Timestamp (ms)")
            plt.ylabel("Pressure (kPa)")
            # plt.plot(timestamps, data)  # TODO: Plot the data with timestamps
            plt.plot(ps_data)  # plot the data without timestamps
            plt.plot(target_data)  # plot the data without timestamps
            plt.pause(0.01)  # Pause a bit so that the plot gets updated

except KeyboardInterrupt:
    print("Closing plot...")
    plt.close()
    print("Closing serial connection...")
    ser.close()
