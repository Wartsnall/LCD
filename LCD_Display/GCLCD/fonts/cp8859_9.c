/*************************** cp8859_9.c ************************

   ISO/IEC 8859 code page, Latin alphabet no 1 Small font

   Copyright(c) RAMTEX 1998-2009

*****************************************************************/
#include <gdisphw.h>

/* Symbol table entry with fixed sized symbols */
static struct
   {
   GSYMHEAD sh;        /* Symbol header */
   SGUCHAR b[9];       /* Symbol data, length = ((cxpix+7)/8)*cypix */
   }
GCODE FCODE cp8859_9sym[0x100] =
   {
   #include "cp8859_9.sym" /* Symbol table */
   };

/* Font structure */
GCODE GFONT FCODE cp8859_9 =
   {
   7,       /* width%8 */
   9,       /* height */
   sizeof(cp8859_9sym[0])-sizeof(GSYMHEAD), /* number of bytes in a symbol (including any alignment padding)*/
   (PGSYMBOL)cp8859_9sym, /* pointer to array of SYMBOLS */
   0x100,   /* num symbols */
   NULL
   };

