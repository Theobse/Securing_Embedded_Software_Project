/* ----------------------------------------------------------------------
 * Project:      NMSIS DSP Library
 * Title:        riscv_vlog_q15
 * Description:  Q15 vector log
 *
 * $Date:        19 July 2021
 * $Revision:    V1.10.0
 *
 * Target Processor: RISC-V Cores
 * -------------------------------------------------------------------- */
/*
 * Copyright (C) 2010-2021 ARM Limited or its affiliates. All rights reserved.
 * Copyright (c) 2019 Nuclei Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "dsp/fast_math_functions.h"


#define LOG_Q15_ACCURACY 15

/* Bit to represent the normalization factor
   It is Ceiling[Log2[LOG_Q15_ACCURACY]] of the previous value.
   The Log2 algorithm is assuming that the value x is
   1 <= x < 2.

   But input value could be as small a 2^-LOG_Q15_ACCURACY
   which would give an integer part of -15.
*/
#define LOG_Q15_INTEGER_PART 4

/* 2.0 in q14 */
#define LOQ_Q15_THRESHOLD (1u << LOG_Q15_ACCURACY)

/* HALF */
#define LOQ_Q15_Q16_HALF LOQ_Q15_THRESHOLD
#define LOQ_Q15_Q14_HALF (LOQ_Q15_Q16_HALF >> 2)


/* 1.0 / Log2[Exp[1]] in q15 */
#define LOG_Q15_INVLOG2EXP 0x58b9u


/* Clay Turner algorithm */
static uint16_t riscv_scalar_log_q15(uint16_t src)
{
   int i;

   int16_t c = __CLZ(src)-16;
   int16_t normalization=0;

   /* 0.5 in q11 */
   uint16_t inc = LOQ_Q15_Q16_HALF >> (LOG_Q15_INTEGER_PART + 1);

   /* Will compute y = log2(x) for 1 <= x < 2.0 */
   uint16_t x;

   /* q11 */
   uint16_t y=0;

   /* q11 */
   int16_t tmp;


   /* Normalize and convert to q14 format */
   x = src;
   if ((c-1) < 0)
   {
     x = x >> (1-c);
   }
   else
   {
     x = x << (c-1);
   }
   normalization = c;



   /* Compute the Log2. Result is in q11 instead of q16
      because we know 0 <= y < 1.0 but
      we want a result allowing to do a
      product on int16 rather than having to go
      through int32
   */
   for(i = 0; i < LOG_Q15_ACCURACY ; i++)
   {
      x = (((int32_t)x*x)) >> (LOG_Q15_ACCURACY - 1);

      if (x >= LOQ_Q15_THRESHOLD)
      {
         y += inc ;
         x = x >> 1;
      }
      inc = inc >> 1;
   }


   /*
      Convert the Log2 to Log and apply normalization.
      We compute (y - normalisation) * (1 / Log2[e]).

   */

   /* q11 */
   //tmp = y - ((int32_t)normalization << (LOG_Q15_ACCURACY + 1));
   tmp = (int16_t)y - (normalization << (LOG_Q15_ACCURACY - LOG_Q15_INTEGER_PART));

   /* q4.11 */
   y = ((int32_t)tmp * LOG_Q15_INVLOG2EXP) >> 15;

   return(y);

}


/**
  @ingroup groupFastMath
 */

/**
  @addtogroup vlog
  @{
 */

/**
  @brief         q15 vector of log values.
  @param[in]     pSrc       points to the input vector in q15
  @param[out]    pDst       points to the output vector in q4.11
  @param[in]     blockSize  number of samples in each vector
  @return        none

 */

void riscv_vlog_q15(
  const q15_t * pSrc,
        q15_t * pDst,
        uint32_t blockSize)
{
  uint32_t  blkCnt;           /* loop counters */

  blkCnt = blockSize;

  while (blkCnt > 0U)
  {
     *pDst++ = riscv_scalar_log_q15(*pSrc++);

     /* Decrement loop counter */
     blkCnt--;
  }
}

/**
  @} end of vlog group
 */
