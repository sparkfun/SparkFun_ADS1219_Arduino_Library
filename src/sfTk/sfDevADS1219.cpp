/**
 * @file sfDevADS1219.cpp
 * @brief Platform independent driver implementation for the TI ADS1219 24-Bit 4-Channel ADC.
 *
 * Want to support open source hardware? Buy a board from SparkFun!
 *
 * This library was written by:
 * Paul Clark
 * SparkFun Electronics
 * December 2023
 *
 * @author SparkFun Electronics
 * @date 2023-2026
 * @copyright Copyright (c) 2023-2026, SparkFun Electronics Inc. This project is released under the MIT License.
 *
 * SPDX-License-Identifier: MIT
 */

#include "sfDevADS1219.h"

//--------------------------------------------------------------------------------------------------
// Begin the ADS1219 device. Requires a bus object to communicate with the device.
//
sfTkError_t sfDevADS1219::begin(sfTkIBus *theBus)
{
    // Nullptr check
    if (theBus == nullptr)
        return ksfTkErrBusNotInit;

    // Set bus pointer
    _theBus = theBus;

    // Perform a soft reset so that we make sure the device is addressable.
    sfTkError_t rc = reset();
    if (rc != ksfTkErrOk)
        return rc;

    sftk_delay_ms(1); // Wait >100us (tRSSTA)

    // After a reset, the Configuration Register should read 0x00
    sfe_ads1219_reg_cfg_t config;
    rc = getConfigurationRegister(config);
    if (rc != ksfTkErrOk)
        return rc;

    if (config.byte != 0)
        return ksfTkErrFail;

    _adcGain = ADS1219_GAIN_1; // The reset restored the default gain

    return ksfTkErrOk;
}

//--------------------------------------------------------------------------------------------------
bool sfDevADS1219::isConnected(void)
{
    if (_theBus == nullptr)
        return false;

    sfe_ads1219_reg_status_t status;
    return _theBus->readRegister(kRegStatusRead, status.byte) == ksfTkErrOk;
}

//--------------------------------------------------------------------------------------------------
sfTkError_t sfDevADS1219::reset(void)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    return _theBus->writeData(kCommandReset);
}

//--------------------------------------------------------------------------------------------------
sfTkError_t sfDevADS1219::startSync(void)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    return _theBus->writeData(kCommandStartSync);
}

//--------------------------------------------------------------------------------------------------
sfTkError_t sfDevADS1219::powerDown(void)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    return _theBus->writeData(kCommandPowerDown);
}

//--------------------------------------------------------------------------------------------------
sfTkError_t sfDevADS1219::setConversionMode(const ads1219_conversion_mode_config_t mode)
{
    sfe_ads1219_reg_cfg_t config;
    sfTkError_t rc = getConfigurationRegister(config); // Read the config register
    if (rc != ksfTkErrOk)
        return rc;

    config.cm = (uint8_t)mode;              // Modify (only) the conversion mode
    return setConfigurationRegister(config); // Write the config register
}

//--------------------------------------------------------------------------------------------------
sfTkError_t sfDevADS1219::setInputMultiplexer(const ads1219_input_multiplexer_config_t mux)
{
    sfe_ads1219_reg_cfg_t config;
    sfTkError_t rc = getConfigurationRegister(config); // Read the config register
    if (rc != ksfTkErrOk)
        return rc;

    config.mux = (uint8_t)mux;               // Modify (only) the input multiplexer
    return setConfigurationRegister(config); // Write the config register
}

//--------------------------------------------------------------------------------------------------
sfTkError_t sfDevADS1219::setGain(const ads1219_gain_config_t gain)
{
    sfe_ads1219_reg_cfg_t config;
    sfTkError_t rc = getConfigurationRegister(config); // Read the config register
    if (rc != ksfTkErrOk)
        return rc;

    config.gain = (uint8_t)gain;             // Modify (only) the gain
    return setConfigurationRegister(config); // Write the config register - also updates the local gain
}

//--------------------------------------------------------------------------------------------------
sfTkError_t sfDevADS1219::setDataRate(const ads1219_data_rate_config_t rate)
{
    sfe_ads1219_reg_cfg_t config;
    sfTkError_t rc = getConfigurationRegister(config); // Read the config register
    if (rc != ksfTkErrOk)
        return rc;

    config.dr = (uint8_t)rate;               // Modify (only) the data rate
    return setConfigurationRegister(config); // Write the config register
}

//--------------------------------------------------------------------------------------------------
sfTkError_t sfDevADS1219::setVoltageReference(const ads1219_vref_config_t vRef)
{
    sfe_ads1219_reg_cfg_t config;
    sfTkError_t rc = getConfigurationRegister(config); // Read the config register
    if (rc != ksfTkErrOk)
        return rc;

    config.vref = (uint8_t)vRef;             // Modify (only) the voltage reference
    return setConfigurationRegister(config); // Write the config register
}

//--------------------------------------------------------------------------------------------------
sfTkError_t sfDevADS1219::readConversion(void)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    uint8_t rawBytes[3];
    size_t readBytes;
    sfTkError_t rc = _theBus->readRegister(kCommandReadData, rawBytes, sizeof(rawBytes), readBytes);
    if (rc != ksfTkErrOk)
        return rc;

    if (readBytes != sizeof(rawBytes)) // Check three bytes were returned
        return ksfTkErrFail;

    // Data is 3-bytes (24-bits), big-endian (MSB first).
    union {
        int32_t i32;
        uint32_t u32;
    } iu32; // Use a union to avoid signed / unsigned ambiguity
    iu32.u32 = rawBytes[0];
    iu32.u32 = (iu32.u32 << 8) | rawBytes[1];
    iu32.u32 = (iu32.u32 << 8) | rawBytes[2];

    // Preserve the 2's complement.
    if (0x00800000 == (iu32.u32 & 0x00800000))
        iu32.u32 = iu32.u32 | 0xFF000000;

    _adcResult = iu32.i32; // Store the signed result

    return ksfTkErrOk;
}

//--------------------------------------------------------------------------------------------------
float sfDevADS1219::getConversionMillivolts(float referenceVoltageMillivolts)
{
    float mV = _adcResult;            // Convert int32_t to float
    mV /= 8388608.0;                  // Convert to a fraction of full-scale (2^23)
    mV *= referenceVoltageMillivolts; // Convert to millivolts
    if (_adcGain == ADS1219_GAIN_4)
        mV /= 4.0; // Correct for the gain
    return mV;
}

//--------------------------------------------------------------------------------------------------
int32_t sfDevADS1219::getConversionRaw(void)
{
    return _adcResult;
}

//--------------------------------------------------------------------------------------------------
bool sfDevADS1219::dataReady(void)
{
    if (_theBus == nullptr)
        return false;

    sfe_ads1219_reg_status_t status;
    if (_theBus->readRegister(kRegStatusRead, status.byte) != ksfTkErrOk)
        return false;

    return status.drdy == 1;
}

//--------------------------------------------------------------------------------------------------
sfTkError_t sfDevADS1219::getConfigurationRegister(sfe_ads1219_reg_cfg_t &config)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    return _theBus->readRegister(kRegConfigRead, config.byte);
}

//--------------------------------------------------------------------------------------------------
sfTkError_t sfDevADS1219::setConfigurationRegister(sfe_ads1219_reg_cfg_t config)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    sfTkError_t rc = _theBus->writeRegister(kRegConfigWrite, config.byte);

    // Update the local copy of the gain for voltage conversion - only if the device was updated
    if (rc == ksfTkErrOk)
        _adcGain = (ads1219_gain_config_t)config.gain;

    return rc;
}
