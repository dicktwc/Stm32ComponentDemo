/**
  ******************************************************************************
  * @file           : at24c256.c
  * @brief          : AT24C256 EEPROM driver source file
  ******************************************************************************
  */

#include "at24c256.h"
#include <string.h>

/**
 * @brief Initialize AT24C256 device
 */
HAL_StatusTypeDef AT24C256_Init(AT24C256_t *dev, I2C_HandleTypeDef *hi2c, uint8_t i2c_addr)
{
    if (dev == NULL || hi2c == NULL)
    {
        return HAL_ERROR;
    }

    dev->hi2c = hi2c;
    dev->i2c_addr = i2c_addr;

    /* Check if device is ready */
    return AT24C256_IsReady(dev, 100);
}

/**
 * @brief Read single byte from AT24C256
 */
HAL_StatusTypeDef AT24C256_ReadByte(AT24C256_t *dev, uint16_t addr, uint8_t *data)
{
    return AT24C256_Read(dev, addr, data, 1);
}

/**
 * @brief Read data from AT24C256
 * Procedure: Send address (16-bit), then read data
 */
HAL_StatusTypeDef AT24C256_Read(AT24C256_t *dev, uint16_t addr, uint8_t *data, uint16_t len)
{
    HAL_StatusTypeDef status;
    uint8_t addr_buf[2];

    if (dev == NULL || data == NULL)
    {
        return HAL_ERROR;
    }

    if (addr + len > AT24C256_CAPACITY)
    {
        return HAL_ERROR;  /* Address out of range */
    }

    /* Convert address to big-endian format */
    addr_buf[0] = (uint8_t)((addr >> 8) & 0xFF);
    addr_buf[1] = (uint8_t)(addr & 0xFF);

    /* Send address (2 bytes) where the data should be read from */
    status = HAL_I2C_Master_Transmit(dev->hi2c, dev->i2c_addr, addr_buf, 2, 1000);
    if (status != HAL_OK)
    {
        return status;
    }

    /* Read data from the device */
    status = HAL_I2C_Master_Receive(dev->hi2c, dev->i2c_addr, data, len, 1000);

    return status;
}

/**
 * @brief Write single byte to AT24C256
 */
HAL_StatusTypeDef AT24C256_WriteByte(AT24C256_t *dev, uint16_t addr, uint8_t data)
{
    return AT24C256_Write(dev, addr, &data, 1);
}

/**
 * @brief Write data to AT24C256
 * Procedure: Send address (16-bit) + data in one transmission
 * Note: Writing across page boundary is not recommended
 */
HAL_StatusTypeDef AT24C256_Write(AT24C256_t *dev, uint16_t addr, const uint8_t *data, uint16_t len)
{
    HAL_StatusTypeDef status;
    uint8_t write_buf[AT24C256_PAGE_SIZE + 2];  /* 2 bytes for address + up to 64 bytes data */
    uint16_t bytes_to_write;
    uint16_t current_addr = addr;
    const uint8_t *current_data = data;
    uint16_t remaining = len;

    if (dev == NULL || data == NULL)
    {
        return HAL_ERROR;
    }

    if (addr + len > AT24C256_CAPACITY)
    {
        return HAL_ERROR;  /* Address out of range */
    }

    /* Write data page by page */
    while (remaining > 0)
    {
        /* Calculate how many bytes we can write in this iteration */
        /* (don't cross page boundary) */
        uint16_t page_offset = current_addr % AT24C256_PAGE_SIZE;
        bytes_to_write = AT24C256_PAGE_SIZE - page_offset;
        if (bytes_to_write > remaining)
        {
            bytes_to_write = remaining;
        }

        /* Prepare write buffer: address (2 bytes) + data */
        write_buf[0] = (uint8_t)((current_addr >> 8) & 0xFF);
        write_buf[1] = (uint8_t)(current_addr & 0xFF);
        memcpy(&write_buf[2], current_data, bytes_to_write);

        /* Send address + data */
        status = HAL_I2C_Master_Transmit(dev->hi2c, dev->i2c_addr, write_buf,
                                        bytes_to_write + 2, 1000);
        if (status != HAL_OK)
        {
            return status;
        }

        /* Wait for write cycle to complete */
        status = AT24C256_IsReady(dev, 100);
        if (status != HAL_OK)
        {
            return status;
        }

        /* Update variables for next iteration */
        current_addr += bytes_to_write;
        current_data += bytes_to_write;
        remaining -= bytes_to_write;
    }

    return HAL_OK;
}

/**
 * @brief Check if device is ready (poll until device responds)
 * After write operation, device doesn't respond to I2C until write cycle completes
 */
HAL_StatusTypeDef AT24C256_IsReady(AT24C256_t *dev, uint32_t timeout)
{
    HAL_StatusTypeDef status;
    uint32_t start_time;

    if (dev == NULL)
    {
        return HAL_ERROR;
    }

    start_time = HAL_GetTick();

    while (1)
    {
        /* Try to send a dummy write (0 bytes) to check if device acknowledges */
        status = HAL_I2C_Master_Transmit(dev->hi2c, dev->i2c_addr, NULL, 0, 100);

        if (status == HAL_OK)
        {
            return HAL_OK;  /* Device is ready */
        }

        /* Check timeout */
        if ((HAL_GetTick() - start_time) > timeout)
        {
            return HAL_TIMEOUT;
        }

        /* Small delay before retry */
        HAL_Delay(1);
    }
}

/**
 * @brief Erase all data (fill entire EEPROM with 0xFF)
 * This fills the entire 32KB with 0xFF
 */
HAL_StatusTypeDef AT24C256_Erase(AT24C256_t *dev)
{
    HAL_StatusTypeDef status;
    uint8_t erase_data[AT24C256_PAGE_SIZE];
    uint16_t addr = 0;

    if (dev == NULL)
    {
        return HAL_ERROR;
    }

    /* Fill erase buffer with 0xFF */
    memset(erase_data, 0xFF, AT24C256_PAGE_SIZE);

    /* Write 0xFF to entire EEPROM, page by page */
    while (addr < AT24C256_CAPACITY)
    {
        status = AT24C256_Write(dev, addr, erase_data, AT24C256_PAGE_SIZE);
        if (status != HAL_OK)
        {
            return status;
        }

        addr += AT24C256_PAGE_SIZE;
    }

    return HAL_OK;
}
