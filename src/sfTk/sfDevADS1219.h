/**
 * @file sfDevADS1219.h
 * @brief Platform independent driver for the TI ADS1219 24-Bit 4-Channel ADC.
 *
 * This file contains the declaration of the sfDevADS1219 class, which implements the
 * device logic for the ADS1219 using the SparkFun Toolkit bus interface. It has no
 * dependency on a specific platform (e.g. Arduino).
 *
 * Qwiic 1x1 : https://www.sparkfun.com/products/23455
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

#pragma once

#include <stdint.h>

// include the sparkfun toolkit headers
#include <sfTk/sfToolkit.h>

// Bus interfaces
#include <sfTk/sfTkIBus.h>

///////////////////////////////////////////////////////////////////////////////
// I2C Addressing
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Default I2C address of the ADS1219 (A1 = DGND, A0 = DGND).
 *
 * The 7-bit address is defined as [1, 0, 0, A1H, A1L, A0H, A0L] where A1/A0 are the
 * physical address pins. These are connected to SCL, SDA, VDD or DGND to provide
 * 16 address permutations (0x40 - 0x4F).
 */
const uint8_t kDefaultADS1219Addr = 0x40;

///////////////////////////////////////////////////////////////////////////////
// Enum Definitions
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Input multiplexer configuration : Configuration Register Bits 7:5
 *
 * | Value | AINP        | AINN        |
 * |-------|-------------|-------------|
 * | 000   | AIN0        | AIN1        |
 * | 001   | AIN2        | AIN3        |
 * | 010   | AIN1        | AIN2        |
 * | 011   | AIN0        | AGND        |
 * | 100   | AIN1        | AGND        |
 * | 101   | AIN2        | AGND        |
 * | 110   | AIN3        | AGND        |
 * | 111   | AVDD / 2    | AVDD / 2    |
 */
typedef enum
{
    ADS1219_CONFIG_MUX_DIFF_P0_N1 = 0, ///< AINP = AIN0, AINN = AIN1 (default)
    ADS1219_CONFIG_MUX_DIFF_P2_N3,     ///< AINP = AIN2, AINN = AIN3
    ADS1219_CONFIG_MUX_DIFF_P1_N2,     ///< AINP = AIN1, AINN = AIN2
    ADS1219_CONFIG_MUX_SINGLE_0,       ///< AINP = AIN0, AINN = AGND
    ADS1219_CONFIG_MUX_SINGLE_1,       ///< AINP = AIN1, AINN = AGND
    ADS1219_CONFIG_MUX_SINGLE_2,       ///< AINP = AIN2, AINN = AGND
    ADS1219_CONFIG_MUX_SINGLE_3,       ///< AINP = AIN3, AINN = AGND
    ADS1219_CONFIG_MUX_SHORTED         ///< AINP and AINN shorted to AVDD / 2
} ads1219_input_multiplexer_config_t;

/** @brief Gain configuration : Configuration Register Bit 4 */
typedef enum
{
    ADS1219_GAIN_1 = 0, ///< Gain of 1 (default)
    ADS1219_GAIN_4      ///< Gain of 4
} ads1219_gain_config_t;

/** @brief Data rate configuration : Configuration Register Bits 3:2 */
typedef enum
{
    ADS1219_DATA_RATE_20SPS = 0, ///< 20 samples per second (default)
    ADS1219_DATA_RATE_90SPS,     ///< 90 samples per second
    ADS1219_DATA_RATE_330SPS,    ///< 330 samples per second
    ADS1219_DATA_RATE_1000SPS    ///< 1000 samples per second
} ads1219_data_rate_config_t;

/** @brief Conversion mode configuration : Configuration Register Bit 1 */
typedef enum
{
    ADS1219_CONVERSION_SINGLE_SHOT = 0, ///< Single-shot conversion mode (default)
    ADS1219_CONVERSION_CONTINUOUS       ///< Continuous conversion mode
} ads1219_conversion_mode_config_t;

