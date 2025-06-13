/***************************** ariel18.c ***********************

   ariel18 font table structure definitions.
   This file has been auto-generated with the IconEdit tool.

   Copyright RAMTEX 1998-2003

*****************************************************************/
#include <gdisphw.h>

/* Code page entry (one codepage range element) */
static struct
   {
   GCPHEAD chp;
   GCP_RANGE cpr[1];     /* Adjust this index if more codepage segments are added */
   }
 GCODE FCODE ariel18cp =
   {
   #include "ariel18.cp" /* Symbol table */
   };

/* Symbol table entry with fixed sized symbols */
static struct
   {
   GSYMHEAD sh;        /* Symbol header */
   SGUCHAR b[54];       /* Symbol data, "variable length" */
   }
GCODE FCODE ariel18sym[96] =
   {
   #include "ariel18.sym" /* Include symbols */
   };

/* Font structure */
GCODE GFONT FCODE ariel18 =
   {
   9,       /* width */
   18,       /* height */
   sizeof(ariel18sym[0])-sizeof(GSYMHEAD), /* number of bytes in a symbol (including any alignment padding)*/
   (PGSYMBOL)ariel18sym,/* pointer to array of SYMBOLS */
   96,      /* num symbols */
   (PGCODEPAGE)&ariel18cp
   };
