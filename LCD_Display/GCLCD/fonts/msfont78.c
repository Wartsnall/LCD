/***************************** msfont78.c ***********************

   msfont78 font table structure definitions.
   This file has been auto-generated with the IconEdit tool.

   Copyright(c) RAMTEX 1998-2004

*****************************************************************/
#include <gdisphw.h>

/* Symbol table entry with fixed sized symbols */
static struct
   {
   GSYMHEAD sh;        /* Symbol header */
   SGUCHAR b[9];       /* Symbol data, "variable length" */
   }
GCODE FCODE msfont78sym[256] =
   {
   #include "msfont78.sym" /* Include symbols */
   };

/* Font structure */
GCODE GFONT FCODE msfont78 =
   {
   7,       /* width */
   9,       /* height */
   sizeof(msfont78sym[0])-sizeof(GSYMHEAD), /* number of bytes in a symbol (including any alignment padding)*/
   (PGSYMBOL)msfont78sym, /* pointer to array of SYMBOLS */
   256,      /* num symbols */
   (PGCODEPAGE)(NULL)
   };

