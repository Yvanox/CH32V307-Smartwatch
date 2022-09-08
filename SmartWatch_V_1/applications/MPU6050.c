/* Port of the MPU6050 example code of the CH32V307
 * Visit: https://gitee.com/verimaker/opench-ch32v307 for more info*/

#include "MPU6050.h"
#include "debug.h"

/* I2C Initialization */
void I2C_Accelerometer_Init(uint32_t bound, uint16_t address)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    I2C_InitTypeDef  I2C_InitTSturcture;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB /*| RCC_APB2Periph_AFIO*/, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    I2C_InitTSturcture.I2C_ClockSpeed = bound;
    I2C_InitTSturcture.I2C_Mode = I2C_Mode_I2C;
    I2C_InitTSturcture.I2C_DutyCycle = I2C_DutyCycle_16_9;
    I2C_InitTSturcture.I2C_OwnAddress1 = address;
    I2C_InitTSturcture.I2C_Ack = I2C_Ack_Enable;
    I2C_InitTSturcture.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C2, &I2C_InitTSturcture);

    I2C_Cmd(I2C2, ENABLE);

    I2C_AcknowledgeConfig(I2C2, ENABLE);
}


/* MPU 6050 Init commands */
u8 MPU_Init(void)
{
    u8 res;

    I2C_Accelerometer_Init(100000,0x02);
    rt_thread_mdelay(200);
    MPU_Write_Byte(MPU_PWR_MGMT1_REG,0X00);
    MPU_Write_Byte(MPU_SAMPLE_RATE_REG, 0x07);
    MPU_Write_Byte(MPU_CFG_REG,0x06);
    MPU_Write_Byte(MPU_ACCEL_CFG_REG,0x00);
    MPU_Write_Byte(MPU_GYRO_CFG_REG,0x00);

    MPU_Write_Byte(MPU_INT_EN_REG,0X00);
    MPU_Write_Byte(MPU_USER_CTRL_REG,0X00);
    MPU_Write_Byte(MPU_FIFO_EN_REG,0X00);
    MPU_Write_Byte(MPU_INTBP_CFG_REG,0X80);
    res=MPU_Read_Byte(MPU_DEVICE_ID_REG);
    if(res==MPU_ADDR)
    {
        MPU_Write_Byte(MPU_PWR_MGMT1_REG,0X01);
        MPU_Write_Byte(MPU_PWR_MGMT2_REG,0X00);
        MPU_Write_Byte(MPU_SAMPLE_RATE_REG, 0x07);
    }else return 1;
    return 0;
}

u8 MPU_Set_Gyro_Fsr(u8 fsr)
{
    return MPU_Write_Byte(MPU_GYRO_CFG_REG,fsr<<3);
}

u8 MPU_Set_Accel_Fsr(u8 fsr)
{
    return MPU_Write_Byte(MPU_ACCEL_CFG_REG,fsr<<3);
}

u8 MPU_Set_LPF(u16 lpf)
{
    u8 data=0;
    if(lpf>=188)data=1;
    else if(lpf>=98)data=2;
    else if(lpf>=42)data=3;
    else if(lpf>=20)data=4;
    else if(lpf>=10)data=5;
    else data=6;
    return MPU_Write_Byte(MPU_CFG_REG,data);
}

u8 MPU_Set_Rate(u16 rate)
{
    u8 data;
    if(rate>1000)rate=1000;
    if(rate<4)rate=4;
    data=1000/rate-1;
    data=MPU_Write_Byte(MPU_SAMPLE_RATE_REG,data);
    return MPU_Set_LPF(rate/2);
}

/* NOT USED */
short MPU_Get_Temperature(void)
{
    u8 buf[2];
    u16 raw;
    float temp;
    MPU_Read_Len(MPU_ADDR,MPU_TEMP_OUTH_REG,2,buf);
    raw=((u16)buf[0]<<8)|buf[1];
    temp=36.53+((double)raw)/340;
    return temp*100;;
}

/* NOT USED */
u8 MPU_Get_Gyroscope(u16 *gx,u16 *gy,u16 *gz)
{
    u8 buf[6],res;
    res=MPU_Read_Len(MPU_ADDR,MPU_GYRO_XOUTH_REG,6,buf);
    if(res==0)
    {
        *gx=((u16)buf[0]<<8)|buf[1];
        *gy=((u16)buf[2]<<8)|buf[3];
        *gz=((u16)buf[4]<<8)|buf[5];
    }
    return res;;
}

u8 MPU_Get_Accelerometer(u16 *ax,u16 *ay,u16 *az)
{
    u8 buf[6],res;
    res=MPU_Read_Len(MPU_ADDR,MPU_ACCEL_XOUTH_REG,6,buf);
    if(res==0)
    {
        *ax=((u16)buf[0]<<8)|buf[1];
        *ay=((u16)buf[2]<<8)|buf[3];
        *az=((u16)buf[4]<<8)|buf[5];
    }
    return res;;
}

