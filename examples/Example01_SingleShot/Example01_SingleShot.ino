/*
  Using the ADS1219 ADC in its default single-shot mode.

  This example shows how to start a single-shot conversion,
  wait for it to complete and then read the voltage.

  By: Paul Clark
  SparkFun Electronics
  Date: 2023/12/11
  SparkFun code, firmware, and software is released under the MIT License.
  Please see LICENSE.md for further details.

  Hardware Connections:
  IoT RedBoard --> ADS1219
  QWIIC        --> QWIIC

  Open the serial monitor at 115200 baud to see the voltage.

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/23455 - Qwiic ADS1219 1x1
*/

// You will need the SparkFun Toolkit. Click here to get it: http://librarymanager/All#SparkFun_Toolkit

#include <SparkFun_ADS1219.h> // Click here to get the library: http://librarymanager/All#SparkFun_ADS1219

SfeADS1219ArdI2C myADC;

void setup()
{
  delay(1000); // Allow time for the microcontroller to start up

  Serial.begin(115200); // Begin the Serial console
  while (!Serial)
  {
    delay(100); // Wait for the user to open the Serial Monitor
  }
  Serial.println("SparkFun ADS1219 Example");

  Wire.begin(); // Begin the I2C bus

  // Initialize ADC - this also performs a soft reset
  while (myADC.begin() == false)
  {
    Serial.println("ADC failed to begin. Please check your wiring! Retrying...");
    delay(1000);
  }

  Serial.println("ADC initialized");

  Serial.println("Reading the differential voltage between AIN0 (+) and AIN1 (-)");
}

void loop()
{
  if (myADC.startSync()) // Start a single-shot conversion. This will return true on success.
  {
    while (myADC.dataReady() == false) // Check if the conversion is complete. This will return true if data is ready.
    {
      delay(10); // The conversion is not complete. Wait a little to avoid pounding the I2C bus.
    }

    myADC.readConversion(); // Read the conversion result from the ADC. Store it internally.
    float milliVolts = myADC.getConversionMillivolts(); // Convert to millivolts.
    Serial.print("ADC voltage (mV): ");
    Serial.println(milliVolts, 3); // Print milliVolts with 3 decimal places
  }
  else
  {
    Serial.println("ADC start conversion failed. Please check your wiring! Retrying...");
    delay(1000);
  }
}
