/* ----------------------------------------------------------------------
 * Project:      NMSIS DSP Library
 * Title:        riscv_fir_sparse_q15.c
 * Description:  Q15 sparse FIR filter processing function
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

#include "dsp/filtering_functions.h"

/**
  @ingroup groupFilters
 */

/**
  @addtogroup FIR_Sparse
  @{
 */

/**
  @brief         Processing function for the Q15 sparse FIR filter.
  @param[in]     S           points to an instance of the Q15 sparse FIR structure
  @param[in]     pSrc        points to the block of input data
  @param[out]    pDst        points to the block of output data
  @param[in]     pScratchIn  points to a temporary buffer of size blockSize
  @param[in]     pScratchOut points to a temporary buffer of size blockSize
  @param[in]     blockSize   number of input samples to process per call
  @return        none

  @par           Scaling and Overflow Behavior
                   The function is implemented using an internal 32-bit accumulator.
                   The 1.15 x 1.15 multiplications yield a 2.30 result and these are added to a 2.30 accumulator.
                   Thus the full precision of the multiplications is maintained but there is only a single guard bit in the accumulator.
                   If the accumulator result overflows it will wrap around rather than saturate.
                   After all multiply-accumulates are performed, the 2.30 accumulator is truncated to 2.15 format and then saturated to 1.15 format.
                   In order to avoid overflows the input signal or coefficients must be scaled down by log2(numTaps) bits.
 */

