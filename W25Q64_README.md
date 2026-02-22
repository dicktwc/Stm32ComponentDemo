# W25Q64 SPI Flash 驅動程式使用指南

## 概述
W25Q64 是一款 64Mbit (8MB) 的 SPI Serial Flash 記憶體，廣泛用於嵌入式系統的數據存儲。

## 硬件連接

### SPI 引腳連接（以 STM32H7 為例）
- **CLK** (時鐘) - SPI1_SCK
- **MOSI** (主輸出從輸入) - SPI1_MOSI  
- **MISO** (主輸入從輸出) - SPI1_MISO
- **CS** (片選) - 任意 GPIO (如 PA4)
- **VCC** - 3.3V
- **GND** - 地

### 推薦的上拉/下拉
- CS 應拉高 (閒置時為高電平) - 通過軟件 `HAL_GPIO_WritePin()` 控制
- CLK、MOSI、MISO 線通常有內部上拉

## 軟件使用

### 1. 初始化 SPI 和 GPIO

首先，在 STM32CubeMX 中配置：
- SPI1 為 Master 模式，時鐘頻率推薦 10-50 MHz
- GPIO (如 PA4) 為輸出，初始狀態為 HIGH

在 main.c 中：
```c
#include "w25q64.h"

W25Q64_t w25q64_dev;

int main(void) {
    // ... 其他初始化 ...
    
    // 初始化 W25Q64
    HAL_StatusTypeDef status = W25Q64_Init(&w25q64_dev, &hspi1, GPIOA, GPIO_PIN_4);
    
    if (status == HAL_OK) {
        printf("W25Q64 initialized successfully\n");
    }
    
    // ... 主程序 ...
}
```

### 2. 主要函數使用

#### 讀取 JEDEC ID
```c
uint8_t id[3];
W25Q64_ReadID(&w25q64_dev, id);
printf("Manufacturer: 0x%02X, Memory Type: 0x%02X, Capacity: 0x%02X\n", 
       id[0], id[1], id[2]);
```

#### 讀取數據
```c
uint8_t buffer[256];
W25Q64_Read(&w25q64_dev, 0x000000, buffer, 256);
```

#### 快速讀取（帶虛擬字節）
```c
uint8_t buffer[256];
W25Q64_FastRead(&w25q64_dev, 0x000000, buffer, 256);
```

#### 擦除扇區 (4KB)
```c
W25Q64_EraseSector(&w25q64_dev, 0x000000);
```

#### 寫入頁面 (最多 256 字節)
```c
uint8_t data[256];
// ... 填充 data ...
W25Q64_PageProgram(&w25q64_dev, 0x000000, data, 256);
```

#### 通用寫入（自動擦除和分頁處理）
```c
uint8_t data[1024];
// ... 填充 data ...
W25Q64_Write(&w25q64_dev, 0x000000, data, 1024);  // 自動處理跨頁寫入
```

#### 擦除 64KB 塊
```c
W25Q64_EraseBlock64(&w25q64_dev, 0x000000);
```

#### 全片擦除
```c
W25Q64_ChipErase(&w25q64_dev);
```

### 3. 运行示例
在 main.c 的主程序中調用：
```c
#include "w25q64_example.h"

int main(void) {
    // ... 初始化 ...
    W25Q64_Example(&hspi1, GPIOA, GPIO_PIN_4);
    
    while (1) {
        // ...
    }
}
```

## 設備參數

| 參數 | 值 |
|------|-----|
| 總容量 | 8 MB (8388608 字節) |
| 頁大小 | 256 字節 |
| 扇區大小 | 4 KB (最小可擦除單位) |
| 塊大小 | 64 KB |
| 總扇區數 | 2048 |
| JEDEC ID | 0xEF6017 |

## SPI 命令列表

| 命令 | 代碼 | 描述 |
|------|------|------|
| Read Data | 0x03 | 讀取數據 |
| Fast Read | 0x0B | 快速讀取（需要虛擬字節） |
| Page Program | 0x02 | 寫入一頁（最多 256 字節） |
| Sector Erase | 0x20 | 擦除 4KB 扇區 |
| Block Erase (64K) | 0xD8 | 擦除 64KB 塊 |
| Chip Erase | 0xC7 | 全片擦除 |
| Write Enable | 0x06 | 啟用寫入 |
| Write Disable | 0x04 | 禁用寫入 |
| Read Status | 0x05 | 讀取狀態寄存器 |
| Read JEDEC ID | 0x9F | 讀取製造商 ID 和芯片類型 |

## 重要注意事項

1. **寫入前需要擦除**：Flash 記憶體只能從 1 寫入到 0。要改變 0 回到 1，必須先擦除扇區。

2. **忙狀態檢查**：寫入和擦除操作後設備會進入忙狀態，使用 `W25Q64_WaitReady()` 等待完成。

3. **每頁最多 256 字節**：`W25Q64_PageProgram()` 單次最多寫入 256 字節。跨頁寫入需要多次調用或使用 `W25Q64_Write()`。

4. **SPI 時鐘頻率**：推薦 10-50 MHz。更高的頻率可能導致讀取錯誤。

5. **寫入時間**：
   - 頁編程：約 3ms
   - 扇區擦除：約 400ms
   - 塊擦除：約 2s
   - 全片擦除：約 200s

## 讀寫流程圖

### 讀取流程
```
開始
  ↓
檢查忙狀態 ← 是
  ↓ 否
發送讀命令和地址
  ↓
接收數據
  ↓
結束 CS
  ↓
完成
```

### 寫入流程
```
開始
  ↓
檢查忙狀態 ← 是
  ↓ 否
擦除扇區
  ↓
等待擦除完成
  ↓
寫入啟用
  ↓
發送寫命令和地址
  ↓
發送數據
  ↓
結束 CS
  ↓
等待寫入完成
  ↓
完成
```

## 故障排除

### 無法讀取 ID
- 檢查 SPI 連接
- 確保 CS 腳正確連接並配置為輸出
- 驗證 SPI 時鐘速度不過高
- 檢查電源連接

### 寫入失敗
- 確保事前已擦除扇區
- 檢查寫地址是否有效 (< 0x800000)
- 驗證寫入數據大小不超過頁大小（256 字節）

### 數據讀取不匹配
- 檢查是否正確擦除和寫入
- 減低 SPI 時鐘頻率並重試
- 檢查傳輸線是否符合信號完整性要求

## 文件結構

- `w25q64.h` - 驅動頭檔，包含 API 定義
- `w25q64.c` - 驅動源碼實現
- `w25q64_example.c` - 使用示例和測試代碼

## 許可證

驅動程式基於 STM32 HAL 庫編寫，遵循相同的許可條款。
