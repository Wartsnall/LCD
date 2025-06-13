/************************** ghwinit.c *****************************

   Low-level driver functions for the hx8346 - HX8325 LCD display controller
   initialization and error handling.

   Notes: Data read require a dummy read before first read

   The following LCD module characteristics MUST be correctly
   defined in GDISPCFG.H:

      GDISPW  Display width in pixels
      GDISPH  Display height in pixels
      GBUFFER If defined most of the functions operates on
              a memory buffer instead of the LCD hardware.
              The memory buffer content is copied to the LCD
              display with ghw_updatehw().
              (Equal to an implementation of delayed write)

   IMPORTANT: For target mode compilation the body of the
                 void ghw_cmd_wait(SGUCHAR ms);
                          function should be customized to generate a delay of
                          minimum the number of milliseconds defined by the parameter.
                          Optionally by adjusting the internal loop counter value.

   HX8346, HX8325 support these mode combinations
        16 bit bus, 16 bit color
        16 bit bus, 18 bit color
        16 bit bus, 24 bit color (HX8325 only)
        32 bit bus. 18 bit color (18 lsb bus bits are used)
        32 bit bus. 24 bit color (24 lsb bus bits are used) (HX8328 only)
         8 bit bus, 24 bit color
   The library also support use of
         8 bit bus, 16 bit color (mapping to 24 bit mode done by software)

   SSD1355 support these mode combinations
         32 bit bus. 18 bit color (18 lsb bus bits are used)
         16 bit bus, 16 bit color (only if GBUFFER & GHW_NO_LCD_READ_SUPPORT is defined)
         8 bit bus, 24 bit color
         8 bit bus, 16 bit color (mapping to hardware 24 bit mode done by software)

   ST7628  support these mode combinations
         8 bit bus, 24 bit color
         8 bit bus, 16 bit color
         16 bit bus, 16 bit color

   HX8353  support these mode combinations
         8 bit bus, 24 bit color (lsb bit reduction)
         8 bit bus, 18 bit color
         8 bit bus, 16 bit color
         16 bit bus, 16 bit color
         32 bit bus, 18 bit color

   ILI9193  support these mode combinations
         8 bit bus, 24 bit color (lsb bit reduction)
         8 bit bus, 18 bit color
         8 bit bus, 16 bit color
         16 bit bus, 16 bit color
         32 bit bus, 18 bit color

   ST7735  support these mode combinations
         8 bit bus, 18(24) bit color (lsb bit reduction)
         8 bit bus, 16 bit color
         16 bit bus, 16 bit color
         32 bit bus, 18 bit color

   Revision date:    03-09-07
   Revision Purpose: Minor optimizations. Corrections to 18 & 24 bit-pr-pixel modes

   Revision data:    17-03-09
   Revision Purpose: SSD1355 and ST7628 support added
                     SSD1355 support  8 bit bus, 16 + 24 bit color (via softare conv)
                                     16 bit bus, 16 bit color in non read mode
                                     32 bit bus, 18 bit color

   Revision date:       07-05-09
   Revision Purpose:    The symbol software palette (data and functions) can
                        be optimized away if not used by defining
                        GHW_PALETTE_SIZE as 0 in gdispcfg.h
   Revision data:       20-07-09
   Revision Purpose:    HX8353 support added
   Revision data:       16-09-09
   Revision Purpose:    Correction to 18 bit pixel, 8 bit data bus mode read
   Revision data:       16-09-09
   Revision Purpose:    Bug corrected for ghw_auto_rd() in ST7628 8 bit bus, 16 bit color mode
   Revision data:       25-06-10
   Revision Purpose:    ILI9163 support added
   Revision date:       11-11-10
   Revision Purpose:    ghw_rgb_to_color(..) updated to use G_RGB_TO_COLOR(r,g,b) macro.
   Revision data:       14-01-11
   Revision Purpose:    ILI9163 driver config and timing simplified to accommodate ST7735 support.
   Revision data:       25-08-11
   Revision Purpose:    HX8347G support added. New switch GHW_HX8347G
   Revision data:       19-01-12
   Revision Purpose:    Minor optimizations. HX8353 support for both 24 bit and 16 bit mode.
   Revision data:       20-01-12
   Revision Purpose:    HX8352B support added. New switch GHW_HX8352B  (incl HX8367 support)
                        HX8353D support added. New switch GHW_HX8353D_CMDINTF
   Revision data:       13-01-12
   Revision Purpose:    HX8347G read in 16 bit mode corrected.
   Revision data:       10-02-14
   Revision Purpose:    ST7773 + ST7789C supported via simplified ILI9363 config.
   Revision data:       16-05-14
   Revision Purpose:    Workaround for ILI9341V 16-bit read mode (ILI9341V behave like ILI9163
                        except for read in 16 bit parallel bus mode returns two RGB pixels
                        in 3x16 bit read unit
   Revision data:       25-09-14
   Revision Purpose:    HX8369 support added. ghw_auto_wr_end() instroduced.
   Revision data:       26-02-16
   Revision Purpose:    ILI9488 (ILI9486L, ILI8327) supported added
   Revision data:       24-04-17
   Revision Purpose:    ILI9488 adjustments

   Version number: 1.8
   Copyright (c) RAMTEX Engineering Aps 2007-2017

*********************************************************************/

#ifdef SGPCMODE
#include "windows.h"  /* Sleep(..) function used by ghw_cmd_wait(..)*/
#endif

#include <gdisphw.h>  /* HW driver prototypes and types */
#include <s6d0129.h>  /* Controller specific definements */

//#define WR_RD_TEST    /* Define to include write-read-back test in ghw_init() */

/* Mirror and rotation definition bits (to simplify initialization below) */
#ifdef GHW_COLOR_SWAP
   #define  RGB_BIT 0x08
#else
   #define  RGB_BIT 0x00
#endif
#ifdef GHW_MIRROR_HOR
   #define  MX_BIT 0x40
#else
   #define  MX_BIT 0x00
#endif
#ifdef GHW_MIRROR_VER
   #define  MY_BIT 0x80
#else
   #define  MY_BIT 0x00
#endif
#ifdef GHW_ROTATED
   #define  MV_BIT 0x20
#else
   #define  MV_BIT 0x00
#endif

#ifdef GHW_BUS32
   #define BUSMODE 0x30
#elif defined(GHW_BUS16)
   #define BUSMODE 0x10
#else
   #define BUSMODE 0x00
#endif

#ifdef GHW_COMSPLIT
  #define COMSPLIT 0x01
#else
  #define COMSPLIT 0x00
#endif

/* Special versions */
#ifdef GHW_ILI9341V
  /* ILI9341V behave like GHW_ILI9163 except for 16 bit readback*/
  #define GHW_ILI9163
#endif

/* HX8347 is equal to HX8346 except for it is using an 8 bit read sequence in 16 bit bus mode
   so here reuse most of the preprocessor definitions */
#ifdef GHW_HX8347A_REGINTF
  #ifndef GHW_HX8346_REGINTF
    #define GHW_HX8346_REGINTF
  #endif
#elif defined( GHW_HX8347A_CMDINTF )
  #ifndef GHW_HX8346_CMDINTF
    #define GHW_HX8346_CMDINTF
  #endif
#endif

#if (defined( GHW_HX8346_REGINTF ) || defined( GHW_HX8347G ) || defined (GHW_HX8352B ) || defined( GHW_HX8325_REGINTF ))

   /* Display control registers for control of primary drawing operations */
   #define  GCTRL_DISPMODE         0x01
   #define  GCTRL_H_WIN_ADR_STRT_H 0x02  /* begin */
   #define  GCTRL_H_WIN_ADR_STRT_L 0x03  /* begin */
   #define  GCTRL_H_WIN_ADR_END_H  0x04  /* end */
   #define  GCTRL_H_WIN_ADR_END_L  0x05  /* end */
   #define  GCTRL_V_WIN_ADR_STRT_H 0x06  /* begin */
   #define  GCTRL_V_WIN_ADR_STRT_L 0x07  /* begin */
   #define  GCTRL_V_WIN_ADR_END_H  0x08  /* end */
   #define  GCTRL_V_WIN_ADR_END_L  0x09  /* end */

   #define  GCTRL_MAD_CTRL         0x16  /* MY,MX,MV,ML,BGR,SS,0,0 */
   #ifdef GHW_HX8352B
    #define  GCTRL_DISP_CTRL3      0x28  /* -,-,GON,DTE,D1,D0,-,- */
    #define  GCTRL_COL_MSB         0x80  /* 0001 0000b   b[2:0] = CA[6:4] ,column msb */
    #define  GCTRL_COL_LSB         0x81  /* 0000 0000b   b[3:0] = CA[3:0] ,column lsb */
    #define  GCTRL_ROW_MSB         0x82  /* 0111 0000b   b[2:0] = PA[6:4] ,set page address */
    #define  GCTRL_ROW_LSB         0x83  /* 0110 0000b   b[3:0] = PA[3:0] ,set page address */
   #elif defined( GHW_HX8347G )
    #define  GCTRL_DISP_CTRL3      0x28  /* -,-,GON,DTE,D1,D0,-,- */
   #else
    #define  GCTRL_DISP_CTRL1      0x26  /* PT1,PT0,GON,DTE,D1,D0,-,- */
   #endif

   #define  GCTRL_RAMWR            0x22  /* Set or get GRAM data */
   #define  GCTRL_RAMRD            0x22  /* Set or get GRAM data */

#elif (defined( GHW_HX8346_CMDINTF ) || defined( GHW_HX8325_CMDINTF ) || \
       defined( GHW_HX8353_CMDINTF ) || defined( GHW_HX8353D_CMDINTF ) || \
       defined( GHW_SSD1355 ) || defined( GHW_ST7628 ) || \
       defined( GHW_ILI9163 ) || defined( GHW_HX8369) || defined(GHW_ILI9488))

   /* Display control registers for control of primary drawing operations */
   #define GCTRL_DISPOFF   0x28
   #define GCTRL_DISPON    0x29
   #define GCTRL_CASET     0x2A
   #define GCTRL_RASET     0x2B
   #define GCTRL_RAMWR     0x2C
   #define GCTRL_RAMRD     0x2E

   /* Display control registers for initialization only */
   #define GCTRL_RESET     0x01
   #define GCTRL_SLPIN     0x10
   #define GCTRL_SLPOUT    0x11
   #define GCTRL_NORON     0x13
   #define GCTRL_INVON     0x21
   #define GCTRL_GAMSET    0x26
   #define GCTRL_COLORSET  0x2D
   #define GCTRL_MADCTRL   0x36
   #define GCTRL_COLMOD    0x3A

   #if (defined( GHW_HX8353_CMDINTF) || defined( GHW_HX8353D_CMDINTF))
   #define GCTRL_IDMOFF    0x38
   #else
   #define GCTRL_SETOSC    0xB0
   #define GCTRL_SETPOWER  0xB1
   #define GCTRL_SETDISP   0xB2
   #define GCTRL_SETCYC    0xB4
   #define GCTRL_SETVCOM   0xB6
   #endif

#else
  #error Unknown controller, Controller and bustype must be selected in gdispcfg.h
#endif

/*
   Define pixel width, height of internal video memory ( only used for the overflow check below)
   Note: If another display controller variant is used the adjust the GCTRLW, GCTRLH definitions
         below accordingly to match the size of the pixel video RAM in the controller.
   Note: If the physical memory range limits are exceeded at runtime then some controllers stop working.
*/
   /* HX8312 (240x320) (208x320)  (180x320) */

/* Fix missing definitions in gdispcfg.h */
#ifndef GHW_XOFFSET
   #define GHW_XOFFSET 0
#endif
#ifndef GHW_YOFFSET
   #define GHW_YOFFSET 0
#endif

/* Set size of active internal RAM, Check display size settings */
#ifdef GHW_ROTATED
   /* Map and swap offsets */
  #define G_XOFFSET GHW_YOFFSET
  #define G_YOFFSET GHW_XOFFSET
#else
  /* Map offsets */
  #define G_XOFFSET GHW_YOFFSET
  #define G_YOFFSET GHW_XOFFSET
#endif

/********************* Chip access definitions *********************/

#ifndef GHW_NOHDW
   #if defined( GHW_SINGLE_CHIP)
      /* User defined access types and simulated register address def */
      #include <bussim.h>
      #ifdef GHW_BUS8
        #define  sgwrby(a,d) simwrby((a),(d))
        #define  sgrdby(a)   simrdby((a))
      #elif defined (GHW_BUS32)
        #define  sgwrdw(a,d) simwrdw((a),(d))
        #define  sgrddw(a)   simrddw((a))
      #else /* bus 16 */
        #define  sgwrwo(a,d) simwrwo((a),(d))
        #define  sgrdwo(a)   simrdwo((a))
      #endif
   #else
      /* Portable I/O functions + hardware port def */
      #include <sgio.h>
   #endif
#else
   #undef GHW_SINGLE_CHIP /* Ignore single chip mode */
#endif

/***********************************************************************/
/** All static LCD driver data is located here in this ghwinit module **/
/***********************************************************************/

#ifdef GBASIC_INIT_ERR

/* Active foreground and background color */
GCOLOR ghw_def_foreground;
GCOLOR ghw_def_background;

