#ifndef GDISPCFG_H
#define GDISPCFG_H
/******************* gdispcfg.h *****************************

   CONFIGURATION FILE FOR THE GRAPHIC DISPLAY LIBRARY
   This header file define the driver feature set used by your
   application and the abstract display hardware properties.

   As a programmer you set these definements in order to adjust
   the display driver code to the needs of your application.
   In many cases the definements in this file is used to remove,
   modify, or replace sections of the underlying library source code.

   This header is included by the Graphic display library files.
   It should normally not be included by the user application.

   Revision data:    080808
   Revision Purpose: Configuration header simplified
                     Virtual font support added.
   Revision data:    17-03-09
   Revision Purpose: SSD1355 and ST7628 support added
   Revision data:    20-07-09
   Revision Purpose: HX8353 support added
   Revision data:    25-06-10
   Revision Purpose: ILI9163 support added
   Revision data:    24-08-11
   Revision Purpose: HX8347G support added
   Revision data:    20-01-12
   Revision Purpose: HX8352B support added
                     HX8353D support added
   Revision data:    15-05-12
   Revision Purpose: HX8347D, HX8347G to HX8347I use same switch
                     Special switches for HX8347A
   Revision data:    25-04-14
   Revision Purpose: Special GHW_ILI9341V switch added.
   Revision data:    25-09-14
   Revision Purpose: HX8369 support added.
   Revision data:    26-02-16
   Revision Purpose: ILI9488, ILI9486L, ILI8327 supported added

   Version number: 1.8
   Copyright (c) RAMTEX International Aps 2006-2016
   Web site, support and upgrade: www.ramtex.dk


************************************************************/

/* Size of display module in pixels */
#define GDISPW 480    /* Width */
#define GDISPH 320    /* Height */

#define GDISPCW 8 // built-in font char width in HW LCD, 8,7,6,5 pg 20

/*#define GHW_HX8346_REGINTF */ /* Using HX8346 (HX8347) (320x240) register index interface mode */
//#define GHW_HX8346_CMDINTF */ /* Using HX8346 (HX8347) (320x240) command interface mode */
/*#define GHW_HX8347A_REGINTF*/ /* Using HX8347A (320x240) register index interface mode */
/*#define GHW_HX8347A_CMDINTF*/ /* Using HX8347A (320x240) command interface mode */
/*#define GHW_HX8347G */        /* Using HX8347D-HX8347G-HX8347I (240x320) (Note HX8347D and later has different register layout than HX8347 */
/*#define GHW_HX8325_REGINTF */ /* Using HX8325 (320x240) register index interface mode */
/*#define GHW_HX8325_CMDINTF */ /* Using HX8325 (320x240) or SPFD54126 command interface mode */
/*#define GHW_HX8352B */        /* Using HX8352B (240x320)(240x400)(240x442) or HX8367A (240x320) register index interface */
/*#define GHW_HX8353_CMDINTF */ /* Using HX8353 (132x162) or ST7773 */
#define GHW_HX8353D_CMDINTF // Using HX8353D (132x162) as per email 
/*#define GHW_SSD1355*/         /* Using SSD1355 (128x160) */
/*#define GHW_ST7628 */         /* Using ST7628  (98x70) */
//#define GHW_ILI9163             /* Using ILI9163 (132x160), NT39122, ILI9340, ILI9341 (240x320), ILI9342 (320x240), ST7715 (132x132), ST7735 (132x162), ST7773 (176x122), ST7789C (240x320) */
/*#define GHW_ILI9341V */       /* Using ILI9341V (240x320) (ILI9341V behave like ILI9163, except for read back in 16 bit bus mode)*/
/*#define GHW_HX8369*/          /* Using HX8369-A (480x864), HX8368-A (320x240) - command interface mode */
/*#define GHW_ILI9488*/         /* Using ILI9488 (320x480),ILI9327(240x432), ILI9486L (320x480). command interface mode */


/* Define display controller bus size (select only one).
   This must match the basic chip configuration after chip hardware reset */
#define GHW_BUS8       /* 8 bit data bus (only with GDISPPIXW = 24) */
/*#define GHW_BUS16*/  /* 16 bit data bus */
/*#define GHW_BUS32*/  /* 32 bit data bus (only with GDISPPIXW > 16) */

/* Define number of bits pr pixel for data storage (select only one) */
 #define GDISPPIXW 16       /* 16 bit RGB mode */
/* #define GDISPPIXW 18 */  /* 18 bit RGB mode */
/* #define GDISPPIXW 24 */  /* 24 bit RGB mode */

/* Define number of view-ports supported,
   See the function SGUCHAR gselvp( SGUCHAR s );
   At least 1 view-port must be defined */
#define GNUMVP 5

