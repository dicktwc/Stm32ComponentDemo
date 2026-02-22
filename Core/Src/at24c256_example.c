/**
  ******************************************************************************
  * @file           : at24c256_example.c
  * @brief          : AT24C256 EEPROM driver usage example
  ******************************************************************************
  */

#include "at24c256.h"
#include <stdio.h>
#include <string.h>


/**
 * @brief Example usage of AT24C256 EEPROM driver
 * This example shows how to:
 * 1. Initialize the device
 * 2. Write data to EEPROM
 * 3. Read data from EEPROM
 */

extern I2C_HandleTypeDef hi2c1;
AT24C256_t eeprom;

void AT24C256_Example(void)
{
    HAL_StatusTypeDef status;
    uint8_t write_buffer[64];
    uint8_t read_buffer[64];
    const char *test_string = "Hello AT24C256 EEPROM!";

    printf("\r\n========== AT24C256 EEPROM Example ==========\r\n");

    /* =====================================================
     * Step 1: Initialize AT24C256
     * ===================================================== */
    printf("[1] Initializing AT24C256...\r\n");
    status = AT24C256_Init(&eeprom, &hi2c1, AT24C256_I2C_ADDR_0);
    if (status != HAL_OK)
    {
        printf("    ERROR: Failed to initialize AT24C256\r\n");
        return;
    }
    printf("    OK: AT24C256 initialized successfully\r\n");

    /* =====================================================
     * Step 2: Write single byte
     * ===================================================== */
    printf("\r\n[2] Writing single byte (0xAA) to address 0x0000...\r\n");
    status = AT24C256_WriteByte(&eeprom, 0x0000, 0xAA);
    if (status != HAL_OK)
    {
        printf("    ERROR: Failed to write byte\r\n");
        return;
    }
    printf("    OK: Byte written\r\n");

    /* =====================================================
     * Step 3: Read single byte
     * ===================================================== */
    printf("[3] Reading single byte from address 0x0000...\r\n");
    status = AT24C256_ReadByte(&eeprom, 0x0000, read_buffer);
    if (status != HAL_OK)
    {
        printf("    ERROR: Failed to read byte\r\n");
        return;
    }
    printf("    OK: Read data = 0x%02X\r\n", read_buffer[0]);
    if (read_buffer[0] != 0xAA)
    {
        printf("    WARNING: Data mismatch (expected 0xAA)\r\n");
    }

    /* =====================================================
     * Step 4: Write string data
     * ===================================================== */
    printf("\r\n[4] Writing string to address 0x0010...\r\n");
    printf("    String: \"%s\"\r\n", test_string);
    status = AT24C256_Write(&eeprom, 0x0010, (const uint8_t *)test_string, strlen(test_string));
    if (status != HAL_OK)
    {
        printf("    ERROR: Failed to write string\r\n");
        return;
    }
    printf("    OK: String written (%d bytes)\r\n", strlen(test_string));

    /* =====================================================
     * Step 5: Read string data back
     * ===================================================== */
    printf("[5] Reading string from address 0x0010...\r\n");
    memset(read_buffer, 0, sizeof(read_buffer));
    status = AT24C256_Read(&eeprom, 0x0010, read_buffer, strlen(test_string));
    if (status != HAL_OK)
    {
        printf("    ERROR: Failed to read string\r\n");
        return;
    }
    printf("    OK: Read string: \"%s\"\r\n", (char *)read_buffer);
    if (strcmp((char *)read_buffer, test_string) != 0)
    {
        printf("    WARNING: String mismatch\r\n");
    }

    /* =====================================================
     * Step 6: Write multiple bytes (page write)
     * ===================================================== */
    printf("\r\n[6] Writing 32 bytes of pattern data...\r\n");
    for (int i = 0; i < 32; i++)
    {
        write_buffer[i] = (uint8_t)(0xA0 + i);
    }
    status = AT24C256_Write(&eeprom, 0x0100, write_buffer, 32);
    if (status != HAL_OK)
    {
        printf("    ERROR: Failed to write pattern\r\n");
        return;
    }
    printf("    OK: 32 bytes written\r\n");

    /* =====================================================
     * Step 7: Read multiple bytes back
     * ===================================================== */
    printf("[7] Reading 32 bytes from address 0x0100...\r\n");
    memset(read_buffer, 0, sizeof(read_buffer));
    status = AT24C256_Read(&eeprom, 0x0100, read_buffer, 32);
    if (status != HAL_OK)
    {
        printf("    ERROR: Failed to read pattern\r\n");
        return;
    }
    printf("    OK: Data read successfully\r\n");
    printf("    Data: ");
    for (int i = 0; i < 32; i++)
    {
        printf("%02X ", read_buffer[i]);
        if ((i + 1) % 16 == 0)
            printf("\r\n           ");
    }
    printf("\r\n");

    /* =====================================================
     * Step 8: Verify data
     * ===================================================== */
    printf("[8] Verifying data...\r\n");
    int mismatches = 0;
    for (int i = 0; i < 32; i++)
    {
        if (read_buffer[i] != (uint8_t)(0xA0 + i))
        {
            mismatches++;
        }
    }
    if (mismatches == 0)
    {
        printf("    OK: All data verified successfully!\r\n");
    }
    else
    {
        printf("    ERROR: %d bytes mismatched\r\n", mismatches);
    }

    printf("\r\n========== Example Complete ==========\r\n");
}