/** @brief Voltage reference configuration : Configuration Register Bit 0 */
typedef enum
{
    ADS1219_VREF_INTERNAL = 0, ///< Internal 2.048V reference (default)
    ADS1219_VREF_EXTERNAL      ///< External reference: REFP and REFN
} ads1219_vref_config_t;

///////////////////////////////////////////////////////////////////////////////
// Register Descriptions
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Configuration Register.
 *
 * A union is used here so that individual values from the register can be
 * accessed or the whole register can be accessed.
 */
typedef union {
    struct
    {
        uint8_t vref : 1; ///< Voltage reference configuration : Configuration Register Bit 0
        uint8_t cm : 1;   ///< Conversion mode configuration : Configuration Register Bit 1
        uint8_t dr : 2;   ///< Data rate configuration : Configuration Register Bits 3:2
        uint8_t gain : 1; ///< Gain configuration : Configuration Register Bit 4
        uint8_t mux : 3;  ///< Input multiplexer configuration : Configuration Register Bits 7:5
    };
    uint8_t byte; ///< The whole register
} sfe_ads1219_reg_cfg_t;

/**
 * @brief Status Register.
 *
 * A union is used here so that individual values from the register can be
 * accessed or the whole register can be accessed.
 */
typedef union {
    struct
    {
        uint8_t id : 7;   ///< Device ID = 0x60 - but datasheet says "Reserved. Values are subject to change without notice"
        uint8_t drdy : 1; ///< Conversion result ready flag : Status Register Bit 7
    };
    uint8_t byte; ///< The whole register
} sfe_ads1219_reg_status_t;

///////////////////////////////////////////////////////////////////////////////

/**
 * @class sfDevADS1219
 * @brief Driver class for the TI ADS1219 24-Bit 4-Channel ADC.
 *
 * This class provides a platform independent interface to configure and read data from
 * the ADS1219, using a SparkFun Toolkit bus object for communication.
 *
 * Usage:
 * - Call begin() with an initialized bus object. This resets the device.
 * - Use the configuration methods to set the input multiplexer, gain, data rate,
 *   conversion mode and voltage reference.
 * - Call startSync() to start a conversion, dataReady() to check for completion,
 *   then readConversion() followed by getConversionMillivolts() or getConversionRaw().
 */
class sfDevADS1219
{
  public:
    sfDevADS1219() : _theBus{nullptr}, _adcGain{ADS1219_GAIN_1}, _adcResult{0}
    {
    }

    /**
     * @brief Initializes the ADS1219 device on the specified bus.
     *
     * Performs a soft reset of the device and verifies the Configuration Register
     * reads back as its default value of 0x00.
     *
     * @param theBus Pointer to the bus interface (sfTkIBus) to use for communication.
     * @return sfTkError_t ksfTkErrOk on success, otherwise an error code.
     */
    sfTkError_t begin(sfTkIBus *theBus = nullptr);

    /**
     * @brief Checks if the ADS1219 is connected and responding.
     *
     * @return true if the device responds to a status register read, false otherwise.
     */
    bool isConnected(void);

    /**
     * @brief Performs a soft reset of the ADC.
     *
     * @return sfTkError_t ksfTkErrOk on success, otherwise an error code.
     */
    sfTkError_t reset(void);

    /**
     * @brief Start or restart conversions.
     *
     * @return sfTkError_t ksfTkErrOk on success, otherwise an error code.
     */
    sfTkError_t startSync(void);

    /**
     * @brief Enter power-down mode.
     *
     * @return sfTkError_t ksfTkErrOk on success, otherwise an error code.
     */
    sfTkError_t powerDown(void);

    /**
     * @brief Reads the ADC conversion data, converts it to a usable form, and
     * saves it to the internal result variable.
     *
     * Use getConversionMillivolts() or getConversionRaw() to retrieve the result.
     *
     * @return sfTkError_t ksfTkErrOk on success, otherwise an error code.
     */
    sfTkError_t readConversion(void);

    /**
     * @brief Configure the conversion mode.
     *
     * @param mode The conversion mode - single-shot or continuous.
     * @return sfTkError_t ksfTkErrOk on success, otherwise an error code.
     */
    sfTkError_t setConversionMode(const ads1219_conversion_mode_config_t mode = ADS1219_CONVERSION_SINGLE_SHOT);

