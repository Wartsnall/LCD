/*
 * spi4.c
 *
 * Created: 6/4/2025 2:26:53 PM
 *  Author: Steve
 
 setup:
 You must set pin #define correctly in spi.h for given hardware or this won't work.
 EG: For uno: MISO is PB4, MOSI is PB4, SCK is PB5
 
 use:

 make sure to SELECT() before i/o and DESELECT() when done with i/o
 
 use spi_mode(x) to set SPI mode 1,2,3, any other value is mode 0.
 
 AVR chip is always master while using this library.
 
 also:
 your c file with main{} will need to #include <avr/io.h> or this won't compile.
 
 SPIPORT is the SPI CS pin. This must be defined,
 */ 
#include <avr/io.h>
#include "spi4.h"
#include <stdint.h>


void init_spi_master(void)
{
	/* SPI pins */
	
	SPIDDR |= (SPI_MOSI | SPI_SCK | SPI_SS);  // Set MOSI and SCK, and SS as outputs
	SPIPORT |= SPI_MISO;            // Enable pull-up on MISO

	
	/* Enable SPI, Master, set clock rate fck/16-(1Mhz */
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
}

void spi_mode(uint8_t mode)  // 0 is normal default.
{
	if (mode == 1)
	{
		SPCR |= (0 << CPOL); //SCK is LOW when Idle
		SPCR |= (1 << CPHA); //Data sampled on trailing edge
	}
	else if (mode == 2)
	{
		SPCR |= (1 << CPOL); //SCK is HIGH when Idle
		SPCR |= (0 << CPHA); //Data sampled on leading edge
	}
	else if (mode == 3)
	{
		SPCR |= (1 << CPOL); //SCK is HIGH when Idle
		SPCR |= (1 << CPHA); //Data sampled on trailing edge
	}
	else
	{
		// Mode 0
		SPCR |= (0 << CPOL); //SCK is LOW when Idle
		SPCR |= (0 << CPHA); //Data sampled on leading edge
	}
}

uint8_t SPI_Transfer(uint8_t data)
{
	SPDR = data;
	while (!(SPSR & (1 << SPIF)));
	return SPDR;
}

/*uint8_t SPI_TransferTx16(unsigned char cs_pin, unsigned char a, unsigned char b)
{
	unsigned char x;
	SPIPORT &= ~cs_pin;   // CS = L 
	x = SPI_Transfer(a);
	SPIPORT |= cs_pin;    // CS = H 
	SPIPORT &= ~cs_pin;   // CS = L 
	x = SPI_Transfer(b);
	SPIPORT |= cs_pin;    // CS = H 
	return x;
}

uint8_t SPI_TransferTx16_SingleCS(unsigned char cs_pin, unsigned char a, unsigned char b)
{
	unsigned char x;
	SPIPORT &= ~cs_pin;   // CS = L 
	x = SPI_Transfer(a);
	x = SPI_Transfer(b);
	SPIPORT |= cs_pin;    // CS = H 
	return x;
}*/