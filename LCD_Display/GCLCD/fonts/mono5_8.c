/***************************** mono5_8.c ************************

   Mono5_8 font table structure definitions

   Copyright RAMTEX 1998-2003

*****************************************************************/
#include <gdisphw.h>

/* Symbol table entry with fixed sized symbols */
static struct
   {
   GSYMHEAD sh;        /* Symbol header */
   SGUCHAR b[8];       /* Symbol data, variable length = (cxpix/8+1)*cypix */
   }
 /* array of GSYMBOLs*/
 GCODE FCODE mono5_8_sym[0x100] =
   {
   #include "mono5_8.sym"
   };

GCODE GFONT FCODE mono5_8 =
   {
   6,      /* width%8 */
   8,      /* height */
   sizeof(mono5_8_sym[0])-sizeof(GSYMHEAD), /* number of bytes in a symbol (including any alignment padding)*/
   (PGSYMBOL)&mono5_8_sym,/* pointer to array of SYMBOLS */
   0x100,  /* num symbols */
   NULL
   };

