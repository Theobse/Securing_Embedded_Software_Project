/* ----------------------------------------------------------------------
 * Project:      NMSIS DSP Library
 * Title:        riscv_min_no_idx_q15.c
 * Description:  Minimum value of a q15 vector without returning the index
 *
 * $Date:        16 November 2021
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

#include "dsp/statistics_functions.h"


/**
  @ingroup groupStats
 */

/**
  @addtogroup Min
  @{
 */

/**
  @brief         Minimum value of a q15 vector without index.
  @param[in]     pSrc       points to the input vector
  @param[in]     blockSize  number of samples in input vector
  @param[out]    pResult    minimum value returned here
  @return        none
 */

void riscv_min_no_idx_q15(
  const q15_t * pSrc,
        uint32_t blockSize,
        q15_t * pResult)
{
  q15_t minVal1, out;       /* Temporary variables to store the output value. */
  uint32_t blkCnt;              /* loop counter */

#if defined(RISCV_MATH_VECTOR)
    size_t l;
    const q15_t *inputx = pSrc;
    vint16m8_t v_x;
    vint16m1_t v_tempa;
    blkCnt = blockSize;
    l = __riscv_vsetvl_e16m1(1);
    v_tempa = __riscv_vmv_s_x_i16m1(pSrc[0], l);
    for (; (l = __riscv_vsetvl_e16m8(blkCnt)) > 0; blkCnt -= l) {
        v_x = __riscv_vle16_v_i16m8(inputx, l);
        inputx += l;
        v_tempa = __riscv_vredmin_vs_i16m8_i16m1(v_x, v_tempa, l);
    }
    out = __riscv_vmv_x_s_i16m1_i16(v_tempa);
#else

  /* Load first input value that act as reference value for comparision */
  out = *pSrc++;

  blkCnt = (blockSize - 1U);


  while (blkCnt > 0U)
  {
    /* Initialize minVal to the next consecutive values one by one */
    minVal1 = *pSrc++;

    /* compare for the minimum value */
    if (out > minVal1)
    {
      /* Update the minimum value */
      out = minVal1;
    }

    /* Decrement the loop counter */
    blkCnt--;
  }
#endif /* #if defined(RISCV_MATH_VECTOR) */
  /* Store the minimum value into destination pointer */
  *pResult = out;
}

/**
  @} end of Min group
 */
