/**
  ******************************************************************************
  * @file           : w25q64.h
  * @brief          : W25Q64 QSPI Flash driver header file
  ******************************************************************************
  */

#ifndef __W25Q64_H__
#define __W25Q64_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>
#include "stm32h7xx_hal.h"

/* W25Q64 Device Parameters */
#define W25Q64_CAPACITY              0x800000    /* 64Mbit = 8MB = 8388608 bytes */
#define W25Q64_PAGE_SIZE             256         /* Page size in bytes */
#define W25Q64_SECTOR_SIZE           0x1000      /* Sector size = 4KB */
#define W25Q64_BLOCK_SIZE            0x10000     /* Block size = 64KB */
#define W25Q64_SECTOR_COUNT          2048        /* Total sectors */

/* SPI Flash Commands */
#define W25Q64_CMD_WRITE_ENABLE      0x06
#define W25Q64_CMD_WRITE_DISABLE     0x04
#define W25Q64_CMD_READ_STATUS       0x05
#define W25Q64_CMD_READ_STATUS2      0x35
#define W25Q64_CMD_READ_STATUS3      0x15
#define W25Q64_CMD_WRITE_STATUS      0x01
#define W25Q64_CMD_READ_DATA         0x03
#define W25Q64_CMD_FAST_READ         0x0B
#define W25Q64_CMD_PAGE_PROGRAM      0x02
#define W25Q64_CMD_SECTOR_ERASE      0x20        /* Erase 4KB sector */
#define W25Q64_CMD_BLOCK32_ERASE     0x52        /* Erase 32KB block */
#define W25Q64_CMD_BLOCK64_ERASE     0xD8        /* Erase 64KB block */
#define W25Q64_CMD_CHIP_ERASE        0xC7
#define W25Q64_CMD_READ_JEDEC_ID     0x9F
#define W25Q64_CMD_READ_UID          0x4B
#define W25Q64_CMD_POWER_DOWN        0xB9
#define W25Q64_CMD_POWER_UP          0xAB

/* Status Register Bits */
#define W25Q64_STATUS_BUSY           0x01        /* Write in progress */
#define W25Q64_STATUS_BP0            0x04        /* Block protect bit 0 */
#define W25Q64_STATUS_BP1            0x08        /* Block protect bit 1 */
#define W25Q64_STATUS_BP2            0x10        /* Block protect bit 2 */

/* JEDEC ID */
#define W25Q64_JEDEC_ID              0xEF6017    /* Expected ID for W25Q64 */

/**
 * @brief W25Q64 device structure (using QSPI interface)
 */
typedef struct
{
    QSPI_HandleTypeDef *hqspi;      /* QSPI handle */
} W25Q64_t;

/**
 * @brief Initialize W25Q64 device
 * @param dev: Pointer to W25Q64 device structure
 * @param hqspi: Pointer to QSPI handle
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef W25Q64_Init(W25Q64_t *dev, QSPI_HandleTypeDef *hqspi);

/**
 * @brief Read JEDEC ID from device
 * @param dev: Pointer to W25Q64 device structure
 * @param id: Pointer to store the ID (should be 3 bytes)
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef W25Q64_ReadID(W25Q64_t *dev, uint8_t *id);

/**
 * @brief Read status register
 * @param dev: Pointer to W25Q64 device structure
 * @retval Status register value
 */
uint8_t W25Q64_ReadStatus(W25Q64_t *dev);

/**
 * @brief Wait for device to become ready
 * @param dev: Pointer to W25Q64 device structure
 * @param timeout: Timeout in milliseconds
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef W25Q64_WaitReady(W25Q64_t *dev, uint32_t timeout);

/**
 * @brief Read data from W25Q64
 * @param dev: Pointer to W25Q64 device structure
 * @param addr: Start address
 * @param data: Pointer to data buffer
 * @param len: Number of bytes to read
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef W25Q64_Read(W25Q64_t *dev, uint32_t addr, uint8_t *data, uint32_t len);

/**
 * @brief Fast read from W25Q64 (with dummy byte)
 * @param dev: Pointer to W25Q64 device structure
 * @param addr: Start address
 * @param data: Pointer to data buffer
 * @param len: Number of bytes to read
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef W25Q64_FastRead(W25Q64_t *dev, uint32_t addr, uint8_t *data, uint32_t len);

/**
 * @brief Write page to W25Q64 (up to 256 bytes)
 * @param dev: Pointer to W25Q64 device structure
 * @param addr: Page start address
 * @param data: Pointer to data buffer
 * @param len: Number of bytes to write (max 256)
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef W25Q64_PageProgram(W25Q64_t *dev, uint32_t addr, uint8_t *data, uint32_t len);

/**
 * @brief Write data to W25Q64 (handles multiple pages and erasing)
 * @param dev: Pointer to W25Q64 device structure
 * @param addr: Start address
 * @param data: Pointer to data buffer
 * @param len: Number of bytes to write
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef W25Q64_Write(W25Q64_t *dev, uint32_t addr, uint8_t *data, uint32_t len);

/**
 * @brief Erase 4KB sector
 * @param dev: Pointer to W25Q64 device structure
 * @param addr: Address within sector
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef W25Q64_EraseSector(W25Q64_t *dev, uint32_t addr);

/**
 * @brief Erase 64KB block
 * @param dev: Pointer to W25Q64 device structure
 * @param addr: Address within block
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef W25Q64_EraseBlock64(W25Q64_t *dev, uint32_t addr);

/**
 * @brief Erase entire chip
 * @param dev: Pointer to W25Q64 device structure
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef W25Q64_ChipErase(W25Q64_t *dev);

/**
 * @brief Enable write operations
 * @param dev: Pointer to W25Q64 device structure
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef W25Q64_WriteEnable(W25Q64_t *dev);

/**
 * @brief Disable write operations
 * @param dev: Pointer to W25Q64 device structure
 * @retval HAL_StatusTypeDef
 */
HAL_StatusTypeDef W25Q64_WriteDisable(W25Q64_t *dev);

#ifdef __cplusplus
}
#endif

#endif /* __W25Q64_H__ */