#if (GHW_PALETTE_SIZE > 0)
/* Default soft palette
   The palette file can be edited directly with the ColorIconEdit program
*/
static GCODE GPALETTE_RGB FCODE ghw_palette[16] =
     #include <gcolor_4.pal>
     ;

/* Operative palette (current palette used for color lookup) */
GCOLOR ghw_palette_opr[16];
#endif

/* Use software font */
static struct
   {
   GSYMHEAD sh;        /* Symbol header */
   SGUCHAR  b[8];           /* Symbol data, fixed size = 8 bytes */
   }
GCODE FCODE sysfontsym[0x80] =
   {
   /* The default font MUST be a monospaced black & white (two-color) font */
   #include <sfs0129.sym> /* System font symbol table */
   };

/* Default system font */
GCODE GFONT FCODE SYSFONT =
   {
   6,      /* width */
   8,      /* height */
   sizeof(sysfontsym[0])-sizeof(GSYMHEAD), /* number of data bytes in a symbol (including any alignment padding)*/
   (PGSYMBOL) sysfontsym,  /* pointer to array of SYMBOLS */
   0x80,   /* num symbols in sysfontsym[] */
   NULL    /* pointer to code page */ /* NULL means code page is not used */
   };

#ifdef GBUFFER
   #ifdef GHW_ALLOCATE_BUF
      /* <stdlib.h> is included via gdisphw.h */
      GCOLOR *gbuf = NULL;           /* Graphic buffer pointer */
      static SGBOOL gbuf_owner = 0;   /* Identify pointer ownership */
   #else
      GCOLOR gbuf[GBUFSIZE];         /* Graphic buffer */
   #endif
   GXT GFAST iltx,irbx;     /* "Dirty area" speed optimizers in buffered mode */
   GYT GFAST ilty,irby;
   SGBOOL  ghw_upddelay;    /* Flag for delayed update */
#else
   GCOLOR ghw_tmpbuf[GDISPW]; /* Row line buffer (for block read-modify-write) */
#endif /* GBUFFER */


#ifdef GHW_INTERNAL_CONTRAST
static SGUCHAR ghw_contrast;/* Current contrast value */
#endif

SGBOOL glcd_err;            /* Internal error */
#ifndef GNOCURSOR
GCURSOR ghw_cursor;         /* Current cursor state */
#endif

#ifdef GHW_PCSIM
/* PC simulator declaration */
void ghw_init_sim( SGUINT dispw, SGUINT disph );
void ghw_exit_sim(void);
void ghw_set_xyrange_sim(GXT xb, GYT yb, GXT xe, GYT ye);
void ghw_set_xy_sim(GXT xb, GYT yb);
void ghw_autowr_sim( GCOLOR cval );
GCOLOR ghw_autord_sim( void );
void ghw_dispon_sim( void );
void ghw_dispoff_sim( void );
#endif
/***********************************************************************/
/** Low level hx8312 interface functions used only by ghw_xxx modules **/
/***********************************************************************/

/* Bit mask values */
GCODE SGUCHAR FCODE sympixmsk[8] = {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};

typedef struct
   {
   SGUCHAR index;
   SGUCHAR delay;
   SGUCHAR value;
   } S1D_REGS;

#if (defined( GHW_HX8346_REGINTF ) || defined( GHW_HX8325_REGINTF ))
/************************ Use register index interface *****************/

/* Array of configuration descriptors, the registers are initialized in the order given in the table */
static GCODE S1D_REGS FCODE as1dregs[] =
   {
   /*
   The initialization sequence below is suitable for most display modules
   configured for register index mode  (IFSEL1,IFSEL0 chip pin setting.
   The interface mode is often preselected by the display module vendor

   If another initialization sequence is recommended by a display vendor
   for a specific display module, then you may replace the initialization
   sequence below with the recommended setting.

   It works in this way:
   Each line will initialize one configuration register with a value and
   optionally introduce a delay after the intialization:
      { register index,  delay (0 = no delay), register value},

   For a detailed explanation of each register the hardware manual for
   the display controller should be consulted.
   */
   /* Gamma for CMO 2.2 (better grey-scale) */
   {0x46,  0,0x22},
   {0x47,  0,0x22},
   {0x48,  0,0x20},
   {0x49,  0,0x35},
   {0x4A,  0,0x00},
   {0x4B,  0,0x77},
   {0x4C,  0,0x24},
   {0x4D,  0,0x75},
   {0x4E,  0,0x12},
   {0x4F,  0,0x28},
   {0x50,  0,0x24},
   {0x51,  0,0x44},

   /* Oscillator on */
   {0x19, 10,0x41}, /* OSCADJ=100 010, OSD_EN=1 */

   /* Display Setting */
   #ifdef GHW_INV_VDATA
   {GCTRL_DISPMODE,  0,0x02}, /* IDMON=0, INVON=0, NORON=1, PTLON=0 */
   #else
   {GCTRL_DISPMODE,  0,0x06}, /* IDMON=0, INVON=1, NORON=1, PTLON=0 */
   #endif
   {GCTRL_MAD_CTRL, 0, (MY_BIT|MX_BIT|MV_BIT|RGB_BIT)}, /*MY, MX, MV, ML, BGR */

   /* Set disp */
   /* {0x28,  0,0x02},  N_BP=0000 0010 */
   /* {0x29,  0,0x02},  N_FP=0000 0010 */
   /* {0x2A,  0,0x02},  P_BP=0000 0010 */
   /* {0x2B,  0,0x02},  P_FP=0000 0010 */
   /* {0x2C,  0,0x02},  I_BP=0000 0010 */
   /* {0x2D,  0,0x02},  I_FP=0000 0010 */
   {0x30, 40,0x08},  /* SAPS1=1000 */

   /* Set syc */
   {0x3A,  0,0x01},  /* N_RTN=0000, N_NW=001 */
   {0x3B,  0,0x01},  /* P_RTN=0000, P_NW=001 */
   {0x3C,  0,0xF0},  /* I_RTN=1111, I_NW=000 */
   {0x3D, 20,0x00},  /* DIV=00 */

   /* Vcom */
   {0x43,  0,0x80}, /* VCOMG=1 */
   {0x44,  0,0x20}, /* VCM=010 0000 */
   {0x45, 10,0x0E}, /* VDV=0 1011 */

   /* Power Supply Setting */
/* {0x1B, 40,0x18},    NIDSENB=0, PON=1, DK=1, XDK=0, */
   {0x1B,  0,0x10}, /* NIDSENB=0, PON=1, DK=0, XDK=0, */
   {0x1C,  0,0x04}, /* AP=110 */
   {0x1D,  0,0x47}, /* VC2=100, VC1=001 */
   {0x1E,  0,0x00}, /* VC3=000 */
   {0x1F,  0,0x03}, /* VRH=1101 */
   {0x20,  0,0x40}, /* BT=1000 */
   {0x23,  0,0x95}, /* N_DC=1001 0101 */
   {0x24,  0,0x95}, /* P_DC=1001 0101 */
   {0x25, 40,0xFF}, /* I_DC=1111 1111 */

   /* VLCD_TRI=0,  0, STB=0 */
   /* VLCD_TRI=1,  0, STB=0 */
   /* Display ON Setting */
   {0x26, 40,0x04}, /*GON=0, DTE=0, D=01 */
   {0x26,  0,0x24}, /*GON=1, DTE=0, D=01 */
   {0x26, 40,0x2C}, /*GON=1, DTE=0, D=11 */
   {0x26,  0,0x3C}, /*GON=1, DTE=1, D=11 */

   /* Fixed the read data issue */
   {0x57,  0,0x02}, /* TEST_Mode=1: into TEST mode */
   {0x56,  0,0x7B}, /* tune the memory timing */
   {0x57,  0,0x00}  /* TEST_Mode=0: exit TEST mode */
   };

#elif defined( GHW_HX8347G )
   /* NOTE HX83467G uses a different register layout than HX8346 / HX8347 */

static GCODE S1D_REGS FCODE as1dregs[] =
   {
   {0x2E,0,0x7F}, /*GDOFF*/
   {0xEA,0,0x00}, /*PTBA[15:8]*/
   {0xEB,0,0x20}, /*PTBA[7:0]*/
   {0xEC,0,0x3C}, /*STBA[15:8]*/
   {0xED,0,0xC4}, /*STBA[7:0]*/
   {0xE8,0,0x50}, /*OPON[7:0]*/
   {0xE9,0,0x38}, /*OPON1[7:0]*/
   {0xF1,0,0x01}, /*OTPS1B*/
   {0xF2,0,0x08}, /*GEN*/
   /*Gamma 2.2 Setting*/
   /*
   {0x40,0,0x01},
   {0x41,0,0x1F},
   {0x42,0,0x25},
   {0x43,0,0x22},
   {0x44,0,0x1E},
   {0x45,0,0x26},
   {0x46,0,0x35},
   {0x47,0,0x75},
   {0x48,0,0x06},
   {0x49,0,0x14},
   {0x4A,0,0x19},
   {0x4B,0,0x1A},
   {0x4C,0,0x17},
   {0x50,0,0x19},
   {0x51,0,0x21},
   {0x52,0,0x1D},
   {0x53,0,0x1A},
   {0x54,0,0x20},
   {0x55,0,0x3E},
   {0x56,0,0x0A},
   {0x57,0,0x4A},
   {0x58,0,0x08},
   {0x59,0,0x05},
   {0x5A,0,0x06},
   {0x5B,0,0x0B},
   {0x5C,0,0x19},
   {0x5D,0,0xCC},
   */
   /*Power Voltage Setting*/
   {0x1B,0,0x1B}, /*VRH=4.65V*/
   {0x1A,0,0x05}, /*BT (VGH~12V,VGL~-7V,DDVDH~5V)*/
   {0x24,0,0x70}, /*VMH(VCOM High voltage ~4.2V)*/
   {0x25,0,0x58}, /*VML(VCOM Low voltage -1.2V)*/
   /*****VCOM offset**/
   {0x23,0,0x7E}, /*for Flicker adjust */
   /*Power on Setting*/
   {0x18,0,0x36}, /*I/P_RADJ,N/P_RADJ, Normal mode 70Hz*/
   {0x19,0,0x01}, /*OSC_EN='1', start Osc*/
   {0x01,0,0x00}, /*DP_STB='0', out deep sleep*/
   {0x1F,5,0x88}, /* GAS=1, VOMG=00, PON=0, DK=1, XDK=0, DVDH_TRI=0, STB=0*/
   {0x1F,5,0x80}, /* GAS=1, VOMG=00, PON=0, DK=0, XDK=0, DVDH_TRI=0, STB=0*/
   {0x1F,5,0x90}, /* GAS=1, VOMG=00, PON=1, DK=0, XDK=0, DVDH_TRI=0, STB=0*/
   {0x1F,5,0xD0}, /* GAS=1, VOMG=10, PON=1, DK=0, XDK=0, DDVDH_TRI=0, STB=0*/

   #if (defined( GHW_BUS8 ) && ((GDISPPIXW == 24) || (GDISPPIXW == 18)))
   {0x17,0,0x06}, /* 262k color (18=6+6+6)*/
   #elif (defined( GHW_BUS8 ) &&  (GDISPPIXW == 16))
   {0x17,0,0x05}, /* 65k color*/
   #elif (defined( GHW_BUS16 ) && (GDISPPIXW == 16))
   {0x17,0,0x05}, /* 65k color (16)*/
   #elif (defined( GHW_BUS16 ) && (GDISPPIXW == 18))
   {0x17,0,0x07}, /* 262k color (18=16+2)*/
   #elif (defined( GHW_BUS32 ) && (GDISPPIXW == 18))
   {0x17,0,0x06}, /* 262k color (18)*/
   #else
     #error GHW_BUSx GDISPPIXW combination is not supported with HX8347G
   #endif

   {GCTRL_MAD_CTRL,  0,(MY_BIT|MX_BIT|MV_BIT|RGB_BIT)}, /*MY, MX, MV, ML, BGR */

   {0x36,0,0x09}, /*SS_P, GS_P,REV_P,BGR_P*/
   /* Disp on (if these two lines are commented out, dispon is done after buffer clear) */
   {0x28,40,0x38}, /*GON=1, DTE=1, D=1000*/
   {0x28,0,0x3C},  /*GON=1, DTE=1, D=1100*/
   /* Display Setting */
   #ifdef GHW_INV_VDATA
   {GCTRL_DISPMODE,  0,0x02}
   #else
   {GCTRL_DISPMODE,  0,0x00}
   #endif
   };

#elif (defined( GHW_HX8346_CMDINTF ) || defined( GHW_HX8325_CMDINTF ))
/************************ Use command interface *****************/

#define CMD 0
#define DAT 1

