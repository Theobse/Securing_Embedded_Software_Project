/* ----------------------------------------------------------------------
 * Project:      NMSIS DSP Library
 * Title:        riscv_cfft_radix4_q31.c
 * Description:  This file has function definition of Radix-4 FFT & IFFT function and
 *               In-place bit reversal using bit reversal table
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

#include "dsp/transform_functions.h"

void riscv_radix4_butterfly_inverse_q31(
        q31_t * pSrc,
        uint32_t fftLen,
  const q31_t * pCoef,
        uint32_t twidCoefModifier);

void riscv_radix4_butterfly_q31(
        q31_t * pSrc,
        uint32_t fftLen,
  const q31_t * pCoef,
        uint32_t twidCoefModifier);

void riscv_bitreversal_q31(
        q31_t * pSrc,
        uint32_t fftLen,
        uint16_t bitRevFactor,
  const uint16_t * pBitRevTab);


/**
  @addtogroup ComplexFFTDeprecated
  @{
 */

/**
  @brief         Processing function for the Q31 CFFT/CIFFT.
  @deprecated    Do not use this function.  It has been superseded by \ref riscv_cfft_q31 and will be removed in the future.
  @param[in]     S    points to an instance of the Q31 CFFT/CIFFT structure
  @param[in,out] pSrc points to the complex data buffer of size <code>2*fftLen</code>. Processing occurs in-place
  @return        none

  @par Input and output formats:
                 Internally input is downscaled by 2 for every stage to avoid saturations inside CFFT/CIFFT process.
                 Hence the output format is different for different FFT sizes.
                 The input and output formats for different FFT sizes and number of bits to upscale are mentioned in the tables below for CFFT and CIFFT:
  @par

| CFFT Size | Input format  | Output format | Number of bits to upscale |
| --------: | ------------: | ------------: | ------------------------: |
| 16        | 1.31          | 5.27          | 4                         |
| 64        | 1.31          | 7.25          | 6                         |
| 256       | 1.31          | 9.23          | 8                         |
| 1024      | 1.31          | 11.21         | 10                        |

| CIFFT Size | Input format  | Output format | Number of bits to upscale |
| ---------: | ------------: | ------------: | ------------------------: |
| 16         | 1.31          | 5.27          | 0                         |
| 64         | 1.31          | 7.25          | 0                         |
| 256        | 1.31          | 9.23          | 0                         |
| 1024       | 1.31          | 11.21         | 0                         |

 */

void riscv_cfft_radix4_q31(
  const riscv_cfft_radix4_instance_q31 * S,
        q31_t * pSrc)
{
  if (S->ifftFlag == 1U)
  {
    /* Complex IFFT radix-4 */
    riscv_radix4_butterfly_inverse_q31(pSrc, S->fftLen, S->pTwiddle, S->twidCoefModifier);
  }
  else
  {
    /* Complex FFT radix-4 */
    riscv_radix4_butterfly_q31(pSrc, S->fftLen, S->pTwiddle, S->twidCoefModifier);
  }

  if (S->bitReverseFlag == 1U)
  {
    /*  Bit Reversal */
    riscv_bitreversal_q31(pSrc, S->fftLen, S->bitRevFactor, S->pBitRevTable);
  }

}

/**
  @} end of ComplexFFTDeprecated group
 */

/*
 * Radix-4 FFT algorithm used is :
 *
 * Input real and imaginary data:
 * x(n) = xa + j * ya
 * x(n+N/4 ) = xb + j * yb
 * x(n+N/2 ) = xc + j * yc
 * x(n+3N 4) = xd + j * yd
 *
 *
 * Output real and imaginary data:
 * x(4r) = xa'+ j * ya'
 * x(4r+1) = xb'+ j * yb'
 * x(4r+2) = xc'+ j * yc'
 * x(4r+3) = xd'+ j * yd'
 *
 *
 * Twiddle factors for radix-4 FFT:
 * Wn = co1 + j * (- si1)
 * W2n = co2 + j * (- si2)
 * W3n = co3 + j * (- si3)
 *
 *  Butterfly implementation:
 * xa' = xa + xb + xc + xd
 * ya' = ya + yb + yc + yd
 * xb' = (xa+yb-xc-yd)* co1 + (ya-xb-yc+xd)* (si1)
 * yb' = (ya-xb-yc+xd)* co1 - (xa+yb-xc-yd)* (si1)
 * xc' = (xa-xb+xc-xd)* co2 + (ya-yb+yc-yd)* (si2)
 * yc' = (ya-yb+yc-yd)* co2 - (xa-xb+xc-xd)* (si2)
 * xd' = (xa-yb-xc+yd)* co3 + (ya+xb-yc-xd)* (si3)
 * yd' = (ya+xb-yc-xd)* co3 - (xa-yb-xc+yd)* (si3)
 *
 */

/**
  @brief         Core function for the Q31 CFFT butterfly process.
  @param[in,out] pSrc             points to the in-place buffer of Q31 data type.
  @param[in]     fftLen           length of the FFT.
  @param[in]     pCoef            points to twiddle coefficient buffer.
  @param[in]     twidCoefModifier twiddle coefficient modifier that supports different size FFTs with the same twiddle factor table.
  @return        none
 */

