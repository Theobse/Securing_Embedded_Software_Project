/* ----------------------------------------------------------------------
 * Project:      NMSIS DSP Library
 * Title:        riscv_mat_vec_mult_q7.c
 * Description:  Q7 matrix and vector multiplication
 *
 * $Date:        23 April 2021
 *
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

#include "dsp/matrix_functions.h"

/**
 * @ingroup groupMatrix
 */



/**
 * @addtogroup MatrixVectMult
 * @{
 */

/**
 * @brief Q7 matrix and vector multiplication.
 * @param[in]       *pSrcMat points to the input matrix structure
 * @param[in]       *pVec points to the input vector
 * @param[out]      *pDst points to the output vector
 */
void riscv_mat_vec_mult_q7(const riscv_matrix_instance_q7 *pSrcMat, const q7_t *pVec, q7_t *pDst)
{
    uint32_t numRows = pSrcMat->numRows;
    uint32_t numCols = pSrcMat->numCols;
    const q7_t *pSrcA = pSrcMat->pData;
    const q7_t *pInA1;       /* input data matrix pointer of Q7 type */
    const q7_t *pInA2;       /* input data matrix pointer of Q7 type */
    const q7_t *pInA3;       /* input data matrix pointer of Q7 type */
    const q7_t *pInA4;       /* input data matrix pointer of Q7 type */
    const q7_t *pInVec;      /* input data vector pointer of Q7 type */
    q7_t *px;                /* output data pointer */
    uint32_t i, row, colCnt; /* loop counters */

    q31_t matData, matData2, vecData, vecData2;

#if defined(RISCV_MATH_VECTOR)
    uint32_t ii, jj;
    size_t l;
    ptrdiff_t bstride = 1;       //  8bit/8bit = 1
    vint32m8_t vres0m8;
    vint8m2x4_t v_tuple;
    vint8m2x2_t v_tuple2;
    vint8m2_t va0m2, va1m2, va2m2, va3m2;
    px = pDst;
    for (jj = numRows; jj > 0; jj -= l) {
      l = __riscv_vsetvl_e32m8(jj);
      vres0m8 = __riscv_vmv_v_x_i32m8(0.0, l);
      pInVec = pVec;
      pInA1 = pSrcA;
      colCnt = numCols;
      for (ii = 0; ii < colCnt / 4; ii++) {
        //vlsseg4e8_v_i8m2 (&va0m2, &va1m2, &va2m2, &va3m2, pInA1, numCols, l);
        v_tuple = __riscv_vlsseg4e8_v_i8m2x4 (pInA1, numCols * bstride, l);
        va0m2 = __riscv_vget_v_i8m2x4_i8m2 (v_tuple, 0);
        va1m2 = __riscv_vget_v_i8m2x4_i8m2 (v_tuple, 1);
        va2m2 = __riscv_vget_v_i8m2x4_i8m2 (v_tuple, 2);
        va3m2 = __riscv_vget_v_i8m2x4_i8m2 (v_tuple, 3);
        vres0m8 = __riscv_vwmacc_vx_i32m8(vres0m8, *(pInVec++), __riscv_vwadd_vx_i16m4(va0m2, 0, l), l);
        vres0m8 = __riscv_vwmacc_vx_i32m8(vres0m8, *(pInVec++), __riscv_vwadd_vx_i16m4(va1m2, 0, l), l);
        vres0m8 = __riscv_vwmacc_vx_i32m8(vres0m8, *(pInVec++), __riscv_vwadd_vx_i16m4(va2m2, 0, l), l);
        vres0m8 = __riscv_vwmacc_vx_i32m8(vres0m8, *(pInVec++), __riscv_vwadd_vx_i16m4(va3m2, 0, l), l);
        pInA1 += 4;
      }
      colCnt = numCols & 0x3;

      for (ii = 0; ii < colCnt / 2; ii++) {
        //vlsseg2e8_v_i8m2(&va0m2, &va1m2, pInA1, numCols, l);
        v_tuple2 = __riscv_vlsseg2e8_v_i8m2x2 (pInA1, numCols * bstride, l);
        va0m2 = __riscv_vget_v_i8m2x2_i8m2 (v_tuple2, 0);
        va1m2 = __riscv_vget_v_i8m2x2_i8m2 (v_tuple2, 1);
        vres0m8 = __riscv_vwmacc_vx_i32m8(vres0m8, *(pInVec++), __riscv_vwadd_vx_i16m4(va0m2, 0, l), l);
        vres0m8 = __riscv_vwmacc_vx_i32m8(vres0m8, *(pInVec++), __riscv_vwadd_vx_i16m4(va1m2, 0, l), l);
        pInA1 += 2;
      }

      if (numCols & 0x1) {
          va0m2 = __riscv_vlse8_v_i8m2(pInA1, numCols, l);
          vres0m8 = __riscv_vwmacc_vx_i32m8(vres0m8, *(pInVec++), __riscv_vwadd_vx_i16m4(va0m2, 0, l), l);
      }
      va0m2 = __riscv_vnclip_wx_i8m2(__riscv_vnsra_wx_i16m4(vres0m8, 7, l), 0, __RISCV_VXRM_RNU, l);
      __riscv_vse8_v_i8m2(px, va0m2, l);
      px += l;
      pSrcA += l * numCols;
    }
#else

    /* Process 4 rows at a time */
    row = numRows >> 2;
    i = 0u;
    px = pDst;



    /* The following loop performs the dot-product of each row in pSrcA with the vector */
    while (row > 0) {
        /* Initialize accumulators */
        q31_t sum1 = 0;
        q31_t sum2 = 0;
        q31_t sum3 = 0;
        q31_t sum4 = 0;

        /* For every row wise process, the pInVec pointer is set
         ** to the starting address of the vector */
        pInVec = pVec;

        /* Loop unrolling: process 4 columns per iteration */
        colCnt = numCols >> 2;

        /* Initialize row pointers so we can track 4 rows at once */
        pInA1 = pSrcA + i;
        pInA2 = pInA1 + numCols;
        pInA3 = pInA2 + numCols;
        pInA4 = pInA3 + numCols;


        // Inner loop: matrix-vector multiplication

        while (colCnt > 0u) {
            // Read 4 values from vector
            vecData = read_q7x4_ia ((q7_t **)&pInVec);
            vecData2 = __SXTB16(__ROR(vecData, 8));
            vecData = __SXTB16(vecData);
            // Read 16 values from the matrix - 4 values from each of 4 rows, and do multiply accumulate
            matData = read_q7x4_ia ((q7_t **)&pInA1);
            matData2 = __SXTB16(__ROR(matData, 8));
            matData = __SXTB16(matData);
            sum1 = __SMLAD(matData, vecData, sum1);
            sum1 = __SMLAD(matData2, vecData2, sum1);
            matData = read_q7x4_ia ((q7_t **)&pInA2);
            matData2 = __SXTB16(__ROR(matData, 8));
            matData = __SXTB16(matData);
            sum2 = __SMLAD(matData, vecData, sum2);
            sum2 = __SMLAD(matData2, vecData2, sum2);
            matData = read_q7x4_ia ((q7_t **)&pInA3);
            matData2 = __SXTB16(__ROR(matData, 8));
            matData = __SXTB16(matData);
            sum3 = __SMLAD(matData, vecData, sum3);
            sum3 = __SMLAD(matData2, vecData2, sum3);
            matData = read_q7x4_ia ((q7_t **)&pInA4);
            matData2 = __SXTB16(__ROR(matData, 8));
            matData = __SXTB16(matData);
            sum4 = __SMLAD(matData, vecData, sum4);
            sum4 = __SMLAD(matData2, vecData2, sum4);

            // Decrement the loop counter
            colCnt--;
        }

        /* process any remaining columns */

        colCnt = numCols & 3u;

        while (colCnt > 0) {
            vecData = *pInVec++;
            sum1 += *pInA1++ * vecData;
            sum2 += *pInA2++ * vecData;
            sum3 += *pInA3++ * vecData;
            sum4 += *pInA4++ * vecData;
            colCnt--;
        }

        /* Saturate and store the result in the destination buffer */
        *px++ = (q7_t)(__SSAT((sum1 >> 7), 8));
        *px++ = (q7_t)(__SSAT((sum2 >> 7), 8));
        *px++ = (q7_t)(__SSAT((sum3 >> 7), 8));
        *px++ = (q7_t)(__SSAT((sum4 >> 7), 8));

        i = i + numCols * 4;

        /* Decrement the row loop counter */
        row--;
    }

    /* process any remaining rows */
    row = numRows & 3u;
    while (row > 0) {

        q31_t sum = 0;
        pInVec = pVec;
        pInA1 = pSrcA + i;

        // loop unrolling - process 4 elements at a time
        colCnt = numCols >> 2;

        while (colCnt > 0) {
            vecData = read_q7x4_ia ((q7_t **)&pInVec);
            vecData2 = __SXTB16(__ROR(vecData, 8));
            vecData = __SXTB16(vecData);
            matData = read_q7x4_ia ((q7_t **)&pInA1);
            matData2 = __SXTB16(__ROR(matData, 8));
            matData = __SXTB16(matData);
            sum = __SMLAD(matData, vecData, sum);
            sum = __SMLAD(matData2, vecData2, sum);
            colCnt--;
        }

        // process remainder of row
        colCnt = numCols & 3u;
        while (colCnt > 0) {
            sum += *pInA1++ * *pInVec++;
            colCnt--;
        }
        *px++ = (q7_t)(__SSAT((sum >> 7), 8));
        i = i + numCols;
        row--;
    }
#endif /*defined(RISCV_MATH_VECTOR)*/
}

/**
 * @} end of MatrixMult group
 */