u8 MPU_Write_Len(u8 addr,u8 reg,u8 len,u8 *buf)
{
    u8 i=0;

    I2C_AcknowledgeConfig( I2C2, ENABLE );


    I2C_GenerateSTART( I2C2, ENABLE );

    while( !I2C_CheckEvent( I2C2, I2C_EVENT_MASTER_MODE_SELECT ) );
    I2C_Send7bitAddress(I2C2,((addr << 1) | 0),I2C_Direction_Transmitter);

    while( !I2C_CheckEvent( I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ) );

    while( I2C_GetFlagStatus( I2C2, I2C_FLAG_TXE ) ==  RESET );
    I2C_SendData(I2C2,reg);


    while(i < len)
    {
        if( I2C_GetFlagStatus( I2C2, I2C_FLAG_TXE ) !=  RESET )
            {
                I2C_SendData(I2C2,buf[i]);
                i++;
            }
    }
//    while( !I2C_CheckEvent( I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED ) );
    while( I2C_GetFlagStatus( I2C2, I2C_FLAG_TXE ) ==  RESET );

    I2C_GenerateSTOP( I2C2, ENABLE );

    return 0;
}

u8 MPU_Read_Len(u8 addr,u8 reg,u8 len,u8 *buf)
{
    u8 i=0;

    I2C_AcknowledgeConfig( I2C2, ENABLE );

    I2C_GenerateSTART( I2C2, ENABLE );

    while( !I2C_CheckEvent( I2C2, I2C_EVENT_MASTER_MODE_SELECT ) ) ;
    I2C_Send7bitAddress(I2C2,(addr << 1) | 0X00,I2C_Direction_Transmitter);

    while( !I2C_CheckEvent( I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ) );

    I2C_SendData(I2C2,reg);


    I2C_GenerateSTART( I2C2, ENABLE );
    while( !I2C_CheckEvent( I2C2, I2C_EVENT_MASTER_MODE_SELECT ) );

    I2C_Send7bitAddress(I2C2,((addr << 1) | 0x01),I2C_Direction_Receiver);
    while( !I2C_CheckEvent( I2C2, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED ) );

    while(i < len)
    {
        if( I2C_GetFlagStatus( I2C2, I2C_FLAG_RXNE ) !=  RESET )
        {
            if(i == (len - 2))
            {
                I2C_AcknowledgeConfig( I2C2, DISABLE );
                buf[i] = I2C_ReceiveData(I2C2);

            }
            else
            {
                buf[i] = I2C_ReceiveData(I2C2);
            }
            i++;
        }
    }

    I2C_GenerateSTOP( I2C2, ENABLE );

    return 0;
}

u8 MPU_Write_Byte(u8 reg,u8 data)
{

    I2C_AcknowledgeConfig( I2C2, ENABLE );

    I2C_GenerateSTART( I2C2, ENABLE );

    while( !I2C_CheckEvent( I2C2, I2C_EVENT_MASTER_MODE_SELECT ) );
    I2C_Send7bitAddress(I2C2,((MPU_ADDR << 1) | 0),I2C_Direction_Transmitter);

    while( !I2C_CheckEvent( I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ) );

    while( I2C_GetFlagStatus( I2C2, I2C_FLAG_TXE ) ==  RESET );
    I2C_SendData(I2C2,reg);

    while( !I2C_CheckEvent( I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED ) );

    while( I2C_GetFlagStatus( I2C2, I2C_FLAG_TXE ) ==  RESET );
    I2C_SendData(I2C2,data);

    I2C_GenerateSTOP( I2C2, ENABLE );

    return 0;
}

u8 MPU_Read_Byte(u8 reg)
{
    u8 res;

    I2C_AcknowledgeConfig( I2C2, ENABLE );

    I2C_GenerateSTART( I2C2, ENABLE );

    while( !I2C_CheckEvent( I2C2, I2C_EVENT_MASTER_MODE_SELECT ) );
    I2C_Send7bitAddress(I2C2,(MPU_ADDR << 1) | 0X00,I2C_Direction_Transmitter);

    while( !I2C_CheckEvent( I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ) );
    I2C_SendData(I2C2,reg);

    while( !I2C_CheckEvent( I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED ) );

    I2C_GenerateSTART( I2C2, ENABLE );
    while( !I2C_CheckEvent( I2C2, I2C_EVENT_MASTER_MODE_SELECT ) );

    I2C_Send7bitAddress(I2C2,((MPU_ADDR << 1) | 0x01),I2C_Direction_Receiver);
    while( !I2C_CheckEvent( I2C2, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED ) );

    while(1)
    {
        if( I2C_GetFlagStatus( I2C2, I2C_FLAG_RXNE ) !=  RESET )
        {
            res = I2C_ReceiveData( I2C2 );
            break;
        }
    }

      I2C_GenerateSTOP( I2C2, ENABLE );

    return res;
}
