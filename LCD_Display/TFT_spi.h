/*
 * TFT_spi.h
 *
 * Created: 6/4/2025 2:31:40 PM
 *  Author: Steve
 */ 

#ifndef TFT_SPI_H_
#define TFT_SPI_H_

#include <avr/io.h>
#include "spi4.h" // basic spi functions
#include "TFT_spi.h" // req for TFT_DC_INIT

// ---------- Data/Command (D/C) configuration ----------D/C is Command LOW
#define TFT_SPI_DC_PORT PORTA
#define TFT_SPI_DC_DDR  DDRA
#define TFT_SPI_DC_PIN  0
#define TFT_SPI_DC_MASK (1 << TFT_SPI_DC_PIN)

#define TFT_DC_INIT()    (TFT_SPI_DC_DDR |= TFT_SPI_DC_MASK)  // Set DC pin as output
#define TFT_DC_LOW()     (TFT_SPI_DC_PORT &= ~TFT_SPI_DC_MASK) // DC = 0 (Command)
#define TFT_DC_HIGH()    (TFT_SPI_DC_PORT |= TFT_SPI_DC_MASK)  // DC = 1 (Data)

// -------------TFT /RST------------------------------
#define TFT_SPI_RST_PORT PORTE
#define TFT_SPI_RST_DDR  DDRE
#define TFT_SPI_RST_PIN  7
#define TFT_SPI_RST_MASK (1 << TFT_SPI_RST_PIN)
#define TFT_RST_LOW()         (TFT_SPI_RST_PORT &= ~TFT_SPI_RST_MASK)  // Active LOW
#define TFT_RST_HIGH()       (TFT_SPI_CS_PORT |=  TFT_SPI_RST_MASK)  // Inactive HIGH

// ---------- Chip Select (CS) configuration ----------
#define TFT_SPI_CS_PORT PORTA //Port for Chip Select
#define TFT_SPI_CS_DDR  DDRA       // Data Direction Register for Chip Select
#define TFT_SPI_CS_PIN  2  // Pin number for Chip Select
#define TFT_SPI_CS_MASK (1 << TFT_SPI_CS_PIN)  //00000100
#define TFT_CS_INIT()   (TFT_SPI_CS_DDR |= TFT_SPI_CS_MASK)
#define TFT_CS_LOW()	(TFT_SPI_CS_PORT &= ~TFT_SPI_DC_MASK) // CS = 0 (selected)
#define TFT_CS_HIGH()	(TFT_SPI_CS_PORT |= TFT_SPI_CS_MASK)  //CS = 1 (deselected)


#define TFT_PINS_INIT()   do { TFT_CS_INIT(); TFT_DC_INIT(); } while(0) 
// used in ghwioini.c ------- Initialization helper ----------
//#define SELECT_CS()         (TFT_SPI_CS_PORT &= ~TFT_SPI_CS_MASK)  // Active LOW
//#define DESELECT_CS()       (TFT_SPI_CS_PORT |=  TFT_SPI_CS_MASK)  // Inactive HIGH

void spi_tft_sendCommand(uint8_t cmd);

// Function to perform SPI communication with the TFT display
void spi_tft_sendData(uint8_t data);

#endif /* TFT_SPI_H_ */