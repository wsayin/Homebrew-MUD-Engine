/******************************************************************************
 *                                                                            *
 *    ###      ###  ##      ##  ########    ##########                        *
 *    #####  #####  ##      ##  ##     ##   ##                                *
 *    ##  ####  ##  ##      ##  ##      ##  ##                                *
 *    ##   ##   ##  ##      ##  ##      ##  ######     ## #####    #######    *
 *    ##        ##  ##      ##  ##      ##  ##          ##    ##  ##     ##   *
 *    ##        ##  ##      ##  ##      ##  ##          ##    ##  ##     ##   *
 *    ##        ##   ##    ##   ##     ##   ##          ##    ##  ##     ##   *
 *    ##        ##    ######    ########    ##########  ##    ##   ########   *
 *                                                                       ##   *
 *                                                                ##     ##   *
 *                                                                 ##    ##   *
 *                                                                  ######    *
 * Developed by William Sayin                                                 *
 * www.github.com/wsayin                                                      *
 * ************************************************************************** *
 * This Source Code Form is subject to the terms of the Mozilla Public        *
 * License, v. 2.0. If a copy of the MPL was not distributed with this        *
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.                   *
 *                                                                            *
 ******************************************************************************/
#include <stdint.h>

#define PHI 0x9e3779b9

static uint32_t Q[4096], c = 362436;
 
void init_rand(uint32_t x)
{
   int i;

   Q[0] = x;
   Q[1] = x + PHI;
   Q[2] = x + PHI + PHI;

   for (i = 3; i < 4096; i++)
      Q[i] = Q[i - 3] ^ Q[i - 2] ^ PHI ^ i;
}
 
uint32_t rand_cmwc(void)
{
   static uint32_t i = 4095;
   uint64_t t;

   i = (i + 1) & 4095;
   t = (18705ULL * Q[i]) + c;
   c = t >> 32;
   Q[i] = 0xfffffffe - t;

   return Q[i];
}
