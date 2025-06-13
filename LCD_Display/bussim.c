/***************************** bussim.c ************************************

   Generic template file for external bus simulator drivers.
   Implemented for serial (SPI) bus simulation using a parallel port.

   The functions below must be implemented as described in the comments.
   Use the proper I/O port instructions given by the actual processor
   architecture, the actual LCD connection method and the I/O access
   capabilities of the target C compiler.

   The portable sgwrby(..) and sgrdby(..) syntax is used for illustration.
   This syntax can be replaced by the compilers native syntax for I/O access.

   The functions in this module is called by the ghwinit.c module when the
   compiler switch GHW_SINGLE_CHIP is defined and GHW_NOHDW is undefined.
   The GHW_BUS16 mode should be used as all data and instructions transfered
   is 16 bit.

   The driver assumes that the address is either 0 or 1.

   Copyright (c) RAMTEX Engineering Aps 2007

****************************************************************************/
#ifndef GHW_NOHDW
#ifdef  GHW_SINGLE_CHIP

/* Define SPI bus bit locations in the output register
   (modify to fit the actual hardware) */
#define  SDO   0x01   /* SPI data out bit (from processor) */
#define  SDI   0x02   /* SPI data in bit  (to processor) */
#define  SCLK  0x04   /* SPI clock bit */
#define  CS    0x08   /* LCD controller chip select */
#define  RST   0x10   /* LCD controller reset (negative pulse) */

#include <bussim.h>

/*#include < Port declaration file for your compiler > */
#include <sgio.h> /* or use the SG function syntax (*/

/*
   Simulate a serial bus write operation for a LCD controller via
   an I/O register interface.

   In this impelementation example the output register is assumed to
   support readback so unused register bits are not modified.

   The parallel port for write is here called P1 and accesed using the SG syntax.
   (Modify to a compiler specific syntax if nessesary to fit the
    actual hardware and compiler)

*/

static SGUCHAR tmp;  /* Temporary hold of (unused) port data bits. Common value to speed transfer */

/*
   Write SPI byte from hardware (SDI receive is ignored)
*/
static void tx_spi(SGUCHAR dat)
   {
   /* Loop while clocking data out, msb first */
   register SGUCHAR msk;
   msk=0x80;
   do
      {
      if (((dat & msk) != 0))
         {
         sgwrby(P1,tmp|SDO);         /* 4.a Set data high  (other bits is unchanged) */
         sgwrby(P1,tmp|SDO|SCLK);    /* 5.a Set clock high (other bits is unchanged) */
         sgwrby(P1,tmp|SDO);         /* 6.a Set clock low  (other bits is unchanged) */
         }
      else
         {
         sgwrby(P1,tmp);               /* 4.b Set data low   (other bits is unchanged)*/
         sgwrby(P1,tmp|SCLK);          /* 5.b Set clock high (other bits is unchanged)*/
         sgwrby(P1,tmp);               /* 6.b Set clock low  (other bits is unchanged)*/
         }
      msk >>= 1;
      }
   while(msk != 0);
   }


/*
   Read SPI byte from hardware (SDO = 0 is transmitted)
*/
static SGUCHAR rx_spi(void)
   {
   register SGUCHAR msk,dat;
   /*  Loop while clocking data in, msb first */
   dat = 0;
   msk=0x80;
   do
      {
      sgwrby(P1,tmp|SCLK);          /* Set clock high (other bits is unchanged) */
      if ((sgrdby(P1) & SDI) != 0)
         dat |= msk;
      sgwrby(P1,tmp);               /* Set clock low  (other bits is unchanged)*/
      msk >>= 1;
      }
   while(msk != 0);
   return dat;
   }


/*
   Write word
*/
void simwrwo(SGUCHAR adr, SGUINT dat)
   {
   /* Read state of unused port bits and initiate used bit for SPI access */
   tmp = (sgrdby(P1) & (~(SCLK|SDO|CS)))|SDI;
   tx_spi(adr);    /* Send device access byte */
   tx_spi(dat>>8); /* Send MSB byte */
   tx_spi(dat);    /* Send LSB byte */
   sgwrby(P1,tmp|CS);
   }

/*
   Read word
*/
SGUINT simrdwo(SGUCHAR adr)
   {
   SGUINT dat;
   /* Read state of unused port bits and initiate used bit for SPI access */
   tmp = (sgrdby(P1) & (~(SCLK|SDO|CS)))|SDI;
   tx_spi(adr);                  /* Send device access byte */
   if (adr == GHWSTA)
      rx_spi();                  /* Make a single dummy read during status read */
                                 /* (Dummy reads during data word read is handled at higher level)*/
   dat = ((SGUINT) rx_spi())<<8; /* Receive MSB byte */
   dat |= (SGUINT) rx_spi();     /* Receive LSB byte */
   sgwrby(P1,tmp|CS);
   return dat;
   }

/*
   Initialize and reset LCD display.
   Is called before simwrby() is invoked for the first time

   The LCD reset line is toggled here if it connected to a bus port.
   (it may alternatively be hard-wired to the reset signal to the processors
   in the target system).

   The sim_reset() function is invoked automatically via the ghw_io_init() function.
*/
void sim_reset( void )
   {
   /* 1. Init data port setup (if required by port architecture) */
   /* 2. Make A0,CS,SDA,SCLK,RST to outputs (if required by port architecture) */
   /* 3. Set SCLK and CS high */
   tmp = (sgrdby(P1) & ~(SDO|SCLK))|CS;
   /* 4. Set LCD reset line /RST active low   (if /RST is connected to a port bit) */
   sgwrby(P1, tmp);
   /* 5. Set LCD reset line /RST passive high (if /RST is connected to a port bit) */
   sgwrby(P1, tmp|RST);
   lastspiadr = 0xff; /* Force re-addressing next time  */
   }

#endif /* GHW_SINGLE_CHIP */
#endif /* GHW_NOHDW */


