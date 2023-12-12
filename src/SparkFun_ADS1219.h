/*
    SparkFun ADS1219 24-Bit 4-Channel ADC Arduino Library

    Qwiic 1x1
    https://www.sparkfun.com/products/23455

    Repository
    https://github.com/sparkfun/SparkFun_ADS1219_Arduino_Library

    SPDX-License-Identifier: MIT

    Copyright (c) 2023 SparkFun Electronics

    Name: SparkFun_ADS1219.h

    Description:
    An Arduino Library for the ADS1219 24-Bit 4-Channel ADC from TI.
    Requires the SparkFun Toolkit:
    https://github.com/sparkfun/SparkFun_Toolkit

*/

#pragma once

#include <stdint.h>

#include <Arduino.h>
#include <SparkFun_Toolkit.h>

///////////////////////////////////////////////////////////////////////////////
// I2C Addressing
///////////////////////////////////////////////////////////////////////////////
// 7-bit address defined as [1, 0, 0, A1H, A1L, A0H, A0L] where A1/A0 are the
// physical address pins. These are connected to SCL, SDA, VDD or DGND to provide
// 16 address permutations (0x40 - 0x4F).
const uint8_t kDefaultADS1219Addr = 0x40; // A1 = DGND, A0 = DGND

///////////////////////////////////////////////////////////////////////////////
// Enum Definitions
///////////////////////////////////////////////////////////////////////////////

// Input multiplexer configuration : Configuration Register Bits 7:5
// These bits configure the input multiplexer.
// 000 : AINP = AIN0, AINN = AIN1 (default)
// 001 : AINP = AIN2, AINN = AIN3
// 010 : AINP = AIN1, AINN = AIN2
// 011 : AINP = AIN0, AINN = AGND
// 100 : AINP = AIN1, AINN = AGND
// 101 : AINP = AIN2, AINN = AGND
// 110 : AINP = AIN3, AINN = AGND
// 111 : AINP and AINN shorted to AVDD / 2
typedef enum
{
    ADS1219_CONFIG_MUX_DIFF_P0_N1 = 0,
    ADS1219_CONFIG_MUX_DIFF_P2_N3,
    ADS1219_CONFIG_MUX_DIFF_P1_N2,
    ADS1219_CONFIG_MUX_SINGLE_0,
    ADS1219_CONFIG_MUX_SINGLE_1,
    ADS1219_CONFIG_MUX_SINGLE_2,
    ADS1219_CONFIG_MUX_SINGLE_3,
    ADS1219_CONFIG_MUX_SHORTED
} ads1219_input_multiplexer_config_t;

// Gain configuration : Configuration Register Bit 4
typedef enum
{
    ADS1219_GAIN_1 = 0,
    ADS1219_GAIN_4
} ads1219_gain_config_t;

// Data rate configuration : Configuration Register Bits 3:2
typedef enum
{
    ADS1219_DATA_RATE_20SPS = 0,
    ADS1219_DATA_RATE_90SPS,
    ADS1219_DATA_RATE_330SPS,
    ADS1219_DATA_RATE_1000SPS
} ads1219_data_rate_config_t;

// Conversion mode configuration : Configuration Register Bit 1
typedef enum
{
    ADS1219_CONVERSION_SINGLE_SHOT = 0,
    ADS1219_CONVERSION_CONTINUOUS
} ads1219_conversion_mode_config_t;

// Voltage reference configuration : Configuration Register Bit 0
// Internal: internal 2.048V reference
// External: external REFP and REFN
typedef enum
{
    ADS1219_VREF_INTERNAL = 0,
    ADS1219_VREF_EXTERNAL
} ads1219_vref_config_t;

///////////////////////////////////////////////////////////////////////////////
// Configuration Register Description
///////////////////////////////////////////////////////////////////////////////

const uint8_t kSfeADS1219RegConfigWrite = 0x40; // Register address for write
const uint8_t kSfeADS1219RegConfigRead = 0x20;  // Register address for read

// A union is used here so that individual values from the register can be
// accessed or the whole register can be accessed.
typedef union
{
    struct
    {
        uint8_t vref : 1; // Voltage reference configuration : Configuration Register Bit 0
        uint8_t cm : 1;   // Conversion mode configuration : Configuration Register Bit 1
        uint8_t dr : 2;   // Data rate configuration : Configuration Register Bits 3:2
        uint8_t gain : 1; // Gain configuration : Configuration Register Bit 4
        uint8_t mux : 3;  // Input multiplexer configuration : Configuration Register Bits 7:5
    };
    uint8_t byte;
} sfe_ads1219_reg_cfg_t;

///////////////////////////////////////////////////////////////////////////////
// Status Register Description
///////////////////////////////////////////////////////////////////////////////

const uint8_t kSfeADS1219RegStatusRead = 0x24; // Register address for read

