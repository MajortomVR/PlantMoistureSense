"""
    Sensor Values:
        0% Water saturation >= 600
        100% Water saturation (Sensor inside a Glass of Water) = 290
"""

import serial
import time
import wx

# USER SPECIFIC
ARDUINO_PORT = '/dev/ttyUSB0'
MAX_SENSOR_VALUE = 500
MIN_SENSOR_VALUE = 350
#---------------

TIMEOUT = 30.0


def clamp(min_value, max_value, value):
    """
    Clamps a value within minValue and maxValue.
    
    Parameters:
        min_value (numeric): The minimum value
        max_value (numeric): The maximum value
        value (numeric): The input value that gets clamped.
        
    Returns:
        numeric: The clamped value    
    """
    return max(min_value, min(max_value, value))


def parse_value(value_str:str) -> int:
    """
    Tries to convert a string into an integer value.
    
    Parameters:
        value_str (string): The string that needs to be converted.
    
    Returns:
        An integer value on success or None if the conversion failed.
    """
    try:
        return int(value_str)        
    except ValueError:
        return None
    

class ArduinoConnection:
    """
    This class handles the Serial connection to the Arduino.
    """
    
    def __init__(self, port_address) -> None:
        try:
            self.arduino = serial.Serial(port=port_address, baudrate=9600, timeout=5)
        except serial.SerialException as e:
            print(f"ERROR: Failed to connect to the Arduino on {port_address}: {e}")
            raise
    
    def close(self):
        """Closes the serial connection to the Arduino."""
        self.arduino.close()
    
    def send_message(self, message:str):
        """Sends a message to the Arduino."""
        try:
            self.arduino.write(message.encode())
        except serial.SerialException as e:
            print(f"ERROR: Failed to send a message to the Arduino: {e}")
    
    def get_message(self) -> str:
        """Gets a message from the arduino if there is one available."""        
        if self.arduino.in_waiting > 0:
            try:                
                return self.arduino.readline().decode('utf-8').strip()
            except serial.SerialException as e:
                print(f"ERROR: Failed to receive a message from the Arduino: {e}")
        
        return None
    

def main():    
    arduino = ArduinoConnection(ARDUINO_PORT)
    timestamp = time.time()
    value = 0
    
    # Get the current humidity value from the Sensor
    try:
        while True:
            arduino.send_message("GetValue\n")
            time.sleep(1)
            message = arduino.get_message()
            
            if message:
                print(f'Received message: "{message}"')
                value = parse_value(message)
                
            # If the timeout is reached break out of the loop
            if time.time() - timestamp > TIMEOUT:
                print("ERROR: Timeout reached!")
                break
            
            # If we got a valid value break out of the loop
            if value and value > 0:
                break
            
            time.sleep(2)
            
    except KeyboardInterrupt:
        print("Aborted by user.")
    finally:
        arduino.close()
    
    
    # Calculate Soil Humidity in %
    # 0% >= 600
    # 100% = 290 is sensor in water
    message = ""
    plant_water_percentage = 0.0
    
    if value:
        range = MAX_SENSOR_VALUE - MIN_SENSOR_VALUE
        plant_water_percentage = 100 - ((value - MIN_SENSOR_VALUE) / range) * 100
        plant_water_percentage = clamp(0, 100, plant_water_percentage)    
        message = f"Soil moisture: {plant_water_percentage:.0f}%"        
    else:
        message = "ERROR: Failed to get a valid sensor value from the Arduino!"
        
    # Open a window and display the humidity:
    options = (wx.OK | wx.ICON_INFORMATION) if plant_water_percentage > 0.01 else wx.ICON_WARNING        
    ap = wx.App()
    wx.MessageBox(message, "Plant Sensor", options)
    
    

if __name__ == "__main__":
    main()