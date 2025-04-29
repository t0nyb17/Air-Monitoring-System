#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

const char* ssid = "YourWiFiName";
const char* password = "YourWiFiPassword";

#define DHTPIN D6
#define DHTTYPE DHT11
#define SMOKEPIN A0
#define BUZZERPIN D1

const int SMOKE_THRESHOLD = 300;
const float TEMP_HIGH = 35.0;
const float HUMIDITY_HIGH = 80.0;

ESP8266WebServer server(80);
DHT dht(DHTPIN, DHTTYPE);

bool buzzerEnabled = true;
bool alarmActive = false;

// Inlined HTML + CSS from your uploaded files
const char index_html[] PROGMEM = R"=====(

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Air Quality Monitor</title>
    <link href="https://fonts.googleapis.com/css2?family=Poppins:wght@400;600&display=swap" rel="stylesheet">
    <style>
        /* Your style.css content is inlined here */
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { font-family: 'Poppins', 'Arial', sans-serif; background-color: #0e0e0e; color: #f5f5f5; min-height: 100vh; display: flex; justify-content: center; align-items: center; padding: 20px; overflow: hidden; }
        .container { max-width: 900px; width: 100%; background: linear-gradient(145deg, #2b2b2b, #141414); padding: 40px; border-radius: 25px; box-shadow: 0 15px 40px rgba(0, 255, 171, 0.3); text-align: center; animation: fadeIn 1s ease-in-out; }
        h1 { color: #00FFAB; font-size: 2.6rem; margin-bottom: 30px; letter-spacing: 1px; font-weight: 600; animation: slideIn 1s ease-in-out; }
        .readings { display: flex; flex-wrap: wrap; justify-content: space-around; gap: 20px; margin-bottom: 30px; animation: fadeIn 1s ease-in-out; }
        .sensor-box { flex: 1 1 270px; background: #2a2a2a; padding: 30px; border-radius: 20px; box-shadow: 0 8px 18px rgba(0, 255, 171, 0.2); transition: all 0.3s ease-in-out; }
        .sensor-box:hover { transform: translateY(-10px); box-shadow: 0 15px 30px rgba(0, 255, 171, 0.3); }
        .label { font-size: 1rem; color: #bbb; margin-bottom: 12px; text-transform: uppercase; letter-spacing: 1px; }
        .value { font-size: 2.5rem; font-weight: bold; margin: 12px 0; }
        .temp-value { color: #FF6B6B; }
        .humidity-value { color: #1E90FF; }
        .air-value { color: #00FFAB; }
        .gauge-container { position: relative; width: 90px; height: 90px; margin: 0 auto; }
        .gauge { width: 100%; height: 100%; transform: rotate(-90deg); }
        .gauge-background { fill: none; stroke: #ddd; stroke-width: 10; }
        .gauge-fill { fill: none; stroke-width: 10; stroke-linecap: round; stroke: #FF6B6B; stroke-dasharray: 440; stroke-dashoffset: 440; transition: stroke-dashoffset 1s ease; }
        .status { margin-top: 30px; padding: 20px; border-radius: 15px; font-weight: bold; font-size: 1.3rem; background: #222; transition: all 0.3s ease; }
        .status.good { color: #00FFAB; border-color: #00FFAB; }
        .status.warning { color: #FFC107; border-color: #FFC107; }
        .status.danger { color: #FF4C4C; border-color: #FF4C4C; }
        .controls { margin-top: 40px; position: relative; z-index: 10; }
        .btn { padding: 14px 28px; background: linear-gradient(135deg, #00FFAB, #1E90FF); color: #0e0e0e; font-weight: bold; font-size: 1.1rem; border: none; border-radius: 30px; cursor: pointer; transition: background 0.3s ease, transform 0.3s ease; }
        .btn:hover { background: linear-gradient(135deg, #1E90FF, #00FFAB); transform: scale(1.05); }
        .alarm-active { animation: pulse 1s infinite; }
        @keyframes pulse { 0% { box-shadow: 0 0 0 0 rgba(255, 76, 76, 0.7); } 70% { box-shadow: 0 0 0 15px rgba(255, 76, 76, 0); } 100% { box-shadow: 0 0 0 0 rgba(255, 76, 76, 0); } }
        @keyframes fadeIn { 0% { opacity: 0; } 100% { opacity: 1; } }
        @keyframes slideIn { 0% { transform: translateX(-50px); opacity: 0; } 100% { transform: translateX(0); opacity: 1; } }
    </style>
</head>
<body>
    <div class="container">
        <h1>Air Quality Monitoring System</h1>

        <div class="readings">
            <div class="sensor-box">
                <div class="label">Temperature</div>
                <div class="gauge-container">
                    <svg class="gauge" viewBox="0 0 100 100">
                        <circle cx="50" cy="50" r="45" class="gauge-background" />
                        <circle cx="50" cy="50" r="45" class="gauge-fill" id="tempGauge" />
                    </svg>
                </div>
                <div class="value temp-value" id="tempReading">--°C</div>
            </div>

            <div class="sensor-box">
                <div class="label">Humidity</div>
                <div class="gauge-container">
                    <svg class="gauge" viewBox="0 0 100 100">
                        <circle cx="50" cy="50" r="45" class="gauge-background" />
                        <circle cx="50" cy="50" r="45" class="gauge-fill" id="humidityGauge" />
                    </svg>
                </div>
                <div class="value humidity-value" id="humidityReading">--%</div>
            </div>

            <div class="sensor-box">
                <div class="label">Air Quality</div>
                <div class="value air-value" id="airReading">--</div>
            </div>
        </div>

        <div class="status" id="statusMessage">Monitoring air quality...</div>

        <div class="controls">
            <button class="btn" id="toggleAlarm" onclick="toggleAlarm()">Disable Alarm</button>
        </div>
    </div>

    <script>
        let buzzerEnabled = true;

        function toggleAlarm() {
            const btn = document.getElementById('toggleAlarm');
            fetch('/toggle-alarm')
                .then(response => response.text())
                .then(data => {
                    buzzerEnabled = data === 'enabled';
                    btn.textContent = buzzerEnabled ? 'Disable Alarm' : 'Enable Alarm';
                });
        }

        function updateAirQualityStatus(temp, humidity, airQuality) {
            const statusElement = document.getElementById('statusMessage');
            let message = '';
            let statusClass = 'good';

            if (airQuality > 700 || temp > 35 || humidity > 85) {
                message = 'DANGER: Air Quality Critical!';
                statusClass = 'danger';
            } else if (airQuality > 400 || temp > 30 || humidity > 75) {
                message = 'WARNING: Air Quality Needs Attention';
                statusClass = 'warning';
            } else {
                message = 'GOOD: Air Quality Normal';
                statusClass = 'good';
            }

            statusElement.textContent = message;
            statusElement.className = 'status ' + statusClass;

            const sensorBoxes = document.querySelectorAll('.sensor-box');
            if (statusClass === 'danger') {
                sensorBoxes.forEach(el => el.classList.add('alarm-active'));
            } else {
                sensorBoxes.forEach(el => el.classList.remove('alarm-active'));
            }
        }

        function updateData() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('tempReading').textContent = data.temperature + '°C';
                    document.getElementById('humidityReading').textContent = data.humidity + '%';
                    document.getElementById('airReading').textContent = data.airQuality;
                    updateAirQualityStatus(data.temperature, data.humidity, data.airQuality);
                    updateGauge('tempGauge', data.temperature, 0, 50);
                    updateGauge('humidityGauge', data.humidity, 0, 100);
                });
        }

        function updateGauge(gaugeId, value, min, max) {
            const gauge = document.getElementById(gaugeId);
            const percentage = (value - min) / (max - min);
            const offset = 440 - (440 * percentage);
            gauge.style.strokeDashoffset = offset;
        }

        setInterval(updateData, 2000);
        updateData();
    </script>
</body>
</html>

)=====";

void checkAirQuality() {
  int smokeValue = analogRead(SMOKEPIN);
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(500);
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
    if (isnan(humidity)) humidity = 0;
    if (isnan(temperature)) temperature = 0;
  }

  bool isAirQualityPoor = (smokeValue > SMOKE_THRESHOLD) || (temperature > TEMP_HIGH) || (humidity > HUMIDITY_HIGH);

  if (isAirQualityPoor && buzzerEnabled) {
    if (!alarmActive) {
      alarmActive = true;
      Serial.println("Alarm activated!");
    }
    static unsigned long lastBuzzerToggle = 0;
    if (millis() - lastBuzzerToggle > 500) {
      static bool buzzerState = false;
      buzzerState = !buzzerState;
      digitalWrite(BUZZERPIN, buzzerState ? HIGH : LOW);
      lastBuzzerToggle = millis();
    }
  } else {
    if (alarmActive) {
      alarmActive = false;
      Serial.println("Alarm deactivated");
    }
    digitalWrite(BUZZERPIN, LOW);
  }

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print("°C, Humidity: ");
  Serial.print(humidity);
  Serial.print("%, Air Quality: ");
  Serial.println(smokeValue);
}

void handleRoot() {
  server.send(200, "text/html", index_html);
}

void handleData() {
  int smokeValue = analogRead(SMOKEPIN);
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  delay(50);

  if (isnan(humidity) || isnan(temperature)) {
    delay(500);
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
    if (isnan(humidity)) humidity = 0;
    if (isnan(temperature)) temperature = 0;
  }

  if (temperature < -10) {
    temperature += 125;
  }

  String json = "{\"temperature\":";
  json += String(temperature, 1);
  json += ",\"humidity\":";
  json += String(humidity, 1);
  json += ",\"airQuality\":";
  json += String(smokeValue);
  json += ",\"alarmActive\":";
  json += alarmActive ? "true" : "false";
  json += "}";

  server.send(200, "application/json", json);
}

void handleToggleAlarm() {
  buzzerEnabled = !buzzerEnabled;
  if (!buzzerEnabled) {
    digitalWrite(BUZZERPIN, LOW);
  }
  server.send(200, "text/plain", buzzerEnabled ? "enabled" : "disabled");
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(BUZZERPIN, OUTPUT);
  digitalWrite(BUZZERPIN, LOW);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/toggle-alarm", handleToggleAlarm);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 1000) {
    checkAirQuality();
    lastCheck = millis();
  }
}