/**
 * @file SparkFun_ADS1219.h
 * @brief SparkFun ADS1219 24-Bit 4-Channel ADC Arduino Library header file
 *
 * This file implements the SparkFunADS1219 class for use with the SparkFun
 * ADS1219 Qwiic breakout board, using the SparkFun Toolkit for I2C communication.
 *
 * Qwiic 1x1 : https://www.sparkfun.com/products/23455
 *
 * Repository : https://github.com/sparkfun/SparkFun_ADS1219_Arduino_Library
 *
 * Requires the SparkFun Toolkit : https://github.com/sparkfun/SparkFun_Toolkit
 *
 * @author SparkFun Electronics
 * @date 2023-2026
 * @copyright Copyright (c) 2023-2026, SparkFun Electronics Inc. This project is released under the MIT License.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

// helps to keep the Toolkit header before the tk calls
// clang-format off
#include <SparkFun_Toolkit.h>
#include "sfTk/sfDevADS1219.h"
// clang-format on

/**
 * @brief Class for interfacing with the ADS1219 ADC using Arduino I2C communication
 *
 * This class provides methods to initialize and communicate with the ADS1219
 * over an I2C bus. It inherits from the sfDevADS1219 class and uses the SparkFun
 * Toolkit for I2C communication.
 *
 * @see sfDevADS1219
 */
class SparkFunADS1219 : public sfDevADS1219
{
  public:
    /**
     * @brief Begins the Device with I2C as the communication bus
     *
     * This method initializes the I2C bus and sets up communication with the ADS1219,
     * performing a soft reset of the device.
     *
     * @param address I2C device address to use for the ADC
     * @param wirePort Wire port to use for I2C communication
     * @return True if successful, false otherwise
     */
    bool begin(const uint8_t address = kDefaultADS1219Addr, TwoWire &wirePort = Wire)
    {
        // Setup Arduino I2C bus
        if (_theI2CBus.init(wirePort, address) != ksfTkErrOk)
            return false;

        _theI2CBus.setStop(false); // Use restarts not stops for I2C reads

        // Begin the device
        return sfDevADS1219::begin(&_theI2CBus) == ksfTkErrOk;
    }

    /**
     * @brief Begins the Device with I2C as the communication bus
     *
     * @param wirePort Wire port to use for I2C communication
     * @param address I2C device address to use for the ADC
     * @return True if successful, false otherwise
     */
    bool begin(TwoWire &wirePort, const uint8_t address = kDefaultADS1219Addr)
    {
        return begin(address, wirePort);
    }

  private:
    sfTkArdI2C _theI2CBus;
};

// for backwards compatibility
/**
 * @brief Deprecated class for interfacing with the ADS1219 - supports version 1.x of this library
 *
 * In version 1.x of this library, the device methods returned true on success. In version 2.0
 * these methods return a sfTkError_t, which is ksfTkErrOk (0) on success. This class preserves
 * the version 1.x boolean return values so that existing code continues to work as before.
 *
 * @deprecated This class is deprecated for version 2.0 of this library. Use SparkFunADS1219 instead.
 */
class SfeADS1219ArdI2C : public SparkFunADS1219
{
  public:
    /** @brief Performs a soft reset of the ADC. @return True if successful, false otherwise. */
    bool reset(void)
    {
        return SparkFunADS1219::reset() == ksfTkErrOk;
    }

    /** @brief Start or restart conversions. @return True if successful, false otherwise. */
    bool startSync(void)
    {
        return SparkFunADS1219::startSync() == ksfTkErrOk;
    }

    /** @brief Enter power-down mode. @return True if successful, false otherwise. */
    bool powerDown(void)
    {
        return SparkFunADS1219::powerDown() == ksfTkErrOk;
    }

    /** @brief Reads the ADC conversion data. @return True if successful, false otherwise. */
    bool readConversion(void)
    {
        return SparkFunADS1219::readConversion() == ksfTkErrOk;
    }

    /** @brief Configure the conversion mode. @return True if successful, false otherwise. */
    bool setConversionMode(const ads1219_conversion_mode_config_t mode = ADS1219_CONVERSION_SINGLE_SHOT)
    {
        return SparkFunADS1219::setConversionMode(mode) == ksfTkErrOk;
    }

    /** @brief Configure the input multiplexer. @return True if successful, false otherwise. */
    bool setInputMultiplexer(const ads1219_input_multiplexer_config_t config = ADS1219_CONFIG_MUX_DIFF_P0_N1)
    {
        return SparkFunADS1219::setInputMultiplexer(config) == ksfTkErrOk;
    }

    /** @brief Configure the gain. @return True if successful, false otherwise. */
    bool setGain(const ads1219_gain_config_t gain = ADS1219_GAIN_1)
    {
        return SparkFunADS1219::setGain(gain) == ksfTkErrOk;
    }

    /** @brief Configure the data rate. @return True if successful, false otherwise. */
    bool setDataRate(const ads1219_data_rate_config_t rate = ADS1219_DATA_RATE_20SPS)
    {
        return SparkFunADS1219::setDataRate(rate) == ksfTkErrOk;
    }

    /** @brief Configure the voltage reference. @return True if successful, false otherwise. */
    bool setVoltageReference(const ads1219_vref_config_t vRef = ADS1219_VREF_INTERNAL)
    {
        return SparkFunADS1219::setVoltageReference(vRef) == ksfTkErrOk;
    }

    /** @brief Read the Configuration Register. @return True if successful, false otherwise. */
    bool getConfigurationRegister(sfe_ads1219_reg_cfg_t &config)
    {
        return SparkFunADS1219::getConfigurationRegister(config) == ksfTkErrOk;
    }

    /** @brief Write the Configuration Register. @return True if successful, false otherwise. */
    bool setConfigurationRegister(sfe_ads1219_reg_cfg_t config)
    {
        return SparkFunADS1219::setConfigurationRegister(config) == ksfTkErrOk;
    }
};