static GCODE S1D_REGS FCODE as1dregs[] =
   {
   /*
   The initialization sequence below is suitable for most display modules
   configured for command interface mode  (IFSEL1,IFSEL0 chip pin setting).
   The interface mode is often preselected by the display module vendor

   If another initialization sequence is recommended by a display vendor
   for a specific display module, then you may replace the initialization
   sequence below with the recommended setting.

   It works in this way:
   Each line will send one configuration byte to the display module and
   optionally introduce a delay after the intialization. The first parameter
   selects if the byte should be written to the controllers command register
   or data register address.
      { cmd/data select,  delay (0 = no delay), register value},

   For a detailed explanation of each command and datathe hardware manual for
   the display controller should be consulted.
   */
   {CMD,100,GCTRL_RESET},     /* 0x01 Reset */
   {CMD,150,GCTRL_SLPOUT},    /* 0x11 Sleep out */

   {CMD,  0,GCTRL_SETOSC},   /* 0xB0 Set Internal Oscillator */
   {DAT,  0,0x07},           /*97 */
   {DAT,150,0x00},           /*00 */

   {CMD,  0,GCTRL_NORON},     /* 0x13 Normal Display Mode On */
   {CMD,  0,GCTRL_INVON},     /* 0x21 display inversion on */
   {CMD, 50,GCTRL_DISPON},    /* 0x29 Display On */

   {CMD,  0,GCTRL_MADCTRL},   /*0x36 Memory Access Control (mirroring) */
   {DAT,  0,(MY_BIT|MX_BIT|MV_BIT|RGB_BIT)},   /* MY,MX,MV,ML,BGR,SS,0,0 */

   {CMD,  0,GCTRL_GAMSET},    /* Gamma setting */
   {DAT,  0,0x01},            /*80 */

   {CMD,  0,GCTRL_SETVCOM},   /* 0xB6 SETVCOM */
   {DAT,  0,0x80},     /*80 */
   {DAT,  0,0x40},     /*00~75H  VCOMH  30 */
   {DAT, 50,0x13},     /* 00~26H Vcom amplitude    0eh */

   {CMD,  0,GCTRL_SETCYC},     /* 0xB4 SETCYC */
   {DAT,  0,0x01},    /*01 default */
   {DAT,  0,0x01},    /*01 default */
   {DAT,  0,0x01},    /*01 default = 00 */
   {DAT,  0,0x00},    /*00 default */
   {DAT,  0,0x1d},    /*1d default = 38 */
   {DAT,  0,0x3f},    /*3f default = 03 */
   {DAT, 30,0x3f},    /*3f default = f8 */

   {CMD,  0,GCTRL_SETDISP},     /* 0xB2 SETDISP */
   {DAT,  0,0x3C},    /*3c default =20 */
   {DAT,  0,0x02},    /* default */
   {DAT,  0,0x02},    /* default */
   {DAT,  0,0x02},    /* default */
   {DAT,  0,0x02},    /* default */
   {DAT,  0,0x02},    /* default */
   {DAT,  0,0x02},    /* default */

   {DAT,  0,0x00},    /*00  Himax internal use */
   {DAT,  0,0x08},    /*08 */
   {DAT,  0,0x08},    /*08 */
   {DAT,  0,0xB0},    /*b0 */
   {DAT,  0,0x00},    /*00 */
   {DAT,  0,0x00},    /*00 */
   {DAT,  0,0x00},    /*00 */
   {DAT,  0,0x00},    /*00 */
   {DAT, 40,0x01},    /*01 */

   {CMD,  0,GCTRL_SETPOWER}, /*0xB1 SETPOWER */
   {DAT,  0,0x10},    /* default = 09 */
   {DAT,  0,0x04},    /* default */
   {DAT,  0,0x00},    /* default */
   {DAT,  0,0x06},    /* default = 05 */
   {DAT,  0,0x08},    /* default = 0,  Vreg1 =00~0E    3.9VBGP */
   {DAT,  0,0x00},    /* default =40, 00 */
   {DAT,  0,0x40},    /* default =30, 10      00~90 */
   {DAT,  0,0x20},    /* default = be 10 */
   {DAT,  0,0x00},    /* default = be 00 */
   {DAT,  0,0x00},    /* default = be 00 */

/*=================65k color================== */
   /* Gamma configuration (if different from linear) */
   {CMD,  0,GCTRL_COLORSET},  /* 0x2d */
   {DAT,  0,0},   /* 1 */
   {DAT,  0,4},   /* 2 */
   {DAT,  0,8},   /*   */
   {DAT,  0,16},  /* 3 */
   {DAT,  0,20},  /*   */
   {DAT,  0,24},  /* 4 */
   {DAT,  0,28},  /*   */
   {DAT,  0,32},  /* 5 */
   {DAT,  0,36},  /*   */
   {DAT,  0,40},  /* 6 */
   {DAT,  0,44},  /*   */
   {DAT,  0,48},  /* 7 */
   {DAT,  0,52},  /*   */
   {DAT,  0,56},  /* 8 */
   {DAT,  0,60},  /*   */
   {DAT,  0,64},  /* 9 */

   {DAT,  0,68},  /*   */
   {DAT,  0,72},  /* 10 */
   {DAT,  0,76},  /*   */
   {DAT,  0,80},  /* 11 */
   {DAT,  0,84},  /*   */
   {DAT,  0,88},  /* 12 */
   {DAT,  0,92},  /*   */
   {DAT,  0,96},  /* 13 */
   {DAT,  0,100}, /*   */
   {DAT,  0,104}, /* 14 */
   {DAT,  0,108}, /*   */
   {DAT,  0,112}, /* 15 */
   {DAT,  0,116}, /*   */
   {DAT,  0,120}, /* 16 */
   {DAT,  0,124}, /*   */
   {DAT,  0,128}, /* 17 */

   {DAT,  0,132}, /*   */
   {DAT,  0,136}, /* 18 */
   {DAT,  0,140}, /*   */
   {DAT,  0,144}, /* 19 */
   {DAT,  0,148}, /*   */
   {DAT,  0,152}, /* 20 */
   {DAT,  0,156}, /*   */
   {DAT,  0,160}, /* 21 */
   {DAT,  0,164}, /*   */
   {DAT,  0,168}, /* 22 */
   {DAT,  0,172}, /*   */
   {DAT,  0,176}, /* 23 */
   {DAT,  0,180}, /*   */
   {DAT,  0,184}, /* 24 */
   {DAT,  0,188}, /*   */
   {DAT,  0,192}, /*   */

   {DAT,  0,196}, /* 25 */
   {DAT,  0,200}, /*   */
   {DAT,  0,204}, /* 26 */
   {DAT,  0,208}, /*   */
   {DAT,  0,212}, /* 27 */
   {DAT,  0,216}, /*   */
   {DAT,  0,220}, /* 28 */
   {DAT,  0,224}, /*   */
   {DAT,  0,228}, /* 29 */
   {DAT,  0,232}, /*   */
   {DAT,  0,236}, /*   */
   {DAT,  0,240}, /* 30 */
   {DAT,  0,244}, /*   */
   {DAT,  0,248}, /* 31 */
   {DAT,  0,252}, /*   */
   {DAT,  0,255}, /* 32 */

   /*----------green-------- */
   {DAT,  0,0},   /* 1 */
   {DAT,  0,4},   /* 2 */
   {DAT,  0,8},   /*   */
   {DAT,  0,16},  /* 3 */
   {DAT,  0,20},  /*   */
   {DAT,  0,24},  /* 4 */
   {DAT,  0,28},  /*   */
   {DAT,  0,32},  /* 5 */
   {DAT,  0,36},  /*   */
   {DAT,  0,40},  /* 6 */
   {DAT,  0,44},  /*   */
   {DAT,  0,48},  /* 7 */
   {DAT,  0,52},  /*   */
   {DAT,  0,56},  /* 8 */
   {DAT,  0,60},  /*   */
   {DAT,  0,64},  /* 9 */

   {DAT,  0,68},  /*   */
   {DAT,  0,72},  /* 10 */
   {DAT,  0,76},  /*   */
   {DAT,  0,80},  /* 11 */
   {DAT,  0,84},  /*   */
   {DAT,  0,88},  /* 12 */
   {DAT,  0,92},  /*   */
   {DAT,  0,96},  /* 13 */
   {DAT,  0,100}, /*   */
   {DAT,  0,104}, /* 14 */
   {DAT,  0,108}, /*   */
   {DAT,  0,112}, /* 15 */
   {DAT,  0,116}, /*   */
   {DAT,  0,120}, /* 16 */
   {DAT,  0,124}, /*   */
   {DAT,  0,128}, /* 17 */

   {DAT,  0,132}, /*   */
   {DAT,  0,136}, /* 18 */
   {DAT,  0,140}, /*   */
   {DAT,  0,144}, /* 19 */
   {DAT,  0,148}, /*   */
   {DAT,  0,152}, /* 20 */
   {DAT,  0,156}, /*   */
   {DAT,  0,160}, /* 21 */
   {DAT,  0,164}, /*   */
   {DAT,  0,168}, /* 22 */
   {DAT,  0,172}, /*   */
   {DAT,  0,176}, /* 23 */
   {DAT,  0,180}, /*   */
   {DAT,  0,184}, /* 24 */
   {DAT,  0,188}, /*   */
   {DAT,  0,192}, /*   */

   {DAT,  0,196}, /* 25 */
   {DAT,  0,200}, /*   */
   {DAT,  0,204}, /* 26 */
   {DAT,  0,208}, /*   */
   {DAT,  0,212}, /* 27 */
   {DAT,  0,216}, /*   */
   {DAT,  0,220}, /* 28 */
   {DAT,  0,224}, /*   */
   {DAT,  0,228}, /* 29 */
   {DAT,  0,232}, /*   */
   {DAT,  0,236}, /*   */
   {DAT,  0,240}, /* 30 */
   {DAT,  0,244}, /*   */
   {DAT,  0,248}, /* 31 */
   {DAT,  0,252}, /*   */
   {DAT,  0,255}, /* 32 */

/*-----------blue----------------- */
   {DAT,  0,0},   /* 1 */
   {DAT,  0,4},   /* 2 */
   {DAT,  0,8},   /*   */
   {DAT,  0,16},  /* 3 */
   {DAT,  0,20},  /*   */
   {DAT,  0,24},  /* 4 */
   {DAT,  0,28},  /*   */
   {DAT,  0,32},  /* 5 */
   {DAT,  0,36},  /*   */
   {DAT,  0,40},  /* 6 */
   {DAT,  0,44},  /*   */
   {DAT,  0,48},  /* 7 */
   {DAT,  0,52},  /*   */
   {DAT,  0,56},  /* 8 */
   {DAT,  0,60},  /*   */
   {DAT,  0,64},  /* 9 */

   {DAT,  0,68},  /*   */
   {DAT,  0,72},  /* 10 */
   {DAT,  0,76},  /*   */
   {DAT,  0,80},  /* 11 */
   {DAT,  0,84},  /*   */
   {DAT,  0,88},  /* 12 */
   {DAT,  0,92},  /*   */
   {DAT,  0,96},  /* 13 */
   {DAT,  0,100}, /*   */
   {DAT,  0,104}, /* 14 */
   {DAT,  0,108}, /*   */
   {DAT,  0,112}, /* 15 */
   {DAT,  0,116}, /*   */
   {DAT,  0,120}, /* 16 */
   {DAT,  0,124}, /*   */
   {DAT,  0,128}, /* 17 */

   {DAT,  0,132}, /*   */
   {DAT,  0,136}, /* 18 */
   {DAT,  0,140}, /*   */
   {DAT,  0,144}, /* 19 */
   {DAT,  0,148}, /*   */
   {DAT,  0,152}, /* 20 */
   {DAT,  0,156}, /*   */
   {DAT,  0,160}, /* 21 */
   {DAT,  0,164}, /*   */
   {DAT,  0,168}, /* 22 */
   {DAT,  0,172}, /*   */
   {DAT,  0,176}, /* 23 */
   {DAT,  0,180}, /*   */
   {DAT,  0,184}, /* 24 */
   {DAT,  0,188}, /*   */
   {DAT,  0,192}, /*   */

   {DAT,  0,196}, /* 25 */
   {DAT,  0,200}, /*   */
   {DAT,  0,204}, /* 26 */
   {DAT,  0,208}, /*   */
   {DAT,  0,212}, /* 27 */
   {DAT,  0,216}, /*   */
   {DAT,  0,220}, /* 28 */
   {DAT,  0,224}, /*   */
   {DAT,  0,228}, /* 29 */
   {DAT,  0,232}, /*   */
   {DAT,  0,236}, /*   */
   {DAT,  0,240}, /* 30 */
   {DAT,  0,244}, /*   */
   {DAT,  0,248}, /* 31 */
   {DAT,  0,252}, /*   */
   {DAT,  0,255}, /* 32 */

   {CMD, 0,GCTRL_RAMWR}     /*memory write */
   };

#elif defined( GHW_HX8352B )

