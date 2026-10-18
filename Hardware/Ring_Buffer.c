#include "stm32f10x.h"
#include "W25Q64.h"
#include "MY_ADC.h"
#include "MY_TIM.h"
#include "MPU6050.h"
#include "GPIO_SEX.h"
#include "Serial.h"


/* ===================== 宏定义 ===================== */
#define RING_BUF_SIZE   512     // 环形缓冲区总大小（2的整数次幂）
#define RING_MASK       (RING_BUF_SIZE - 1)
#define SAMPLE_SIZE     18      // 单次采样字节数：4字节ADC + 14字节MPU6050
#define PAGE_SIZE       256     // Flash单页大小
#define FLASH_TIMEOUT   30      // Flash操作超时（单位：1000ms，30秒超时）

// 临界区保护：关总中断，防止串口中断并发修改缓冲区
#define ENTER_CRITICAL()  __disable_irq()
#define EXIT_CRITICAL()   __enable_irq()

// Flash状态机状态
#define FLASH_IDLE       0   // 空闲
#define FLASH_ERASING    1   // 扇区擦除中
#define FLASH_WRITING    2   // 页写入中

/* ===================== 全局变量 ===================== */
uint8_t  ring_buffer[RING_BUF_SIZE];
volatile uint16_t write_idx = 0;
volatile uint16_t read_idx  = 0;
volatile uint16_t data_cnt  = 0;
volatile uint8_t  page_ready = 0;
uint32_t flash_addr = 0x000000;
volatile uint8_t record_flag = 0;

/* ===================== 内部静态变量 ===================== */
static uint8_t  flash_state = FLASH_IDLE;
static uint8_t  flash_page_buf[PAGE_SIZE];   // Flash页写入缓存
static uint16_t flash_timeout_cnt = 0;        // 超时计数器

extern volatile uint16_t adc_ldr;
extern volatile uint16_t adc_ntc;

/* ================================================================
 *                      100ms 采集任务
 * 功能：读取ADC、MPU6050数据，写入环形缓冲区
 * ================================================================ */
void Task_100ms_Collect(void)
{
    if(record_flag == 0) return;

    // 异常防护：数据量超过缓冲区总大小，强制复位，防止越界
    if(data_cnt >= RING_BUF_SIZE)
    {
        write_idx = 0;
        read_idx = 0;
        data_cnt = 0;
        page_ready = 0;
        return;
    }

    // 剩余空间不足，丢弃本次采样
    if(data_cnt > (RING_BUF_SIZE - SAMPLE_SIZE))
    {
        return;
    }

    uint8_t sample[SAMPLE_SIZE];
    uint8_t idx = 0;

    // 1. 装填ADC数据（光照+热敏，共4字节）
    sample[idx++] = (uint8_t)(adc_ldr >> 8);
    sample[idx++] = (uint8_t)(adc_ldr & 0xFF);
    sample[idx++] = (uint8_t)(adc_ntc >> 8);
    sample[idx++] = (uint8_t)(adc_ntc & 0xFF);

    // 2. 装填MPU6050六轴数据（共14字节）
    MPU6050_Data_t MPU;
    MPU6050_GETDATA(&MPU);
    uint8_t *ptr = (uint8_t *)&MPU;
    for(uint8_t i = 0; i < 14; i++)
    {
        sample[idx++] = ptr[i];
    }

    // 3. 写入环形缓冲区（临界区保护）
    ENTER_CRITICAL();
    for(uint8_t i = 0; i < SAMPLE_SIZE; i++)
    {
        ring_buffer[write_idx] = sample[i];
        write_idx = (write_idx + 1) & RING_MASK;
    }
    data_cnt += SAMPLE_SIZE;
    EXIT_CRITICAL();

    // 4. 满一页则标记待写入
    if(data_cnt >= PAGE_SIZE && flash_state == FLASH_IDLE)
    {
        page_ready = 1;
    }
}

/* ================================================================
 *                    1000ms Flash写入任务（非阻塞状态机）
 * 功能：后台处理Flash擦除、写入，每次仅查询一次状态，绝不阻塞主循环
 * ================================================================ */
void Task_Flash_Write(void)
{
    // ========== 状态1：扇区擦除中 ==========
    if(flash_state == FLASH_ERASING)
    {
        flash_timeout_cnt++;
        // 超时保护：Flash硬件异常则强制复位，避免状态机死锁
        if(flash_timeout_cnt >= FLASH_TIMEOUT)
        {
            flash_state = FLASH_IDLE;
            flash_timeout_cnt = 0;
            page_ready = 0;
            return;
        }

        if(W25Q64_Is_Idle())
        {
            // 擦除完成，自动发起页写入
            W25Q64_PageProgram_Start(flash_addr, flash_page_buf, PAGE_SIZE);
            flash_state = FLASH_WRITING;
            flash_timeout_cnt = 0;
        }
        return;
    }

    // ========== 状态2：页写入中 ==========
    if(flash_state == FLASH_WRITING)
    {
        flash_timeout_cnt++;
        // 超时保护
        if(flash_timeout_cnt >= FLASH_TIMEOUT)
        {
            flash_state = FLASH_IDLE;
            flash_timeout_cnt = 0;
            page_ready = 0;
            return;
        }

        if(W25Q64_Is_Idle())
        {
            // 写入成功，更新缓冲区读指针和计数
            ENTER_CRITICAL();
            if(data_cnt >= PAGE_SIZE)
            {
                read_idx = (read_idx + PAGE_SIZE) & RING_MASK;
                data_cnt -= PAGE_SIZE;
            }
            EXIT_CRITICAL();

            // 地址偏移，8MB容量循环覆盖
            flash_addr += PAGE_SIZE;
            if(flash_addr >= 0x800000)
            {
                flash_addr = 0;
            }

            flash_state = FLASH_IDLE;
            flash_timeout_cnt = 0;
            page_ready = 0;
        }
        return;
    }

    // ========== 状态0：空闲，处理新页 ==========
    if(flash_state == FLASH_IDLE && page_ready == 1)
    {
        if(data_cnt < PAGE_SIZE)
        {
            page_ready = 0;
            return;
        }

        // 1. 从环形缓冲区拷贝一页数据到本地缓存
        uint16_t temp_read = read_idx;
        for(uint16_t i = 0; i < PAGE_SIZE; i++)
        {
            flash_page_buf[i] = ring_buffer[(temp_read + i) & RING_MASK];
        }

        // 2. 4KB对齐地址则先擦除扇区，否则直接写入
        if((flash_addr & 0xFFF) == 0)
        {
            W25Q64_Sector_Erase_Start(flash_addr);
            flash_state = FLASH_ERASING;
            flash_timeout_cnt = 0;
        }
        else
        {
            W25Q64_PageProgram_Start(flash_addr, flash_page_buf, PAGE_SIZE);
            flash_state = FLASH_WRITING;
            flash_timeout_cnt = 0;
        }
    }
}


// 调试：读取Flash从0地址开始的32字节，串口十六进制打印
void Flash_Debug_Print(void)
{
    uint8_t buf[32];
    
    // 读取芯片0地址开始的32字节（真实读硬件，不是读缓冲区）
    W25Q64_Read_Data(0, buf, 32);
    
    Serial_SendString("\r\n--- Flash[0~31] ---\r\n");
    for(uint8_t i = 0; i < 32; i++)
    {
        Serial_SendHex(buf[i]);
        Serial_SendByte(' ');
        if((i+1) % 8 == 0)
            Serial_SendString("\r\n");
    }
    Serial_SendString("----------------\r\n");
}
