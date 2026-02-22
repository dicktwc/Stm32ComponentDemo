/**
  ******************************************************************************
  * @file           : at24c256.h
  * @brief          : AT24C256 EEPROM driver header file
  ******************************************************************************
  */

#ifndef __AT24C256_H__
#define __AT24C256_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

/* AT24C256 Device Parameters */
#define AT24C256_ADDR_BASE          0xA0    /* Base I2C address (0b1010xxx0) */
#define AT24C256_CAPACITY           0x8000  /* 32KB = 32768 bytes */
#define AT24C256_PAGE_SIZE          64      /* Page size in bytes */
#define AT24C256_WRITE_TIME         5       /* Write cycle time in milliseconds */

/* Address pin combinations (A2, A1, A0) */
#define AT24C256_I2C_ADDR_0         0xA0    /* A2=0, A1=0, A0=0 */
#define AT24C256_I2C_ADDR_1         0xA2    /* A2=0, A1=0, A0=1 */
#define AT24C256_I2C_ADDR_2         0xA4    /* A2=0, A1=1, A0=0 */
#define AT24C256_I2C_ADDR_3         0xA6    /* A2=0, A1=1, A0=1 */
#define AT24C256_I2C_ADDR_4         0xA8    /* A2=1, A1=0, A0=0 */
#define AT24C256_I2C_ADDR_5         0xAA    /* A2=1, A1=0, A0=1 */
#define AT24C256_I2C_ADDR_6         0xAC    /* A2=1, A1=1, A0=0 */
#define AT24C256_I2C_ADDR_7         0xAE    /* A2=1, A1=1, A0=1 */

/**
 * @brief AT24C256 EEPROM device structure
 */
typedef struct
{
    I2C_HandleTypeDef *hi2c;        /* I2C handle */
    uint8_t i2c_addr;              /* I2C device address */
} AT24C256_t;

/**
 * @brief Initialize AT24C256 device
 * @param dev: Pointer to AT24C256 device structure
 * @param hi2c: Pointer to I2C handle
 * @param i2c_addr: I2C address of AT24C256
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef AT24C256_Init(AT24C256_t *dev, I2C_HandleTypeDef *hi2c, uint8_t i2c_addr);

/**
 * @brief Read data from AT24C256
 * @param dev: Pointer to AT24C256 device structure
 * @param addr: Start address in EEPROM (0x0000 - 0x7FFF)
 * @param data: Pointer to data buffer
 * @param len: Number of bytes to read
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef AT24C256_Read(AT24C256_t *dev, uint16_t addr, uint8_t *data, uint16_t len);

/**
 * @brief Read single byte from AT24C256
 * @param dev: Pointer to AT24C256 device structure
 * @param addr: Address in EEPROM
 * @param data: Pointer to store read byte
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef AT24C256_ReadByte(AT24C256_t *dev, uint16_t addr, uint8_t *data);

/**
 * @brief Write data to AT24C256
 * @param dev: Pointer to AT24C256 device structure
 * @param addr: Start address in EEPROM (0x0000 - 0x7FFF)
 * @param data: Pointer to data buffer
 * @param len: Number of bytes to write
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef AT24C256_Write(AT24C256_t *dev, uint16_t addr, const uint8_t *data, uint16_t len);

/**
 * @brief Write single byte to AT24C256
 * @param dev: Pointer to AT24C256 device structure
 * @param addr: Address in EEPROM
 * @param data: Byte to write
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef AT24C256_WriteByte(AT24C256_t *dev, uint16_t addr, uint8_t data);

/**
 * @brief Check if device is ready (poll until device is ready after write)
 * @param dev: Pointer to AT24C256 device structure
 * @param timeout: Maximum time to wait in milliseconds
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef AT24C256_IsReady(AT24C256_t *dev, uint32_t timeout);

/**
 * @brief Erase all data (fill with 0xFF)
 * @param dev: Pointer to AT24C256 device structure
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef AT24C256_Erase(AT24C256_t *dev);

#ifdef __cplusplus
}
#endif

#endif /* __AT24C256_H__ */