static GCODE S1D_REGS FCODE as1dregs[] =
   {
   /*
   The initialization sequence below is sets the basic configurations.

   If the display controller OTP is preconfigured during display module
   production test (ex gamma correction table and power settings) then
   the basic command set below should be enough.

   However if another initialization sequence is recommended by a display vendor
   for a specific display module, then you may replace the initialization
   sequence below with the recommended setting.

   It works in this way:
   Each line will send one configuration byte to the display module and
   optionally introduce a delay after the intialization. The first parameter
   selects the configuration register to update.
      { register index,  delay (0 = no delay), register value },

   Note: For the library only requirement is that the GCTRL_MAP_CTRL command
   settings tracks the gdispcfg.h mirroring and rotation settings.
   All the rest of the commands can be freely adapted to a specific display
   module from a specific vendor without impact on library functionality.

   For a detailed explanation of each command and data the hardware manual for
   the display controller should be consulted.
   */
   #if (defined( GHW_BUS8 ) && ((GDISPPIXW == 24) || (GDISPPIXW == 18)))
   {0x17,0,0x06}, /* 262k color (18=6+6+6)*/
   #elif (defined( GHW_BUS8 ) &&  (GDISPPIXW == 16))
   {0x17,0,0x05}, /* 65k color (wr: 8+8, rd: 6+6+6 ) */
   #elif (defined( GHW_BUS16 ) && (GDISPPIXW == 16))
   {0x17,0,0x05}, /* 65k color (16) rd: (6+6)+(6+6)+(6+6)*/
   #elif (defined( GHW_BUS16 ) && (GDISPPIXW == 18))
   {0x17,0,0x07}, /* 262k color (18=16+2) rd: (6+6)+(6+6)+(6+6)*/
   #elif (defined( GHW_BUS32 ) && (GDISPPIXW == 18))
   {0x17,0,0x06}, /* 262k color (18) rd: (6+6)+(6+6)+(6+6) */
   #else
     #error GHW_BUSx GDISPPIXW combination is not supported with HX8352B
   #endif

   {GCTRL_MAD_CTRL,  0,(MY_BIT|MX_BIT|MV_BIT|RGB_BIT)}, /*MY, MX, MV, ML, BGR */

   #ifdef GHW_INV_VDATA
   {0x01,  0,0x02},
   #else
   {0x01,  0,0x00},
   #endif

   /* Oscillator + power on */
   {0x19,  6,0x01},       /* Enable oscillator */
   {0x1f,  0,0x8c},       /* STB=0 */
   {0x1c,  4,0x04},       /* AP = 100  (medium current) */
   {0x1f,  4,0x84},       /* DK=0 */
   {0x1f,  4,0x94},       /* PON=1 */
   {0x1f,  4,0xd4}        /* VCOMG=1 */
   };

#elif (defined( GHW_HX8353_CMDINTF ) || defined( GHW_HX8353D_CMDINTF ))
/************************ Use command interface *****************/

#define CMD 0
#define DAT 1

static GCODE S1D_REGS FCODE as1dregs[] =
   {
   /*
   The initialization sequence below is sets the basic configurations.

   If the display controller OTP is preconfigured during display module
   production test (ex gamma correction table and power settings) then
   the basic command set below should be enough.

   However if another initialization sequence is recommended by a display vendor
   for a specific display module, then you may replace the initialization
   sequence below with the recommended setting.

   It works in this way:
   Each line will send one configuration byte to the display module and
   optionally introduce a delay after the intialization. The first parameter
   selects if the byte should be written to the controllers command register
   or data register address.
      { cmd/data select,  delay (0 = no delay), register value },

   Note: For the library only requirement is that the GCTRL_MADCTRL command
   (0x36) settings tracks the gdispcfg.h mirroring and rotation settings.
   All the rest of the commands can be freely adapted to a specific display
   module from a specific vendor without impact on library functionality.

   For a detailed explanation of each command and data the hardware manual for
   the display controller should be consulted.
   */
   {CMD,100,GCTRL_RESET},     /* 0x01 Reset */
   {CMD,150,GCTRL_SLPOUT},    /* 0x11 Sleep out */
   {CMD,100,GCTRL_IDMOFF},    /* 0x38 Idle mode off */

   #ifdef GHW_HX8353D_CMDINTF
   {CMD,  0,GCTRL_COLMOD},    /* 0x3A color mode for write/read */
   #if (defined( GHW_BUS8 ) && ((GDISPPIXW == 24) || (GDISPPIXW == 18)))
   {DAT,0,0x06}, /* 262k color (18=6+6+6)*/
   #elif (defined( GHW_BUS8 ) &&  (GDISPPIXW == 16))
   {DAT,0,0x05}, /* 65k color (wr: 8+8, rd: 6+6+6 ) */
   #elif (defined( GHW_BUS16 ) && (GDISPPIXW == 16))
   {DAT,0,0x05}, /* 65k color (16) rd: (6+6)+(6+6)+(6+6)*/
   #elif (defined( GHW_BUS32 ) && (GDISPPIXW == 18))
   {DAT,0,0x06}, /* 262k color (18) rd: (6+6)+(6+6)+(6+6) */
   #else
     #error GHW_BUSx GDISPPIXW combination is not supported with HX8353D
   #endif

   #else
   /* GHW_HX8353_CMDINTF */
   {CMD,  0,GCTRL_COLMOD},    /* 0x3A color mode for write/read */
   {DAT,0,0x06},              /* Always use 262k color mode (18=6+6+6)*/
   #endif

   {CMD,  0,GCTRL_NORON},     /* 0x13 Normal Display Mode On */
   #ifdef GHW_INV_VDATA
   {CMD,  0,GCTRL_INVON},     /* 0x21 display inversion on */
   #endif

   {CMD,  0,GCTRL_MADCTRL},   /*0x36 Memory Access Control (mirroring) */
   {DAT,  0,(MY_BIT|MX_BIT|MV_BIT|RGB_BIT)},   /* MY,MX,MV,ML,BGR,SS,0,0 */

   {CMD,  0,GCTRL_GAMSET},    /* Gamma setting */
   {DAT,  0,0x01}             /*80 */
   };

#elif defined( GHW_SSD1355 )

#define CMD 0
#define DAT 1

static GCODE S1D_REGS FCODE as1dregs[] =
   {
   {CMD,0,0x01}, // Software reset

   {CMD,0,0xfd}, // Unlock extended commands
   {DAT,0,0xb3},

   {CMD,0,0xd2},  // Set display clock/ OSc freq

   {CMD,0,0xd3}, // Set Vcom h
   {DAT,0,0x01}, // 0.74*VCC

   {CMD,0,0xd6}, // High power protection
   {DAT,0,0x07},

   {CMD,0,0xcc}, // Enable internal VSL
   {DAT,0,0xb0},
   {DAT,0,0x16},

   {CMD,0,0x35}, // Enable tearing effect
   {DAT,0,0x00},

   {CMD,0,0xca}, // Set mux ratio
   {DAT,0,GDISPH-1},

   {CMD,0,0xbd},  // Set first precharge voltage
   {DAT,0,0x09},

   {CMD,0,0xce},  // Set Second precharge speed
   {DAT,0,0x0e},

   {CMD,0,0xcf},  // Set Second precharge speed
   {DAT,0,0x09},

   {CMD,0,0xcd},  // Set phase length
   {DAT,0,0xff},

   {CMD,0,0x51},  // Write luminence
   {DAT,0,0xf0},  // D7-D4 = Light intensity 00 - f0

   {CMD,0,GCTRL_MADCTRL},  // Memory access control
   {DAT,0,MY_BIT|MX_BIT|MV_BIT|RGB_BIT},
   {DAT,0,BUSMODE|COMSPLIT},

//   #if ((GDISPPIXW == 16) && defined (GHW_BUS16))
   #if (GDISPPIXW == 16)
   {CMD,0,0x3a},  // Display interface mode
   {DAT,0,0x05},
   #endif

   {CMD,0,0x11},   // Sleep out
   {CMD,0,0x13},   // Normal display on

   /*
   // Gamma configuration (if different from linear)
   {CMD,0,0xbe},
   {DAT,0,1},      // GS0
   {DAT,0,6},
   {DAT,0,7},
   {DAT,0,8},
   {DAT,0,10},
   {DAT,0,12},
   {DAT,0,13},
   {DAT,0,13},
   {DAT,0,14},
   {DAT,0,14},
   {DAT,0,15},
   {DAT,0,16},
   {DAT,0,17},
   {DAT,0,18},
   {DAT,0,19},
   {DAT,0,23},
   {DAT,0,28},
   {DAT,0,33},
   {DAT,0,34},
   {DAT,0,38},
   {DAT,0,41},
   {DAT,0,49},
   {DAT,0,50},
   {DAT,0,53},
   {DAT,0,65},
   {DAT,0,67},
   {DAT,0,79},
   {DAT,0,85},
   {DAT,0,96},
   {DAT,0,106},
   {DAT,0,116},
   {DAT,0,127},

   {DAT,0,1},      // GS3-
   {DAT,0,6},
   {DAT,0,7},
   {DAT,0,8},
   {DAT,0,10},
   {DAT,0,12},
   {DAT,0,13},
   {DAT,0,13},
   {DAT,0,14},
   {DAT,0,14},
   {DAT,0,15},
   {DAT,0,16},
   {DAT,0,17},
   {DAT,0,18},
   {DAT,0,19},
   {DAT,0,23},
   {DAT,0,28},
   {DAT,0,33},
   {DAT,0,34},
   {DAT,0,38},
   {DAT,0,41},
   {DAT,0,49},
   {DAT,0,50},
   {DAT,0,53},
   {DAT,0,65},
   {DAT,0,67},
   {DAT,0,79},
   {DAT,0,85},
   {DAT,0,96},
   {DAT,0,106},
   {DAT,0,116},
   {DAT,0,127},

   {DAT,0,1},      // GS6-
   {DAT,0,6},
   {DAT,0,7},
   {DAT,0,8},
   {DAT,0,10},
   {DAT,0,12},
   {DAT,0,13},
   {DAT,0,13},
   {DAT,0,14},
   {DAT,0,14},
   {DAT,0,15},
   {DAT,0,16},
   {DAT,0,17},
   {DAT,0,18},
   {DAT,0,19},
   {DAT,0,23},
   {DAT,0,28},
   {DAT,0,33},
   {DAT,0,34},
   {DAT,0,38},
   {DAT,0,41},
   {DAT,0,49},
   {DAT,0,50},
   {DAT,0,53},
   {DAT,0,65},
   {DAT,0,67},
   {DAT,0,79},
   {DAT,0,85},
   {DAT,0,96},
   {DAT,0,106},
   {DAT,0,116},
   {DAT,0,127},
   */
   {CMD,0,0x0}   // Nop
   };
#elif defined( GHW_ST7628 )

#define CMD 0
#define DAT 1

static GCODE S1D_REGS FCODE as1dregs[] =
   {
   {CMD,100,GCTRL_RESET},     /* 0x01 Reset */
   {CMD,150,GCTRL_SLPOUT},    /* 0x11 Sleep out */
   {CMD,  0,GCTRL_NORON},     /* 0x13 Normal Display Mode On */
   {CMD,  0,GCTRL_INVON},     /* 0x21 display inversion */
   {CMD, 50,GCTRL_DISPON},    /* 0x29 Display On */
   {CMD,  0,GCTRL_MADCTRL},   /*0x36 Memory Access Control (mirroring) */
   {DAT,  0,(MY_BIT|MX_BIT|MV_BIT|RGB_BIT)},   /* MY,MX,MV,ML,BGR,0,0,0 */

   {CMD,  0,0xb0},      /* Duty cycle set */
   {DAT,  0,GDISPH-1},

   {CMD,  0,0x25},      /* Contrast */
   {DAT,  0,0x3f},      /* 0-7f (0x3f = reset default) */

   {CMD,  0,GCTRL_COLMOD},
   #if (GDISPIXW == 24)
   {DAT,  0,0x07},
   #elif (GDISPIXW == 18)
   {DAT,  0,0x06},
   #else
   {DAT,  0,0x05},
   #endif
   };
#elif (defined( GHW_ILI9163 ) || defined( GHW_ILI9488 ))  /* or ILI9341 or NT39122 or ST7735 */
#define CMD 0
#define DAT 1

static GCODE S1D_REGS FCODE as1dregs[] =
   {
   {CMD,120,0x00},         /* NOP, wait for any hardware startup completed */
   {CMD,120,GCTRL_RESET},  /* Reset registers to default */
   {CMD,120,GCTRL_SLPOUT}, /* Exit any sleep mode, copy vendor specific default setup to registers */

   {CMD,  0,GCTRL_GAMSET},  /* Set gamma curve 3 */
   {DAT,  0,0x04},

   {CMD,  0,GCTRL_MADCTRL},  /* Set video buffer scan modes */
   {DAT,  0,  MV_BIT | RGB_BIT | MX_BIT | MY_BIT }, /* Rotation, x,y mirroring, rgb-bgr modes */

   #ifdef GHW_INV_VDATA
   {CMD,  0,0x21},
   #else
   {CMD,  0,0x20},
   #endif

   /* // For (for ILI9341, ILI9488 etc Adjust frame rate controls (if hardware reset / vendor defaults result in screen flickering)
   {CMD,  0,0xB1},
   {DAT,  0,0x00},
   {DAT,  0,0x1d},   // frequency 65 Hz (Check data sheet for the specific variant for exact config value options)
   */

   {CMD,  0,GCTRL_COLMOD},  /*Pixel format */
   #if ((GDISPPIXW == 24) && defined (GHW_ILI9488))
   {DAT,0,0x77},    /* ILI9488 support 24 bit bus mode */
   #elif (GDISPPIXW == 18)
   {DAT,  0,0x66},  /* (ST7735 only use 0x06) */
   #else
   {DAT,  0,0x55},  /* (ST7735 only use 0x05)  */
   #endif

   {CMD,  0,GCTRL_DISPOFF}  //display off
//   {CMD,  0, GCTRL_DISPON}  /*  Easier to debug when on initially */
   };

#elif defined( GHW_HX8369 )

 #define CMD 0
 #define DAT 1

