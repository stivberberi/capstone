# Capstone

Repo to host all code associated with my final year undergraduate capstone
project at McMaster University. The project focuses on a self-regulating
tourniquet that uses sensors to determine the limb occlusion pressure of a
patient to auto-adjust the cuff pressure to match.

## Setting up ESP32 Project

First setup the project by setting the target and menu options (e.g.,
clock-speed, etc.):

- idf.py set-target esp32
- idf.py menuconfig -> optional if you want to use defualt configs

### Flashing

1. `idf.py build`
2. `idf.py flash {-p PORT}` -> idf.py will try to find the proper port on its
   own so -p is optional. Will also automatically build so previous command is
   unnecessary.
3. `idf.py monitor {-p PORT}` -> Used to monitor the serial output

Alternatively, can combine all of the above into one command:
`idf.py flash monitor`

### Serial Plotting

To aid with plotting pressure sensor data, the
`serial_plotter/serial_plotter.py` script can be used to plot data.

## Project Layout

- main/capstone.c -> Main app code, calls all other components
- components/ -> Directory containing all project components; each has its own
  CMakeLists.txt file
- components/pressure_sensor/ -> Pressure sensor related code; setup of sensor,
  ADC reading, etc.
