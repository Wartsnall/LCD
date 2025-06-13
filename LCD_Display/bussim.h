#ifndef BUSSIM_H
#define BUSSIM_H
/***************************** bussim.c ************************************

   Definitions for the OLED bus simulator access functions.

   The definitions below should be modified to reflect the port layout in
   the target system and the given access function implementation.

   The definitions below assume the following hardware bit functionality of
   the "address" byte parameter to the access functions.

      A0 = RS  (Data/command select)

   RAMTEX Engineering Aps 2004
   Web site, support and upgrade: www.ramtex.dk

****************************************************************************/

/* Define SPI address byte for display controller, modify to fit hardware */
#define DEVICE_ADRESS 0x70


/*************** Do not modify the definitions below ********************/

#include <gdisphw.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Single chip mode -> access via user supplied access driver functions */

#define GHWRD   (DEVICE_ADRESS|0x3)  /* RS=1, RW=1 */
#define GHWWR   (DEVICE_ADRESS|0x2)  /* RS=1, RW=0 */
#define GHWSTA  (DEVICE_ADRESS|0x1)  /* RS=0, RW=1 */
#define GHWCMD  (DEVICE_ADRESS|0x0)  /* RS=0, RW=0 */
#define GHWRDW  (DEVICE_ADRESS|0x3)  /* RS=1, RW=1 */
#define GHWWRW  (DEVICE_ADRESS|0x2)  /* RS=1, RW=0 */
#define GHWSTAW (DEVICE_ADRESS|0x1)  /* RS=0, RW=1 */
#define GHWCMDW (DEVICE_ADRESS|0x0)  /* RS=0, RW=0 */


void simwrwo(   SGUCHAR address, SGUINT dat);
SGUINT simrdwo( SGUCHAR address);
void sim_reset( void );

#ifdef __cplusplus
}
#endif

#endif /* BUSSIM_H */
