#include "kth_read.hpp"


bool KTH7111::KTH7111_Init(bool fx)
{
    bool write_ok = false;
    uint16_t angle_zero = 0;
    encoder_dir = fx;
    write_ok =KTH7111_RotationDirection(fx); //写入编码器角度方向
    // KTH7111_WriteReg(0x00, 0x00); //设置零点
    // KTH7111_WriteReg(0x01, 0x00); //设置零点 32786
    KTH7111_WriteReg(0x0d, 0x88); //设置滤波深度

    KTH7111_BurnMTP(); //写入MTP 
    return write_ok;
}

void KTH7111::KTH_CS_Enble(bool status)
{
    HAL_GPIO_WritePin(_cs_port, _cs_pin, status ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void KTH7111::SPI_Set_MOSI_Output()
{
    __HAL_SPI_DISABLE(_hspi);
    SET_BIT(_hspi->Instance->CR1, SPI_CR1_BIDIOE);  // MOSI 输出（主机发送）
    __HAL_SPI_ENABLE(_hspi);
}

void KTH7111::SPI_Set_MOSI_Input()
{
    __HAL_SPI_DISABLE(_hspi);
    CLEAR_BIT(_hspi->Instance->CR1, SPI_CR1_BIDIOE);// MOSI 输入（主机接收）
    __HAL_SPI_ENABLE(_hspi); 
}



/**
 * @brief CRC8/ITU 校验
 * 多项式：x8+x2+x+1 (0x07)
 * 初始值：0x00
 * 异或值：0x55
 */
uint8_t KTH7111::CRC8_ITU(const uint8_t *data, uint8_t len)
{
    uint8_t crc = 0x00;  /* 初始值 */
    for (uint8_t i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x07;  /* 多项式 */
            else
                crc = crc << 1;
        }
    }
    crc ^= 0x55;
    return crc;
}


void KTH7111::KTH7111_Send_JIAO_CMD() //读取角度
{
    HAL_StatusTypeDef status = HAL_ERROR;
    uint8_t cmd = 0x00;   //发送命令：8bit（0x00）
    uint8_t rx_buff[3] = {0}; /* 收 2字节角度 + 1字节CRC */
    uint8_t crc = 0;
    
    KTH_CS_Enble(false);/* 片选拉低 */
    status = HAL_SPI_Transmit(_hspi,&cmd, 1, 1);//发送命令：8bit（0x00）
    if(status != HAL_OK) return;

    SPI_Set_MOSI_Input();//MOSI变为MISO

    status = HAL_SPI_Receive(_hspi,rx_buff, 3, 1); //接收数据：2字节角度 + 1字节CRC
    if(status != HAL_OK) return;

    SPI_Set_MOSI_Output();//MISO 变为MOSI

    KTH_CS_Enble(true);/* 片选拉高 */
    crc = CRC8_ITU(rx_buff, 2); //校验CRC
    if(crc != rx_buff[2]) return; //CRC校验未通过
    single_turn = (uint16_t)(rx_buff[0] << 8) | (uint16_t)rx_buff[1]; //转成16位数据
    // if(encoder_dir == false) single_turn = KTH7111_PRECISION - single_turn; //角度方向取反
    theta = single_turn*RAW_TO_RAD;//转成弧度
    angle = theta*RAD_TO_DEG;//转成角度
}


/**
 * @brief 获取KTH7111传感器的弧度值
 * @return float 返回当前存储的弧度值theta
 */
float KTH7111::Get_KTH7111_Radian(void) //获取弧度
{
    return theta;  // 返回成员变量theta的值，该值存储了传感器的弧度测量值
}

float KTH7111::Get_KTH7111_Degree(void) //获取角度
{
    return angle;
}

/* ---------------------- 读寄存器 ---------------------- */
uint8_t KTH7111::KTH7111_ReadReg(uint8_t addr)
{
    uint8_t cmd[2] = {0x11, addr};  /*发送命令：8bit 0x11 + 8bit 寄存器地址*/
    uint8_t rx_buff[2] = {0}; /* 收 8bit 寄存器值 + 8bitCRC */
    uint8_t crc = 0; 
    HAL_StatusTypeDef status = HAL_ERROR;

    KTH_CS_Enble(false);
    status = HAL_SPI_Transmit(_hspi,cmd, 2, 10);//发送命令：8bit 0x11 + 8bit 寄存器地址
    if(status != HAL_OK) 
    {
        KTH_CS_Enble(true);
        return 0;
    }

    SPI_Set_MOSI_Input();//MOSI变为MISO

    for(volatile uint32_t i = 0; i < 28; i++) { __NOP(); } //延时1us
    status = HAL_SPI_Receive(_hspi,rx_buff, 2, 10); //接收数据：8bit 寄存器值 + 8bitCRC
    if(status != HAL_OK)
    {
        KTH_CS_Enble(true);
        SPI_Set_MOSI_Output();//MISO 变为MOSI
        return 0;
    }
    for(volatile uint32_t i = 0; i < 28; i++) { __NOP(); } //延时1us
    SPI_Set_MOSI_Output();//MISO 变为MOSI
    KTH_CS_Enble(true);

    

    crc = CRC8_ITU(rx_buff, 1); //校验CRC
    if(crc != rx_buff[1]) return 0; //CRC校验未通过 
    return rx_buff[0]; //返回寄存器值 
}



/* ---------------------- 写寄存器 ---------------------- */
bool KTH7111::KTH7111_WriteReg(uint8_t addr, uint8_t data)
{
    uint8_t unlock[4] = {0x20, 0x24, 0x01, 0x01};  /* 解锁密码 0x20240101 */
    uint8_t cmd[3] = {0x33, addr, data};            /* 命令：0x33 + addr + data */
    uint8_t rx = 0;

    KTH_CS_Enble(false);
    /* 第一帧：解锁 */
    HAL_SPI_Transmit(_hspi, unlock, 4, 10);
    KTH_CS_Enble(true);

    KTH_CS_Enble(false);
    for(volatile uint32_t i = 0; i < 28; i++) { __NOP(); } //延时1us
    /* 第二帧：发写命令 */
    HAL_SPI_Transmit(_hspi, cmd, 3, 10);
    for(volatile uint32_t i = 0; i < 28; i++) { __NOP(); } //延时1us

    SPI_Set_MOSI_Input();//MOSI变为MISO
    /* 第三帧：收确认（同一帧返回的寄存器值） */
    HAL_SPI_Receive(_hspi, &rx, 1, 10);
    KTH_CS_Enble(true);
    for(volatile uint32_t i = 0; i < 28; i++) { __NOP(); } //延时1us
    SPI_Set_MOSI_Output();//MISO 变为MOSI
    
    
    return rx == data;
}



/**
 * @brief 控制编码器旋转方向
 * @param RD 旋转方向参数，true为顺时针，false为逆时针
 */
bool KTH7111::KTH7111_RotationDirection(bool RD)  //控制编码器旋转方向 返回是否修改成功
{
    //改编码器测角度 倒着装 
    uint8_t rx_temp = 0;
    uint8_t rx_new = 0;    
    uint8_t tx = 0;
    if(RD == false)  //如果RD为1，设置为顺时针方向
    {
        rx_temp = KTH7111_ReadReg(0x02); 
        tx = rx_temp|0x80;//最高位置1
        KTH7111_WriteReg(0x02,tx);
        rx_new = KTH7111_ReadReg(0x02);
        if(rx_new&0x80) return true;
        else return false;
    }else
    {
        rx_temp = KTH7111_ReadReg(0x02);
        tx = rx_temp&(~0x80);//最高位清零
        KTH7111_WriteReg(0x02,tx);
        rx_new = KTH7111_ReadReg(0x02);
        if(rx_new&0x80) return false;
        else return true;
    }
}



HAL_StatusTypeDef KTH7111::KTH7111_BurnMTP(void)//保存当前参数到MTP
{
    uint8_t burn_cmd[3] = {0x22, 0x55, 0xAA}; //
    HAL_StatusTypeDef status = HAL_ERROR;
    SPI_Set_MOSI_Input();//MOSI变为MISO
    KTH_CS_Enble(false);
    status = HAL_SPI_Transmit(_hspi, burn_cmd, 3, 10);
    KTH_CS_Enble(true);
    vTaskDelay(pdMS_TO_TICKS(500));//两次保存400ms以上
    return status;
}



// uint8_t add[]= {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0d,0x10,0x11,0x12,0x16,0x72};
// uint8_t data_tx[]= {0x00,0x00,0x00,0xff,0x03,0x00,0x07,0x00,0x00,0xa5,0xa5,0x88,0x00,0xaC,0x02,0x08,0x00};

// uint8_t len = sizeof(data_tx)/sizeof(data_tx[0]);
// uint8_t data_rx[20];


/* ---------------------- 读多个寄存器 ---------------------- */
void KTH7111::KTH7111_Read_MoreRegs(uint8_t *add, uint8_t *data_rx, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++)
    {
        data_rx[i] = KTH7111_ReadReg(add[i]);
    }
}

