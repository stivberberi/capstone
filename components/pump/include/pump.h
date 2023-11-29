#define RELAY_GPIO 22
#define SOLENOID_GPIO 25

void setup_pump_and_solenoid(void);

// pump control
void start_pump(void);
void stop_pump(void);

// solenoid control
void start_solenoid(void);
void stop_solenoid(void);
