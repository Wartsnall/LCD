/*
* LCD_Display.c
*
* Created: 6/3/2025 5:32:12 PM Modified to spi4.c edits. 
*  Author: Steve
* Description:  This file handles SPI communication for the TFT display. Code for communication with other SPI devices (Data SD SPI, HV SPI, Relay SPI) is commented for future expansion.
*  4 delays commented out in ghwioini.c for debugging. 
*/ 

#include <avr/io.h>
#include <gdisp.h>  
#include "hx8357d.h"
#include "bussim.h"
#include "spi4.h"
#include "TFT_spi.h" // Includes Chip Select definitions for TFT SPI

void init_pins(void) 
{
	// Set CS pins as output
	TFT_SPI_CS_DDR |= TFT_SPI_CS_MASK; //port A DDR pin 2 set for O/P

	// Set CS pins high (inactive state)
	TFT_SPI_CS_PORT |= TFT_SPI_CS_MASK;
}

int main(void)
{
	// Initialize SPI and pins
	init_pins();
	init_spi_master();
	//Init display hardware and print an error message
	//on the center of the LCD display module from manual pg 26
	
	// turn back light on
	DDRB |= (1<<PB7); //Sets PB7 as output, leaves other pins unchanged
	PORTB |= (1<<PB7); // Sets PB7 high, leaves other pins unchanged
		
	ghw_io_init(); // in ghwioini.c
	gputs("\nUsing SYSFONT");
	gselfont(&ariel9);
	gputs("Using ariel9");
	gselfont(&SYSFONT);
	gputs("\nUsing SYSFONT");
	
	while (1)
	{
		static int i=0;
		i++; //Dummy operation so we can put a break point here
	}
	
// Future Expansion: Code for other SPI devices	
	// Communicate with TFT SD SPI
	/*
	SPIPORT &= ~TFT_SD_SPI_CS;  // Select TFT SD SPI
	SPI_Transfer(data);
	SPIPORT |= TFT_SD_SPI_CS;   // Deselect TFT SD SPI
	*/
	// Communicate with Data SD SPI
	/*
	SPIPORT &= ~DATA_SD_SPI_CS;  // Select Data SD SPI
	SPI_Transfer(data);
	SPIPORT |= DATA_SD_SPI_CS;   // Deselect Data SD SPI
	*/
	// Communicate with HV SPI
	/*
	SPIPORT &= ~HV_SPI_CS;  // Select HV SPI
	SPI_Transfer(data);
	SPIPORT |= HV_SPI_CS;   // Deselect HV SPI
	*/
	// Communicate with Relay SPI
	/*gh
	SPIPORT &= ~RLY_SPI_CS;  // Select Relay SPI
	SPI_Transfer(data);
	SPIPORT |= RLY_SPI_CS;   // Deselect Relay SPI
	*/

	return 0;
}