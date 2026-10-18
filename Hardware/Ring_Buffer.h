#ifndef __RING_BUFFER_H
#define __RING_BUFFER_H

#include "stm32f10x.h"

extern uint8_t  ring_buffer[];
extern volatile uint16_t write_idx;
extern volatile uint16_t read_idx;
extern volatile uint16_t data_cnt;
extern volatile uint8_t  page_ready;
extern uint32_t flash_addr;
extern volatile uint8_t record_flag;

void Task_100ms_Collect(void);
void Task_Flash_Write(void);
void Flash_Debug_Print(void);
#endif