#if defined(GHW_ROTATED)
  #define __GDISPW GDISPH
  #define __GDISPH GDISPW
#else
  #define __GDISPW GDISPW
  #define __GDISPH GDISPH
#endif


static GCODE S1D_REGS FCODE as1dregs[] =
   {
   {CMD,1000,0x00}, // Nop, (assure power on delay after hdw reset)
   {CMD,10,GCTRL_RESET}, // Software reset

   {CMD,0,0xB9},//SETEXTC Open for extention commands (3 byte key)
   {DAT,0,0xFF},
   {DAT,0,0x83},
   {DAT,0,0x69},

   {CMD,0,GCTRL_SETPOWER},//SETPOWER
   {DAT,0,0x01},
   {DAT,0,0x00},
   {DAT,0,0x34},
   {DAT,0,0x06},
   {DAT,0,0x00},
   {DAT,0,0x0F},
   {DAT,0,0x0F},
   {DAT,0,0x2A},
   {DAT,0,0x32},
   {DAT,0,0x3F},
   {DAT,0,0x3F},
   {DAT,0,0x07},
   {DAT,0,0x23},
   {DAT,0,0x01},
   {DAT,0,0xE6},
   {DAT,0,0xE6},
   {DAT,0,0xE6},
   {DAT,0,0xE6},
   {DAT,0,0xE6},

   {CMD,0,GCTRL_SETDISP},//SETDISP
   {DAT,0,0x00},
   // Select scan resolution
   #if (__GDISPH > 854)
   {DAT,0,0x00},  // REG_SEL 480x864
   #elif (__GDISPH > 800)
   {DAT,0,0x10},  // REG_SEL 480x854
   #elif (__GDISPH > 720)
   {DAT,0,0x20},  // REG_SEL 480x800
   #elif (__GDISPH > 640)
   {DAT,0,0x50},  // REG_SEL 480x864
   #elif (__GDISPH > 854) && (__GDISPW > 360)
   {DAT,0,0x30},  // REG_SEL 480x640
   #else
   {DAT,0,0x40},  // REG_SEL 360x640
   #endif
   {DAT,0,0x0A},
   {DAT,0,0x0A},
   {DAT,0,0x70},
   {DAT,0,0x00},
   {DAT,0,0xFF},
   {DAT,0,0x00},
   {DAT,0,0x00},
   {DAT,0,0x00},
   {DAT,0,0x00},
   {DAT,0,0x03},
   {DAT,0,0x03},
   {DAT,0,0x00},
   {DAT,0,0x01},

   {CMD,0,GCTRL_SETCYC},//SETCYC
   {DAT,0,0x00},
   {DAT,0,0x18},
   {DAT,0,0x80},
   {DAT,0,0x10},
   {DAT,0,0x01},

   {CMD,0,GCTRL_SETVCOM},//SETVCOM
   {DAT,0,0x2C},
   {DAT,0,0x2C},

   {CMD,0,0xD5},//SETGIP  (
   {DAT,0,0x00},
   {DAT,0,0x05},
   {DAT,0,0x03},
   {DAT,0,0x00},
   {DAT,0,0x01},
   {DAT,0,0x09},
   {DAT,0,0x10},
   {DAT,0,0x80},
   {DAT,0,0x37},
   {DAT,0,0x37},
   {DAT,0,0x20},
   {DAT,0,0x31},
   {DAT,0,0x46},
   {DAT,0,0x8A},
   {DAT,0,0x57},
   {DAT,0,0x9B},
   {DAT,0,0x20},
   {DAT,0,0x31},
   {DAT,0,0x46},
   {DAT,0,0x8A},
   {DAT,0,0x57},
   {DAT,0,0x9B},
   {DAT,0,0x07},
   {DAT,0,0x0F},
   {DAT,0,0x02},
   {DAT,0,0x00},

   {CMD,0,GCTRL_COLMOD}, //COLMOD
   #if (GDISPPIXW == 24)
   {DAT,0,0x77},
   #elif (GDISPPIXW == 18)
   {DAT,0,0x66},
   #elif (GDISPPIXW == 16)
   {DAT,0,0x55},
   #else
     #error GHW_BUSx GDISPPIXW mode with HX8369
   #endif

   {CMD,0,GCTRL_MADCTRL},//MADCTL
   {DAT,0,(MY_BIT|MX_BIT|MV_BIT|RGB_BIT)},   /* MY,MX,MV,ML,BGR,0,0,0 */

   {CMD,1000,GCTRL_SLPOUT},//SLPOUT
   /* LUT initialization required for 16 and 18 bit pr pixel mode is initializaed via a software loop */
   {CMD,0,0x0}   // Nop
   };

#endif

/*
   Send a command
*/
static void ghw_cmd(SGUCHAR cmd)
   {
   #ifndef GHW_NOHDW
   #ifdef GHW_BUS8
   sgwrby(GHWCMD,cmd);    /* Register */
   #elif  defined( GHW_BUS32 )
   sgwrdw(GHWCMDDW,(SGULONG) cmd);    /* Register */
   #else
   sgwrwo(GHWCMDW,(SGUINT) cmd);    /* Register */
   #endif
   #else  /* GHW_NOHDW */
   cmd++; /* silience 'not used' warning */
   #endif
   }

#if (defined( GHW_HX8346_REGINTF ) || defined( GHW_HX8347G ) || defined( GHW_HX8352B ) || defined( GHW_HX8325_REGINTF ))
/*
   Send a command + data
*/
static void ghw_cmd_wr(SGUCHAR cmd, SGUCHAR cmddat)
   {
   #ifndef GHW_NOHDW
   #ifdef GHW_BUS8
   sgwrby(GHWCMD,cmd);                /* Register */
   sgwrby(GHWWR,cmddat);              /* Data */
   #elif  defined (GHW_BUS32)
   sgwrdw(GHWCMDDW,(SGULONG) cmd);    /* Register */
   sgwrdw(GHWWRDW,(SGULONG) cmddat); /* Data */
   #else
   sgwrwo(GHWCMDW,(SGUINT) cmd);      /* Register */
   sgwrwo(GHWWRW,(SGUINT) cmddat);    /* Data */
   #endif
   #else  /* GHW_NOHDW */
   cmd++; /* silience 'not used' warning */
   cmddat++;
   #endif
   }

#else

/* Write command data byte */
static void ghw_cmddat(SGUCHAR dat)
   {
   #ifndef GHW_NOHDW
   #ifdef GHW_BUS8
   sgwrby(GHWWR, dat);
   #elif  defined (GHW_BUS32)
   sgwrdw(GHWWRDW,(SGULONG) dat); /* Data */
   #else
   sgwrwo(GHWWRW, (SGUINT) dat);
   #endif
   #else
   dat++; /* Silence not used warning */
   #endif
   }

#endif

#if (defined(GBUFFER) || !defined( GHW_NO_LCD_READ_SUPPORT ))

/* Make a single data read operation */
static SGUCHAR ghw_rddat( void )
   {
   #ifndef GHW_NOHDW
   #ifdef GHW_BUS8
   return sgrdby(GHWRD);
   #elif  defined (GHW_BUS32)
   return (SGUCHAR) sgrddw(GHWRDDW);
   #else
   return (SGUCHAR)(sgrdwo(GHWRDW));
   #endif
   #else /* GHW_NOHDW */
   return 0;
   #endif
   }

#endif

/*
   Set the y range.
   The row position is set to y.
   After last write on row y2 the write position is reset to y
   Internal ghw function
*/
#if (( defined( GHW_HX8369 ) || defined(GHW_ILI9488)) && defined( GHW_BUS16 ) && ((GDISPPIXW == 18)||(GDISPPIXW == 24)))
static SGUCHAR ghw_word_is_started;
#endif
void ghw_set_xyrange(GXT xb, GYT yb, GXT xe, GYT ye)
   {
   #ifdef GHW_PCSIM
   ghw_set_xyrange_sim( xb, yb, xe, ye);
   #endif

   #if (defined( GHW_HX8346_REGINTF ) || defined( GHW_HX8347G ) || \
        defined( GHW_HX8352B ) || defined( GHW_HX8325_REGINTF ))

   ghw_cmd_wr(GCTRL_H_WIN_ADR_STRT_H,(SGUCHAR)(((SGUINT) (xb+G_XOFFSET)) >> 8));
   ghw_cmd_wr(GCTRL_H_WIN_ADR_STRT_L,(SGUCHAR) (xb+G_XOFFSET));
   ghw_cmd_wr(GCTRL_H_WIN_ADR_END_H ,(SGUCHAR)(((SGUINT) (xe+G_XOFFSET)) >> 8));
   ghw_cmd_wr(GCTRL_H_WIN_ADR_END_L ,(SGUCHAR) (xe+G_XOFFSET));

   ghw_cmd_wr(GCTRL_V_WIN_ADR_STRT_H,(SGUCHAR)(((SGUINT) (yb+G_YOFFSET)) >> 8));
   ghw_cmd_wr(GCTRL_V_WIN_ADR_STRT_L,(SGUCHAR) (yb+G_YOFFSET));
   ghw_cmd_wr(GCTRL_V_WIN_ADR_END_H ,(SGUCHAR)(((SGUINT) (ye+G_YOFFSET)) >> 8));
   ghw_cmd_wr(GCTRL_V_WIN_ADR_END_L ,(SGUCHAR) (ye+G_YOFFSET));

   #ifdef GHW_HX8352B
   /* Set address counter to start of window area */
   #ifdef GHW_ROTATED
   ghw_cmd_wr(GCTRL_COL_MSB,(SGUCHAR)(((SGUINT) (yb+G_YOFFSET)) >> 8));
   ghw_cmd_wr(GCTRL_COL_LSB,(SGUCHAR) (yb+G_YOFFSET));
   ghw_cmd_wr(GCTRL_ROW_MSB,(SGUCHAR)(((SGUINT) (xb+G_XOFFSET)) >> 8));
   ghw_cmd_wr(GCTRL_ROW_LSB,(SGUCHAR) (xb+G_XOFFSET));
   #else
   ghw_cmd_wr(GCTRL_COL_MSB,(SGUCHAR)(((SGUINT) (xb+G_XOFFSET)) >> 8));
   ghw_cmd_wr(GCTRL_COL_LSB,(SGUCHAR) (xb+G_XOFFSET));
   ghw_cmd_wr(GCTRL_ROW_MSB,(SGUCHAR)(((SGUINT) (yb+G_YOFFSET)) >> 8));
   ghw_cmd_wr(GCTRL_ROW_LSB,(SGUCHAR) (yb+G_YOFFSET));
   #endif
   #endif

   #elif (defined( GHW_HX8346_CMDINTF ) || defined( GHW_HX8325_CMDINTF )  || \
          defined( GHW_ILI9163) || defined( GHW_ILI9488) || defined( GHW_HX8369))

   ghw_cmd(GCTRL_CASET);
   ghw_cmddat((SGUCHAR)(((SGUINT) (xb+G_XOFFSET)) >> 8));
   ghw_cmddat((SGUCHAR) (xb+G_XOFFSET));
   ghw_cmddat((SGUCHAR)(((SGUINT) (xe+G_XOFFSET)) >> 8));
   ghw_cmddat((SGUCHAR) (xe+G_XOFFSET));

   ghw_cmd(GCTRL_RASET);
   ghw_cmddat((SGUCHAR)(((SGUINT) (yb+G_YOFFSET)) >> 8));
   ghw_cmddat((SGUCHAR) (yb+G_YOFFSET));
   ghw_cmddat((SGUCHAR)(((SGUINT) (ye+G_YOFFSET)) >> 8));
   ghw_cmddat((SGUCHAR) (ye+G_YOFFSET));

   #elif  defined( GHW_SSD1355 ) || defined( GHW_ST7628 )

   ghw_cmd(GCTRL_CASET);
   ghw_cmddat((SGUCHAR) (xb+G_XOFFSET));
   ghw_cmddat((SGUCHAR) (xe+G_XOFFSET));

   ghw_cmd(GCTRL_RASET);
   ghw_cmddat((SGUCHAR) (yb+G_YOFFSET));
   ghw_cmddat((SGUCHAR) (ye+G_YOFFSET));

   #endif

   ghw_cmd(GCTRL_RAMWR); /* Reset address position to range start */
   #if (( defined( GHW_HX8369 ) || defined(GHW_ILI9488)) && defined( GHW_BUS16 ) && ((GDISPPIXW == 18)||(GDISPPIXW == 24)))
   ghw_word_is_started = 0;
   #endif


   }

void ghw_setxypos(GXT xb, GYT yb)
   {
   #ifdef GHW_PCSIM
   ghw_set_xy_sim( xb, yb);
   #endif
   ghw_set_xyrange(xb, yb, GDISPW-1, GDISPH-1);
   }

#if ((defined( GHW_HX8369 ) || defined(GHW_ILI9488)) && defined( GHW_BUS16 ) && ((GDISPPIXW == 24) || (GDISPPIXW == 18)))
/*
  Note HX8369 in 16 bit bus mode, 18,24 bit pr pixel mode, only support write of 2 pixels using 3 words
  We must keep track on the write pixel order

  (In all other cases ghw_auto_wr_end() maps to nothing in s6d0129.h)
*/
static SGUCHAR ghw_wr_word_is_ready;
static SGUINT  ghw_wr_word2;

