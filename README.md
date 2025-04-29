![Screenshot 2025-04-29 235727](https://github.com/user-attachments/assets/1e97346f-0d04-4e72-9d0a-560743949b5d)
# Air-Monitoring-System
Air Quality Monitoring System using ESP8266

This project monitors air quality using an ESP8266, a DHT11 sensor for temperature and humidity, and a gas sensor (analog) for air quality. It uses a buzzer to signal poor air conditions and hosts a stylish web interface to show live sensor data.

Hardware Used:
ESP8266 (NodeMCU-12E)

DHT11 sensor (Temperature & Humidity)

MQ sensor (Air Quality via analog pin A0)

Buzzer (on digital pin D1)

Features:
WiFi Server:

Connects to a local WiFi network.

Runs a web server at http://192.168.219.204

Web Interface:

HTML + inlined CSS

Displays live readings of:

Temperature (°C)

Humidity (%)

Air Quality (sensor value)

Dynamic gauges and alert status (GOOD, WARNING, DANGER)

Button to enable/disable the alarm (buzzer).

Real-time Monitoring:

JavaScript fetches updated readings every 2 seconds from /data endpoint.

Buzzer Alarm:

Triggers if:

Temperature > 35°C

Humidity > 80%

Air Quality > 300

Can be disabled via toggle button.

Blinks (beeps) periodically when active.

Endpoints:
/ → Returns the full HTML UI page.

/data → Returns sensor data as JSON.

/toggle-alarm → Toggles the buzzer state (enabled/disabled).

Code:
In loop():

Handles incoming client requests.

Periodically checks sensor readings and triggers buzzer if limits are exceeded.

In checkAirQuality():

Reads DHT11 and analog values.

Verifies data validity.

Activates/deactivates buzzer based on threshold.

Prints values to serial monitor for debugging.

End
