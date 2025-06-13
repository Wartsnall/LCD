/***************************** ms58p.c ************************

   ms58p font table structure definitions

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
 GCODE FCODE ms58psym[0x100] =
   {
   #include "ms58p.sym"
   };

GCODE GFONT FCODE ms58p =
   {
   6,      /* width%8 */
   8,      /* height */
   sizeof(ms58psym[0])-sizeof(GSYMHEAD), /* number of bytes in a symbol (including any alignment padding)*/
   (PGSYMBOL)&ms58psym,/* pointer to array of SYMBOLS */
   0x100,  /* num symbols */
   NULL
   };

