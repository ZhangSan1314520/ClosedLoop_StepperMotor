#include "BL24C16F.hpp"

I2C_HandleTypeDef* BL24C16F::_hi2c = nullptr;

// ============================================================
// 内部辅助：获取器件地址（含 block select）
// BL24C16F 的 2048 字节分为 8 个 block，每个 block 256 字节
// 器件地址 = 0x50 | (addr >> 8)，即第 8~10 位选择 block
// ============================================================
uint8_t BL24C16F::_dev_addr(uint16_t addr)
{
    // addr >> 8 = 0~7，取低 3 位作为 block 编号
    // 器件地址 = 基址(0x50) + block
    return DEV_ADDR | ((addr >> 8) & 0x07);
}


// ============================================================
// 内部辅助：获取页内偏移
// 每个 block 256 字节，偏移 = addr 的低 8 位
// ============================================================
uint8_t BL24C16F::_page_offset(uint16_t addr)
{
    return (uint8_t)(addr & 0xFF);
}


// ============================================================
// 初始化：探测 EEPROM 是否在线
// 发送器件地址并等待 ACK，超时则判定设备不存在
// ============================================================
bool BL24C16F::init(void)
{
    // 以写方式探测设备：发送起始位 + 器件地址(写) + 等待 ACK
    // HAL_I2C_IsDeviceReady: 尝试 3 次，每次间隔 100ms
    if (HAL_I2C_IsDeviceReady(_hi2c,
                              DEV_ADDR << 1,    // 8位地址(7位左移1位)
                              3,                // 重试次数
                              100) == HAL_OK)   // 超时时间(ms)
    {
        return true;    // 设备在线
    }

    return false;       // 设备未响应
}


// ============================================================
// 读取一个字节
// ============================================================
uint8_t BL24C16F::read_byte(uint16_t addr)
{
    uint8_t data = 0xFF;    // 默认值（读取失败返回 0xFF）

    // 获取本地址对应的器件地址和页内偏移
    uint8_t dev_addr = _dev_addr(addr) << 1;    // 7位 → 8位
    uint8_t offset   = _page_offset(addr);

    // 使用 HAL_I2C_Mem_Read：先发器件地址(写)写偏移，再发器件地址(读)读数据
    // MemAddress 为页内偏移 (8位)
    // MemAddSize 为 8 位地址长度
    if (HAL_I2C_Mem_Read(_hi2c, dev_addr, offset,
                         I2C_MEMADD_SIZE_8BIT,
                         &data, 1,
                         100) != HAL_OK)
    {
        return 0xFF;     // 读取失败返回 0xFF
    }

    return data;
}


// ============================================================
// 写入一个字节
// ============================================================
bool BL24C16F::write_byte(uint16_t addr, uint8_t data)
{
    // 直接调用页写入函数写 1 个字节即可
    uint8_t dev_addr = _dev_addr(addr) << 1;
    uint8_t offset   = _page_offset(addr);

    if (HAL_I2C_Mem_Write(_hi2c, dev_addr, offset,
                          I2C_MEMADD_SIZE_8BIT,
                          &data, 1,
                          100) != HAL_OK)
    {
        return false;    // 写入失败
    }

    // 等待内部写入完成（~5ms EEPROM 擦写时间）
    _wait_write_complete();

    return true;
}


// ============================================================
// 连续读取多个字节（无页边界限制，EEPROM 支持连续读）
// ============================================================
bool BL24C16F::read(uint16_t addr, uint8_t *buf, uint16_t len)
{
    // 参数检查
    if (buf == NULL || len == 0)
        return false;
    if (addr + len > EEPROM_SIZE)
        return false;

    uint8_t dev_addr = _dev_addr(addr) << 1;    // 起始地址的器件地址
    uint8_t offset   = _page_offset(addr);       // 起始地址的页内偏移

    // 连续读：从起始地址开始，逐个递增地址读取
    // BL24C16F 在读取到当前 block 末尾时会自动回卷到该 block 起始
    // 注意：跨 block 时 (offset + len > 256) 需要分段读取
    while (len > 0)
    {
        // 计算当前 block 内剩余可读字节数
        uint16_t chunk = 256 - offset;
        if (chunk > len) chunk = len;

        // 从当前地址读取 chunk 个字节
        if (HAL_I2C_Mem_Read(_hi2c, dev_addr, offset,
                             I2C_MEMADD_SIZE_8BIT,
                             buf, chunk,
                             100) != HAL_OK)
        {
            return false;    // 读取失败
        }

        // 更新指针和计数器
        addr  += chunk;
        buf   += chunk;
        len   -= chunk;

        // 如果还有剩余数据需要读取，切换到下一个 block
        if (len > 0)
        {
            dev_addr = _dev_addr(addr) << 1;    // 下一个 block 的器件地址
            offset   = _page_offset(addr);        // 下一个 block 的页内偏移
        }
    }

    return true;
}


// ============================================================
// 连续写入多个字节（自动处理页边界 = 16 字节/页）
// ============================================================
bool BL24C16F::write(uint16_t addr, const uint8_t *buf, uint16_t len)
{
    // 参数检查
    if (buf == NULL || len == 0)
        return false;
    if (addr + len > EEPROM_SIZE)
        return false;

    while (len > 0)
    {
        // 计算当前页内的写入位置
        uint8_t dev_addr = _dev_addr(addr) << 1;     // 当前 block 的器件地址
        uint8_t offset   = _page_offset(addr);        // 当前页内偏移

        // 计算当前页内一次最多能写入的字节数
        // 页大小 16 字节，从页内偏移 offset 开始，最多写 (16 - offset%16) 字节
        uint8_t page_remain = PAGE_SIZE - (offset % PAGE_SIZE);
        uint16_t chunk = (len < page_remain) ? len : page_remain;

        // 写入当前页的 chunk 个字节
        if (HAL_I2C_Mem_Write(_hi2c, dev_addr, offset,
                              I2C_MEMADD_SIZE_8BIT,
                              (uint8_t *)buf, chunk,    // const → 非 const 转换
                              100) != HAL_OK)
        {
            return false;    // 写入失败
        }

        // 等待内部写入完成
        _wait_write_complete();

        // 更新指针和计数器
        addr += chunk;
        buf  += chunk;
        len  -= chunk;
    }

    return true;
}


// ============================================================
// 等待 EEPROM 内部写入完成（ACK 轮询）
// EEPROM 在内部擦写期间不会响应 ACK
// 轮询器件地址直到收到 ACK，超时则放弃
// ============================================================
void BL24C16F::_wait_write_complete(void)
{
    // 轮询等待：发送器件地址(写)，如果收到 ACK 表示写入完成
    // 最多等待 50ms（EEPROM 典型写入时间 5ms，留足余量）
    uint32_t timeout_ms = 50;

    // 使用 HAL_I2C_IsDeviceReady 进行 ACK 轮询
    // 每 1ms 轮询一次
    while (HAL_I2C_IsDeviceReady(_hi2c,
                                 DEV_ADDR << 1,    // 8位器件地址(写)
                                 1,                // 每次尝试 1 次
                                 1) != HAL_OK)     // 每次超时 1ms
    {
        // 检查超时
        if (timeout_ms == 0)
            break;          // 超时，放弃等待

        timeout_ms--;
    }
}
