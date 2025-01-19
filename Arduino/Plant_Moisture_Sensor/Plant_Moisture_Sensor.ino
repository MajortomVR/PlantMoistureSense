/*
  I measured 289 when put in a glass of water
  642 room air moisture
*/
const unsigned long PERIODIC_MEASUREMENT_INTERVAL_MILLISECONDS = 5UL * 60UL * 1000UL; // every 5 minutes

const int LED_PIN = 3;
const int SENSOR_POWER_PIN = 2;           // VCC for Sensor
const int SENSOR_ADC_PIN = A0;
bool serialConnectionOpen = false;

int lastSensorMeasurementValue = 0;
unsigned long lastPeriodicMeasurementTimestamp = 0;
unsigned long lastLedFlashTimestamp = 0;

/**
  Initial Setup.
*/
void setup() {
  Serial.begin(9600);
  pinMode(SENSOR_POWER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  // Deactivate Sensor
  digitalWrite(SENSOR_POWER_PIN, LOW);
}

/**
  Called when a client has successfully connected.
*/
void handleConnectionOpened() {
  serialConnectionOpen = true;
  Serial.println("ID=PLANT_SENSOR_1.0;");
}

/**
  Called when a client has closed the connection.
*/
void handleConnectionClosed() {
  serialConnectionOpen = false;
}

/**
  Sensor reading.
*/
int readSensor() {
  digitalWrite(LED_PIN, HIGH);
  // Activate Sensor
  digitalWrite(SENSOR_POWER_PIN, HIGH);
  delay(500);

  const int measurementCount = 5;
  int total = 0;

  for (int i=0; i<measurementCount; i++) {
    total += analogRead(SENSOR_ADC_PIN);
    delay(5);
  }
  
  digitalWrite(LED_PIN, LOW);
  return total / measurementCount;  
}

/**
  MAIN LOOP
*/
void loop() {  
  // Deactivate Sensor (To reduce power consumption it is only activated when needed)
  digitalWrite(SENSOR_POWER_PIN, LOW);
  delay(100);

  // When a client connects, send an identifier back. This is used to find all the connected plant sensors by the client program.
  if (Serial && !serialConnectionOpen) {
    handleConnectionOpened();
  }
  if (!Serial && serialConnectionOpen) {
    handleConnectionClosed();
  }

  // Connection Management
  if (Serial && serialConnectionOpen) {    
    bool start_measurement = false;

    // Serial communication message handling
    while (Serial.available() > 0) {    
      char incomingByte = Serial.read();
      start_measurement = true;
    }
    
    if (start_measurement) {
      Serial.println("Measuring...");
      lastSensorMeasurementValue = readSensor();
      Serial.println(lastSensorMeasurementValue);
    }
  }

  // Periodic measurements
  if (millis() - lastPeriodicMeasurementTimestamp >= PERIODIC_MEASUREMENT_INTERVAL_MILLISECONDS) {
    lastPeriodicMeasurementTimestamp = millis();
    lastSensorMeasurementValue = readSensor();
  }

  // TODO: Get max value from EEPROM
  if (lastSensorMeasurementValue > 500 && millis() - lastLedFlashTimestamp >= 3000) {
    lastLedFlashTimestamp = millis();
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
  }  
}