void riscv_fir_sparse_q15(
        riscv_fir_sparse_instance_q15 * S,
  const q15_t * pSrc,
        q15_t * pDst,
        q15_t * pScratchIn,
        q31_t * pScratchOut,
        uint32_t blockSize)
{
        q15_t *pState = S->pState;                     /* State pointer */
  const q15_t *pCoeffs = S->pCoeffs;                   /* Coefficient pointer */
        q15_t *px;                                     /* Temporary pointers for scratch buffer */
        q15_t *py = pState;                            /* Temporary pointers for state buffer */
        q15_t *pb = pScratchIn;                        /* Temporary pointers for scratch buffer */
        q15_t *pOut = pDst;                            /* Working pointer for output */
        int32_t *pTapDelay = S->pTapDelay;             /* Pointer to the array containing offset of the non-zero tap values. */
        uint32_t delaySize = S->maxDelay + blockSize;  /* state length */
        uint16_t numTaps = S->numTaps;                 /* Number of filter coefficients in the filter  */
        int32_t readIndex;                             /* Read index of the state buffer */
        uint32_t tapCnt, blkCnt;                       /* loop counters */
        q31_t *pScr2 = pScratchOut;                    /* Working pointer for scratch buffer of output values */
        q15_t coeff = *pCoeffs++;                      /* Read the first coefficient value */

#if defined (RISCV_MATH_LOOPUNROLL)
        q31_t in1, in2;                                /* Temporary variables */
#if defined (RISCV_MATH_DSP)
        q31_t coeff32;
#endif /* defined (RISCV_MATH_DSP) */
#endif /* defined (RISCV_MATH_LOOPUNROLL) */

  /* BlockSize of Input samples are copied into the state buffer */
  /* StateIndex points to the starting position to write in the state buffer */
  riscv_circularWrite_q15(py, (int32_t) delaySize, &S->stateIndex, 1,pSrc, 1, blockSize);

  /* Loop over the number of taps. */
  tapCnt = numTaps;

  /* Read Index, from where the state buffer should be read, is calculated. */
  readIndex = (int32_t) (S->stateIndex - blockSize) - *pTapDelay++;

  /* Wraparound of readIndex */
  if (readIndex < 0)
  {
    readIndex += (int32_t) delaySize;
  }

  /* Working pointer for state buffer is updated */
  py = pState;

  /* blockSize samples are read from the state buffer */
  riscv_circularRead_q15(py, (int32_t) delaySize, &readIndex, 1,
                       pb, pb, (int32_t) blockSize, 1, blockSize);

  /* Working pointer for the scratch buffer of state values */
  px = pb;

  /* Working pointer for scratch buffer of output values */
  pScratchOut = pScr2;

#if defined (RISCV_MATH_VECTOR)
  uint32_t vblkCnt = blockSize;
  size_t l;
  vint16m4_t vx;
  for (; (l = __riscv_vsetvl_e16m4(vblkCnt)) > 0; vblkCnt -= l) {
    vx = __riscv_vle16_v_i16m4(px, l);
    px += l;
    __riscv_vse32_v_i32m8(pScratchOut, __riscv_vwmul_vx_i32m8(vx, coeff, l), l);
    pScratchOut += l;
  }
#else
#if defined (RISCV_MATH_LOOPUNROLL)

  /* Loop unrolling: Compute 4 outputs at a time. */
  blkCnt = blockSize >> 2U;

#if defined (RISCV_MATH_DSP)
#if __RISCV_XLEN == 64
  coeff32 = __RV_PKBB16(coeff, coeff);
#else
#if defined (NUCLEI_DSP_N2)
  coeff32 = __RV_DPKBB16(coeff, coeff);
#else
  coeff32 = (((q31_t)coeff) << 16) | ((q31_t)coeff & 0xffff);
#endif /* defined (NUCLEI_DSP_N2) */
#endif /* __RISCV_XLEN == 64 */
#endif /* defined (RISCV_MATH_DSP) */

  while (blkCnt > 0U)
  {
#if defined (RISCV_MATH_DSP)
    write_q31x2_ia(&pScratchOut, (q63_t)__RV_SMUL16(read_q15x2_ia(&px), coeff32));
    write_q31x2_ia(&pScratchOut, (q63_t)__RV_SMUL16(read_q15x2_ia(&px), coeff32));
#else
    /* Perform multiplication and store in the scratch buffer */
    *pScratchOut++ = ((q31_t) *px++ * coeff);
    *pScratchOut++ = ((q31_t) *px++ * coeff);
    *pScratchOut++ = ((q31_t) *px++ * coeff);
    *pScratchOut++ = ((q31_t) *px++ * coeff);
#endif /* (RISCV_MATH_DSP) */

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
    /* Perform Multiplication and store in the scratch buffer */
    *pScratchOut++ = ((q31_t) *px++ * coeff);

    /* Decrement loop counter */
    blkCnt--;
  }
#endif /* defined (RISCV_MATH_VECTOR) */
  /* Load the coefficient value and
   * increment the coefficient buffer for the next set of state values */
  coeff = *pCoeffs++;

  /* Read Index, from where the state buffer should be read, is calculated. */
  readIndex = (int32_t) (S->stateIndex - blockSize) - *pTapDelay++;

  /* Wraparound of readIndex */
  if (readIndex < 0)
  {
    readIndex += (int32_t) delaySize;
  }

  /* Loop over the number of taps. */
  tapCnt = (uint32_t) numTaps - 2U;

  while (tapCnt > 0U)
  {
    /* Working pointer for state buffer is updated */
    py = pState;

    /* blockSize samples are read from the state buffer */
    riscv_circularRead_q15(py, (int32_t) delaySize, &readIndex, 1,
                         pb, pb, (int32_t) blockSize, 1, blockSize);

    /* Working pointer for the scratch buffer of state values */
    px = pb;

    /* Working pointer for scratch buffer of output values */
    pScratchOut = pScr2;

#if defined (RISCV_MATH_VECTOR)
    vblkCnt = blockSize;
    for (; (l = __riscv_vsetvl_e16m4(vblkCnt)) > 0; vblkCnt -= l) {
      vx = __riscv_vle16_v_i16m4(px, l);
      px += l;
      __riscv_vse32_v_i32m8(pScratchOut, __riscv_vadd_vv_i32m8(__riscv_vle32_v_i32m8(pScratchOut, l), __riscv_vwmul_vx_i32m8(vx, coeff, l), l), l);
      pScratchOut += l;
    }
#else
#if defined (RISCV_MATH_LOOPUNROLL)

    /* Loop unrolling: Compute 4 outputs at a time. */
    blkCnt = blockSize >> 2U;

    while (blkCnt > 0U)
    {
      /* Perform Multiply-Accumulate */
      *pScratchOut++ += (q31_t) *px++ * coeff;
      *pScratchOut++ += (q31_t) *px++ * coeff;
      *pScratchOut++ += (q31_t) *px++ * coeff;
      *pScratchOut++ += (q31_t) *px++ * coeff;

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
      /* Perform Multiply-Accumulate */
      *pScratchOut++ += (q31_t) *px++ * coeff;

      /* Decrement loop counter */
      blkCnt--;
    }
#endif /* defined (RISCV_MATH_VECTOR) */

    /* Load the coefficient value and
     * increment the coefficient buffer for the next set of state values */
    coeff = *pCoeffs++;

    /* Read Index, from where the state buffer should be read, is calculated. */
    readIndex = (int32_t) (S->stateIndex - blockSize) - *pTapDelay++;

    /* Wraparound of readIndex */
    if (readIndex < 0)
    {
      readIndex += (int32_t) delaySize;
    }

    /* Decrement loop counter */
    tapCnt--;
  }

  /* Compute last tap without the final read of pTapDelay */

  /* Working pointer for state buffer is updated */
  py = pState;

  /* blockSize samples are read from the state buffer */
  riscv_circularRead_q15(py, (int32_t) delaySize, &readIndex, 1,
                       pb, pb, (int32_t) blockSize, 1, blockSize);

  /* Working pointer for the scratch buffer of state values */
  px = pb;

  /* Working pointer for scratch buffer of output values */
  pScratchOut = pScr2;

#if defined (RISCV_MATH_VECTOR)
  vblkCnt = blockSize;
  for (; (l = __riscv_vsetvl_e16m4(vblkCnt)) > 0; vblkCnt -= l) {
    vx = __riscv_vle16_v_i16m4(px, l);
    px += l;
    __riscv_vse32_v_i32m8(pScratchOut, __riscv_vadd_vv_i32m8(__riscv_vle32_v_i32m8(pScratchOut, l), __riscv_vwmul_vx_i32m8(vx, coeff, l), l), l);
    pScratchOut += l;
  }
#else
#if defined (RISCV_MATH_LOOPUNROLL)

  /* Loop unrolling: Compute 4 outputs at a time. */
  blkCnt = blockSize >> 2U;

  while (blkCnt > 0U)
  {
    /* Perform Multiply-Accumulate */
    *pScratchOut++ += (q31_t) *px++ * coeff;
    *pScratchOut++ += (q31_t) *px++ * coeff;
    *pScratchOut++ += (q31_t) *px++ * coeff;
    *pScratchOut++ += (q31_t) *px++ * coeff;

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
    /* Perform Multiply-Accumulate */
    *pScratchOut++ += (q31_t) *px++ * coeff;

    /* Decrement loop counter */
    blkCnt--;
  }
#endif /* defined (RISCV_MATH_VECTOR) */

  /* All the output values are in pScratchOut buffer.
     Convert them into 1.15 format, saturate and store in the destination buffer. */
#if defined (RISCV_MATH_LOOPUNROLL)

  /* Loop unrolling: Compute 4 outputs at a time. */
  blkCnt = blockSize >> 2U;

  while (blkCnt > 0U)
  {
    in1 = *pScr2++;
    in2 = *pScr2++;

    write_q15x2_ia (&pOut, __PKHBT((q15_t) __SSAT(in1 >> 15, 16), (q15_t) __SSAT(in2 >> 15, 16), 16));

    in1 = *pScr2++;
    in2 = *pScr2++;

    write_q15x2_ia (&pOut, __PKHBT((q15_t) __SSAT(in1 >> 15, 16), (q15_t) __SSAT(in2 >> 15, 16), 16));

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
    *pOut++ = (q15_t) __SSAT(*pScr2++ >> 15, 16);

    /* Decrement loop counter */
    blkCnt--;
  }

}

/**
  @} end of FIR_Sparse group
 */