void _ghw_auto_wr_end(void)
   {
   if (ghw_wr_word_is_ready)
      {
      /* Second tx unit pending, flush stored tx unit */
      ghw_wr_word_is_ready = 0;
      sgwrwo(GHWWRW, ghw_wr_word2);  /* Complete first pixel tx (ignore first part of second pixel) */
      }
   }
#endif

/*
   Write databyte to controller (at current position) and increment
   internal xadr.

   Internal ghw function
*/
void ghw_auto_wr(GCOLOR dat)
   {
   #ifdef GHW_PCSIM
   ghw_autowr_sim( dat );
   #endif

   #ifndef GHW_NOHDW

   #if ((defined( GHW_HX8369 ) || defined(GHW_ILI9488)) && defined( GHW_BUS16 ) && ((GDISPPIXW == 24) || (GDISPPIXW == 18)))
   /* Special packed TX data mode. Pixel data transmitted as 2 pixels stored in 3 tx units:  R1G1, B1R2, R2G2 */
   if (ghw_wr_word_is_ready)
      {
      /* Second pixel, use stored values */
      ghw_wr_word_is_ready = 0;
      #if (GDISPPIXW == 18)
      sgwrwo(GHWWRW, ghw_wr_word2 | ((dat >> 10) & 0xfc));     /* TX unit 2 */
      sgwrwo(GHWWRW, ((dat << 4) & 0xfc00) | (dat << 2)&0xfc); /* TX unit 3 */
      #else
      sgwrwo(GHWWRW, ghw_wr_word2 | (dat >> 16) & 0xff);  /* TX unit 2 */
      sgwrwo(GHWWRW, (dat & 0xffff));  /* TX unit 3 */
      #endif
      }
   else
      {
      /* First pixel, Write first tx unis, delay second tx unit  */
      ghw_wr_word_is_ready = 1;
      #if (GDISPPIXW == 18)
      sgwrwo(GHWWRW, ((dat >> 2) & 0xfc00) | ((dat >> 4) & 0x00fc));
      ghw_wr_word2 = (SGUINT)(dat<<10) & 0xfc00;    /* Prepare for TX unit 2 */
      #else
      sgwrwo(GHWWRW, dat >> 8);         /* TX unit 1 */
      ghw_wr_word2 = (SGUINT)(dat & 0xff) << 8; /* Prepare for TX unit 2 */
      #endif
      }

   #else

   #if (defined( GHW_BUS8 ) && (GDISPPIXW == 24))
      /* 8 bit bus mode (24 bit color stored as r,g,b) */
      sgwrby(GHWWR, (SGUCHAR)(dat>>16)); /* MSB */
      sgwrby(GHWWR, (SGUCHAR)(dat>>8));
      sgwrby(GHWWR, (SGUCHAR)(dat));     /* LSB */
   #elif (defined( GHW_BUS8 ) && (GDISPPIXW == 18))
      /* 8 bit bus mode ( 18 bit color stored as r,g,b) */
      sgwrby(GHWWR, (SGUCHAR)(dat>>10)); /* MSB */
      sgwrby(GHWWR, (SGUCHAR)(dat>>4));
      sgwrby(GHWWR, (SGUCHAR)(dat<<2));  /* LSB */
   #elif (defined( GHW_BUS8 ) && (GDISPPIXW == 16))
    #if defined( GHW_ST7628  ) || defined( GHW_HX8347G ) || \
        defined( GHW_HX8352B ) || defined( GHW_HX8353D_CMDINTF) || \
        defined( GHW_SSD1355 ) || defined( GHW_ILI9163 ) || defined( GHW_HX8369 ) ||  defined(GHW_ILI9488)
      sgwrby(GHWWR, (SGUCHAR)(dat>>8));
      sgwrby(GHWWR, (SGUCHAR)(dat));     /* LSB */
    #else
      {
      SGUCHAR r,g,b;
      /* 8 bit bus mode, controller require 24 bit rgb */
      /* Expand data to 24 bit RGB = RRRRRR** GGGGGG** BBBBBB** */
      /* from RGB16 format MSB (RRRRRGGGGGGBBBBB) */
      r = (SGUCHAR)(dat >> 8) & 0xf8;
      g = (SGUCHAR)(dat >> 3) & 0xfc;
      b = (SGUCHAR)(dat << 3) & 0xf8;
      sgwrby(GHWWR, r|(r>>5)); /* MSB */
      sgwrby(GHWWR, g|(g>>6));
      sgwrby(GHWWR, b|(b>>5)); /* LSB */
      }
    #endif
   #elif (defined( GHW_BUS16 ) && (GDISPPIXW == 16))
      /* 16 bit bus mode, 16 bit color */
      sgwrwo(GHWWRW, dat);           /* 16 bit color */
   #elif (defined( GHW_BUS16 ) && (GDISPPIXW == 18))
      /* 16 bit bus mode, 18 bit color */
      sgwrwo(GHWWRW, (SGUINT) (dat >> 2));        /* 18 bit color, 16 msb */
      sgwrwo(GHWWRW, (SGUINT)  (dat & 0x03));     /* 18 bit color,  2 lsb */
   #elif (defined( GHW_BUS16 ) && (GDISPPIXW == 24))
      /* 16 bit bus mode, 24 bit color */
      sgwrwo(GHWWRW, (SGUINT) (dat >> 8));        /* 24 bit color, 16 msb */
      #ifdef GHW_ST7628
      sgwrwo(GHWWRW, (SGUINT)  (dat & 0xff) << 8);/* 24 bit color,  8 msb */
      #else
      sgwrwo(GHWWRW, (SGUINT)  (dat & 0xff));     /* 24 bit color,  8 lsb */
      #endif
   #elif (defined( GHW_BUS32 ) && (GDISPPIXW == 18))
      /* 32 bit bus mode, 18 bit color */
      sgwrdw(GHWWRDW, dat);           /* 18 bit color */
   #elif (defined( GHW_BUS32 ) && (GDISPPIXW == 24))
      /* 32 bit bus mode, 24 bit color */
      sgwrdw(GHWWRDW, dat);           /* 24 bit color */
   #else
      #error Illegal GDISPPIXW / GHW_BUSn combination in gdispcfg.h
   #endif

   #endif
   #endif /* GHW_NOHDW */
   }

#if (defined(GBUFFER) || !defined( GHW_NO_LCD_READ_SUPPORT ))
/*
   Perform required dummy reads after column position setting
   Note HX8346 only required one dummy read in 8 bit mode
   although the datasheet (ver 2007-4-18) say 3 reads.

   Note HX8352 (etc) in 16 bit pr pixel mode only support read of 2 pixels using 3 words
   we must keep track on the read pixel order
*/
#if ((defined( GHW_HX8352B ) || defined( GHW_ILI9341V ) || defined( GHW_HX8347G ) || defined( GHW_HX8353D_CMDINTF ) || defined( GHW_HX8369 ) || defined(GHW_ILI9488)) && defined( GHW_BUS16 ))
static SGUCHAR ghw_word_is_ready;
#endif
void ghw_auto_rd_start(void)
   {
   ghw_cmd(GCTRL_RAMRD);
   ghw_rddat(); /* Single dummy read operation */
   #if ((defined( GHW_HX8352B ) || defined( GHW_ILI9341V ) || defined( GHW_HX8347G ) || defined( GHW_HX8353D_CMDINTF ) || defined( GHW_HX8369 ) || defined(GHW_ILI9488)) && defined( GHW_BUS16 ))
   ghw_word_is_ready = 0;
   #endif
   }

/*
   Read at address set by previous (write auto-increment) operation
*/
GCOLOR ghw_auto_rd(void)
   {
   #ifndef GHW_NOHDW
   #if ((defined( GHW_HX8352B ) || defined( GHW_ILI9341V ) || defined( GHW_HX8347G ) || defined( GHW_HX8353D_CMDINTF ) || defined( GHW_HX8369 ) || defined(GHW_ILI9488)) && defined( GHW_BUS16 ))
      /* Only support read of two 18 (24) bit pixels arranged in 3x16-bit words-> must keep track
         on wether this is the first or second pixel in bus unit */
      static SGUINT r2 = 0;
      static SGUINT r3 = 0;
      if (ghw_word_is_ready)
         {
         /* Second pixel, use stored values */
         ghw_word_is_ready = 0;
         #if (GDISPPIXW == 16)
         return (GCOLOR)(((r2 << 8)&0xf800) | ((r3 >> 5)&0x07e0) | ((r3 >> 3 ) & 0x001f));
         #elif (GDISPPIXW == 18)
         return (GCOLOR)((((GCOLOR)(r2 & 0x00fc))<<10) | ((r3>>4)&0xfc0) | ((r3>>2) & 0x003f));
         #elif (GDISPPIXW == 24)
         return (GCOLOR)((((GCOLOR)r2)<<16) | r3);
         #else
          #error Illegal GDISPPIXW / GHW_BUSn combination in gdispcfg.h
         #endif
         }
      else
         {
         /* First pixel, read whole bus unit (3 words) in one operation */
         SGUINT r1;
         r1 = sgrdwo(GHWRDW); /* r1+g1  (RRRRRR**GGGGGG**) */
         r2 = sgrdwo(GHWRDW); /* b1+r2  (BBBBBB**RRRRRR**) (saved for next pixel read) */
         r3 = sgrdwo(GHWRDW); /* g2+b2  (GGGGGG**BBBBBB**) (saved for next pixel read) */
         ghw_word_is_ready = 1;
         #if (GDISPPIXW == 16)
         return (GCOLOR)( (r1 & 0xf800) | ((r1 << 3)&0x07e0) | ((r2 >> 11) & 0x001f));
         #elif (GDISPPIXW == 18)
         return (GCOLOR)((((GCOLOR)(r1 & 0xfc00))<<2)  | ((r1<<4)&0xfc0) | ((r2>>10)& 0x003f));
         #elif (GDISPPIXW == 24)
         return (GCOLOR)((((GCOLOR)r1)<<8) | ((r2>>8)&0xff));
         #else
         #error Illegal GDISPPIXW / GHW_BUSn combination in gdispcfg.h
         #endif
         }

      #elif (defined( GHW_BUS8 ) && (GDISPPIXW == 24))
         GCOLOR ret;
         /* 8 bit bus mode */
         /* 24 bit color mode (3 lsb bytes is r,g,b) */
         /* Left aligned color info. HX8346 do color extention to LSB color bits so no masking is needed */
         ret =  (((GCOLOR) sgrdby(GHWRD)) << 16);  /* MSB (RRRRRR**) */
         ret |= (((GCOLOR) sgrdby(GHWRD)) << 8);   /*     (GGGGGG**) */
         ret |=  ((GCOLOR) sgrdby(GHWRD));         /* LSB (BBBBBB**) */
         return ret;
      #elif (defined( GHW_BUS8 ) && (GDISPPIXW == 18))
         GCOLOR ret;
         /* 8 bit bus mode */
         /* 18 bit color mode  */
         /* Left aligned color info */
         ret =  (((GCOLOR) (sgrdby(GHWRD) & 0xfc)) << 10);  /* MSB (RRRRRR**) */
         ret |= (((GCOLOR) (sgrdby(GHWRD) & 0xfc)) << 4);   /*     (GGGGGG**) */
         ret |= (((GCOLOR) (sgrdby(GHWRD) & 0xfc)) >> 2);   /* LSB (BBBBBB**) */
         return ret;
      #elif (defined( GHW_BUS8 ) && (GDISPPIXW == 16))
         GCOLOR ret;
         #if defined( GHW_ST7628 )
         ret  = (((GCOLOR) sgrdby(GHWRD)) << 8);   /*     (RRRRRGGG) */
         ret |=  ((GCOLOR) sgrdby(GHWRD));         /*     (GGGBBBBB) */
         #else
         /* 8 bit bus mode, controller returns 24 bit rgb */
         /* 16 bit color mode  RGB{5:6:5} */
         ret =  (((GCOLOR) ( sgrdby(GHWRD) & 0xf8)) << 8);  /* (RRRRR*** ********) */
         ret |= (((GCOLOR) ( sgrdby(GHWRD) & 0xfc)) << 3);  /* (*****GGG GGG*****) */
         ret |=  ((GCOLOR) ((sgrdby(GHWRD) & 0xf8) >> 3));  /* (******** ***BBBBB) */
         #endif
         return ret;
      #elif (defined( GHW_BUS16 ) && (GDISPPIXW == 16))
         #if (defined( GHW_HX8347A_REGINTF ) || defined( GHW_HX8347A_CMDINTF ))
         GCOLOR ret;
         /* HX8347 in 16 bit bus mode, returns 24 bit rgb as 3 x bytes */
         /* Reformat to 16 bit color mode  RGB{5:6:5} */
         ret =  (((GCOLOR) ( sgrdwo(GHWRD) & 0xf8)) << 8);  /* (RRRRR*** ********) */
         ret |= (((GCOLOR) ( sgrdwo(GHWRD) & 0xfc)) << 3);  /* (*****GGG GGG*****) */
         ret |=  ((GCOLOR) ((sgrdwo(GHWRD) & 0xf8) >> 3));  /* (******** ***BBBBB) */
         return ret;
         #else
         return sgrdwo(GHWRDW);
         #endif
      #elif (defined( GHW_BUS16 ) && (GDISPPIXW == 18))
         GCOLOR ret;
         ret =  ((GCOLOR) sgrdwo(GHWRDW)) << 2;   /* 18 bit color 16 msb */
         ret |= ((GCOLOR) sgrdwo(GHWRDW)) & 0x3;  /* + 2 lsb */
         return ret;
      #elif (defined( GHW_BUS16 ) && (GDISPPIXW == 24))
         GCOLOR ret;
         ret =  ((GCOLOR) sgrdwo(GHWRDW)) << 8;   /* 24 bit color 16 msb */
         ret |= ((GCOLOR) sgrdwo(GHWRDW)) & 0xff; /* + 8 lsb */
         return ret;
      #elif (defined( GHW_BUS32 ) && (GDISPPIXW == 18))
         return sgrddw(GHWRDDW) & 0x3ffff;
      #elif (defined( GHW_BUS32 ) && (GDISPPIXW == 24))
         return sgrddw(GHWRDDW) & 0xffffff;
      #else
         #error Illegal GDISPPIXW / GHW_BUSn combination in gdispcfg.h
      #endif

   #else
     #ifdef GHW_PCSIM
      return ghw_autord_sim();
     #else
      return 0;
     #endif
   #endif /* GHW_NOHDW */
   }

