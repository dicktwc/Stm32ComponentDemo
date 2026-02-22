/**
  ******************************************************************************
  * @file           : w25q64_example.c
  * @brief          : W25Q64 QSPI Flash driver usage example
  ******************************************************************************
  */

#include "w25q64.h"
#include <stdio.h>

/* W25Q64 device instance */
W25Q64_t w25q64_dev;

/**
 * @brief Test and demonstrate W25Q64 functionality
 * Note: Make sure QSPI is initialized before calling this function
 * 
 * @param hqspi: Pointer to QSPI handle (e.g., &hqspi)
 */
void W25Q64_Example(QSPI_HandleTypeDef *hqspi)
{
    HAL_StatusTypeDef status;
    uint8_t id[3];
    uint8_t write_buffer[256];
    uint8_t read_buffer[256];

    printf("\r\n========== W25Q64 QSPI Flash Memory Test ==========\r\n");

    /* Initialize W25Q64 device */
    printf("Initializing W25Q64...\r\n");
    status = W25Q64_Init(&w25q64_dev, hqspi);
    if (status != HAL_OK)
    {
        printf("Failed to initialize W25Q64!\r\n");
        return;
    }
    printf("W25Q64 initialized successfully.\r\n");

    /* Read JEDEC ID */
    printf("\nReading JEDEC ID...\r\n");
    status = W25Q64_ReadID(&w25q64_dev, id);
    if (status == HAL_OK)
    {
        printf("JEDEC ID: 0x%02X 0x%02X 0x%02X\r\n", id[0], id[1], id[2]);
        printf("(Format: Manufacturer, Memory Type, Capacity)\r\n");
    }
    else
    {
        printf("Failed to read JEDEC ID!\r\n");
    }

    /* Read status register */
    printf("\nReading Status Register...\r\n");
    printf("Status Register check will be done during operations.\r\n");

    /* Prepare test data */
    printf("\nPreparing test data...\r\n");
    for (uint16_t i = 0; i < 256; i++)
    {
        write_buffer[i] = (uint8_t)(i % 256);
    }

    /* Test address (use 0x000000 for this example) */
    uint32_t test_addr = 0x000000;

    /* Erase sector before writing */
    printf("Erasing sector at address 0x%06X...\r\n", test_addr);
    status = W25Q64_EraseSector(&w25q64_dev, test_addr);
    if (status == HAL_OK)
    {
        printf("Sector erased successfully.\r\n");
        /* Wait for erase to complete */
        HAL_Delay(100);
    }
    else
    {
        printf("Failed to erase sector!\r\n");
    }

    /* Write data */
    printf("\nWriting 256 bytes of test data at address 0x%06X...\r\n", test_addr);
    status = W25Q64_PageProgram(&w25q64_dev, test_addr, write_buffer, 256);
    if (status == HAL_OK)
    {
        printf("Data written successfully.\r\n");
        /* Wait for write to complete */
        HAL_Delay(10);
    }
    else
    {
        printf("Failed to write data!\r\n");
    }

    /* Read data back */
    printf("\nReading 256 bytes from address 0x%06X...\r\n", test_addr);
    status = W25Q64_Read(&w25q64_dev, test_addr, read_buffer, 256);
    if (status == HAL_OK)
    {
        printf("Data read successfully.\r\n");

        /* Verify data */
        uint8_t verify_ok = 1;
        for (uint16_t i = 0; i < 256; i++)
        {
            if (read_buffer[i] != write_buffer[i])
            {
                verify_ok = 0;
                printf("Data mismatch at offset %d: wrote 0x%02X, read 0x%02X\r\n", 
                       i, write_buffer[i], read_buffer[i]);
            }
        }

        if (verify_ok)
        {
            printf("Verification: PASSED - All data matches!\r\n");
        }
        else
        {
            printf("Verification: FAILED - Data mismatch detected!\r\n");
        }
    }
    else
    {
        printf("Failed to read data!\r\n");
    }

    /* Display some read data */
    printf("\nFirst 32 bytes read:\r\n");
    for (uint8_t i = 0; i < 32; i++)
    {
        printf("0x%02X ", read_buffer[i]);
        if ((i + 1) % 16 == 0)
            printf("\r\n");
    }

    printf("\n========== W25Q64 Test Complete ==========\r\n");
}

/**
 * @brief Write arbitrary data to W25Q64 with auto-erase
 * This function uses W25Q64_Write() which automatically handles erasing
 * 
 * @param addr: Start address
 * @param data: Data to write
 * @param len: Number of bytes to write
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef W25Q64_WriteData(uint32_t addr, uint8_t *data, uint32_t len)
{
    return W25Q64_Write(&w25q64_dev, addr, data, len);
}

/**
 * @brief Read data from W25Q64
 * 
 * @param addr: Start address
 * @param data: Buffer to store read data
 * @param len: Number of bytes to read
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef W25Q64_ReadData(uint32_t addr, uint8_t *data, uint32_t len)
{
    return W25Q64_Read(&w25q64_dev, addr, data, len);
}

/**
 * @brief Read data from W25Q64 using fast read mode
 * 
 * @param addr: Start address
 * @param data: Buffer to store read data
 * @param len: Number of bytes to read
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef W25Q64_FastReadData(uint32_t addr, uint8_t *data, uint32_t len)
{
    return W25Q64_FastRead(&w25q64_dev, addr, data, len);
}
