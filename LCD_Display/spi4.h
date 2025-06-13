/*
 * spi4.h
 *
 * Created: 6/4/2025 2:24:43 PM
 *  Author: Steve
 *
 * Description:
 * This file allows for multiple SPI devices by taking CS pin as an argument. It includes setup and utility functions for SPI communication.
 * You must set pin #define correctly for given hardware or this won't work.
 * For 2560: MISO is PB3, MOSI is PB2, SCK is PB1
 *
 * Usage:
 * - Make sure to SELECT_CS() before I/O and DESELECT_CS() when done with I/O for the SPI device.
 * - AVR chip is always master while using this library.
 * - Your main.c file will need to #include <avr/io.h> for this to compile.
 */ 


#ifndef SPI4_H_
#define SPI4_H_

#include <avr/io.h>

/* SPI Pin Definitions for Atmega2560 */
#define DDR_SPI DDRB
#define SPIPORT PORTB
#define SPIDDR DDRB
#define SPI_SCK (1 << 1)  /* Clock Pin (PB1)*/
#define SPI_MISO (1 << 3) /* Master-In Slave-Out(PB3) */
#define SPI_MOSI (1 << 2) /* Master-Out Slave-In )PB2) */
#define SPI_SS (1 << 0)
/* Chip Select Definitions  are in individual SPI device .h files */


//#define DATA_SD_SPI_CS_PORT PORTA /* Port for Data SD SPI Chip Select 
//#define DATA_SD_SPI_CS_PIN 4      /* Pin number for Data SD SPI Chip Select 
//#define DATA_SD_SPI_CS (1 << DATA_SD_SPI_CS_PIN) /* Bit mask for Data SD SPI Chip Select 

//#define HV_SPI_CS_PORT PORTA /* Port for HV SPI Chip Select 
//#define HV_SPI_CS_PIN 5      /* Pin number for HV SPI Chip Select 
//#define HV_SPI_CS (1 << HV_SPI_CS_PIN) /* Bit mask for HV SPI Chip Select 

//#define RLY_SPI_CS_PORT PORTA /* Port for Relay SPI Chip Select 
//#define RLY_SPI_CS_PIN 6      /* Pin number for Relay SPI Chip Select 
//#define RLY_SPI_CS (1 << RLY_SPI_CS_PIN) /* Bit mask for Relay SPI Chip Select 

/* Utility macros for Chip Select */
#define SELECT_CS(port, cs_pin) ((port) &= ~(cs_pin)) // Activate (clears) CS 
#define DESELECT_CS(port, cs_pin) ((port) |= (cs_pin)) //Deactivate Chip Select

// SPI Function Declarations
void init_spi_master(void);
void spi_mode(unsigned char mode);
uint8_t SPI_Transfer(uint8_t data);
uint8_t SPI_TransferTx16(unsigned char cs_pin, unsigned char a, unsigned char b);
uint8_t SPI_TransferTx16_SingleCS(unsigned char cs_pin, unsigned char a, unsigned char b);

#endif /* SPI4_H_ */