/************************* GHWIOINI.C ******************************

   Inittialize and deinitialize target specific LCD resources.

   Creation date: 011111

   Revision date:
   Revision Purpose:

   Version number: 2.00
   Copyright (c) RAMTEX Engineering Aps 2001
   From: RAMTEX/gclcd/s6d0129/cfgio
*********************************************************************/
#ifndef GHW_NOHDW
/*#include <sgio.h>*/      /* Portable I/O functions + hardware port def */
#endif
#include <stdint.h>
#include <gdisphw.h>
#include "hx8357d.h"
#include "TFT_spi.h"
#include <util/delay.h> // For _delay_ms
#ifdef GHW_SINGLE_CHIP
//void sim_reset( void );
#endif

#ifdef GBASIC_INIT_ERR
/*
   ghw_io_init()

   This function is called once by ginit() via ghw_init() before any LCD
   controller registers is addressed. Any target system specific
   initialization like I/O port initialization can be placed here.

*/
void ghw_io_init(void)
{
   #ifndef GHW_NOHDW
   #ifdef GHW_SINGLE_CHIP
   //sim_reset();  /* Initiate LCD bus simulation ports */
   #endif
   
     // Initialize pins for CS and DC (and RST if needed)
     TFT_PINS_INIT();

    // Hardware Reset Sequence
	TFT_RST_LOW();
    //_delay_ms(10);
    
	TFT_RST_HIGH();
    //_delay_ms(150);
	
	// Send initialization commands
	const uint8_t *addr = hx8357d_init;
	while (*addr)
	{
		uint8_t cmd = *addr++;
		uint8_t numArgs = *addr & 0x7F;
		//uint8_t delayFlag = *addr++ & 0x80;
		
		spi_tft_sendCommand(cmd);
		for (uint8_t i = 0; i < numArgs; i++)
		{
			spi_tft_sendData(*addr++);
		}

		//if (delayFlag)
		{
			//_delay_ms(5 * numArgs);
			//for(uint8_t i = 0; i < numArgs; i++)
			//_delay_ms(5); // 5 ms * numArgs total

		}
	}
	#endif
}

/*
  This function is called once by gexit() via ghw_exit() as the last operation
  after all LCD controller operations has stopped.
  Any target system specific de-initialization, like I/O port deallocation
  can be placed here. In most embedded systems this function can be empty.
*/
void ghw_io_exit(void)
   {
   #ifndef GHW_NOHDW
   /* Insert required code here, if any */
   #endif
   }

#endif /* GBASIC_INIT_ERR */

