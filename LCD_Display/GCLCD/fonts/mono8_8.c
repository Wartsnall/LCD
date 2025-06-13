/***************************** mono8_8.c ************************

   mono8_8.c font table structure definitions

   Copyright RAMTEX 1998-2001

*******************************************************************/
#include <gdisphw.h>

/* Symbol table entry with fixed sized symbols */
static struct
   {
   GSYMHEAD sh;        /* Symbol header */
   SGUCHAR b[8];       /* Symbol data. This structure is recured by IconEdit */
   }
/* array of GSYMBOLs*/
GCODE FCODE mono8_8sym[0x80] =
   {
   #include "mono8_8.sym"
   };

/* A font */
GCODE GFONT FCODE mono8_8 =
   {
   8,       /* width%8 */
   8,       /* height */
   sizeof(mono8_8sym[0])-sizeof(GSYMHEAD), /* number of bytes in a symbol (including any alignment padding)*/
   (PGSYMBOL)mono8_8sym, /* pointer to array of SYMBOLS */
   0x80,    /* num symbols */
   NULL
   };