/* ---------------------- 写多个寄存器 是否保存---------------------- */
void KTH7111::KTH7111_Write_MoreRegs(uint8_t *add, uint8_t *data_tx, uint8_t len,bool save)
{
    for (uint8_t i = 0; i < len; i++)
    {
        KTH7111_WriteReg(add[i], data_tx[i]);
    }
    if (save == true)
    {
        KTH7111_BurnMTP();
    }
    
}


/* ---------------------- 寄存器方式校准编码器 ---------------------- */
uint8_t KTH7111::KTH7111_Get_ANLC_Status(void) //获取校准状态 0：未校准 1：校准中 2：校准失败 3：校准完成
{
    return (KTH7111_ReadReg(0x72) >> 4) & 0x03;
}


bool KTH7111::KTH7111_ANLC_Calibration(uint32_t timeout_s)
{
    // foc1.work_mode = open1_Mode; //设置工作模式为开环模式
    
    uint8_t reg16 = KTH7111_ReadReg(0x16) | 0x08; //自校准效果使能到角度输出
    KTH7111_WriteReg(0x16, reg16);
    vTaskDelay(pdMS_TO_TICKS(5));
    
    reg16 |= 0x10; // 将 REG_CAL 寄存器设置为 1，启动非线性校准
    KTH7111_WriteReg(0x16, reg16);
    
    uint32_t start = xTaskGetTickCount();
    uint8_t status;
    do {
        vTaskDelay(pdMS_TO_TICKS(10));
        status = KTH7111_Get_ANLC_Status();
        if (status == 2 || (xTaskGetTickCount() - start) > pdMS_TO_TICKS(timeout_s*1000)) {
            reg16 &= ~0x10; //将 REG_CAL 寄存器设置为 0，停止非线性校准
            KTH7111_WriteReg(0x16, reg16);
            return false;
        }
    } while (status == 1);//校准中  如果status等于0也会跳出来 ?????
    
    reg16 &= ~0x10;//将 REG_CAL 寄存器设置为 0，停止非线性校准
    KTH7111_WriteReg(0x16, reg16);
    KTH7111_BurnMTP(); //保存校准参数到MTP
    return true;
}


void KTH7111::SetZeroAngle_RuanJian(float zero) //设置软件零点
{
    //待做 ????
}


bool KTH7111::KTH7111_SetNowZero(void)  //设置当前位置为零点 返回是否修改成功
{
    uint8_t rx_temp = 0;
    uint8_t rx_new = 0;    
    uint8_t tx = 0;
    rx_temp = KTH7111_ReadReg(0x02); //读寄存器值 1000 0000
    tx = rx_temp|0x40;//AUTO_ZERO_SET 置1
    KTH7111_WriteReg(0x02,tx); //写寄存器值 1100 0000
    vTaskDelay(pdMS_TO_TICKS(50));//等待零点写入
    rx_new = KTH7111_ReadReg(0x02);//读寄存器值 1010 0100
    if(rx_new&0x40) return true;
    else return false;

}