void riscv_radix4_butterfly_q31(
        q31_t * pSrc,
        uint32_t fftLen,
  const q31_t * pCoef,
        uint32_t twidCoefModifier)
{
        unsigned long n1, n2, ia1, ia2, ia3, i0, i1, i2, i3, j, k;
        q31_t t1, t2, r1, r2, s1, s2, co1, co2, co3, si1, si2, si3;

        q31_t xa, xb, xc, xd;
        q31_t ya, yb, yc, yd;
        q31_t xa_out, xb_out, xc_out, xd_out;
        q31_t ya_out, yb_out, yc_out, yd_out;

        q31_t *ptr1;
#if defined (RISCV_MATH_DSP) && (defined (NUCLEI_DSP_N3) || (__RISCV_XLEN == 64))
        q31_t* pSi0;
        q31_t* pSi1;
        q31_t* pSi2;
        q31_t* pSi3;

        q63_t input0, input1, input2, input3;
        q63_t mid0, mid1, mid2, mid3;
        q63_t temp0, temp1, temp2, temp3;
        q63_t coef1, coef2, coef3;
        q63_t result0, result1, out;
        q63_t xa64, xb64, xc64, xd64;
        q63_t xa_out64;
#endif /* defined (RISCV_MATH_DSP) && (defined (NUCLEI_DSP_N3) || (__RISCV_XLEN == 64)) */

  /* Total process is divided into three stages */

  /* process first stage, middle stages, & last stage */


  /* start of first stage process */

  /*  Initializations for the first stage */
  n2 = fftLen;
  n1 = n2;
  /* n2 = fftLen/4 */
  n2 >>= 2U;
  i0 = 0U;
  ia1 = 0U;

  j = n2;

#if defined (RISCV_MATH_DSP) && (defined (NUCLEI_DSP_N3) || (__RISCV_XLEN == 64))
  pSi0 = pSrc;
  pSi1 = pSi0 + 2 * n2;
  pSi2 = pSi1 + 2 * n2;
  pSi3 = pSi2 + 2 * n2;
  /*  Calculation of first stage */
  do
  {
    /*  Butterfly implementation */
    input0 = read_q31x2(pSi0); /* read ya | xa */
    input1 = read_q31x2(pSi1); /* read yb | xb */
    input2 = read_q31x2(pSi2); /* read yc | xc */
    input3 = read_q31x2(pSi3); /* read yd | xd */

#if __RISCV_XLEN == 64
    input0 = __RV_SRA32(input0, 4);
    input1 = __RV_SRA32(input1, 4);
    input2 = __RV_SRA32(input2, 4);
    input3 = __RV_SRA32(input3, 4);

    /* mid0 = ya + yc | xa + xc */
    mid0 = __RV_ADD32(input0, input2);
    /* mid1 = ya - yc | xa - xc */
    mid1 = __RV_SUB32(input0, input2);
    /* mid2 = yb + yd | xb + xd */
    mid2 = __RV_ADD32(input1, input3);
    /* mid3 = yb - yd | xb - xd */
    mid3 = __RV_SUB32(input1, input3);

    /* temp0 = (ya + yc) + (yb + yd) | (xa + xc) + (xb + xd) = i_mid0 | r_mid0 */
    temp0 = __RV_ADD32(mid0, mid2);
    write_q31x2_ia(&pSi0, temp0);

    /* temp1 = (ya + yc) - (yb + yd) | (xa + xc) - (xb + xd) = i_mid1 | r_mid1 */
    temp1 = __RV_SUB32(mid0, mid2);
    coef2 = read_q31x2((q31_t*)pCoef + (ia1 * 4U));
    result0 = __RV_KMDA32(temp1, coef2);
    result1 = __RV_SMXDS32(temp1, coef2);
    out = __RV_PKTT32(result1, result0);
    out = __RV_KSLRA32(out, 1);
    write_q31x2_ia(&pSi1, out);

    /* temp2 = (ya - yc) - (xb - xd) | (xa - xc) + (yb - yd) = i_mid2 | r_mid2 */
    temp2 = __RV_CRSA32(mid1, mid3);
    coef1 = read_q31x2((q31_t*)pCoef + (ia1 * 2U));
    result0 = __RV_KMDA32(temp2, coef1);
    result1 = __RV_SMXDS32(temp2, coef1);
    out = __RV_PKTT32(result1, result0);
    out = __RV_KSLRA32(out, 1);
    write_q31x2_ia(&pSi2, out);

    /* temp3 = (xb - xd) + (ya - yc) | (xa - xc) - (yb - yd) = i_mid3 | r_mid3 */
    temp3 = __RV_CRAS32(mid1, mid3);
    coef3 = read_q31x2((q31_t*)pCoef + (ia1 * 6U));
    result0 = __RV_KMDA32(temp3, coef3);
    result1 = __RV_SMXDS32(temp3, coef3);
    out = __RV_PKTT32(result1, result0);
    out = __RV_KSLRA32(out, 1);
    write_q31x2_ia(&pSi3, out);
#else
#if defined (NUCLEI_DSP_N3)
    input0 = __RV_DKSLRA32(input0, -4);
    input1 = __RV_DKSLRA32(input1, -4);
    input2 = __RV_DKSLRA32(input2, -4);
    input3 = __RV_DKSLRA32(input3, -4);

    /* mid0 = ya + yc | xa + xc */
    mid0 = __RV_DADD32(input0, input2);
    /* mid1 = ya - yc | xa - xc */
    mid1 = __RV_DSUB32(input0, input2);
    /* mid2 = yb + yd | xb + xd */
    mid2 = __RV_DADD32(input1, input3);
    /* mid3 = yb - yd | xb - xd */
    mid3 = __RV_DSUB32(input1, input3);

    /* temp0 = (ya + yc) + (yb + yd) | (xa + xc) + (xb + xd) = i_mid0 | r_mid0 */
    temp0 = __RV_DADD32(mid0, mid2);
    write_q31x2_ia(&pSi0, temp0);

    /* temp1 = (ya + yc) - (yb + yd) | (xa + xc) - (xb + xd) = i_mid1 | r_mid1 */
    temp1 = __RV_DSUB32(mid0, mid2);
    coef2 = read_q31x2((q31_t*)pCoef + (ia1 * 4U));
    result0 = __RV_DKMDA32(temp1, coef2);
    result1 = __RV_DSMXDS32(temp1, coef2);
    out = __RV_DPKTT32(result1, result0);
    out = __RV_DKSLRA32(out, 1);
    write_q31x2_ia(&pSi1, out);

    /* temp2 = (ya - yc) - (xb - xd) | (xa - xc) + (yb - yd) = i_mid2 | r_mid2 */
    temp2 = __RV_DCRSA32(mid1, mid3);
    coef1 = read_q31x2((q31_t*)pCoef + (ia1 * 2U));
    result0 = __RV_DKMDA32(temp2, coef1);
    result1 = __RV_DSMXDS32(temp2, coef1);
    out = __RV_DPKTT32(result1, result0);
    out = __RV_DKSLRA32(out, 1);
    write_q31x2_ia(&pSi2, out);

    /* temp3 = (xb - xd) + (ya - yc) | (xa - xc) - (yb - yd) = i_mid1 | r_mid1 */
    temp3 = __RV_DCRAS32(mid1, mid3);
    coef3 = read_q31x2((q31_t*)pCoef + (ia1 * 6U));
    result0 = __RV_DKMDA32(temp3, coef3);
    result1 = __RV_DSMXDS32(temp3, coef3);
    out = __RV_DPKTT32(result1, result0);
    out = __RV_DKSLRA32(out, 1);
    write_q31x2_ia(&pSi3, out);
#endif /* defined (NUCLEI_DSP_N3) */
#endif /* __RISCV_XLEN == 64 */

    ia1 = ia1 + twidCoefModifier;

  } while (--j);

#else
  /*  Calculation of first stage */
  do
  {
    /*  index calculation for the input as, */
    /*  pSrc[i0 + 0], pSrc[i0 + fftLen/4], pSrc[i0 + fftLen/2U], pSrc[i0 + 3fftLen/4] */
    i1 = i0 + n2;
    i2 = i1 + n2;
    i3 = i2 + n2;

    /* input is in 1.31(q31) format and provide 4 guard bits for the input */

    /*  Butterfly implementation */
    /* xa + xc */
    r1 = (pSrc[(2U * i0)] >> 4U) + (pSrc[(2U * i2)] >> 4U);
    /* xa - xc */
    r2 = (pSrc[(2U * i0)] >> 4U) - (pSrc[(2U * i2)] >> 4U);

    /* xb + xd */
    t1 = (pSrc[(2U * i1)] >> 4U) + (pSrc[(2U * i3)] >> 4U);

    /* ya + yc */
    s1 = (pSrc[(2U * i0) + 1U] >> 4U) + (pSrc[(2U * i2) + 1U] >> 4U);
    /* ya - yc */
    s2 = (pSrc[(2U * i0) + 1U] >> 4U) - (pSrc[(2U * i2) + 1U] >> 4U);

    /* xa' = xa + xb + xc + xd */
    pSrc[2U * i0] = (r1 + t1);
    /* (xa + xc) - (xb + xd) */
    r1 = r1 - t1;
    /* yb + yd */
    t2 = (pSrc[(2U * i1) + 1U] >> 4U) + (pSrc[(2U * i3) + 1U] >> 4U);

    /* ya' = ya + yb + yc + yd */
    pSrc[(2U * i0) + 1U] = (s1 + t2);

    /* (ya + yc) - (yb + yd) */
    s1 = s1 - t2;

    /* yb - yd */
    t1 = (pSrc[(2U * i1) + 1U] >> 4U) - (pSrc[(2U * i3) + 1U] >> 4U);
    /* xb - xd */
    t2 = (pSrc[(2U * i1)] >> 4U) - (pSrc[(2U * i3)] >> 4U);

    /*  index calculation for the coefficients */
    ia2 = 2U * ia1;
    co2 = pCoef[(ia2 * 2U)];
    si2 = pCoef[(ia2 * 2U) + 1U];

    /* xc' = (xa-xb+xc-xd)co2 + (ya-yb+yc-yd)(si2) */
    pSrc[2U * i1] = (((int32_t) (((q63_t) r1 * co2) >> 32)) +
                     ((int32_t) (((q63_t) s1 * si2) >> 32))) << 1U;

    /* yc' = (ya-yb+yc-yd)co2 - (xa-xb+xc-xd)(si2) */
    pSrc[(2U * i1) + 1U] = (((int32_t) (((q63_t) s1 * co2) >> 32)) -
                            ((int32_t) (((q63_t) r1 * si2) >> 32))) << 1U;

    /* (xa - xc) + (yb - yd) */
    r1 = r2 + t1;
    /* (xa - xc) - (yb - yd) */
    r2 = r2 - t1;

    /* (ya - yc) - (xb - xd) */
    s1 = s2 - t2;
    /* (ya - yc) + (xb - xd) */
    s2 = s2 + t2;

    co1 = pCoef[(ia1 * 2U)];
    si1 = pCoef[(ia1 * 2U) + 1U];

    /* xb' = (xa+yb-xc-yd)co1 + (ya-xb-yc+xd)(si1) */
    pSrc[2U * i2] = (((int32_t) (((q63_t) r1 * co1) >> 32)) +
                     ((int32_t) (((q63_t) s1 * si1) >> 32))) << 1U;

    /* yb' = (ya-xb-yc+xd)co1 - (xa+yb-xc-yd)(si1) */
    pSrc[(2U * i2) + 1U] = (((int32_t) (((q63_t) s1 * co1) >> 32)) -
                            ((int32_t) (((q63_t) r1 * si1) >> 32))) << 1U;

    /*  index calculation for the coefficients */
    ia3 = 3U * ia1;
    co3 = pCoef[(ia3 * 2U)];
    si3 = pCoef[(ia3 * 2U) + 1U];

    /* xd' = (xa-yb-xc+yd)co3 + (ya+xb-yc-xd)(si3) */
    pSrc[2U * i3] = (((int32_t) (((q63_t) r2 * co3) >> 32)) +
                     ((int32_t) (((q63_t) s2 * si3) >> 32))) << 1U;

    /* yd' = (ya+xb-yc-xd)co3 - (xa-yb-xc+yd)(si3) */
    pSrc[(2U * i3) + 1U] = (((int32_t) (((q63_t) s2 * co3) >> 32)) -
                            ((int32_t) (((q63_t) r2 * si3) >> 32))) << 1U;

    /*  Twiddle coefficients index modifier */
    ia1 = ia1 + twidCoefModifier;

    /*  Updating input index */
    i0 = i0 + 1U;

  } while (--j);
#endif /* defined (RISCV_MATH_DSP) && (defined (NUCLEI_DSP_N3) || (__RISCV_XLEN == 64)) */


  /* end of first stage process */

  /* data is in 5.27(q27) format */


  /* start of Middle stages process */


  /* each stage in middle stages provides two down scaling of the input */

  twidCoefModifier <<= 2U;


  for (k = fftLen / 4U; k > 4U; k >>= 2U)
  {
    /*  Initializations for the first stage */
    n1 = n2;
    n2 >>= 2U;
    ia1 = 0U;

    /*  Calculation of first stage */
    for (j = 0U; j <= (n2 - 1U); j++)
    {
      /*  index calculation for the coefficients */
#if defined (RISCV_MATH_DSP) && (defined (NUCLEI_DSP_N3) || (__RISCV_XLEN == 64))
      coef1 = read_q31x2((q31_t*)pCoef + (ia1 * 2U));
      coef2 = read_q31x2((q31_t*)pCoef + (ia1 * 4U));
      coef3 = read_q31x2((q31_t*)pCoef + (ia1 * 6U));
#else
      ia2 = ia1 + ia1;
      ia3 = ia2 + ia1;
      co1 = pCoef[(ia1 * 2U)];
      si1 = pCoef[(ia1 * 2U) + 1U];
      co2 = pCoef[(ia2 * 2U)];
      si2 = pCoef[(ia2 * 2U) + 1U];
      co3 = pCoef[(ia3 * 2U)];
      si3 = pCoef[(ia3 * 2U) + 1U];
#endif /* defined (RISCV_MATH_DSP) && (defined (NUCLEI_DSP_N3) || (__RISCV_XLEN == 64)) */
      /*  Twiddle coefficients index modifier */
      ia1 = ia1 + twidCoefModifier;

      for (i0 = j; i0 < fftLen; i0 += n1)
      {
        /*  index calculation for the input as, */
        /*  pSrc[i0 + 0], pSrc[i0 + fftLen/4], pSrc[i0 + fftLen/2U], pSrc[i0 + 3fftLen/4] */
#if defined (RISCV_MATH_DSP) && (defined (NUCLEI_DSP_N3) || (__RISCV_XLEN == 64))
        pSi0 = pSrc + 2 * i0;
        pSi1 = pSi0 + 2 * n2;
        pSi2 = pSi0 + 4 * n2;
        pSi3 = pSi0 + 6 * n2;

        /*  Butterfly implementation */
        input0 = read_q31x2(pSi0); /* read ya | xa */
        input1 = read_q31x2(pSi1); /* read yb | xb */
        input2 = read_q31x2(pSi2); /* read yc | xc */
        input3 = read_q31x2(pSi3); /* read yd | xd */
#if __RISCV_XLEN == 64
        /* mid0 = ya + yc | xa + xc */
        mid0 = __RV_ADD32(input0, input2);
        /* mid1 = ya - yc | xa - xc */
        mid1 = __RV_SUB32(input0, input2);
        /* mid2 = yb + yd | xb + xd */
        mid2 = __RV_ADD32(input1, input3);
        /* mid3 = yb - yd | xb - xd */
        mid3 = __RV_SUB32(input1, input3);

        /* temp0 = (ya + yc) + (yb + yd) | (xa + xc) + (xb + xd) = i_mid0 | r_mid0 */
        temp0 = __RV_ADD32(mid0, mid2);
        write_q31x2_ia(&pSi0, __RV_SRA32(temp0, 2));
        /* temp1 = (ya + yc) - (yb + yd) | (xa + xc) - (xb + xd) = i_mid1 | r_mid1 */
        temp1 = __RV_SUB32(mid0, mid2);
        result0 = __RV_KMDA32(temp1, coef2);
        result1 = __RV_SMXDS32(temp1, coef2);
        out = __RV_PKTT32(result1, result0);
        out = __RV_SRA32(out, 1);
        write_q31x2_ia(&pSi1, out);
        /* temp2 = (ya - yc) - (xb - xd) | (xa - xc) + (yb - yd) = i_mid2 | r_mid2 */
        temp2 = __RV_CRSA32(mid1, mid3);
        result0 = __RV_KMDA32(temp2, coef1);
        result1 = __RV_SMXDS32(temp2, coef1);
        out = __RV_PKTT32(result1, result0);
        out = __RV_SRA32(out, 1);
        write_q31x2_ia(&pSi2, out);
        /* temp3 = (xb - xd) + (ya - yc) | (xa - xc) - (yb - yd) = i_mid3 | r_mid3 */
        temp3 = __RV_CRAS32(mid1, mid3);
        result0 = __RV_KMDA32(temp3, coef3);
        result1 = __RV_SMXDS32(temp3, coef3);
        out = __RV_PKTT32(result1, result0);
        out = __RV_SRA32(out, 1);
        write_q31x2_ia(&pSi3, out);
#else
#if defined (NUCLEI_DSP_N3)
        /* mid0 = ya + yc | xa + xc */
        mid0 = __RV_DADD32(input0, input2);
        /* mid1 = ya - yc | xa - xc */
        mid1 = __RV_DSUB32(input0, input2);
        /* mid2 = yb + yd | xb + xd */
        mid2 = __RV_DADD32(input1, input3);
        /* mid3 = yb - yd | xb - xd */
        mid3 = __RV_DSUB32(input1, input3);

        /* temp0 = (ya + yc) + (yb + yd) | (xa + xc) + (xb + xd) = i_mid0 | r_mid0 */
        temp0 = __RV_DADD32(mid0, mid2);
        write_q31x2_ia(&pSi0, __RV_DKSLRA32(temp0, -2));
        /* temp1 = (ya + yc) - (yb + yd) | (xa + xc) - (xb + xd) = i_mid1 | r_mid1 */
        temp1 = __RV_DSUB32(mid0, mid2);
        result0 = __RV_DKMDA32(temp1, coef2);
        result1 = __RV_DSMXDS32(temp1, coef2);
        out = __RV_DPKTT32(result1, result0);
        out = __RV_DKSLRA32(out, -1);
        write_q31x2_ia(&pSi1, out);
        /* temp2 = (ya - yc) - (xb - xd) | (xa - xc) + (yb - yd) = i_mid2 | r_mid2 */
        temp2 = __RV_DCRSA32(mid1, mid3);
        result0 = __RV_DKMDA32(temp2, coef1);
        result1 = __RV_DSMXDS32(temp2, coef1);
        out = __RV_DPKTT32(result1, result0);
        out = __RV_DKSLRA32(out, -1);
        write_q31x2_ia(&pSi2, out);
        /* temp3 = (xb - xd) + (ya - yc) | (xa - xc) - (yb - yd) = i_mid3 | r_mid3 */
        temp3 = __RV_DCRAS32(mid1, mid3);
        result0 = __RV_DKMDA32(temp3, coef3);
        result1 = __RV_DSMXDS32(temp3, coef3);
        out = __RV_DPKTT32(result1, result0);
        out = __RV_DKSLRA32(out, -1);
        write_q31x2_ia(&pSi3, out);
#endif /* defined (NUCLEI_DSP_N3) */
#endif /* __RISCV_XLEN == 64 */

#else
        i1 = i0 + n2;
        i2 = i1 + n2;
        i3 = i2 + n2;

        /*  Butterfly implementation */
        /* xa + xc */
        r1 = pSrc[2U * i0] + pSrc[2U * i2];
        /* xa - xc */
        r2 = pSrc[2U * i0] - pSrc[2U * i2];

        /* ya + yc */
        s1 = pSrc[(2U * i0) + 1U] + pSrc[(2U * i2) + 1U];
        /* ya - yc */
        s2 = pSrc[(2U * i0) + 1U] - pSrc[(2U * i2) + 1U];

        /* xb + xd */
        t1 = pSrc[2U * i1] + pSrc[2U * i3];

        /* xa' = xa + xb + xc + xd */
        pSrc[2U * i0] = (r1 + t1) >> 2U;
        /* xa + xc -(xb + xd) */
        r1 = r1 - t1;

        /* yb + yd */
        t2 = pSrc[(2U * i1) + 1U] + pSrc[(2U * i3) + 1U];
        /* ya' = ya + yb + yc + yd */
        pSrc[(2U * i0) + 1U] = (s1 + t2) >> 2U;

        /* (ya + yc) - (yb + yd) */
        s1 = s1 - t2;

        /* (yb - yd) */
        t1 = pSrc[(2U * i1) + 1U] - pSrc[(2U * i3) + 1U];
        /* (xb - xd) */
        t2 = pSrc[2U * i1] - pSrc[2U * i3];

        /* xc' = (xa-xb+xc-xd)co2 + (ya-yb+yc-yd)(si2) */
        pSrc[2U * i1] = (((int32_t) (((q63_t) r1 * co2) >> 32)) +
                         ((int32_t) (((q63_t) s1 * si2) >> 32))) >> 1U;

        /* yc' = (ya-yb+yc-yd)co2 - (xa-xb+xc-xd)(si2) */
        pSrc[(2U * i1) + 1U] = (((int32_t) (((q63_t) s1 * co2) >> 32)) -
                                ((int32_t) (((q63_t) r1 * si2) >> 32))) >> 1U;

        /* (xa - xc) + (yb - yd) */
        r1 = r2 + t1;
        /* (xa - xc) - (yb - yd) */
        r2 = r2 - t1;

        /* (ya - yc) -  (xb - xd) */
        s1 = s2 - t2;
        /* (ya - yc) +  (xb - xd) */
        s2 = s2 + t2;

        /* xb' = (xa+yb-xc-yd)co1 + (ya-xb-yc+xd)(si1) */
        pSrc[2U * i2] = (((int32_t) (((q63_t) r1 * co1) >> 32)) +
                         ((int32_t) (((q63_t) s1 * si1) >> 32))) >> 1U;

        /* yb' = (ya-xb-yc+xd)co1 - (xa+yb-xc-yd)(si1) */
        pSrc[(2U * i2) + 1U] = (((int32_t) (((q63_t) s1 * co1) >> 32)) -
                                ((int32_t) (((q63_t) r1 * si1) >> 32))) >> 1U;

        /* xd' = (xa-yb-xc+yd)co3 + (ya+xb-yc-xd)(si3) */
        pSrc[2U * i3] = (((int32_t) (((q63_t) r2 * co3) >> 32)) +
                         ((int32_t) (((q63_t) s2 * si3) >> 32))) >> 1U;

        /* yd' = (ya+xb-yc-xd)co3 - (xa-yb-xc+yd)(si3) */
        pSrc[(2U * i3) + 1U] = (((int32_t) (((q63_t) s2 * co3) >> 32)) -
                                ((int32_t) (((q63_t) r2 * si3) >> 32))) >> 1U;
#endif /* defined (RISCV_MATH_DSP) && (defined (NUCLEI_DSP_N3) || (__RISCV_XLEN == 64)) */
      }
    }
    twidCoefModifier <<= 2U;
  }

  /* End of Middle stages process */

  /* data is in 11.21(q21) format for the 1024 point as there are 3 middle stages */
  /* data is in 9.23(q23) format for the 256 point as there are 2 middle stages */
  /* data is in 7.25(q25) format for the 64 point as there are 1 middle stage */
  /* data is in 5.27(q27) format for the 16 point as there are no middle stages */


  /* start of Last stage process */
  /*  Initializations for the last stage */
  j = fftLen >> 2;
  ptr1 = &pSrc[0];

  /*  Calculations of last stage */
  do
  {
#if defined (RISCV_MATH_DSP) && (defined (NUCLEI_DSP_N3) || (__RISCV_XLEN == 64))
    xa64 = read_q31x2_ia((q31_t **)&ptr1);
    xb64 = read_q31x2_ia((q31_t **)&ptr1);
    xc64 = read_q31x2_ia((q31_t **)&ptr1);
    xd64 = read_q31x2_ia((q31_t **)&ptr1);
#if __RISCV_XLEN == 64
    /* xa' = xa + xb + xc + xd */ /* ya' = ya + yb + yc + yd */
    xa_out64 = __RV_ADD32( __RV_ADD32( __RV_ADD32(xa64, xb64), xc64), xd64);
    /* pointer updation for writing */
    ptr1 = ptr1 - 8U;
    write_q31x2_ia((q31_t **)&ptr1, xa_out64);
    /*   xc_out = (xa - xb + xc - xd);yc_out = (ya - yb + yc - yd);*/
    xa_out64 = __RV_SUB32( __RV_ADD32( __RV_SUB32(xa64, xb64), xc64), xd64);
    write_q31x2_ia((q31_t **)&ptr1, xa_out64);
    /*    xb_out = (xa + yb - xc - yd);yb_out = (ya - xb - yc + xd);*/
    xa_out64 = __RV_CRAS32( __RV_SUB32( __RV_CRSA32(xa64, xb64), xc64), xd64);
    write_q31x2_ia((q31_t **)&ptr1, xa_out64);
    /*        xd_out = (xa - yb - xc + yd); yd_out = (ya + xb - yc - xd);*/
    xa_out64 = __RV_CRSA32(__RV_SUB32(__RV_CRAS32(xa64, xb64), xc64), xd64);
    write_q31x2_ia((q31_t **)&ptr1, xa_out64);

#else
#if defined (NUCLEI_DSP_N3)
    /* xa' = xa + xb + xc + xd */ /* ya' = ya + yb + yc + yd */
    xa_out64 = __RV_DADD32( __RV_DADD32( __RV_DADD32(xa64, xb64), xc64), xd64);
    /* pointer updation for writing */
    ptr1 = ptr1 - 8U;
    write_q31x2_ia((q31_t **)&ptr1, xa_out64);
    /*   xc_out = (xa - xb + xc - xd);yc_out = (ya - yb + yc - yd);*/
    xa_out64 = __RV_DSUB32( __RV_DADD32( __RV_DSUB32(xa64, xb64), xc64), xd64);
    write_q31x2_ia((q31_t **)&ptr1, xa_out64);
    /*    xb_out = (xa + yb - xc - yd);yb_out = (ya - xb - yc + xd);*/
    xa_out64 = __RV_DCRAS32( __RV_DSUB32( __RV_DCRSA32(xa64, xb64), xc64), xd64);
    write_q31x2_ia((q31_t **)&ptr1, xa_out64);
    /*        xd_out = (xa - yb - xc + yd); yd_out = (ya + xb - yc - xd);*/
    xa_out64 = __RV_DCRSA32( __RV_DSUB32( __RV_DCRAS32(xa64, xb64), xc64), xd64);
    write_q31x2_ia((q31_t **)&ptr1, xa_out64);
#endif /* defined (NUCLEI_DSP_N3) */
#endif /* __RISCV_XLEN == 64 */
#else
    /* Read xa (real), ya(imag) input */
    xa = *ptr1++;
    ya = *ptr1++;

    /* Read xb (real), yb(imag) input */
    xb = *ptr1++;
    yb = *ptr1++;

    /* Read xc (real), yc(imag) input */
    xc = *ptr1++;
    yc = *ptr1++;

    /* Read xd (real), yd(imag) input */
    xd = *ptr1++;
    yd = *ptr1++;

    /* xa' = xa + xb + xc + xd */
    xa_out = xa + xb + xc + xd;

    /* ya' = ya + yb + yc + yd */
    ya_out = ya + yb + yc + yd;

    /* pointer updation for writing */
    ptr1 = ptr1 - 8U;

    /* writing xa' and ya' */
    *ptr1++ = xa_out;
    *ptr1++ = ya_out;

    xc_out = (xa - xb + xc - xd);
    yc_out = (ya - yb + yc - yd);

    /* writing xc' and yc' */
    *ptr1++ = xc_out;
    *ptr1++ = yc_out;

    xb_out = (xa + yb - xc - yd);
    yb_out = (ya - xb - yc + xd);

    /* writing xb' and yb' */
    *ptr1++ = xb_out;
    *ptr1++ = yb_out;

    xd_out = (xa - yb - xc + yd);
    yd_out = (ya + xb - yc - xd);

    /* writing xd' and yd' */
    *ptr1++ = xd_out;
    *ptr1++ = yd_out;

#endif /* defined (RISCV_MATH_DSP) && (defined (NUCLEI_DSP_N3) || (__RISCV_XLEN == 64)) */
  } while (--j);

  /* output is in 11.21(q21) format for the 1024 point */
  /* output is in 9.23(q23) format for the 256 point */
  /* output is in 7.25(q25) format for the 64 point */
  /* output is in 5.27(q27) format for the 16 point */

  /* End of last stage process */

}


