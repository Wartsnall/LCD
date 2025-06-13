/*
 * TFT_spi.c
 * to hold specific functions for comms to TFT
 * Created: 6/3/2025 2:33:57 PM
 *  Author: Steve
*  spi_tft_sendData modified to handle CS duties
*
*/ 
#include <avr/io.h>
#include "spi4.h"
#include "TFT_spi.h"

// Set D/C low for command, send byte over SPI
void spi_tft_sendCommand(uint8_t cmd) 
{
	TFT_DC_LOW();       // D/C = 0 for command
	TFT_CS_LOW();   // Select the TFT 
	SPI_Transfer(cmd);  // Send byte via SPI (implement this for your MCU)
	TFT_CS_HIGH();  // De-select TFT
}

// Function to perform SPI communication with the TFT display
void spi_tft_sendData(uint8_t data) 
{
	TFT_DC_HIGH();      // D/C = 1 for data
	TFT_CS_LOW();   // Select the TFT
	SPI_Transfer(data); // Send byte via SPI
	TFT_CS_HIGH();  // De-select TFT
}