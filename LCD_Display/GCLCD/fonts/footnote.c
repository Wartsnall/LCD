/***************************** footnote.c ************************

   footnote font table structure definitions

   Copyright RAMTEX 1998-2007

*****************************************************************/
#include <gdisphw.h>

/* Symbol table entry with fixed sized symbols */
static struct
   {
   GSYMHEAD sh;        /* Symbol header */
   SGUCHAR b[8];       /* Symbol data, variable length = (cxpix/8+1)*cypix */
   }
 /* array of GSYMBOLs*/
 GCODE FCODE footnotesym[0x100] =
   {
   #include "footnote.sym"
   };

GCODE GFONT FCODE footnote =
   {
   4,      /* width%8 */
   8,      /* height */
   sizeof(footnotesym[0])-sizeof(GSYMHEAD), /* number of bytes in a symbol (including any alignment padding)*/
   (PGSYMBOL)&footnotesym,/* pointer to array of SYMBOLS */
   0x100,  /* num symbols */
   NULL
   };

