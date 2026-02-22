# AT24C256 EEPROM 驅動程序

## 概述
AT24C256是一個256Kb (32KB) 的I2C EEPROM晶片，具有以下特性：

| 參數 | 值 |
|------|-----|
| 容量 | 256Kb (32,768 bytes) |
| I2C 地址 | 0xA0 - 0xAE (基於A2, A1, A0引腳) |
| 頁面大小 | 64 bytes |
| 寫入時間 | ~5ms |
| 工作溫度 | -40°C ~ +85°C |

## 文件說明

### 1. at24c256.h
驅動程序的頭文件，定義了：
- 設備結構體 `AT24C256_t`
- I2C地址常量
- 公有API函數聲明

### 2. at24c256.c
驅動程序的實現文件，包含：
- `AT24C256_Init()` - 初始化設備
- `AT24C256_Read()` - 讀取多個字節
- `AT24C256_ReadByte()` - 讀取單個字節
- `AT24C256_Write()` - 寫入多個字節
- `AT24C256_WriteByte()` - 寫入單個字節
- `AT24C256_IsReady()` - 檢查設備就緒狀態
- `AT24C256_Erase()` - 擦除所有數據 (填充為0xFF)

### 3. at24c256_example.c
包含兩個使用示例：
- `AT24C256_Example()` - 基本讀寫操作演示
- `AT24C256_ConfigExample()` - 配置數據存儲示例

## 硬件連接

```
AT24C256          STM32H7
─────────         ──────────
VCC        ──→    3.3V
GND        ──→    GND
SCL (pin6) ──→    I2C1_SCL (PB8)
SDA (pin5) ──→    I2C1_SDA (PB9)
A0  (pin1) ──→    GND (地址位0)
A1  (pin2) ──→    GND (地址位1)
A2  (pin3) ──→    GND (地址位2)
WP  (pin7) ──→    GND (寫保護，低電平允許寫入)
```

## I2C 地址設置

根據A2, A1, A0引腳的配置，可以有8個不同的地址：

| A2 | A1 | A0 | I2C 地址 | 常量名 |
|----|----|----|----------|--------|
| 0  | 0  | 0  | 0xA0    | AT24C256_I2C_ADDR_0 |
| 0  | 0  | 1  | 0xA2    | AT24C256_I2C_ADDR_1 |
| 0  | 1  | 0  | 0xA4    | AT24C256_I2C_ADDR_2 |
| 0  | 1  | 1  | 0xA6    | AT24C256_I2C_ADDR_3 |
| 1  | 0  | 0  | 0xA8    | AT24C256_I2C_ADDR_4 |
| 1  | 0  | 1  | 0xAA    | AT24C256_I2C_ADDR_5 |
| 1  | 1  | 0  | 0xAC    | AT24C256_I2C_ADDR_6 |
| 1  | 1  | 1  | 0xAE    | AT24C256_I2C_ADDR_7 |

## 使用方法

### 基本初始化

```c
#include "at24c256.h"

AT24C256_t eeprom;

// 初始化EEPROM驅動
if (AT24C256_Init(&eeprom, &hi2c1, AT24C256_I2C_ADDR_0) != HAL_OK)
{
    printf("EEPROM初始化失敗\r\n");
    return;
}
```

### 寫入數據

```c
// 寫入單個字節
uint8_t data = 0xAA;
AT24C256_WriteByte(&eeprom, 0x0000, data);

// 寫入多個字節
uint8_t buffer[] = "Hello EEPROM";
AT24C256_Write(&eeprom, 0x0100, buffer, sizeof(buffer));
```

### 讀取數據

```c
// 讀取單個字節
uint8_t data;
AT24C256_ReadByte(&eeprom, 0x0000, &data);

// 讀取多個字節
uint8_t buffer[64];
AT24C256_Read(&eeprom, 0x0100, buffer, 64);
```

### 檢查設備狀態

```c
//等待設備就緒 (通常在寫入後使用)
if (AT24C256_IsReady(&eeprom, 100) == HAL_OK)
{
    printf("EEPROM就緒\r\n");
}
```

### 擦除所有數據

```c
// 填充整個EEPROM為0xFF
if (AT24C256_Erase(&eeprom) == HAL_OK)
{
    printf("EEPROM已擦除\r\n");
}
```

## 重要注意事項

1. **頁面邊界**: AT24C256的頁面大小為64字節。驅動程序會自動處理跨頁寫入，但建議單次寫入不超過64字節。

2. **寫入時序**: 寫入每個頁面後，設備需要約5ms的時間完成寫入操作。驅動程序通過`AT24C256_IsReady()`進行輪詢等待。

3. **地址範圍**: EEPROM地址範圍為0x0000~0x7FFF (0-32767)。

4. **寫保護**: 將WP引腳接地以允許寫入，或接VCC以保護EEPROM。

5. **I2C時序**: 確保I2C時鐘頻率不超過400kHz。

## 集成到項目

1. 在main.c中添加包含：
```c
#include "at24c256.h"
```

2. 在main()函數中初始化：
```c
AT24C256_t eeprom;
MX_I2C1_Init();  // 初始化I2C1
AT24C256_Init(&eeprom, &hi2c1, AT24C256_I2C_ADDR_0);
```

3. 調用驅動函數執行讀寫操作。

## 示例代碼執行

在main.c的main()函數中調用示例代碼：

```c
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MPU_Config();
    
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_USART1_UART_Init();
    
    printf("Hello UART1\r\n");
    
    // 初始化EEPROM
    AT24C256_t eeprom;
    AT24C256_Init(&eeprom, &hi2c1, AT24C256_I2C_ADDR_0);
    
    // 執行示例
    AT24C256_Example();
    // AT24C256_ConfigExample();
    
    while (1)
    {
        // 主循環
    }
}
```

## 調試和验证

使用串口監視器（波特率: 115200）觀察程序輸出，驗證讀寫操作是否成功。

## 常見問題

### Q: 設備無法初始化
**A:** 檢查I2C引腳連接、上拉電阻，確保A0/A1/A2引腳的地址設置與代碼中的地址一致。

### Q: 讀取和寫入數據不匹配
**A:** 確保WP引腳已接地以允許寫入，檢查I2C時序和時鐘頻率。

### Q: 寫入速度較慢
**A:** 這是正常的，因為每次頁面寫入後需要等待5ms的寫入完成時間。

## 許可證
MIT License

## 作者
STM32 I2C EEPROM Driver
