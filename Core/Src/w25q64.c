/**
  ******************************************************************************
  * @file           : w25q64.c
  * @brief          : W25Q64 QSPI Flash driver source file
  ******************************************************************************
  */

#include "w25q64.h"
#include <string.h>

/* Private function prototypes */
HAL_StatusTypeDef W25Q64_WriteEnable(W25Q64_t *dev);
HAL_StatusTypeDef W25Q64_WriteDisable(W25Q64_t *dev);
uint8_t W25Q64_ReadStatus(W25Q64_t *dev);
HAL_StatusTypeDef W25Q64_WaitReady(W25Q64_t *dev, uint32_t timeout);


/**
 * @brief Initialize W25Q64 device
 */
HAL_StatusTypeDef W25Q64_Init(W25Q64_t *dev, QSPI_HandleTypeDef *hqspi)
{
    if (dev == NULL || hqspi == NULL)
    {
        return HAL_ERROR;
    }

    dev->hqspi = hqspi;

    /* Verify device is accessible by reading JEDEC ID */
    uint8_t id[3];
    HAL_StatusTypeDef status = W25Q64_ReadID(dev, id);
    
    if (status == HAL_OK)
    {
        /* Optional: Verify JEDEC ID matches W25Q64 */
        uint32_t jedec = (id[0] << 16) | (id[1] << 8) | id[2];
        if (jedec != W25Q64_JEDEC_ID)
        {
            /* Device found but may not be W25Q64 */
            /* Proceed anyway as it might be a compatible device */
        }
    }

    return status;
}

/**
 * @brief Read JEDEC ID
 */
