#ifndef _SPI_H_
#define _SPI_H_

#include "hw.h"
#include <stdint.h>

void SPI_init();
void SPI_transfer(uint8_t data);
void SPI_transfer_block(uint8_t *pBuf, uint16_t count);

#endif