    /**
     * @brief Configure the input multiplexer.
     *
     * @param config The input multiplexer configuration.
     * @return sfTkError_t ksfTkErrOk on success, otherwise an error code.
     */
    sfTkError_t setInputMultiplexer(const ads1219_input_multiplexer_config_t config = ADS1219_CONFIG_MUX_DIFF_P0_N1);

    /**
     * @brief Configure the gain.
     *
     * @param gain The gain - 1 or 4.
     * @return sfTkError_t ksfTkErrOk on success, otherwise an error code.
     */
    sfTkError_t setGain(const ads1219_gain_config_t gain = ADS1219_GAIN_1);

    /**
     * @brief Configure the data rate (samples per second).
     *
     * @param rate The data rate.
     * @return sfTkError_t ksfTkErrOk on success, otherwise an error code.
     */
    sfTkError_t setDataRate(const ads1219_data_rate_config_t rate = ADS1219_DATA_RATE_20SPS);

    /**
     * @brief Configure the voltage reference.
     *
     * @param vRef The voltage reference - internal or external.
     * @return sfTkError_t ksfTkErrOk on success, otherwise an error code.
     */
    sfTkError_t setVoltageReference(const ads1219_vref_config_t vRef = ADS1219_VREF_INTERNAL);

    /**
     * @brief Return the conversion result which was read by readConversion.
     *
     * Converts the result to mV using referenceVoltageMillivolts and the gain.
     *
     * @param referenceVoltageMillivolts Usually the internal 2.048V reference voltage.
     * But the user can override with (REFP - REFN) when using the external reference.
     * @return float The voltage in millivolts.
     */
    float getConversionMillivolts(float referenceVoltageMillivolts = 2048.0);

    /**
     * @brief Return the raw conversion result which was read by readConversion.
     *
     * @return int32_t The raw signed conversion result. 24-bit (2's complement).
     * NOT adjusted for gain.
     */
    int32_t getConversionRaw(void);

    /**
     * @brief Check the data ready flag.
     *
     * @return true if data is ready, false otherwise (or on a communication error).
     */
    bool dataReady(void);

    /**
     * @brief Read the ADS1219 Configuration Register into a sfe_ads1219_reg_cfg_t struct.
     *
     * @param[out] config Reference of a sfe_ads1219_reg_cfg_t struct to hold the register contents.
     * @return sfTkError_t ksfTkErrOk on success, otherwise an error code.
     */
    sfTkError_t getConfigurationRegister(sfe_ads1219_reg_cfg_t &config);

    /**
     * @brief Write a sfe_ads1219_reg_cfg_t struct into the ADS1219 Configuration Register.
     *
     * @param config A sfe_ads1219_reg_cfg_t struct holding the register contents.
     * @return sfTkError_t ksfTkErrOk on success, otherwise an error code.
     */
    sfTkError_t setConfigurationRegister(sfe_ads1219_reg_cfg_t config);

  private:
    // Register addresses
    static constexpr uint8_t kRegConfigWrite = 0x40; // Configuration register - write
    static constexpr uint8_t kRegConfigRead = 0x20;  // Configuration register - read
    static constexpr uint8_t kRegStatusRead = 0x24;  // Status register - read

    // Commands
    static constexpr uint8_t kCommandReset = 0x06;     // Reset the device
    static constexpr uint8_t kCommandStartSync = 0x08; // Start or restart conversions
    static constexpr uint8_t kCommandPowerDown = 0x02; // Enter power-down mode
    static constexpr uint8_t kCommandReadData = 0x10;  // Read conversion data by command

    /**
     * @brief Pointer to the bus interface used to communicate with the ADS1219.
     */
    sfTkIBus *_theBus;

    /**
     * @brief Local copy of the ADC gain - needed for conversion to mV.
     */
    ads1219_gain_config_t _adcGain;

    /**
     * @brief Local store for the ADC conversion result. 24-Bit, 2's complement.
     */
    int32_t _adcResult;
};