// A union is used here so that individual values from the register can be
// accessed or the whole register can be accessed.
typedef union
{
    struct
    {
        uint8_t id : 7;   // Device ID = 0x60 - but datasheet says "Reserved. Values are subject to change without notice"
        uint8_t drdy : 1; // Conversion result ready flag : Status Register Bit 7
    };
    uint8_t byte;
} sfe_ads1219_reg_status_t;

///////////////////////////////////////////////////////////////////////////////
// Command Definitions
///////////////////////////////////////////////////////////////////////////////

const uint8_t kSfeADS1219CommandReset = 0x06;     // Reset the device
const uint8_t kSfeADS1219CommandStartSync = 0x08; // Start or restart conversions
const uint8_t kSfeADS1219CommandPowerDown = 0x02; // Enter power-down mode
const uint8_t kSfeADS1219CommandReadData = 0x10;  // Read conversion data by command

///////////////////////////////////////////////////////////////////////////////

class SfeADS1219Driver
{
public:
    // @brief Constructor. Instantiate the driver object using the specified address (if desired).
    SfeADS1219Driver()
        : _adcGain{ADS1219_GAIN_1}, _adcResult{0}
    {
    }

    /// @brief Reset the ADS1219.
    /// @return true if the Configuration Register reads as 0x00.
    bool begin(void);

    /// @brief Performs a soft reset of the ADC.
    /// @return True if successful, false otherwise.
    bool reset(void);

    /// @brief Start or restart conversions.
    /// @return True if successful, false otherwise.
    bool startSync(void);

    /// @brief Enter power-down mode.
    /// @return True if successful, false otherwise.
    bool powerDown(void);

    /// @brief Reads the ADC conversion data, converts it to a usable form, and
    /// saves it to the internal result variable.
    /// @return true if successful, false otherwise.
    bool readConversion(void);

    /// @brief Configure the input multiplexer.
    /// @return True if successful, false otherwise.
    bool setInputMultiplexer(const ads1219_input_multiplexer_config_t config = ADS1219_CONFIG_MUX_DIFF_P0_N1);

    /// @brief  Return the conversion result which was read by readConversion.
    /// Convert it to mV using referenceVoltageMillivolts and the _adcGain.
    /// @param  referenceVoltageMillivolts Usually the internal 2.048V reference voltage.
    /// But the user can override with (REFP - REFN) when using the external reference.
    /// @return The voltage in millivolts
    float getConversionMillivolts(float referenceVoltageMillivolts = 2048);

    /// @brief  Return the raw conversion result which was read by readConversion.
    /// @return The raw signed conversion result. 24-bit (2's complement).
    /// NOT adjusted for gain.
    int32_t getConversionRaw(void);

    /// @brief Check the data ready flag.
    /// @return true if data is ready.
    bool dataReady(void);

protected:
    /// @brief Sets the communication bus to the specified bus.
    /// @param theBus Bus to set as the communication devie.
    void setCommunicationBus(sfeTkArdI2C *theBus);

private:
    sfeTkArdI2C *_theBus; // Pointer to bus device.

    ads1219_gain_config_t _adcGain; // Local configuration value. ADC gain - needed for conversion to mV.

    int32_t _adcResult; // Local store for the ADC conversion result. 24-Bit, shifted left for correct 2's complement
};

class SfeADS1219ArdI2C : public SfeADS1219Driver
{
public:
    SfeADS1219ArdI2C()
    {
    }

    /// @brief  Sets up Arduino I2C driver using the default I2C address then calls the super class begin.
    /// @return True if successful, false otherwise.
    bool begin(void)
    {
        if (_theI2CBus.init(kDefaultADS1219Addr) != kSTkErrOk)
            return false;

        setCommunicationBus(&_theI2CBus);

        _theI2CBus.setStop(false); // Use restarts not stops for I2C reads

        return SfeADS1219Driver::begin();
    }

    /// @brief  Sets up Arduino I2C driver using the specified I2C address then calls the super class begin.
    /// @return True if successful, false otherwise.
    bool begin(const uint8_t &address)
    {
        if (_theI2CBus.init(address) != kSTkErrOk)
            return false;

        setCommunicationBus(&_theI2CBus);

        _theI2CBus.setStop(false); // Use restarts not stops for I2C reads

        return SfeADS1219Driver::begin();
    }

    /// @brief  Sets up Arduino I2C driver using the specified I2C address then calls the super class begin.
    /// @return True if successful, false otherwise.
    bool begin(TwoWire &wirePort, const uint8_t &address)
    {
        if (_theI2CBus.init(wirePort, address) != kSTkErrOk)
            return false;

        setCommunicationBus(&_theI2CBus);

        _theI2CBus.setStop(false); // Use restarts not stops for I2C reads

        return SfeADS1219Driver::begin();
    }

private:
    sfeTkArdI2C _theI2CBus;
};
