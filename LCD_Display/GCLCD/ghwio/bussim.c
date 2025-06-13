/***************************** bussim.c ************************************

   Generic template file for external bus simulator drivers.

   The functions bodies below must be implemented as described in the comments.
   Use the proper I/O port instructions given by the actual processor
   architecture, the actual LCD connection method and the I/O access
   capabilities of the target C compiler.

   Alternatively this module can be used as a universal stub during initial
   test compilation of the target software.

   The functions in this module is called by single-chip-processor version
   of the ghwinit.c module.

   Copyright (c) RAMTEX Engineering Aps 2006-2009

****************************************************************************/
#ifndef GHW_NOHDW
#ifdef  GHW_SINGLE_CHIP

#include <bussim.h>
/*#include < Port declaration file for your compiler > */

#ifdef GHW_BUS8
/*
   Simulate a bus write operation for a LCD controller with an Intel
   like bus interface (i.e. use of separate /RD and /WR strobes).
   This access function is 8 bit processor data bus interfaces.

   The address parameter adr is assumed to be either 0 or 1.
*/
void simwrby(SGUCHAR adr, SGUCHAR dat)
   {
   /* A: Set C/D line according to adr, Set /CE line active low */
   /* B1: Make data port an output (if required by port architecture) */
   /* B2: Write data to data port */
   /* C: Set /WR active low, (Delay min 80 ns), */
   /* D: Set /WR passive high */
   /* E: Set /CE passive high */
   }

/*
   Simulate a bus read operation for a LCD controller with an Intel
   like bus interface (i.e. use of separate /RD and /WR strobes).
   This access function is 8 bit processor data bus interfaces.

   The address parameter adr is assumed to be either 0 or 1.
*/
SGUCHAR simrdby(SGUCHAR adr)
   {
   SGUCHAR dat = 0;
   /* a: Set C/D line according to adr. Set /CE line active low */
   /* b: Make data port an input (if required by port architecture) */
   /* c: Set /RD active low, (Delay min 150ns), */
   /* d: Read data from data port */
   /* e1:Set /RD passive high, */
   /* e2:Set /CE passive high (could be ignored) */
   return dat;
   }

#elif defined( GHW_BUS16 )
/* 16 bit bus mode */

/*
   Simulate a bus write operation for a LCD controller with an Intel
   like bus interface (i.e. use of separate /RD and /WR strobes).
   This access function is 16 bit processor data bus interfaces.

   The address parameter adr is assumed to be either 0 or 1.
*/
void simwrwo(SGUCHAR adr, SGUINT dat)
   {
   /* A: Set C/D line according to adr, Set /CE line active low */
   /* B1: Make data port an output (if required by port architecture) */
   /* B2: Write data to data port */
   /* C: Set /WR active low, (Delay min 80 ns), */
   /* D: Set /WR passive high */
   /* E: Set /CE passive high */
   }

/*
   Simulate a bus read operation for a LCD controller with an Intel
   like bus interface (i.e. use of separate /RD and /WR strobes).
   This access function is 16 bit processor data bus interfaces.

   The address parameter adr is assumed to be either 0 or 1.
*/
SGUINT simrdwo(SGUCHAR adr)
   {
   SGUINT dat = 0;
   /* a: Set C/D line according to adr. Set /CE line active low */
   /* b: Make data port an input (if required by port architecture) */
   /* c: Set /RD active low, (Delay min 150ns), */
   /* d: Read data from data port */
   /* e1:Set /RD passive high, */
   /* e2:Set /CE passive high (could be ignored) */
   return dat;
   }

#else
/* 32 bit bus mode (the display may use the lsb bits of the dat parameter) */

/*
   Simulate a bus write operation for a LCD controller with an Intel
   like bus interface (i.e. use of separate /RD and /WR strobes).
   This access function is 32 (18) bit processor data bus interfaces.

   The address parameter adr is assumed to be either 0 or 1.
*/
void simwrdw(SGUCHAR adr, SGLONG dat)
   {
   /* A: Set C/D line according to adr, Set /CE line active low */
   /* B1: Make data port an output (if required by port architecture) */
   /* B2: Write data to data port */
   /* C: Set /WR active low, (Delay min 80 ns), */
   /* D: Set /WR passive high */
   /* E: Set /CE passive high */
   }

/*
   Simulate a bus read operation for a LCD controller with an Intel
   like bus interface (i.e. use of separate /RD and /WR strobes).
   This access function is 32 (18) bit processor data bus interfaces.

   The address parameter adr is assumed to be either 0 or 1.
*/
SGULONG simrddw(SGUCHAR adr)
   {
   SGULONG dat = 0;
   /* a: Set C/D line according to adr. Set /CE line active low */
   /* b: Make data port an input (if required by port architecture) */
   /* c: Set /RD active low, (Delay min 150ns), */
   /* d: Read data from data port */
   /* e1:Set /RD passive high, */
   /* e2:Set /CE passive high (could be ignored) */
   return dat;
   }

#endif

/*
  Initialize and reset LCD display.
  Is called before simwrby() and simrdby() is invoked for the first time

  The controller reset line is toggled here if it connected to a bus port.
  (it may alternatively be hard-wired to the reset signal to the processors
  in the target system).

  The sim_reset() function is invoked automatically via the ginit() function.
*/
void sim_reset( void )
   {
   /* 1. Init data port setup (if required by port architecture) */
   /* 2. Make C/D, /RD, /WR, /CE to outputs (if required by port architecture) */
   /* 3. Set LCD reset line /RST active low   (if /RST is connected to a port bit) */
   /* 4. Set LCD reset line /RST passive high (if /RST is connected to a port bit) */
   }

#endif /* GHW_SINGLE_CHIP */
#endif /* GHW_NOHDW */