/**
  @brief         Core function for the Q31 CIFFT butterfly process.
  @param[in,out] pSrc             points to the in-place buffer of Q31 data type.
  @param[in]     fftLen           length of the FFT.
  @param[in]     pCoef            points to twiddle coefficient buffer.
  @param[in]     twidCoefModifier twiddle coefficient modifier that supports different size FFTs with the same twiddle factor table.
  @return        none
 */

/*
 * Radix-4 IFFT algorithm used is :
 *
 * CIFFT uses same twiddle coefficients as CFFT Function
 *  x[k] = x[n] + (j)k * x[n + fftLen/4] + (-1)k * x[n+fftLen/2] + (-j)k * x[n+3*fftLen/4]
 *
 *
 * IFFT is implemented with following changes in equations from FFT
 *
 * Input real and imaginary data:
 * x(n) = xa + j * ya
 * x(n+N/4 ) = xb + j * yb
 * x(n+N/2 ) = xc + j * yc
 * x(n+3N 4) = xd + j * yd
 *
 *
 * Output real and imaginary data:
 * x(4r) = xa'+ j * ya'
 * x(4r+1) = xb'+ j * yb'
 * x(4r+2) = xc'+ j * yc'
 * x(4r+3) = xd'+ j * yd'
 *
 *
 * Twiddle factors for radix-4 IFFT:
 * Wn = co1 + j * (si1)
 * W2n = co2 + j * (si2)
 * W3n = co3 + j * (si3)

 * The real and imaginary output values for the radix-4 butterfly are
 * xa' = xa + xb + xc + xd
 * ya' = ya + yb + yc + yd
 * xb' = (xa-yb-xc+yd)* co1 - (ya+xb-yc-xd)* (si1)
 * yb' = (ya+xb-yc-xd)* co1 + (xa-yb-xc+yd)* (si1)
 * xc' = (xa-xb+xc-xd)* co2 - (ya-yb+yc-yd)* (si2)
 * yc' = (ya-yb+yc-yd)* co2 + (xa-xb+xc-xd)* (si2)
 * xd' = (xa+yb-xc-yd)* co3 - (ya-xb-yc+xd)* (si3)
 * yd' = (ya-xb-yc+xd)* co3 + (xa+yb-xc-yd)* (si3)
 *
 */

