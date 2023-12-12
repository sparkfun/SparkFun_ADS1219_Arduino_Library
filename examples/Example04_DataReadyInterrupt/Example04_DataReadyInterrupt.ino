/*
  Using the ADS1219 ADC Data Ready pin as an interrupt.

  The ADS1219 can sample at 1000 samples per second if desired.
  When sampling that fast, polling the DRDY flag in the Status Register
  slows down the sampling. It is much more efficient to use the DRDY
  pin as an interrupt and use that to trigger the read.

  Create an oscilloscope: use the Serial Plotter to plot the ADC readings!

  By: Paul Clark
  SparkFun Electronics
  Date: 2023/12/11
  SparkFun code, firmware, and software is released under the MIT License.
  Please see LICENSE.md for further details.

  Hardware Connections:
  IoT RedBoard --> ADS1219
  QWIIC        --> QWIIC
  Connect an interrupt-capable IO pin to the DRDY breakout pad, using a jumper wire.

  Open the serial monitor at 115200 baud to see the voltage.

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/23455 - Qwiic ADS1219 1x1
*/

// You will need the SparkFun Toolkit. Click here to get it: http://librarymanager/All#SparkFun_Toolkit

#include <SparkFun_ADS1219.h> // Click here to get the library: http://librarymanager/All#SparkFun_ADS1219

SfeADS1219ArdI2C myADC;

const int interruptPin = 4; // This is the FREE pin on the ESP32 Thing Plus C. Change this if required.

bool interruptSeen = false; // A global flag to indicate if the DRDY interrupt has been seen.

void dataReadyISR(void)
{
  // Always keep interrupt Interrupt Service Routines as short as possible.
  // Do not try to print or perform bus reads and writes inside them.
  // Here we only set the interruptSeen flag.
  // The ADC conversion result will be read in the loop().
  interruptSeen = true;
}

void setup()
{

  delay(1000); // Allow time for the microcontroller to start up

  Serial.begin(115200); // Begin the Serial console
  while (!Serial)
  {
    delay(100); // Wait for the user to open the Serial Monitor
  };
  Serial.println("SparkFun ADS1219 Example");

  Wire.begin(); // Begin the I2C bus

  // Initialize ADC - this also performs a soft reset
  while (myADC.begin() == false)
  {
    Serial.println("ADC failed to begin. Please check your wiring! Retrying...");
    delay(1000);
  }

  // Configure the input multiplexer
  myADC.setInputMultiplexer(ADS1219_CONFIG_MUX_SINGLE_0); // Read the voltage between AIN0 and GND

  // Configure the gain to x4, so we can measuure small voltages with higher resolution
  myADC.setGain(ADS1219_GAIN_4);

  // Configure the data rate. The ADC will sample at 1000 samples per second
  myADC.setDataRate(ADS1219_DATA_RATE_1000SPS);

  // Put the ADC into continuous mode
  myADC.setConversionMode(ADS1219_CONVERSION_CONTINUOUS);

  interruptSeen = false; // Make sure the interrupt flag is clear

  // Configure the interrupt. DRDY goes low when data is ready.
  attachInterrupt(digitalPinToInterrupt(interruptPin), dataReadyISR, FALLING);

  myADC.startSync(); // Start continuous conversions
}

void loop()
{
  if (interruptSeen) // Check if the conversion is complete.
  {
    interruptSeen = false; // Clear the flag ready for the next interrupt
    myADC.readConversion(); // Read the conversion result from the ADC. Store it internally.
    int32_t rawADC = myADC.getConversionRaw(); // Just to be different, read the raw ADC value.
    Serial.println(rawADC); // Print the raw ADC value
  }
}