/* Feature optimization compilation keywords */
#define GBASIC_INIT_ERR      /* Enable Basic initialization and error handling */
#define GVIEWPORT            /* Enable high-level (view port) functions */
#define GGRAPHICS            /* Enable Graphics */
#define GSOFT_SYMBOLS        /* Enable Software symbols */
#define GSOFT_FONTS          /* Enable Soft fonts */
/*#define GVIRTUAL_FONTS*/   /* Enable virtual font support (static lookup) */
/*#define GVIRTUAL_FONTS_DYN*/ /* Enable named virtual font support (dynamic lookup) */
#define GBASIC_TEXT          /* Enable Basic text */
#define GS_ALIGN             /* Enable extended string alignment */
/*#define GMULTIBYTE */      /* Enable multi-byte support */
/*#define GMULTIBYTE_UTF8 */ /* Enable UTF-8 multi-byte support */
/*#define GWIDECHAR */       /* Enable wide-char support */
/*#define GFUNC_VP */        /* Enable named viewport functions xxx_vp()*/
/*#define GSCREENS*/         /* Enable screens */
/*#define GEXTMODE*/         /* Enable application specific view port data extensions */
                             /* (view port data extensions are defined in gvpapp.h) */
#define GNOCURSOR            /* Turn visual cursor handling off or on*/
                             /* Define for max speed, undefine to have cursor support */
#define GNOTXTSPACE          /* Turn extra character or line space handling off and on */
                             /* Define for max speed, undefine to have line and character spacing */
/*#define GVIRTUAL_FILES*/        /* Enable virtual "file" support (dynamic lookup) */
/*#define GVIRTUAL_FILES_STATIC*/ /* Enable static virtual "file" support (fast lookup) */

/* Tabulator definitions */
#define GCONSTTAB            /* Tab table contain constants (undefine to use variable tabs) */

#ifdef  GCONSTTAB
 /* Define const tab increments in number of pixel */
 #define GTABSIZE  (GDISPW/6)
#else
 /* variable tab tabel is used */
 #define GMAXTABS 10           /* Max. number of tab positions (defines tabulator table size) */
#endif

/* Define value for switch between normal and multi-byte string chars
   If the char is above this value then this char plus the next char is
   used to form a 16 bit wide char (ex in the range 0x8000 to 0xffff) */
#define G_MULTIBYTE_LIMIT 0x80

/* Select buffered implementation (speed optimization with
   external display RAM buffer or use direct operation on
   module RAM. Define or undefine GBUFFER */
/*#define GBUFFER*/ /* Extern buffer for data manipulation, fast */

/* If GHW_ALLOCATE_BUF is defined the graphic buffer is allocated using malloc.
   instead of using a (faster) static buffer */
#ifdef GBUFFER
  /* #define GHW_ALLOCATE_BUF */ /* Allocate buffer on heap */
#endif

/* If GWARNING is defined, illegal runtime values will cause
   issue of a display message and stop of the system.
   The soft error handler function G_WARNING(str) defined in
   gdisphw.h is used for message output.
   If undefined parameters will be forced within a legal range
   and used afterwards. */
#define GWARNING

/* If GERROR is defined, states and situations which may result
   in a fatal runtime state will cause a display message to be
   issued and the system stopped. The soft error handler function
   G_ERROR(str) defined in gdisphw.h is used for message output.
   If undefined the situation is ignored or an exit is performed. */
#define GERROR

/* If GDATACHECK is defined the internal data is checked
   for errors. Maybe some faulty part of the main code overwrites
   the internal data of the LCD driver, such and error will be
   cached with this define. Undefine for max speed. */
#define GDATACHECK

/* Variable used for X and W */
#if (GDISPW <= 255)
#define GXT   unsigned char
#else
#define GXT   unsigned short
#endif
/* Variable used for Y and H */
#if (GDISPH <= 255)
#define GYT   unsigned char
#else
#define GYT   unsigned short
#endif

#ifdef GHW_PCSIM
   /* Simulator mode only switches */
   /* Define to minimize a console application when the LCD simulator is used */
   #define GHW_MINIMIZE_CONSOLE
   /* Define to limit simulator updates to the high-level functions.
      The simulator operations is faster when defined */
   #define GHW_FAST_SIM_UPDATE
#endif

#ifndef GHW_PCSIM
   /* Memory type qualifiers only for target compilation mode (don't care in PC simulation mode) */
   /* The following defines allow you to optimize the parameter passing and to use target compiler specific
      memory allocation keywords (default will be used for PC simulator mode) */

   /* Keyword used for fast variables optimization (critical params)*/
   #define GFAST  /* nothing */
   /* Keyword used for very fast variables optimization (critical params)*/
   #define GVFAST /* nothing */
   /* Type qualifier used on pointer (parameter) to strings when the object
      is not modified by the function (part of C strong prototyping */
   #define GCONSTP const
   /* type qualifier used for fixed data (graphic tables etc) */
   #define GCODE  const
   /* Memory type qualifier used for fixed data (if GCODE setting is not enough) */
   #define FCODE  /* nothing */
   /* Memory type qualifier used for pointer to fixed data (if GCODE var * is not enough) */
   #define PFCODE /* nothing */
   /* Keyword used for generic pointers to data strings, if generic pointers is not default */
   #define PGENERIC /* nothing */
