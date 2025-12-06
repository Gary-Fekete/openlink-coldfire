/*
 * File:		common.h
 * Purpose:		File to be included by all project files
 *
 * Notes:
 */

#ifndef _COMMON_H_
#define _COMMON_H_

#define M5223EVB
/********************************************************************/

/*
 * Debug prints ON (#define) or OFF (#undef)
 */

/* #define DEBUG_PRINT 1  */
/* #define DEBUG_PRINT_D0D1 1  */

/* 
 * Include the generic CPU header file 
 */
#include "mcf5xxx.h"

/*
 * Include the specific CPU header file
 */
#include "mcf5223/mcf5223.h"

/* 
 * Include the board specific header file 
 */

#if (defined(M5223EVB))
#include "m5223evb.h"
#else
#error "No valid platform defined"
#endif

/* 
 * Include any toolchain specfic header files 
 */

/*
#if (defined(__MWERKS__))
#include "build/mwerks/mwerks.h"
#define __CFM68K__
#define __MC68K__ 0
#elif (defined(__DCC__))
#include "build/wrs/diab.h"
#elif (defined(__ghs__))
#include "build/ghs/ghs.h"
#endif
*/

/* 
 * Include common utilities
 */

/*
#include "assert.h"
#include "io.h"
#include "stdlib.h"
*/
/********************************************************************/

#endif /* _COMMON_H_ */
