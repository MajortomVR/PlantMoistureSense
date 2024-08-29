/*
  I measured 289 when put in a glass of water
  642 room air moisture
*/

int SENSOR_POWER_PIN = 2;
int SENSOR_ADC_PIN = A0;

void setup() {
  Serial.begin(9600);
  pinMode(SENSOR_POWER_PIN, OUTPUT);    
}

void loop() {
  // Deactivate Sensor (To reduce power consumption it is only activated when needed)
  digitalWrite(SENSOR_POWER_PIN, LOW);  
  delay(500);
  bool start_measurement = false;

  // Serial communication message handling
  while (Serial.available() > 0) {    
    char incomingByte = Serial.read();
    start_measurement = true;
  }

  // If we have a start_measurement signal -> Read the sensor value from the ADC and send it on the serial connection.
  if (start_measurement) {
    Serial.println("Measuring...");    
    // Activate Sensor
    digitalWrite(SENSOR_POWER_PIN, HIGH);
    delay(1000);

    int measurementCount = 5;
    int value = 0;

    for (int i=0; i<measurementCount; i++) {
      value += analogRead(SENSOR_ADC_PIN);
      delay(5);
    }

    value /= measurementCount;
    
    Serial.println(value);
  }
}
