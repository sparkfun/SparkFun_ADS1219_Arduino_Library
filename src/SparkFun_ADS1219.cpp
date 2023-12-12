/*
    SparkFun ADS1219 24-Bit 4-Channel ADC Arduino Library

    Qwiic 1x1
    https://www.sparkfun.com/products/23455

    Repository
    https://github.com/sparkfun/SparkFun_ADS1219_Arduino_Library

    SPDX-License-Identifier: MIT

    Copyright (c) 2023 SparkFun Electronics

    Name: SparkFun_ADS1219.cpp

    Description:
    An Arduino Library for the ADS1219 24-Bit 4-Channel ADC from TI.
    Requires the SparkFun Toolkit:
    https://github.com/sparkfun/SparkFun_Toolkit

*/

#include "SparkFun_ADS1219.h"

/// @brief Reset the ADS1219.
/// @return true if the Configuration Register reads as 0x00.
bool SfeADS1219Driver::begin()
{
    // Perform a soft reset so that we make sure the device is addressable.
    if (!reset())
        return false;

    sfe_ads1219_reg_cfg_t config;
    bool result = (_theBus->readRegisterByte(kSfeADS1219RegConfigRead, config.byte) == kSTkErrOk);
    return (result && (config.byte == 0));
}

/// @brief Performs a soft reset of the ADC.
/// @return True if successful, false otherwise.
bool SfeADS1219Driver::reset()
{
    return (_theBus->writeByte(kSfeADS1219CommandReset) == kSTkErrOk);
}

/// @brief Start or restart conversions.
/// @return True if successful, false otherwise.
bool SfeADS1219Driver::startSync()
{
    return (_theBus->writeByte(kSfeADS1219CommandStartSync) == kSTkErrOk);
}

/// @brief Enter power-down mode.
/// @return True if successful, false otherwise.
bool SfeADS1219Driver::powerDown()
{
    return (_theBus->writeByte(kSfeADS1219CommandPowerDown) == kSTkErrOk);
}

/// @brief Reads the ADC conversion data, converts it to a usable form, and
/// saves it to the internal result variable.
/// @return true if successful, false otherwise.
bool SfeADS1219Driver::readConversion()
{
    uint8_t rawBytes[3];
    bool result = (_theBus->readRegisterRegion(kSfeADS1219CommandReadData, (uint8_t *)rawBytes, 3) == 3);
    if (result)
    {
        // Data is 3-bytes (24-bits), big-endian (MSB first).
        _adcResult = rawBytes[0];
        _adcResult = (_adcResult << 8) | rawBytes[1];
        _adcResult = (_adcResult << 8) | rawBytes[2];
        // Preserve the 2's complement.
        if (_adcResult & (1 << 23))
            _adcResult |= 0xFF000000;
    }
    return result;
}

/// @brief  Return the conversion result which was read by readConversion.
/// Convert it to mV using referenceVoltageMillivolts and the _adcGain.
/// @param  referenceVoltageMillivolts Usually the internal 2.048V reference voltage.
/// But the user can override with (REFP - REFN) when using the external reference.
/// @return The voltage in millivolts
float SfeADS1219Driver::getConversionMillivolts(float referenceVoltageMillivolts)
{
    float mV = _adcResult;            // Convert int32_t to float
    mV /= 8388608.0;                  // Convert to a fraction of full-scale (2^23)
    mV *= referenceVoltageMillivolts; // Convert to millivolts
    if (_adcGain == ADS1219_GAIN_4)
        mV /= 4.0; // Correct for the gain
    return mV;
}

/// @brief  Return the raw conversion result which was read by readConversion.
/// @return The raw signed conversion result. 24-bit (2's complement).
/// NOT adjusted for gain.
int32_t SfeADS1219Driver::getConversionRaw(void)
{
    return _adcResult;
}

/// @brief Check the data ready flag.
/// @return true if data is ready.
bool SfeADS1219Driver::dataReady(void)
{
    sfe_ads1219_reg_status_t status;
    bool result = (_theBus->readRegisterByte(kSfeADS1219RegStatusRead, status.byte) == kSTkErrOk);
    return (result && (status.drdy == 1));
}

void SfeADS1219Driver::setCommunicationBus(sfeTkArdI2C *theBus)
{
    _theBus = theBus;
}
