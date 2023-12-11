/* ----------------------------------------------------------------------
 * Project:      NMSIS DSP Library
 * Title:        riscv_mat_mult_f16.c
 * Description:  Floating-point matrix multiplication
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

#include "dsp/matrix_functions_f16.h"

#if defined(RISCV_FLOAT16_SUPPORTED)


/**
 * @ingroup groupMatrix
 */


/**
 * @addtogroup MatrixMult
 * @{
 */

/**
 * @brief Floating-point matrix multiplication.
 * @param[in]       *pSrcA points to the first input matrix structure
 * @param[in]       *pSrcB points to the second input matrix structure
 * @param[out]      *pDst points to output matrix structure
 * @return     		The function returns either
 * <code>RISCV_MATH_SIZE_MISMATCH</code> or <code>RISCV_MATH_SUCCESS</code> based on the outcome of size checking.
 */



riscv_status riscv_mat_mult_f16(
  const riscv_matrix_instance_f16 * pSrcA,
  const riscv_matrix_instance_f16 * pSrcB,
        riscv_matrix_instance_f16 * pDst)
{
  float16_t *pIn1 = pSrcA->pData;                /* Input data matrix pointer A */
  float16_t *pIn2 = pSrcB->pData;                /* Input data matrix pointer B */
  float16_t *pInA = pSrcA->pData;                /* Input data matrix pointer A */
  float16_t *pInB = pSrcB->pData;                /* Input data matrix pointer B */
  float16_t *pOut = pDst->pData;                 /* Output data matrix pointer */
  float16_t *px;                                 /* Temporary output data matrix pointer */
  _Float16 sum;                                 /* Accumulator */
  uint16_t numRowsA = pSrcA->numRows;            /* Number of rows of input matrix A */
  uint16_t numColsB = pSrcB->numCols;            /* Number of columns of input matrix B */
  uint16_t numColsA = pSrcA->numCols;            /* Number of columns of input matrix A */
  uint32_t col, i = 0U, row = numRowsA, colCnt;  /* Loop counters */
  riscv_status status;                             /* Status of matrix multiplication */

#ifdef RISCV_MATH_MATRIX_CHECK

  /* Check for matrix mismatch condition */
  if ((pSrcA->numCols != pSrcB->numRows) ||
      (pSrcA->numRows != pDst->numRows)  ||
      (pSrcB->numCols != pDst->numCols)    )
  {
    /* Set status as RISCV_MATH_SIZE_MISMATCH */
    status = RISCV_MATH_SIZE_MISMATCH;
  }
  else

#endif /* #ifdef RISCV_MATH_MATRIX_CHECK */

  {
#if defined(RISCV_MATH_VECTOR)
    uint32_t ii, jj, kk;
    size_t l;
    ptrdiff_t bstride = 2;       //  16bit/8bit = 2
    vfloat16m4x2_t v_tuple;
    vfloat16m4_t va0m4, vres0m4, vres1m4, vres2m4, vres3m4;
    vfloat16m8_t va0m8, vres0m8, vres1m8;
    colCnt = numColsB;

    /* ch = 4, mul = 4 */
    for (jj = colCnt / 4; jj > 0; jj--) {
      px = pOut;
      pInA = pIn1;
      for (ii = numRowsA; ii > 0; ii -= l) {
        l = __riscv_vsetvl_e16m4(ii);
        pInB = pIn2;
        vres0m4 = __riscv_vfmv_v_f_f16m4(0.0, l);
        vres1m4 = __riscv_vmv_v_v_f16m4(vres0m4, l);
        vres2m4 = __riscv_vmv_v_v_f16m4(vres0m4, l);
        vres3m4 = __riscv_vmv_v_v_f16m4(vres0m4, l);
        for (kk = 0; kk < numColsA; kk++) {
          va0m4 = __riscv_vlse16_v_f16m4(pInA + kk, numColsA * bstride, l);
          vres0m4 = __riscv_vfmacc_vf_f16m4(vres0m4, *(pInB + 0), va0m4, l);
          vres1m4 = __riscv_vfmacc_vf_f16m4(vres1m4, *(pInB + 1), va0m4, l);
          vres2m4 = __riscv_vfmacc_vf_f16m4(vres2m4, *(pInB + 2), va0m4, l);
          vres3m4 = __riscv_vfmacc_vf_f16m4(vres3m4, *(pInB + 3), va0m4, l);
          pInB += numColsB;
        }
        //vssseg2e16_v_f16m4(px, numColsB * bstride, vres0m4, vres1m4, l);
       v_tuple = __riscv_vset_v_f16m4_f16m4x2 (v_tuple, 0, vres0m4);
       v_tuple = __riscv_vset_v_f16m4_f16m4x2 (v_tuple, 1, vres1m4);
       __riscv_vssseg2e16_v_f16m4x2 (px, numColsB * bstride, v_tuple, l);
        //vssseg2e16_v_f16m4(px + 2, numColsB * bstride, vres2m4, vres3m4, l);
       v_tuple = __riscv_vset_v_f16m4_f16m4x2 (v_tuple, 0, vres2m4);
       v_tuple = __riscv_vset_v_f16m4_f16m4x2 (v_tuple, 1, vres3m4);
       __riscv_vssseg2e16_v_f16m4x2 (px + 2, numColsB * bstride, v_tuple, l);
        px += l * numColsB;
        pInA += l * numColsA;
      }
      pIn2 += 4;
      pOut += 4;
    }
    /* ch = 2, mul = 8 */
    colCnt = colCnt & 0x3;
    for (jj = colCnt / 2; jj > 0; jj--) {
      px = pOut;
      pInA = pIn1;
      for (ii = numRowsA; ii > 0; ii -= l) {
        l = __riscv_vsetvl_e16m8(ii);
        pInB = pIn2;
        vres0m8 = __riscv_vfmv_v_f_f16m8(0.0, l);
        vres1m8 = __riscv_vmv_v_v_f16m8(vres0m8, l);
        for (kk = 0; kk < numColsA; kk++) {
          va0m8 = __riscv_vlse16_v_f16m8(pInA + kk, numColsA * bstride, l);
          vres0m8 = __riscv_vfmacc_vf_f16m8(vres0m8, *(pInB + 0), va0m8, l);
          vres1m8 = __riscv_vfmacc_vf_f16m8(vres1m8, *(pInB + 1), va0m8, l);
          pInB += numColsB;
        }
        __riscv_vsse16_v_f16m8(px, numColsB * bstride, vres0m8, l);
        __riscv_vsse16_v_f16m8(px + 1, numColsB * bstride, vres1m8, l);
        px += l * numColsB;
        pInA += l * numColsA;
      }
      pIn2 += 2;
      pOut += 2;
    }
    /* ch = 1, mul = 8 */
    colCnt = colCnt & 0x1;
    for (jj = colCnt; jj > 0; jj--) {
      px = pOut;
      pInA = pIn1;
      for (ii = numRowsA; ii > 0; ii -= l) {
        l = __riscv_vsetvl_e16m8(ii);
        pInB = pIn2;
        vres0m8 = __riscv_vfmv_v_f_f16m8(0.0, l);
        for (kk = 0; kk < numColsA; kk++) {
          va0m8 = __riscv_vlse16_v_f16m8(pInA + kk, numColsA * bstride, l);
          vres0m8 = __riscv_vfmacc_vf_f16m8(vres0m8, *(pInB + 0), va0m8, l);
          pInB += numColsB;
        }
        __riscv_vsse16_v_f16m8(px, numColsB * bstride, vres0m8, l);
        px += l * numColsB;
        pInA += l * numColsA;
      }
      pIn2 += 1;
      pOut += 1;
    }

    /* Set status as RISCV_MATH_SUCCESS */
    status = RISCV_MATH_SUCCESS;
#else
    /* The following loop performs the dot-product of each row in pSrcA with each column in pSrcB */
    /* row loop */
    do
    {
      /* Output pointer is set to starting address of row being processed */
      px = pOut + i;

      /* For every row wise process, column loop counter is to be initiated */
      col = numColsB;

      /* For every row wise process, pIn2 pointer is set to starting address of pSrcB data */
      pIn2 = pSrcB->pData;

      /* column loop */
      do
      {
        /* Set the variable sum, that acts as accumulator, to zero */
        sum = 0.0f16;

        /* Initialize pointer pIn1 to point to starting address of column being processed */
        pIn1 = pInA;

#if defined (RISCV_MATH_LOOPUNROLL)

        /* Loop unrolling: Compute 4 MACs at a time. */
        colCnt = numColsA >> 2U;

        /* matrix multiplication */
        while (colCnt > 0U)
        {
          /* c(m,n) = a(1,1) * b(1,1) + a(1,2) * b(2,1) + .... + a(m,p) * b(p,n) */

          /* Perform the multiply-accumulates */
          sum += (_Float16)*pIn1++ * (_Float16)*pIn2;
          pIn2 += numColsB;

          sum += (_Float16)*pIn1++ * (_Float16)*pIn2;
          pIn2 += numColsB;

          sum += (_Float16)*pIn1++ * (_Float16)*pIn2;
          pIn2 += numColsB;

          sum += (_Float16)*pIn1++ * (_Float16)*pIn2;
          pIn2 += numColsB;

          /* Decrement loop counter */
          colCnt--;
        }

        /* Loop unrolling: Compute remaining MACs */
        colCnt = numColsA & 0x3U;

#else

        /* Initialize cntCnt with number of columns */
        colCnt = numColsA;

#endif /* #if defined (RISCV_MATH_LOOPUNROLL) */

        while (colCnt > 0U)
        {
          /* c(m,n) = a(1,1) * b(1,1) + a(1,2) * b(2,1) + .... + a(m,p) * b(p,n) */

          /* Perform the multiply-accumulates */
          sum += (_Float16)*pIn1++ * (_Float16)*pIn2;
          pIn2 += numColsB;

          /* Decrement loop counter */
          colCnt--;
        }

        /* Store result in destination buffer */
        *px++ = sum;

        /* Decrement column loop counter */
        col--;

        /* Update pointer pIn2 to point to starting address of next column */
        pIn2 = pInB + (numColsB - col);

      } while (col > 0U);

      /* Update pointer pInA to point to starting address of next row */
      i = i + numColsB;
      pInA = pInA + numColsA;

      /* Decrement row loop counter */
      row--;

    } while (row > 0U);

    /* Set status as RISCV_MATH_SUCCESS */
    status = RISCV_MATH_SUCCESS;
#endif /* defined(RISCV_MATH_VECTOR) */
  }

  /* Return to application */
  return (status);
}


/**
 * @} end of MatrixMult group
 */

#endif /* #if defined(RISCV_FLOAT16_SUPPORTED) */ 

