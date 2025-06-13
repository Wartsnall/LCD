/***************************** msfont58.c ************************

   msfont58 font table structure definitions

   Copyright RAMTEX 1998-2001

*****************************************************************/
#include <gdisphw.h>

/* Symbol table entry with fixed sized symbols */
static struct
   {
   GSYMHEAD sh;        /* Symbol header */
   SGUCHAR b[8];       /* Symbol data, variable length = (cxpix/8+1)*cypix */
   }
 /* array of GSYMBOLs*/
 GCODE FCODE msfont58sym[0x100] =
   {
   #include "msfont58.sym"
   };

GCODE GFONT FCODE msfont58 =
   {
   8,      /* width%8 */
   8,      /* height */
   sizeof(msfont58sym[0])-sizeof(GSYMHEAD), /* number of bytes in a symbol (including any alignment padding)*/
   (PGSYMBOL)&msfont58sym,/* pointer to array of SYMBOLS */
   0x100,  /* num symbols */
   NULL
   };