#endif /* GBUFFER || !GHW_NO_LCD_READ_SUPPORT */


/*********************** local configuration functions *******************/

/****************************************************************/
/**        Initialization and error handling functions         **/
/****************************************************************/

/*
   Change default (palette) colors
*/
void ghw_setcolor(GCOLOR fore, GCOLOR back)
   {
   #if ((GDISPPIXW == 24) && (defined( GHW_HX8346_CMDINTF ) || \
        defined( GHW_HX8346_REGINTF ) || defined( GHW_HX8347G )  || defined( GHW_HX8352B ) || defined( GHW_HX8353_CMDINTF )))
   /* Mask unused bits */
   fore &= 0xfcfcfc;
   back &= 0xfcfcfc;
   #endif
   /* Update active colors */
   ghw_def_foreground = fore;
   ghw_def_background = back;
   }

/*
   Convert an RGB structure to a color value using the current color mode
*/
GCOLOR ghw_rgb_to_color( GCONSTP GPALETTE_RGB *palette )
   {
   if (palette == NULL)
      return 0;

   return G_RGB_TO_COLOR(palette->r,palette->g,palette->b);
   }

#if (GHW_PALETTE_SIZE > 0)
/*
   Load a new palette or update the existing palette
   (Palette is only used with symbols using 2 or 4 bits pr pixel)
*/
SGBOOL ghw_palette_wr(SGUINT start_index, SGUINT num_elements, GCONSTP GPALETTE_RGB PFCODE *palette)
   {
   if ((num_elements == 0) ||
       ((start_index + num_elements) > 16) ||
       (palette == NULL))
      {
      glcd_err = 1;
      return 1;
      }
   glcd_err = 0;

   /* (Partial) update of operative palette values */
   while(num_elements-- > 0)
      {
      /* Make local palette copy here to be compatible with compilers
         having a non-standard conforming handling of pointer
         (i.e when PFCODE memory qualifer is used) */
      GPALETTE_RGB pal;
      pal.r = palette->r;
      pal.g = palette->g;
      pal.b = palette->b;
      ghw_palette_opr[start_index++] = ghw_rgb_to_color(&pal);
      palette++;

      /* ghw_palette_opr[start_index++] = ghw_rgb_to_color(&palette++); */
      }

   return glcd_err;
   }
#endif

/*
   Fast set or clear of LCD module RAM buffer
   Internal ghw function
*/
static void ghw_bufset(GCOLOR color)
   {
   GBUFINT cnt;
   cnt = 0;
   ghw_set_xyrange(0,0,GDISPW-1,GDISPH-1);
   do
      {
      /* Clear using X,Y autoincrement */
      #ifdef GBUFFER
      gbuf[cnt] = color;      /* Set ram buffer as well */
      #endif

      #if defined( GHW_HX8346_REGINTF )
      if (color != G_BLACK)   /* hardware buffer is cleared to black by controller at reset
                                 Only nessesary to clear video buffer here if another color is used */
         ghw_auto_wr(color);
      #ifndef GBUFFER
      else
          break; /* All clearing done in hardware, skip completely */
      #endif
      #else
      ghw_auto_wr(color);      /* Set LCD buffer */
      #endif
      }
   while (++cnt < ((GBUFINT) GDISPW) * ((GBUFINT) GDISPH)); /* Loop until x+y wrap */
   ghw_auto_wr_end();
   }

#if (defined( WR_RD_TEST ) && !defined(GHW_NO_LCD_READ_SUPPORT))
/*
   Make write-readback test on controller memory.

   This test returns ok (== 0) when the write-readback test succeded. This indicates that
   the processor / display hardware interface / library configuration combination is
   working ok.

   This test will fail if some databus or control signals is not connected correctly.

   This test will fail if 16/8 bit bus mode selection and color resolution settings
   in the library configuration file does not match the actual bus and color resolution
   configuration for the hardware selected via chip pins. (ex display and processor
   16/8 bit bus width, 8080/6800 bus type settings, word / byte address offsets, etc).

   Note that often a display module vendor have hardwired some HX8312 chip interface
   configuration signals in the display module. Therefore often only one or a few of the
   library configuration possibilities will be avaiable with a given display module
   hardware.

   This test may fail if illegal GDISPW, GDISPH, GHW_XOFFSET, GHW_YOFFSET
   configuration settings cause overrun of the on-chip video RAM.

   This test can be exectuted correctly with only logic power on the display module.

   Return 0 if no error,
   Return != 0 if some readback error is detected (the bit pattern may give information
   about connector pins in error)
*/
#define  GPRINTF( format, data ) printf((format), (data) )  /* Info via printf */
//#define  GPRINTF( format, data ) /* Use no info */

static GCOLOR ghw_wr_rd_test(void)
   {
   #ifndef GHW_NOHDW
   int i;
   GCOLOR msk,result;

   /*printf("\n");*/
   ghw_set_xyrange(0,0,GDISPW-1,GDISPH-1);

   #if (GDISPPIXW > 16)
   /* 24 (18) bit color mode */
   GPRINTF("\n%s","");
   for (i = 0, msk = 1; i < GDISPPIXW; i++)
      {
      ghw_auto_wr(msk);
      GPRINTF("0x%06x ", (unsigned long) msk);
      ghw_auto_wr(~msk);
      GPRINTF(" 0x%06x\n", (unsigned long) (~msk) & 0xffffff);
      msk <<= 1;
      }
   ghw_auto_wr_end();
   GPRINTF("\n%s","");

   ghw_set_xyrange(0,0,GDISPW-1,GDISPH-1);
   ghw_auto_rd_start();
   for (i=0, msk=1, result=0; i < GDISPPIXW; i++)
      {
      GCOLOR val1,val2;
      val1 = ghw_auto_rd();
      val2 = ghw_auto_rd();
      result |= (val1 ^ msk);
      result |= (val2 ^ (~msk));
      GPRINTF("0x%06lx ",  (unsigned long) val1 & 0xffffff);
      GPRINTF(" 0x%06lx\n", (unsigned long) val2 & 0xffffff);
      msk <<= 1;
      }
   result &= GHW_COLOR_CMP_MSK;  /* Mask bits unused by controller */

   #else
   /* 16 bit color mode */
   GPRINTF("\n%s","");
   for (i = 0, msk = 1; i < GDISPPIXW; i++)
      {
      ghw_auto_wr(msk);
      GPRINTF("0x%04x ", (unsigned int) msk);
      ghw_auto_wr(~msk);
      GPRINTF(" 0x%04x\n", (unsigned int) (~msk & 0xffff));
      msk <<= 1;
      }
   ghw_auto_wr_end();
   GPRINTF("\n%s","");
   ghw_set_xyrange(0,0,GDISPW-1,GDISPH-1);
   ghw_auto_rd_start();
   for (i=0, msk=1, result=0; i < GDISPPIXW; i++)
      {
      GCOLOR val;
      val = ghw_auto_rd();
      result |= (val ^ msk);
      GPRINTF("0x%04x ",   (unsigned short) val);
      val = ghw_auto_rd();
      GPRINTF(" 0x%04x\n", (unsigned short) val );
      result |= (val ^ (~msk));
      msk <<= 1;
      }
   #endif
   return result;  /* 0 = Nul errors */
   #else
   return 0; /* 0 = Nul errors */
   #endif
   }

#endif /* WR_RD_TEST */

/*
   Waits a number of milli seconds

   NOTE:
   For target mode compilation the body function this function
   should be customized (adjusted) so it generate a delay of at
   least the number of milli seconds given by the function parameter.

   (If the target compiler has an intrinsic feature for generating
   micro delays then it may be used instead of the ms loop)

   The function is only used during display controller initialization
   and display on / off
*/
static void ghw_cmd_wait(SGUCHAR ms)
   {
   #ifdef SGPCMODE
   #ifdef IOTESTER_USB
   iot_sync(0);  // Assure target  I/O via PC USB driver is always completed before start waiting delay
   #endif
   Sleep(ms); /* delay 1 ms */
   #else
   static volatile SGUINT wait1ms;
   while (ms-- != 0)
      {
      wait1ms = 2000;     /* Adjust to achieve a 1 ms loop below */
      while( wait1ms != 0)
         wait1ms--;
      }
   #endif
   }

/*
   Initialize display, clear ram  (low-level)
   Clears glcd_err status before init

   Return 0 if no error,
   Return != 0 if some error
*/
SGBOOL ghw_init(void)
   {
   short i;

   #ifdef GBUFFER
   iltx = 1;
   ilty = 1;
   irbx = 0;
   irby = 0;
   ghw_upddelay = 0;
   #endif

   glcd_err = 0;
   ghw_io_init(); /* Set any hardware interface lines, controller hardware reset */

   #if (defined( GHW_ALLOCATE_BUF) && defined( GBUFFER ))
   if (gbuf == NULL)
      {
      /* Allocate graphic ram buffer */
      if ((gbuf = (GCOLOR *)calloc(ghw_gbufsize(),1)) == NULL)
         glcd_err = 1;
      else
         gbuf_owner = 1;
      }
   #endif

   if (glcd_err != 0)
      return 1;

   #ifdef GHW_PCSIM
   /* Tell simulator about the visual LCD screen organization */
   ghw_init_sim( GDISPW, GDISPH );
   if (glcd_err != 0)
      return 1;
   #endif
   /* Set default colors */
   ghw_setcolor( GHW_PALETTE_FOREGROUND, GHW_PALETTE_BACKGROUND );

   #if (GHW_PALETTE_SIZE > 0)
   /* Load palette */
   ghw_palette_wr(0, sizeof(ghw_palette)/sizeof(GPALETTE_RGB), (GCONSTP GPALETTE_RGB PFCODE *)&ghw_palette[0]);
   #endif

   /*
      Wait for hardware reset to complete
      If a delay after reset is already introdued by other parts of the
      system you may reduce the wait time parameter in the call below or
      comment out the call
   */
   ghw_cmd_wait(150); /* Wait for hardware to stabilize after power on */

   /*
   {  // Report ILI9488 status register
   SGULONG dat = 0;
   ghw_cmd(0x09); // Read status ID command
   ghw_rddat();

   dat |= ((SGULONG) ghw_rddat())<<24;
   dat |= ((SGULONG) ghw_rddat())<<16;
   dat |= ((SGULONG) ghw_rddat())<<8;
   dat |= ((SGULONG) ghw_rddat());
   GPRINTF("\nDisplay status: 0x%lx",dat);
   }
   */

   /*
   { // Report ILI9488 Manufacturers ID settings
   SGUCHAR dat[3];
   ghw_cmd(0x04); // Read manufacturer ID command
   dat[0] = ghw_rddat();
   dat[1] = ghw_rddat();
   dat[2] = ghw_rddat();
   GPRINTF("\nDisplay ID1: 0x%02x, ID2: 0x%02x, ID3: 0x%02x, ",
         (unsigned int)dat[0],
         (unsigned int)dat[1],
         (unsigned int)dat[2]);
   }
   */


   /* Initialize controller according to configuration file */
   #if (defined( GHW_HX8346_REGINTF ) || defined( GHW_HX8347G ) || defined( GHW_HX8352B ) ||defined( GHW_HX8325_REGINTF ))
   /* Register mode */
   for (i=0; i < sizeof(as1dregs)/sizeof(S1D_REGS); i++)
      {
      ghw_cmd_wr(as1dregs[i].index,as1dregs[i].value);
      if (as1dregs[i].delay != 0)
         ghw_cmd_wait( as1dregs[i].delay );
      }
   #else
   /* Command mode */
   for (i=0; i < sizeof(as1dregs)/sizeof(S1D_REGS); i++)
      {
      if (as1dregs[i].index == CMD)
         ghw_cmd(as1dregs[i].value);
      else
         ghw_cmddat(as1dregs[i].value);
      if (as1dregs[i].delay != 0)
         ghw_cmd_wait( as1dregs[i].delay );
      }

   #if (defined(GHW_HX8369) && ((GDISPPIXW == 18) || (GDISPPIXW == 16)))
   /* Special setup */
   /* Set HX8369 5->8 or 6->8 bit color lane lookup table */
   ghw_cmd(0x2D); // COLSET, followed by 3x64 parameter bytes
   for (i=0;i<64;i++)
      #if (GDISPPIXW == 18)
      ghw_cmddat((SGUCHAR)((i<<2)|(i>>4)));  // Red 6->8 convert
      #else
      ghw_cmddat((SGUCHAR)((i<<3)|(i>>2)));  // Red 5->8 convert (32 entries used)
      #endif
   for (i=0;i<64;i++)
      ghw_cmddat((SGUCHAR)((i<<2)|(i>>4)));  // Green 6->8 convert
   for (i=0;i<64;i++)
      #if (GDISPPIXW == 18)
      ghw_cmddat((SGUCHAR)((i<<2)|(i>>4)));  // Blue 6->8 convert
      #else
      ghw_cmddat((SGUCHAR)((i<<3)|(i>>2)));  // Blue 5->8 convert (32 entries used)
      #endif
   #endif

   #if ((defined( GHW_HX8369 ) || defined(GHW_ILI9488)) && defined( GHW_BUS16 ) && ((GDISPPIXW == 24) || (GDISPPIXW == 18)))
   ghw_wr_word_is_ready = 0;
   #endif

   #endif /* command mode */

   /*
      Stimuli test loops for initial oscilloscope test of display interface bus signals
      Uncomment to use the test loop for the given data bus width.
      It is recommended to check all display bus signals with each of the I/O access
      statements in the loop one by one.
   */
   /*
   #ifdef GHW_BUS8
   for(;;)
      {
      SGUCHAR dat;
      sgwrby(GHWCMD,0xff);
      sgwrby(GHWWR,0x00);
      //dat = sgrdby(GHWSTA);
      //dat = sgrdby(GHWRD);
      }
   #endif
   #ifdef GHW_BUS16
   for(;;)
      {
      SGUINT dat;
      sgwrwo(GHWCMDW,0xffff);
      sgwrwo(GHWWRW,0x0000);
      dat = sgrdwo(GHWSTAW);
      dat = sgrdwo(GHWRDW);
      }
   #endif
   */

   #if (defined( WR_RD_TEST ) && !defined(GHW_NO_LCD_READ_SUPPORT))
   /*
      NOTE:
      The call of ghw_wr_rd_test() should be commented out in serial mode.
      In serial mode the display controller does not provide read-back facility
      and this test will always fail.
   */
   if (ghw_wr_rd_test() != ((GCOLOR) 0))
      {
      /* Controller memory write-readback error detected
      (Check the cable or power connections to the display) */
      G_WARNING("Hardware interface error\nCheck display connections\n");  /* Test Warning message output */
      glcd_err = 1;
      return 1;
      }
   #endif

   /* Test RGB-BGR setting (configured with GHW_COLOR_SWAP) */
//    ghw_dispon(); /* placed here to ease initial debug */
//    ghw_bufset( G_RED );
//    ghw_bufset( G_GREEN );
//    ghw_bufset( G_BLUE );
   ghw_bufset( ghw_def_background );
   ghw_dispon();     /* Turn display on after buffer clear */

   #ifndef GNOCURSOR
   ghw_cursor = GCURSIZE1;    /* Cursor is off initially */
   /* ghw_cursor = GCURSIZE1 | GCURON; */ /* Uncomment to set cursor on initially */
   #endif

   ghw_updatehw();  /* Flush to display hdw or simulator */


   return (glcd_err != 0) ? 1 : 0;
   }

