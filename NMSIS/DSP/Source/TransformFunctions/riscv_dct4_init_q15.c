/* ----------------------------------------------------------------------
 * Project:      NMSIS DSP Library
 * Title:        riscv_dct4_init_q15.c
 * Description:  Initialization function of DCT-4 & IDCT4 Q15
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
#include "riscv_common_tables.h"

/**
 * @defgroup DCT4Q15 DCT4 Q15
 */

/**
  @ingroup DCT4_IDCT4
 */

 /**
  @addtogroup DCT4Q15
  @{
 */

/**
  @brief         Initialization function for the Q15 DCT4/IDCT4.
  @deprecated    Do not use this function. It will be removed in future versions.
  @param[in,out] S         points to an instance of Q15 DCT4/IDCT4 structure
  @param[in]     S_RFFT    points to an instance of Q15 RFFT/RIFFT structure
  @param[in]     S_CFFT    points to an instance of Q15 CFFT/CIFFT structure
  @param[in]     N          length of the DCT4
  @param[in]     Nby2       half of the length of the DCT4
  @param[in]     normalize  normalizing factor
  @return        execution status
                   - \ref RISCV_MATH_SUCCESS        : Operation successful
                   - \ref RISCV_MATH_ARGUMENT_ERROR : <code>N</code> is not a supported transform length

  @par           Normalizing factor
                   The normalizing factor is <code>sqrt(2/N)</code>, which depends on the size of transform <code>N</code>.
                   Normalizing factors in 1.15 format are mentioned in the table below for different DCT sizes:

| DCT Size  | Normalizing factor value (hexadecimal)  |
| --------: | ---------------------------------------:|
| 2048      | 0x400                                   |
| 512       | 0x800                                   |
| 128       | 0x1000                                  |

 */

riscv_status riscv_dct4_init_q15(
  riscv_dct4_instance_q15 * S,
  riscv_rfft_instance_q15 * S_RFFT,
  riscv_cfft_radix4_instance_q15 * S_CFFT,
  uint16_t N,
  uint16_t Nby2,
  q15_t normalize)
{
  /*  Initialise the default riscv status */
  riscv_status status = RISCV_MATH_SUCCESS;

  /* Initialize the DCT4 length */
  S->N = N;

  /* Initialize the half of DCT4 length */
  S->Nby2 = Nby2;

  /* Initialize the DCT4 Normalizing factor */
  S->normalize = normalize;

  /* Initialize Real FFT Instance */
  S->pRfft = S_RFFT;

  /* Initialize Complex FFT Instance */
  S->pCfft = S_CFFT;

  switch (N)
  {
    /* Initialize the table modifier values */
  case 8192U:
    S->pTwiddle = WeightsQ15_8192;
    S->pCosFactor = cos_factorsQ15_8192;
    break;

  case 2048U:
    S->pTwiddle = WeightsQ15_2048;
    S->pCosFactor = cos_factorsQ15_2048;
    break;

  case 512U:
    S->pTwiddle = WeightsQ15_512;
    S->pCosFactor = cos_factorsQ15_512;
    break;

  case 128U:
    S->pTwiddle = WeightsQ15_128;
    S->pCosFactor = cos_factorsQ15_128;
    break;

  default:
    status = RISCV_MATH_ARGUMENT_ERROR;
  }

  /* Initialize the RFFT/RIFFT */
  riscv_rfft_init_q15(S->pRfft, S->N, 0U, 1U);

  /* return the status of DCT4 Init function */
  return (status);
}

/**
  @} end of DCT4Q15 group
 */
