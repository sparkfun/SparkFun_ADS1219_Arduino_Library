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

    delay(1); // Wait >100us (tRSSTA)

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

/// @brief Configure the conversion mode.
/// @return True if successful, false otherwise.
bool SfeADS1219Driver::setConversionMode(const ads1219_conversion_mode_config_t mode)
{
    sfe_ads1219_reg_cfg_t config;
    if (_theBus->readRegisterByte(kSfeADS1219RegConfigRead, config.byte) != kSTkErrOk) // Read the config register
        return false;
    config.cm = (uint8_t)mode; // Modify (only) the conversion mode
    return (_theBus->writeRegisterByte(kSfeADS1219RegConfigWrite, config.byte) == kSTkErrOk); // Write the config register
}

/// @brief Configure the input multiplexer.
/// @return True if successful, false otherwise.
bool SfeADS1219Driver::setInputMultiplexer(const ads1219_input_multiplexer_config_t mux)
{
    sfe_ads1219_reg_cfg_t config;
    if (_theBus->readRegisterByte(kSfeADS1219RegConfigRead, config.byte) != kSTkErrOk) // Read the config register
        return false;
    config.mux = (uint8_t)mux; // Modify (only) the input multiplexer
    return (_theBus->writeRegisterByte(kSfeADS1219RegConfigWrite, config.byte) == kSTkErrOk); // Write the config register
}

/// @brief Configure the gain.
/// @return True if successful, false otherwise.
bool SfeADS1219Driver::setGain(const ads1219_gain_config_t gain)
{
    sfe_ads1219_reg_cfg_t config;
    if (_theBus->readRegisterByte(kSfeADS1219RegConfigRead, config.byte) != kSTkErrOk) // Read the config register
        return false;
    config.gain = (uint8_t)gain; // Modify (only) the gain
    _adcGain = gain; // Update the local copy of the gain for voltage conversion
    return (_theBus->writeRegisterByte(kSfeADS1219RegConfigWrite, config.byte) == kSTkErrOk); // Write the config register
}

/// @brief Configure the data rate (samples per second).
/// @return True if successful, false otherwise.
bool SfeADS1219Driver::setDataRate(const ads1219_data_rate_config_t rate)
{
    sfe_ads1219_reg_cfg_t config;
    if (_theBus->readRegisterByte(kSfeADS1219RegConfigRead, config.byte) != kSTkErrOk) // Read the config register
        return false;
    config.dr = (uint8_t)rate; // Modify (only) the data rate
    return (_theBus->writeRegisterByte(kSfeADS1219RegConfigWrite, config.byte) == kSTkErrOk); // Write the config register
}

/// @brief Configure the voltage reference.
/// @return True if successful, false otherwise.
bool SfeADS1219Driver::setVoltageReference(const ads1219_vref_config_t vRef)
{
    sfe_ads1219_reg_cfg_t config;
    if (_theBus->readRegisterByte(kSfeADS1219RegConfigRead, config.byte) != kSTkErrOk) // Read the config register
        return false;
    config.vref = (uint8_t)vRef; // Modify (only) the voltage reference
    return (_theBus->writeRegisterByte(kSfeADS1219RegConfigWrite, config.byte) == kSTkErrOk); // Write the config register
}

/// @brief Reads the ADC conversion data, converts it to a usable form, and
/// saves it to the internal result variable.
/// @return true if successful, false otherwise.
bool SfeADS1219Driver::readConversion()
{
    uint8_t rawBytes[3];
    size_t readBytes;
    bool result = (_theBus->readRegisterRegion(kSfeADS1219CommandReadData, (uint8_t *)rawBytes, 3, readBytes) == kSTkErrOk);
    result = result && (readBytes == 3); // Check three bytes were returned
    if (result)
    {
        // Data is 3-bytes (24-bits), big-endian (MSB first).
        union {
            int32_t i32;
            uint32_t u32;
        } iu32; // Use a union to avoid signed / unsigned ambiguity
        iu32.u32 = rawBytes[0];
        iu32.u32 = (iu32.u32 << 8) | rawBytes[1];
        iu32.u32 = (iu32.u32 << 8) | rawBytes[2];
        // Preserve the 2's complement.
        if (0x00100000 == (iu32.u32 & 0x00100000))
            iu32.u32 = iu32.u32 | 0xFF000000;
        _adcResult = iu32.i32; // Store the signed result
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

/// @brief  Read the ADS1219 Configuration Register into a sfe_ads1219_reg_cfg_t struct.
/// @param  config Pointer to the sfe_ads1219_reg_cfg_t struct to hold the register contents.
/// @return True if successful, false otherwise.
bool SfeADS1219Driver::getConfigurationRegister(sfe_ads1219_reg_cfg_t *config)
{
    return (_theBus->readRegisterByte(kSfeADS1219RegConfigRead, config->byte) == kSTkErrOk); // Read the config register
}

/// @brief  Write a sfe_ads1219_reg_cfg_t struct into the ADS1219 Configuration Register.
/// @param  config A sfe_ads1219_reg_cfg_t struct holding the register contents.
/// @return True if successful, false otherwise.
bool SfeADS1219Driver::setConfigurationRegister(sfe_ads1219_reg_cfg_t config)
{
    return (_theBus->writeRegisterByte(kSfeADS1219RegConfigWrite, config.byte) == kSTkErrOk); // Write the config register
}

/// @brief  PRIVATE: update the local pointer to the I2C bus.
/// @param  theBus Pointer to the bus object.
void SfeADS1219Driver::setCommunicationBus(sfeTkArdI2C *theBus)
{
    _theBus = theBus;
}