/*
   Return last error state. Called from applications to
   check for LCD HW or internal errors.
   The error state is reset by ghw_init and all high_level
   LCD functions.

   Return == 0 : No errors
   Return != 0 : Some errors
*/
SGUCHAR ghw_err(void)
   {
   #if (defined(_WIN32) && defined( GHW_PCSIM))
   if (GSimError())
      return 1;
   #endif
   return (glcd_err == 0) ? 0 : 1;
   }


/*
   Display a (fatal) error message.
   The LCD display module is always cleared and initialized to
   the system font in advance.
   The error message is automatically centered on the screen
   and any \n characters in the string is processed.

   str = ASCII string to write at display center
*/
void ghw_puterr( PGCSTR str )
   {
   PGCSTR idx;
   SGUINT xcnt;
   GXT xp;
   GYT yp,h,y, sidx;
   PGSYMBYTE psym;
   GCOLOR pval;
   SGUCHAR val;
   #ifdef GBUFFER
   GBUFINT gbufidx;
   #endif

   if (ghw_init() != 0)  /* (Re-) initialize display */
      return;            /* Some initialization error */

   /* Count number of lines in string */
   idx=str;
   if (idx == NULL)
      return;
   xcnt = 1;
   while(*idx)
      {
      if (*(idx++) == '\n')
         xcnt++;
      }

   /* Set start character line */
   h = SYSFONT.symheight;
   yp = (xcnt*h > GDISPH) ? 0 : ((GDISPH-1)-xcnt*h)/2;
   /* Set character height in pixel lines */

   idx=str;
   do
      {
      xcnt=0;  /* Set start x position so line is centered */
      while ((idx[xcnt]!=0) && (idx[xcnt]!='\n') && (xcnt < GDISPBW))
         {
         xcnt++;
         }

      /* Calculate start position for centered line */
      xp = (GDISPW-xcnt*SYSFONT.symwidth)/2;

      /* Display text line */
      while (xcnt-- > 0)
         {
         /* Point to graphic content for character symbol */
         psym = &(sysfontsym[(*idx) & 0x7f].b[0]);
         ghw_set_xyrange(xp,yp,xp+SYSFONT.symwidth-1,yp+(h-1));

         /* Display rows in symbol */
         for (y = 0; y < h; y++)
            {
            /* Get symbol row value */
            val = *psym++;
            /* Initiate LCD controller address pointer */
            #ifdef GBUFFER
            gbufidx = GINDEX(xp, (GBUFINT)yp+y );
            #endif

            /* Display colums in symbol row */
            for (sidx = 0; sidx < SYSFONT.symwidth; sidx++)
               {
               if ((val & sympixmsk[sidx]) != 0)
                  pval = ghw_def_foreground;
               else
                  pval = ghw_def_background;

               /* End of symbol or end of byte reached */
               #ifdef GBUFFER
               gbuf[gbufidx++] = pval;
               #endif
               ghw_auto_wr(pval);
               }
            }

         ghw_auto_wr_end();
         idx++;
         xp += SYSFONT.symwidth; /* Move to next symbol in line */
         }

      /* Next text line */
      yp += h;
      if (*idx == '\n')
         idx++;
      }
   while ((*idx != 0) && (yp < GDISPH));

   ghw_updatehw();  /* Flush to display hdw or simulator */
   }

void ghw_exit(void)
   {
   #if defined( GHW_ALLOCATE_BUF)
   if (gbuf != NULL)
      {
      if (gbuf_owner != 0)
         {
         /* Buffer is allocated by ginit, so release graphic buffer here */
         free(gbuf);
         gbuf_owner = 0;
         }
      gbuf = NULL;
      }
   #endif
   ghw_io_exit();         /* Release any LCD hardware resources, if required */
   #ifdef GHW_PCSIM
   ghw_exit_sim(); /* Release simulator resources */
   #endif
   }

#ifndef GNOCURSOR
/*
   Replace cursor type data (there is no HW cursor support in hx8312)
*/
void ghw_setcursor( GCURSOR type)
   {
   ghw_cursor = type;
   #ifdef GHW_ALLOCATE_BUF
   if (gbuf == NULL)
      glcd_err = 1;
   #endif
   }
#endif


/*
   Turn display off
*/
void ghw_dispoff(void)
   {
   #ifdef GHW_PCSIM
   ghw_dispoff_sim();
   #endif
   #if (defined( GHW_HX8346_REGINTF ) || defined( GHW_HX8325_REGINTF ))

   ghw_cmd_wr(GCTRL_DISP_CTRL1,0x38); /* GON=1,DTE=1,D1-0=10 */
   ghw_cmd_wait( 100 );               /* Wait 2 frames */
   ghw_cmd_wr(GCTRL_DISP_CTRL1,0x28); /* GON=1,DTE=0,D1-0=10 */
   ghw_cmd_wait( 100 );               /* Wait 2 frames */
   ghw_cmd_wr(GCTRL_DISP_CTRL1,0x00); /* GON=0,DTE=1,D1-0=00 */
   #elif defined( GHW_HX8347G ) || defined( GHW_HX8352B )
   ghw_cmd_wr(GCTRL_DISP_CTRL3,0x38); /* GON=1,DTE=1,D1-0=10 */
   ghw_cmd_wait( 100 );               /* Wait 2 frames */
   ghw_cmd_wr(GCTRL_DISP_CTRL3,0x04); /* GON=1,DTE=1,D1-0=11 */
   ghw_cmd_wait( 100 );               /* Wait 2 frames */
   #else
   ghw_cmd(GCTRL_DISPOFF);
   #endif
   }

/*
   Turn display on (to be used after a ghw_pwroff() call)
*/
void ghw_dispon(void)
   {
   #ifdef GHW_PCSIM
   ghw_dispon_sim();
   #endif

   #if (defined( GHW_HX8346_REGINTF ) || defined( GHW_HX8325_REGINTF ))
   ghw_cmd_wr(GCTRL_DISP_CTRL1,0x08); /* GON=0,DTE=0,D1-0=01 */
   ghw_cmd_wait( 100 );               /* Wait 2 frames */
   ghw_cmd_wr(GCTRL_DISP_CTRL1,0x28); /* GON=1,DTE=0,D1-0=01 */
   ghw_cmd_wr(GCTRL_DISP_CTRL1,0x2c); /* GON=1,DTE=0,D1-0=11 */
   ghw_cmd_wait( 100 );               /* Wait 2 frames */
   ghw_cmd_wr(GCTRL_DISP_CTRL1,0x3c); /* GON=1,DTE=1,D1-0=11 */
   #elif defined( GHW_HX8347G ) || defined( GHW_HX8352B )
   ghw_cmd_wr(GCTRL_DISP_CTRL3,0x38); /* GON=1,DTE=1,D1-0=10 */
   ghw_cmd_wait( 100 );               /* Wait 2 frames */
   ghw_cmd_wr(GCTRL_DISP_CTRL3,0x3c); /* GON=1,DTE=1,D1-0=11 */
   ghw_cmd_wait( 100 );               /* Wait 2 frames */
   #else
   ghw_cmd(GCTRL_DISPON);
   #endif
   }

#if defined( GHW_ALLOCATE_BUF)
/*
   Size of buffer requied to save the whole screen state
*/
GBUFINT ghw_gbufsize( void )
   {
   return (GBUFINT) GBUFSIZE * sizeof(GCOLOR) + (GBUFINT) sizeof(GHW_STATE);
   }

#ifdef GSCREENS
/*
   Check if screen buf owns the screen ressources.
*/
SGUCHAR ghw_is_owner( SGUCHAR *buf )
   {
   return (((GCOLOR *)buf == gbuf) && (gbuf != NULL)) ? 1 : 0;
   }

/*
   Save the current state to the screen buffer
*/
SGUCHAR *ghw_save_state( SGUCHAR *buf )
   {
   GHW_STATE *ps;
   if (!ghw_is_owner(buf))
      return NULL;

   ps = (GHW_STATE *)(&gbuf[GBUFSIZE]);
   ps->upddelay = (ghw_upddelay != 0);
   #ifndef GNOCURSOR
   ps->cursor = ghw_cursor;
   #endif
   ps->foreground = ghw_def_foreground; /* Palette may vary, save it */
   ps->background = ghw_def_background;
   return (SGUCHAR *) gbuf;
   }

/*
   Set state to buf.
   If buffer has not been initiated by to a screen before, only
   the pointer is updated. Otherwise the the buffer
*/
void ghw_set_state(SGUCHAR *buf, SGUCHAR doinit)
   {
   if (gbuf != NULL)
      {
      /* The LCD controller has been initiated before */
      if (gbuf_owner != 0)
         {
         /* Buffer was allocated by ginit, free it so screen can be used instead*/
         free(gbuf);
         gbuf_owner = 0;
         gbuf = NULL;
         }
      }

   if (doinit != 0)
      {
      /* First screen initialization, just set buffer pointer and
         leave rest of initialization to a later call of ghw_init() */
      gbuf = (GCOLOR *) buf;
      gbuf_owner = 0;
      }
   else
      {
      gbuf = (GCOLOR *) buf;
      if (gbuf != NULL)
         {
         GHW_STATE *ps;
         ps = (GHW_STATE *)(&gbuf[GBUFSIZE]);

         #ifndef GNOCURSOR
         ghw_cursor = ps->cursor;
         #endif
         ghw_upddelay = 0;        /* Force update of whole screen */
         iltx = 0;
         ilty = 0;
         irbx = GDISPW-1;
         irby = GDISPH-1;
         ghw_updatehw();
         ghw_upddelay = (ps->upddelay != 0) ? 1 : 0;
         /* Restore drawing color */
         ghw_setcolor(ps->foreground, ps->background);
         }
      }
   }
#endif  /* GSCREENS */
#endif  /* GHW_ALLOCATE_BUF */

#endif /* GBASIC_INIT_ERR */