void riscv_radix4_butterfly_inverse_q31(
        q31_t * pSrc,
        uint32_t fftLen,
  const q31_t * pCoef,
        uint32_t twidCoefModifier)
{
        uint32_t n1, n2, ia1, ia2, ia3, i0, i1, i2, i3, j, k;
        q31_t t1, t2, r1, r2, s1, s2, co1, co2, co3, si1, si2, si3;
        q31_t xa, xb, xc, xd;
        q31_t ya, yb, yc, yd;
        q31_t xa_out, xb_out, xc_out, xd_out;
        q31_t ya_out, yb_out, yc_out, yd_out;

        q31_t *ptr1;
#if __RISCV_XLEN == 64
        q63_t xa64, xb64, xc64, xd64;
        q63_t xa_out64;
#endif /* __RISCV_XLEN == 64 */
  /* input is be 1.31(q31) format for all FFT sizes */
  /* Total process is divided into three stages */
  /* process first stage, middle stages, & last stage */

  /* Start of first stage process */

  /* Initializations for the first stage */
  n2 = fftLen;
  n1 = n2;
  /* n2 = fftLen/4 */
  n2 >>= 2U;
  i0 = 0U;
  ia1 = 0U;

  j = n2;

  do
  {
    /* input is in 1.31(q31) format and provide 4 guard bits for the input */

    /*  index calculation for the input as, */
    /*  pSrc[i0 + 0], pSrc[i0 + fftLen/4], pSrc[i0 + fftLen/2U], pSrc[i0 + 3fftLen/4] */
    i1 = i0 + n2;
    i2 = i1 + n2;
    i3 = i2 + n2;

    /*  Butterfly implementation */
    /* xa + xc */
    r1 = (pSrc[2U * i0] >> 4U) + (pSrc[2U * i2] >> 4U);
    /* xa - xc */
    r2 = (pSrc[2U * i0] >> 4U) - (pSrc[2U * i2] >> 4U);

    /* xb + xd */
    t1 = (pSrc[2U * i1] >> 4U) + (pSrc[2U * i3] >> 4U);

    /* ya + yc */
    s1 = (pSrc[(2U * i0) + 1U] >> 4U) + (pSrc[(2U * i2) + 1U] >> 4U);
    /* ya - yc */
    s2 = (pSrc[(2U * i0) + 1U] >> 4U) - (pSrc[(2U * i2) + 1U] >> 4U);

    /* xa' = xa + xb + xc + xd */
    pSrc[2U * i0] = (r1 + t1);
    /* (xa + xc) - (xb + xd) */
    r1 = r1 - t1;
    /* yb + yd */
    t2 = (pSrc[(2U * i1) + 1U] >> 4U) + (pSrc[(2U * i3) + 1U] >> 4U);
    /* ya' = ya + yb + yc + yd */
    pSrc[(2U * i0) + 1U] = (s1 + t2);

    /* (ya + yc) - (yb + yd) */
    s1 = s1 - t2;

    /* yb - yd */
    t1 = (pSrc[(2U * i1) + 1U] >> 4U) - (pSrc[(2U * i3) + 1U] >> 4U);
    /* xb - xd */
    t2 = (pSrc[2U * i1] >> 4U) - (pSrc[2U * i3] >> 4U);

    /*  index calculation for the coefficients */
    ia2 = 2U * ia1;
    co2 = pCoef[ia2 * 2U];
    si2 = pCoef[(ia2 * 2U) + 1U];

    /* xc' = (xa-xb+xc-xd)co2 - (ya-yb+yc-yd)(si2) */
    pSrc[2U * i1] = (((int32_t) (((q63_t) r1 * co2) >> 32)) -
                     ((int32_t) (((q63_t) s1 * si2) >> 32))) << 1U;

    /* yc' = (ya-yb+yc-yd)co2 + (xa-xb+xc-xd)(si2) */
    pSrc[2U * i1 + 1U] = (((int32_t) (((q63_t) s1 * co2) >> 32)) +
                          ((int32_t) (((q63_t) r1 * si2) >> 32))) << 1U;

    /* (xa - xc) - (yb - yd) */
    r1 = r2 - t1;
    /* (xa - xc) + (yb - yd) */
    r2 = r2 + t1;

    /* (ya - yc) + (xb - xd) */
    s1 = s2 + t2;
    /* (ya - yc) - (xb - xd) */
    s2 = s2 - t2;

    co1 = pCoef[ia1 * 2U];
    si1 = pCoef[(ia1 * 2U) + 1U];

    /* xb' = (xa+yb-xc-yd)co1 - (ya-xb-yc+xd)(si1) */
    pSrc[2U * i2] = (((int32_t) (((q63_t) r1 * co1) >> 32)) -
                     ((int32_t) (((q63_t) s1 * si1) >> 32))) << 1U;

    /* yb' = (ya-xb-yc+xd)co1 + (xa+yb-xc-yd)(si1) */
    pSrc[(2U * i2) + 1U] = (((int32_t) (((q63_t) s1 * co1) >> 32)) +
                            ((int32_t) (((q63_t) r1 * si1) >> 32))) << 1U;

    /*  index calculation for the coefficients */
    ia3 = 3U * ia1;
    co3 = pCoef[ia3 * 2U];
    si3 = pCoef[(ia3 * 2U) + 1U];

    /* xd' = (xa-yb-xc+yd)co3 - (ya+xb-yc-xd)(si3) */
    pSrc[2U * i3] = (((int32_t) (((q63_t) r2 * co3) >> 32)) -
                     ((int32_t) (((q63_t) s2 * si3) >> 32))) << 1U;

    /* yd' = (ya+xb-yc-xd)co3 + (xa-yb-xc+yd)(si3) */
    pSrc[(2U * i3) + 1U] = (((int32_t) (((q63_t) s2 * co3) >> 32)) +
                            ((int32_t) (((q63_t) r2 * si3) >> 32))) << 1U;

    /*  Twiddle coefficients index modifier */
    ia1 = ia1 + twidCoefModifier;

    /*  Updating input index */
    i0 = i0 + 1U;

  } while (--j);

  /* data is in 5.27(q27) format */
  /* each stage provides two down scaling of the input */


  /* Start of Middle stages process */

  twidCoefModifier <<= 2U;

  /*  Calculation of second stage to excluding last stage */
  for (k = fftLen / 4U; k > 4U; k >>= 2U)
  {
    /*  Initializations for the first stage */
    n1 = n2;
    n2 >>= 2U;
    ia1 = 0U;

    for (j = 0; j <= (n2 - 1U); j++)
    {
      /*  index calculation for the coefficients */
      ia2 = ia1 + ia1;
      ia3 = ia2 + ia1;
      co1 = pCoef[(ia1 * 2U)];
      si1 = pCoef[(ia1 * 2U) + 1U];
      co2 = pCoef[(ia2 * 2U)];
      si2 = pCoef[(ia2 * 2U) + 1U];
      co3 = pCoef[(ia3 * 2U)];
      si3 = pCoef[(ia3 * 2U) + 1U];
      /*  Twiddle coefficients index modifier */
      ia1 = ia1 + twidCoefModifier;

      for (i0 = j; i0 < fftLen; i0 += n1)
      {
        /*  index calculation for the input as, */
        /*  pSrc[i0 + 0], pSrc[i0 + fftLen/4], pSrc[i0 + fftLen/2U], pSrc[i0 + 3fftLen/4] */
        i1 = i0 + n2;
        i2 = i1 + n2;
        i3 = i2 + n2;

        /*  Butterfly implementation */
        /* xa + xc */
        r1 = pSrc[2U * i0] + pSrc[2U * i2];
        /* xa - xc */
        r2 = pSrc[2U * i0] - pSrc[2U * i2];

        /* ya + yc */
        s1 = pSrc[(2U * i0) + 1U] + pSrc[(2U * i2) + 1U];
        /* ya - yc */
        s2 = pSrc[(2U * i0) + 1U] - pSrc[(2U * i2) + 1U];

        /* xb + xd */
        t1 = pSrc[2U * i1] + pSrc[2U * i3];

        /* xa' = xa + xb + xc + xd */
        pSrc[2U * i0] = (r1 + t1) >> 2U;
        /* xa + xc -(xb + xd) */
        r1 = r1 - t1;
        /* yb + yd */
        t2 = pSrc[(2U * i1) + 1U] + pSrc[(2U * i3) + 1U];
        /* ya' = ya + yb + yc + yd */
        pSrc[(2U * i0) + 1U] = (s1 + t2) >> 2U;

        /* (ya + yc) - (yb + yd) */
        s1 = s1 - t2;

        /* (yb - yd) */
        t1 = pSrc[(2U * i1) + 1U] - pSrc[(2U * i3) + 1U];
        /* (xb - xd) */
        t2 = pSrc[2U * i1] - pSrc[2U * i3];

        /* xc' = (xa-xb+xc-xd)co2 - (ya-yb+yc-yd)(si2) */
        pSrc[2U * i1] = (((int32_t) (((q63_t) r1 * co2) >> 32U)) -
                         ((int32_t) (((q63_t) s1 * si2) >> 32U))) >> 1U;

        /* yc' = (ya-yb+yc-yd)co2 + (xa-xb+xc-xd)(si2) */
        pSrc[(2U * i1) + 1U] = (((int32_t) (((q63_t) s1 * co2) >> 32U)) +
                                ((int32_t) (((q63_t) r1 * si2) >> 32U))) >> 1U;

        /* (xa - xc) - (yb - yd) */
        r1 = r2 - t1;
        /* (xa - xc) + (yb - yd) */
        r2 = r2 + t1;

        /* (ya - yc) +  (xb - xd) */
        s1 = s2 + t2;
        /* (ya - yc) -  (xb - xd) */
        s2 = s2 - t2;

        /* xb' = (xa+yb-xc-yd)co1 - (ya-xb-yc+xd)(si1) */
        pSrc[2U * i2] = (((int32_t) (((q63_t) r1 * co1) >> 32)) -
                         ((int32_t) (((q63_t) s1 * si1) >> 32))) >> 1U;

        /* yb' = (ya-xb-yc+xd)co1 + (xa+yb-xc-yd)(si1) */
        pSrc[(2U * i2) + 1U] = (((int32_t) (((q63_t) s1 * co1) >> 32)) +
                                ((int32_t) (((q63_t) r1 * si1) >> 32))) >> 1U;

        /* xd' = (xa-yb-xc+yd)co3 - (ya+xb-yc-xd)(si3) */
        pSrc[(2U * i3)] = (((int32_t) (((q63_t) r2 * co3) >> 32)) -
                           ((int32_t) (((q63_t) s2 * si3) >> 32))) >> 1U;

        /* yd' = (ya+xb-yc-xd)co3 + (xa-yb-xc+yd)(si3) */
        pSrc[(2U * i3) + 1U] = (((int32_t) (((q63_t) s2 * co3) >> 32)) +
                                ((int32_t) (((q63_t) r2 * si3) >> 32))) >> 1U;
      }
    }
    twidCoefModifier <<= 2U;
  }

  /* End of Middle stages process */

  /* data is in 11.21(q21) format for the 1024 point as there are 3 middle stages */
  /* data is in 9.23(q23) format for the 256 point as there are 2 middle stages */
  /* data is in 7.25(q25) format for the 64 point as there are 1 middle stage */
  /* data is in 5.27(q27) format for the 16 point as there are no middle stages */


  /* Start of last stage process */


  /*  Initializations for the last stage */
  j = fftLen >> 2;
  ptr1 = &pSrc[0];

  /*  Calculations of last stage */
  do
  {
#if defined RISCV_MATH_DSP && (__RISCV_XLEN == 64)
    xa64 = read_q31x2_ia((q31_t **)&ptr1);
    xb64 = read_q31x2_ia((q31_t **)&ptr1);
    xc64 = read_q31x2_ia((q31_t **)&ptr1);
    xd64 = read_q31x2_ia((q31_t **)&ptr1);
    /* xa' = xa + xb + xc + xd */ /* ya' = ya + yb + yc + yd */
    xa_out64 = __RV_KADD32( __RV_KADD32( __RV_KADD32(xa64, xb64), xc64), xd64);
    /* pointer updation for writing */
    ptr1 = ptr1 - 8U;
    write_q31x2_ia((q31_t **)&ptr1, xa_out64);
    /* xc_out = (xa - xb + xc - xd);yc_out = (ya - yb + yc - yd);*/
    xa_out64 = __RV_KSUB32( __RV_KADD32( __RV_KSUB32(xa64, xb64), xc64), xd64);
    write_q31x2_ia((q31_t **)&ptr1, xa_out64);
    /* xb_out = (xa + yb - xc - yd);yb_out = (ya - xb - yc + xd);*/
    xa_out64 = __RV_KCRAS32( __RV_KSUB32( __RV_KCRSA32(xa64, xb64), xc64), xd64);
    write_q31x2_ia((q31_t **)&ptr1, xa_out64);
    /* xd_out = (xa - yb - xc + yd); yd_out = (ya + xb - yc - xd);*/
    xa_out64 = __RV_KCRSA32( __RV_KSUB32( __RV_KCRAS32(xa64, xb64), xc64), xd64);
    write_q31x2_ia((q31_t **)&ptr1, xa_out64);

#else
    /* Read xa (real), ya(imag) input */
    xa = *ptr1++;
    ya = *ptr1++;

    /* Read xb (real), yb(imag) input */
    xb = *ptr1++;
    yb = *ptr1++;

    /* Read xc (real), yc(imag) input */
    xc = *ptr1++;
    yc = *ptr1++;

    /* Read xc (real), yc(imag) input */
    xd = *ptr1++;
    yd = *ptr1++;

    /* xa' = xa + xb + xc + xd */
    xa_out = xa + xb + xc + xd;

    /* ya' = ya + yb + yc + yd */
    ya_out = ya + yb + yc + yd;

    /* pointer updation for writing */
    ptr1 = ptr1 - 8U;

    /* writing xa' and ya' */
    *ptr1++ = xa_out;
    *ptr1++ = ya_out;

    xc_out = (xa - xb + xc - xd);
    yc_out = (ya - yb + yc - yd);

    /* writing xc' and yc' */
    *ptr1++ = xc_out;
    *ptr1++ = yc_out;

    xb_out = (xa - yb - xc + yd);
    yb_out = (ya + xb - yc - xd);

    /* writing xb' and yb' */
    *ptr1++ = xb_out;
    *ptr1++ = yb_out;

    xd_out = (xa + yb - xc - yd);
    yd_out = (ya - xb - yc + xd);

    /* writing xd' and yd' */
    *ptr1++ = xd_out;
    *ptr1++ = yd_out;
#endif /* defined RISCV_MATH_DSP && (__RISCV_XLEN == 64) */

  } while (--j);

  /* output is in 11.21(q21) format for the 1024 point */
  /* output is in 9.23(q23) format for the 256 point */
  /* output is in 7.25(q25) format for the 64 point */
  /* output is in 5.27(q27) format for the 16 point */

  /* End of last stage process */
}