HAL_StatusTypeDef W25Q64_ReadID(W25Q64_t *dev, uint8_t *id)
{
    QSPI_CommandTypeDef cmd;
    HAL_StatusTypeDef status;

    if (dev == NULL || id == NULL)
    {
        return HAL_ERROR;
    }

    /* Wait for device ready */
    status = W25Q64_WaitReady(dev, 1000);
    if (status != HAL_OK)
    {
        return status;
    }

    /* Configure QSPI command for Read JEDEC ID */
    memset(&cmd, 0, sizeof(cmd));
    cmd.Instruction = W25Q64_CMD_READ_JEDEC_ID;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode = QSPI_DATA_1_LINE;
    cmd.DummyCycles = 0;
    cmd.NbData = 3;
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    //cmd.ChipSelectHighTime = QSPI_CS_HIGH_TIME_2_CYCLE;

    /* Receive ID */
    status = HAL_QSPI_Command(dev->hqspi, &cmd, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
    if (status != HAL_OK)
    {
        return status;
    }

    status = HAL_QSPI_Receive(dev->hqspi, id, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
    
    return status;
}

/**
 * @brief Read status register
 */
uint8_t W25Q64_ReadStatus(W25Q64_t *dev)
{
    QSPI_CommandTypeDef cmd;
    uint8_t status_reg = 0xFF;

    if (dev == NULL)
    {
        return status_reg;
    }

    /* Configure QSPI command for Read Status */
    memset(&cmd, 0, sizeof(cmd));
    cmd.Instruction = W25Q64_CMD_READ_STATUS;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode = QSPI_DATA_1_LINE;
    cmd.DummyCycles = 0;
    cmd.NbData = 1;
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    //cmd.ChipSelectHighTime = QSPI_CS_HIGH_TIME_2_CYCLE;

    if (HAL_QSPI_Command(dev->hqspi, &cmd, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK)
    {
        HAL_QSPI_Receive(dev->hqspi, &status_reg, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
    }

    return status_reg;
}

/**
 * @brief Wait for device ready
 */
HAL_StatusTypeDef W25Q64_WaitReady(W25Q64_t *dev, uint32_t timeout)
{
    uint32_t tick_start = HAL_GetTick();

    if (dev == NULL)
    {
        return HAL_ERROR;
    }

    while (HAL_GetTick() - tick_start < timeout)
    {
        uint8_t status = W25Q64_ReadStatus(dev);
        if ((status & W25Q64_STATUS_BUSY) == 0)
        {
            return HAL_OK;
        }
        HAL_Delay(1);
    }

    return HAL_TIMEOUT;
}

/**
 * @brief Read data from W25Q64
 */
HAL_StatusTypeDef W25Q64_Read(W25Q64_t *dev, uint32_t addr, uint8_t *data, uint32_t len)
{
    QSPI_CommandTypeDef cmd;
    HAL_StatusTypeDef status;

    if (dev == NULL || data == NULL)
    {
        return HAL_ERROR;
    }

    if (addr + len > W25Q64_CAPACITY)
    {
        return HAL_ERROR;  /* Address out of range */
    }

    /* Wait for device ready */
    status = W25Q64_WaitReady(dev, 1000);
    if (status != HAL_OK)
    {
        return status;
    }

    /* Configure QSPI command for Read Data */
    memset(&cmd, 0, sizeof(cmd));
    cmd.Instruction = W25Q64_CMD_READ_DATA;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.Address = addr;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode = QSPI_DATA_1_LINE;
    cmd.DummyCycles = 0;
    cmd.NbData = len;
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    //cmd.ChipSelectHighTime = QSPI_CS_HIGH_TIME_2_CYCLE;

    status = HAL_QSPI_Command(dev->hqspi, &cmd, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
    if (status != HAL_OK)
    {
        return status;
    }

    status = HAL_QSPI_Receive(dev->hqspi, data, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);

    return status;
}

/**
 * @brief Fast read from W25Q64 (with dummy byte)
 */
HAL_StatusTypeDef W25Q64_FastRead(W25Q64_t *dev, uint32_t addr, uint8_t *data, uint32_t len)
{
    QSPI_CommandTypeDef cmd;
    HAL_StatusTypeDef status;

    if (dev == NULL || data == NULL)
    {
        return HAL_ERROR;
    }

    if (addr + len > W25Q64_CAPACITY)
    {
        return HAL_ERROR;
    }

    status = W25Q64_WaitReady(dev, 1000);
    if (status != HAL_OK)
    {
        return status;
    }

    /* Configure QSPI command for Fast Read */
    memset(&cmd, 0, sizeof(cmd));
    cmd.Instruction = W25Q64_CMD_FAST_READ;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.Address = addr;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode = QSPI_DATA_1_LINE;
    cmd.DummyCycles = 8;  /* 1 dummy byte = 8 cycles */
    cmd.NbData = len;
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    //cmd.ChipSelectHighTime = QSPI_CS_HIGH_TIME_2_CYCLE;

    status = HAL_QSPI_Command(dev->hqspi, &cmd, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
    if (status != HAL_OK)
    {
        return status;
    }

    status = HAL_QSPI_Receive(dev->hqspi, data, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);

    return status;
}

/**
 * @brief Write enable
 */
HAL_StatusTypeDef W25Q64_WriteEnable(W25Q64_t *dev)
{
    QSPI_CommandTypeDef cmd;

    if (dev == NULL)
    {
        return HAL_ERROR;
    }

    /* Configure QSPI command for Write Enable */
    memset(&cmd, 0, sizeof(cmd));
    cmd.Instruction = W25Q64_CMD_WRITE_ENABLE;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode = QSPI_DATA_NONE;
    cmd.DummyCycles = 0;
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    //cmd.ChipSelectHighTime = QSPI_CS_HIGH_TIME_2_CYCLE;

    return HAL_QSPI_Command(dev->hqspi, &cmd, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
}

/**
 * @brief Write disable
 */
HAL_StatusTypeDef W25Q64_WriteDisable(W25Q64_t *dev)
{
    QSPI_CommandTypeDef cmd;

    if (dev == NULL)
    {
        return HAL_ERROR;
    }

    /* Configure QSPI command for Write Disable */
    memset(&cmd, 0, sizeof(cmd));
    cmd.Instruction = W25Q64_CMD_WRITE_DISABLE;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode = QSPI_DATA_NONE;
    cmd.DummyCycles = 0;
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    //cmd.ChipSelectHighTime = QSPI_CS_HIGH_TIME_2_CYCLE;

    return HAL_QSPI_Command(dev->hqspi, &cmd, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
}

/**
 * @brief Page program (write up to 256 bytes)
 */
HAL_StatusTypeDef W25Q64_PageProgram(W25Q64_t *dev, uint32_t addr, uint8_t *data, uint32_t len)
{
    QSPI_CommandTypeDef cmd;
    HAL_StatusTypeDef status;

    if (dev == NULL || data == NULL)
    {
        return HAL_ERROR;
    }

    if (len > W25Q64_PAGE_SIZE || addr + len > W25Q64_CAPACITY)
    {
        return HAL_ERROR;
    }

    /* Wait for device ready */
    status = W25Q64_WaitReady(dev, 1000);
    if (status != HAL_OK)
    {
        return status;
    }

    /* Enable write */
    status = W25Q64_WriteEnable(dev);
    if (status != HAL_OK)
    {
        return status;
    }

    /* Configure QSPI command for Page Program */
    memset(&cmd, 0, sizeof(cmd));
    cmd.Instruction = W25Q64_CMD_PAGE_PROGRAM;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.Address = addr;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode = QSPI_DATA_1_LINE;
    cmd.DummyCycles = 0;
    cmd.NbData = len;
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    //cmd.ChipSelectHighTime = QSPI_CS_HIGH_TIME_2_CYCLE;

    status = HAL_QSPI_Command(dev->hqspi, &cmd, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
    if (status != HAL_OK)
    {
        return status;
    }

    status = HAL_QSPI_Transmit(dev->hqspi, data, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);

    return status;
}

/**
 * @brief Generic write with automatic page handling
 */
HAL_StatusTypeDef W25Q64_Write(W25Q64_t *dev, uint32_t addr, uint8_t *data, uint32_t len)
{
    HAL_StatusTypeDef status;
    uint32_t bytes_written = 0;
    uint32_t current_addr = addr;
    uint32_t bytes_remaining = len;

    if (dev == NULL || data == NULL)
    {
        return HAL_ERROR;
    }

    if (addr + len > W25Q64_CAPACITY)
    {
        return HAL_ERROR;
    }

    while (bytes_remaining > 0)
    {
        /* Calculate number of bytes to write in this iteration */
        uint32_t bytes_to_write = W25Q64_PAGE_SIZE - (current_addr % W25Q64_PAGE_SIZE);
        if (bytes_to_write > bytes_remaining)
        {
            bytes_to_write = bytes_remaining;
        }

        /* Check if we need to erase the sector first */
        static uint32_t last_erased_sector = 0xFFFFFFFF;
        uint32_t current_sector = current_addr / W25Q64_SECTOR_SIZE;
        
        if (current_sector != last_erased_sector)
        {
            status = W25Q64_EraseSector(dev, current_addr);
            if (status != HAL_OK)
            {
                return status;
            }
            last_erased_sector = current_sector;
        }

        /* Program the page */
        status = W25Q64_PageProgram(dev, current_addr, 
                                    &data[bytes_written], bytes_to_write);
        if (status != HAL_OK)
        {
            return status;
        }

        /* Update counters */
        bytes_written += bytes_to_write;
        current_addr += bytes_to_write;
        bytes_remaining -= bytes_to_write;
    }

    return HAL_OK;
}

/**
 * @brief Erase 4KB sector
 */
HAL_StatusTypeDef W25Q64_EraseSector(W25Q64_t *dev, uint32_t addr)
{
    QSPI_CommandTypeDef cmd;
    HAL_StatusTypeDef status;

    if (dev == NULL)
    {
        return HAL_ERROR;
    }

    if (addr >= W25Q64_CAPACITY)
    {
        return HAL_ERROR;
    }

    /* Wait for device ready */
    status = W25Q64_WaitReady(dev, 1000);
    if (status != HAL_OK)
    {
        return status;
    }

    /* Enable write */
    status = W25Q64_WriteEnable(dev);
    if (status != HAL_OK)
    {
        return status;
    }

    /* Configure QSPI command for Sector Erase */
    memset(&cmd, 0, sizeof(cmd));
    cmd.Instruction = W25Q64_CMD_SECTOR_ERASE;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.Address = addr;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode = QSPI_DATA_NONE;
    cmd.DummyCycles = 0;
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    //cmd.ChipSelectHighTime = QSPI_CS_HIGH_TIME_2_CYCLE;

    status = HAL_QSPI_Command(dev->hqspi, &cmd, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);

    return status;
}

/**
 * @brief Erase 64KB block
 */
HAL_StatusTypeDef W25Q64_EraseBlock64(W25Q64_t *dev, uint32_t addr)
{
    QSPI_CommandTypeDef cmd;
    HAL_StatusTypeDef status;

    if (dev == NULL)
    {
        return HAL_ERROR;
    }

    if (addr >= W25Q64_CAPACITY)
    {
        return HAL_ERROR;
    }

    status = W25Q64_WaitReady(dev, 1000);
    if (status != HAL_OK)
    {
        return status;
    }

    status = W25Q64_WriteEnable(dev);
    if (status != HAL_OK)
    {
        return status;
    }

    /* Configure QSPI command for Block Erase 64KB */
    memset(&cmd, 0, sizeof(cmd));
    cmd.Instruction = W25Q64_CMD_BLOCK64_ERASE;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.Address = addr;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode = QSPI_DATA_NONE;
    cmd.DummyCycles = 0;
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    //cmd.ChipSelectHighTime = QSPI_CS_HIGH_TIME_2_CYCLE;

    status = HAL_QSPI_Command(dev->hqspi, &cmd, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);

    return status;
}

/**
 * @brief Erase entire chip
 */
HAL_StatusTypeDef W25Q64_ChipErase(W25Q64_t *dev)
{
    QSPI_CommandTypeDef cmd;
    HAL_StatusTypeDef status;

    if (dev == NULL)
    {
        return HAL_ERROR;
    }

    status = W25Q64_WaitReady(dev, 1000);
    if (status != HAL_OK)
    {
        return status;
    }

    status = W25Q64_WriteEnable(dev);
    if (status != HAL_OK)
    {
        return status;
    }

    /* Configure QSPI command for Chip Erase */
    memset(&cmd, 0, sizeof(cmd));
    cmd.Instruction = W25Q64_CMD_CHIP_ERASE;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_NONE;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode = QSPI_DATA_NONE;
    cmd.DummyCycles = 0;
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    //cmd.ChipSelectHighTime = QSPI_CS_HIGH_TIME_2_CYCLE;

    status = HAL_QSPI_Command(dev->hqspi, &cmd, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);

    return status;
}
