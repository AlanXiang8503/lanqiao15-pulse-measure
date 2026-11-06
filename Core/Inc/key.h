#ifndef __KEY_H_
#define __KEY_H_

#include "main.h"

extern volatile uint32_t xms;
extern volatile uint32_t key3_start;//按键3开始，清0用

uint8_t Key_read(void);
uint8_t Key_scan(void);

#endif