#endif

/** Color buffer handling types **/

/* Define type to hold color information for a pixel */
#if   (GDISPPIXW <= 8)
  #define GCOLOR  SGUCHAR
#elif (GDISPPIXW <= 16)
  #define GCOLOR  SGUINT
#else
  #define GCOLOR  SGULONG
#endif

/* Define integer optimized for buffer indexing and buffer size values */
#if (((GDISPPIXW > 8) && ((GDISPW*GDISPH*4) > 0xffff)) || \
     ((GDISPW * GDISPH *((GDISPPIXW+7)/8)) > 0xffff))
  #define GBUFINT SGULONG
#else
  #define GBUFINT SGUINT
#endif

#define  GHW_PALETTE_SIZE 16 /* Size of software palette */

/****************** LOW-LEVEL DRIVER CONFIGURATIONS *******************/

/* Adapt library to scan line layout selected by display module vendor */
 #define GHW_MIRROR_VER    /* Mirror the display vertically */
/*#define GHW_MIRROR_HOR*/ /* Mirror the display horizontally */
 #define GHW_XOFFSET  0    /* Set display x start offset in on-chip video ram */
 #define GHW_YOFFSET  0    /* Set display y start offset in on-chip video ram */
/*#define GHW_ROTATED */   /* Define to rotate display 90 (270) degrees (remember to swap values used in GDISPH,GDISPW definitions) */
 #define GHW_COLOR_SWAP    /* Define to change R,G,B order to B,G,R order */
 #define GHW_COMSPLIT      /* Define to used split COM line controls (SSD1355) */

/* #define GHW_INV_VDATA*/ /* Invert video data */

/* The chip does not support hardware or download fonts (do not modify) */
#define GHW_NO_HDW_FONT

/* There is no contrast regulation support (do not modify) */
#undef GHW_INTERNAL_CONTRAST

/*#define GHW_NO_LCD_READ_SUPPORT*/ /* Use reduced feature set where read-write,
                                       read-modify-write operations not allowed */

/****************** COLOR DEFINITION *******************/
/* Enable code generation for color and gray-shade support */
#define GHW_USING_COLOR
#define GHW_INVERTGRAPHIC_SYM  /* Define to accept symbols created with using 0 as black */
#define GHW_USING_RGB    /* RGB color mode is used (do not modify) */

/* Define the default colors used for text foreground and background
  G_BLACK, G_RED ,G_GREEN, G_YELLOW, G_BLUE,
  G_MAGENTA, G_CYAN, G_WHITE, G_LIGHTGRAY, or G_DARKGRAY
*/
#define GHW_PALETTE_BACKGROUND  G_WHITE
#define GHW_PALETTE_FOREGROUND  G_BLACK

/*
   Map "pure color names" to binary RGB color values
   These definitions map a symbolic color value to the corresponding RGB value
   matching the GDISPPIXW color type settings
*/
#define G_BLACK       G_RGB_TO_COLOR(0x00,0x00,0x00)
#define G_RED         G_RGB_TO_COLOR(0xff,0x00,0x00)
#define G_GREEN       G_RGB_TO_COLOR(0x00,0xff,0x00)
#define G_YELLOW      G_RGB_TO_COLOR(0xff,0xff,0x00)
#define G_BLUE        G_RGB_TO_COLOR(0x00,0x00,0xff)
#define G_MAGENTA     G_RGB_TO_COLOR(0xff,0x00,0xff)
#define G_CYAN        G_RGB_TO_COLOR(0x00,0xff,0xff)
#define G_WHITE       G_RGB_TO_COLOR(0xff,0xff,0xff)
/* Names for extended colors */
#define G_LLIGHTGREY  G_RGB_TO_COLOR(0xd0,0xd0,0xd0)
#define G_LIGHTGREY   G_RGB_TO_COLOR(0xb8,0xb8,0xb8)
#define G_GREY        G_RGB_TO_COLOR(0xa8,0xa8,0xa8)
#define G_DARKGREY    G_RGB_TO_COLOR(0x90,0x90,0x90)
#define G_DDARKGREY   G_RGB_TO_COLOR(0x78,0x78,0x78)
#define G_DARKBLUE    G_RGB_TO_COLOR(0x00,0x00,0xc0)
#define G_DARKGREEN   G_RGB_TO_COLOR(0x00,0xc0,0x00)
#define G_ORANGE      G_RGB_TO_COLOR(0xff,0x80,0x00)

/* End of gdispcfg.h */
#endif
