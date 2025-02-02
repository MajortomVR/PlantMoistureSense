#include <EEPROM.h>

/*
  Message Structure:
  - CMD_GET;[VAR_NAME];
    FW_VERSION
    SENSOR_NAME
    SENSOR_VALUE

    Example:
        "CMD_GET;FW_VERSION;"

  - CMD_SET;[VAR_NAME];[VALUE];
    SENSOR_NAME  -  <24 character string>

    Example:
        "CMD_SET;SENSOR_NAME;Plant Sensor 1;"

  Return structure:
    [RETURN_VALUE]; (without brackets)
    Errors start with ERROR

    Examples:
      413;
      ERROR ...;
*/

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


/*
  EEPROM Struct
  int version
  char sensorName[24]

*/

enum eSerialCmd {
  CMD_UNKNOWN,
  CMD_GET,
  CMD_SET
};
struct Message {
  eSerialCmd cmd;
  String variable;
  String value;
};

/**
  Returns the command
*/
eSerialCmd parseCommand(String &receivedMessage) {
  if (receivedMessage.startsWith("CMD_GET;")) return CMD_GET;
  if (receivedMessage.startsWith("CMD_SET;")) return CMD_SET;
  return CMD_UNKNOWN;
}

/**
  Parse the Message
    Example Message.
*/
Message parseMessage(String &receivedMessage) {
  Message msg;
  msg.cmd = parseCommand(receivedMessage);
  int cmdEndIndex = receivedMessage.indexOf(';');  
  int cmdVariableEndIndex = receivedMessage.indexOf(';', cmdEndIndex + 1);
  int cmdValueEndIndex = receivedMessage.indexOf(';', cmdVariableEndIndex + 1);  
  
  if (cmdEndIndex >= 0 && cmdVariableEndIndex >= 0) {
    msg.variable = receivedMessage.substring(cmdEndIndex + 1, cmdVariableEndIndex);
    if (cmdValueEndIndex >= 0) msg.value = receivedMessage.substring(cmdVariableEndIndex + 1, cmdValueEndIndex);
  }
  
  return msg;
}

/**
  Handle the command that was received over the serial connection.
*/
void handleMessage(Message &message) {
  switch (message.cmd) {
    // GET
    case CMD_GET:
      if (message.variable == "FW_VERSION" || message.variable == "VERSION") {
        Serial.println("V1.1;");
      } else if (message.variable == "SENSOR_NAME") {
        Serial.println("Plant Sensor 13;");
      } else if (message.variable == "SENSOR_VALUE") {        
        lastSensorMeasurementValue = readSensor();
        Serial.println(String(lastSensorMeasurementValue) + ';');
      } else {
        Serial.println("ERROR: Unknown request Get: [" + message.variable + "];");
      }
      break;
    // SET
    case CMD_SET:
      if (message.variable == "SENSOR_NAME") {
        Serial.println("Setting Sensor Name to [" + message.value + "];");
      }
      break;
    // UNKNOWN
    case CMD_UNKNOWN:
      Serial.println("ERROR: Unknown Command!;");
      break;
  }
}

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
      String receivedData = Serial.readStringUntil('\n');

      if (receivedData.length() > 0) {
        Message msg = parseMessage(receivedData);
        handleMessage(msg);
        /*
        if (msg.cmd == CMD_GET) {
          Serial.println("RETURN_CMD_GET [" + msg.variable + "] [" + msg.value + "]");
        } else if (msg.cmd == CMD_SET) {
          Serial.println("RETURN_CMD_SET [" + msg.variable + "] [" + msg.value + "]");
        } else if (msg.cmd == CMD_UNKNOWN) {
          Serial.println("UNKNOWN CMD!");
        }
        */
      }      
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
