/*************************** cp885914.c ************************

   ISO/IEC 8859 code page, Latin alphabet no 1
   Large font.

*****************************************************************/
#include <gdisphw.h>

/* Symbol table entry with fixed sized symbols */
static struct
   {
   GSYMHEAD sh;         /* Symbol header */
   SGUCHAR b[28];       /* Symbol data, length = ((cxpix+7)/8)*cypix */
   }
GCODE FCODE cp8859_14sym[0x100] =
   {
   #include "cp8859_14.sym" /* Symbol table */
   };

/* Font structure */
GCODE GFONT FCODE cp8859_14 =
   {
   8,        /* width */
   14,       /* height */
   sizeof(cp8859_14sym[0])-sizeof(GSYMHEAD), /* number of bytes in a symbol (including any alignment padding)*/
   (PGSYMBOL)cp8859_14sym, /* pointer to array of SYMBOLS */
   0x100,    /* num symbols */
   NULL
   };