/**
 * @brief Advanced example: EEPROM as persistent storage for configuration
 */
typedef struct
{
    uint32_t magic;        /* Magic number for validation */
    uint32_t version;      /* Configuration version */
    uint16_t sensor_id;    /* Sensor ID */
    float calibration;     /* Calibration value */
    uint8_t reserved[48];  /* Reserved for future use */
} Config_t;

void AT24C256_ConfigExample(void)
{
    HAL_StatusTypeDef status;
    Config_t config_write, config_read;

    printf("\r\n========== AT24C256 Config Storage Example ==========\r\n");

    /* Initialize EEPROM if not already done */
    if (AT24C256_IsReady(&eeprom, 100) != HAL_OK)
    {
        AT24C256_Init(&eeprom, &hi2c1, AT24C256_I2C_ADDR_0);
    }

    /* Prepare configuration data */
    printf("[1] Preparing configuration data...\r\n");
    config_write.magic = 0x12345678;
    config_write.version = 1;
    config_write.sensor_id = 100;
    config_write.calibration = 3.14159f;
    printf("    Magic: 0x%08X\r\n", config_write.magic);
    printf("    Version: %u\r\n", config_write.version);
    printf("    Sensor ID: %u\r\n", config_write.sensor_id);
    printf("    Calibration: %.5f\r\n", config_write.calibration);

    /* Write configuration to EEPROM */
    printf("[2] Writing configuration to EEPROM (address 0x1000)...\r\n");
    status = AT24C256_Write(&eeprom, 0x1000, (const uint8_t *)&config_write, sizeof(Config_t));
    if (status != HAL_OK)
    {
        printf("    ERROR: Failed to write configuration\r\n");
        return;
    }
    printf("    OK: Configuration written (%d bytes)\r\n", sizeof(Config_t));

    /* Read configuration back from EEPROM */
    printf("[3] Reading configuration from EEPROM...\r\n");
    status = AT24C256_Read(&eeprom, 0x1000, (uint8_t *)&config_read, sizeof(Config_t));
    if (status != HAL_OK)
    {
        printf("    ERROR: Failed to read configuration\r\n");
        return;
    }

    /* Verify configuration */
    printf("[4] Verifying configuration...\r\n");
    printf("    Magic: 0x%08X %s\r\n", config_read.magic,
           config_read.magic == config_write.magic ? "(OK)" : "(MISMATCH)");
    printf("    Version: %u %s\r\n", config_read.version,
           config_read.version == config_write.version ? "(OK)" : "(MISMATCH)");
    printf("    Sensor ID: %u %s\r\n", config_read.sensor_id,
           config_read.sensor_id == config_write.sensor_id ? "(OK)" : "(MISMATCH)");
    printf("    Calibration: %.5f %s\r\n", config_read.calibration,
           config_read.calibration == config_write.calibration ? "(OK)" : "(MISMATCH)");

    printf("\r\n========== Config Example Complete ==========\r\n");
}
