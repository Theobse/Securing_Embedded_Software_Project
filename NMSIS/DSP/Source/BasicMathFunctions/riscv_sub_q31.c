/* ----------------------------------------------------------------------
 * Project:      NMSIS DSP Library
 * Title:        riscv_sub_q31.c
 * Description:  Q31 vector subtraction
 *
 * $Date:        23 April 2021
 * $Revision:    V1.9.0
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

#include "dsp/basic_math_functions.h"

/**
  @ingroup groupMath
 */

/**
  @addtogroup BasicSub
  @{
 */

/**
  @brief         Q31 vector subtraction.
  @param[in]     pSrcA      points to the first input vector
  @param[in]     pSrcB      points to the second input vector
  @param[out]    pDst       points to the output vector
  @param[in]     blockSize  number of samples in each vector
  @return        none

  @par           Scaling and Overflow Behavior
                   The function uses saturating arithmetic.
                   Results outside of the allowable Q31 range [0x80000000 0x7FFFFFFF] are saturated.
 */

void riscv_sub_q31(
  const q31_t * pSrcA,
  const q31_t * pSrcB,
        q31_t * pDst,
        uint32_t blockSize)
{
  uint32_t blkCnt;                               /* Loop counter */

#if defined(RISCV_MATH_VECTOR)
  blkCnt = blockSize;                               /* Loop counter */
  size_t l;
  vint32m8_t vx, vy;

  for (; (l = __riscv_vsetvl_e32m8(blkCnt)) > 0; blkCnt -= l)
  {
    vx = __riscv_vle32_v_i32m8(pSrcA, l);
    pSrcA += l;
    vy = __riscv_vle32_v_i32m8(pSrcB, l);
    pSrcB += l;
    __riscv_vse32_v_i32m8(pDst, __riscv_vssub_vv_i32m8(vx, vy, l), l);
    pDst += l;
  }
#else

#if defined (RISCV_MATH_LOOPUNROLL)

  /* Loop unrolling: Compute 4 outputs at a time */
  blkCnt = blockSize >> 2U;
  while (blkCnt > 0U)
  {
    /* C = A - B */
#if defined (RISCV_MATH_DSP) && __RISCV_XLEN == 64
    write_q31x2_ia(&pDst, __RV_KSUB32(read_q31x2_ia((q31_t **)&pSrcA), read_q31x2_ia((q31_t **)&pSrcB)));
    write_q31x2_ia(&pDst, __RV_KSUB32(read_q31x2_ia((q31_t **)&pSrcA), read_q31x2_ia((q31_t **)&pSrcB)));
#else
#if defined (RISCV_MATH_DSP) && defined (NUCLEI_DSP_N2)
    write_q31x2_ia(&pDst, __dksub32(read_q31x2_ia((q31_t **)&pSrcA), read_q31x2_ia((q31_t **)&pSrcB)));
    write_q31x2_ia(&pDst, __dksub32(read_q31x2_ia((q31_t **)&pSrcA), read_q31x2_ia((q31_t **)&pSrcB)));
#else
    /* Subtract and store result in destination buffer. */
    *pDst++ = __QSUB(*pSrcA++, *pSrcB++);
    *pDst++ = __QSUB(*pSrcA++, *pSrcB++);
    *pDst++ = __QSUB(*pSrcA++, *pSrcB++);
    *pDst++ = __QSUB(*pSrcA++, *pSrcB++);

#endif /* RISCV_MATH_DSP && NUCLEI_DSP_N2 */
#endif /* RISCV_MATH_DSP && __RISCV_XLEN == 64 */
    /* Decrement loop counter */
    blkCnt--;
  }

  /* Loop unrolling: Compute remaining outputs */
  blkCnt = blockSize & 0x3U;

#else

  /* Initialize blkCnt with number of samples */
  blkCnt = blockSize;

#endif /* #if defined (RISCV_MATH_LOOPUNROLL) */

  while (blkCnt > 0U)
  {
    /* C = A - B */

    /* Subtract and store result in destination buffer. */
    *pDst++ = __QSUB(*pSrcA++, *pSrcB++);

    /* Decrement loop counter */
    blkCnt--;
  }
#endif /* defined(RISCV_MATH_VECTOR) */
}

/**
  @} end of BasicSub group
 */
