/* ----------------------------------------------------------------------
 * Project:      NMSIS DSP Library
 * Title:        riscv_biquad_cascade_df1_init_f16.c
 * Description:  Floating-point Biquad cascade DirectFormI(DF1) filter initialization function
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

#include "dsp/filtering_functions_f16.h"

#if defined(RISCV_FLOAT16_SUPPORTED)
/**
  @ingroup groupFilters
 */

/**
  @addtogroup BiquadCascadeDF1
  @{
 */

/**
  @brief         Initialization function for the floating-point Biquad cascade filter.
  @param[in,out] S           points to an instance of the floating-point Biquad cascade structure.
  @param[in]     numStages   number of 2nd order stages in the filter.
  @param[in]     pCoeffs     points to the filter coefficients.
  @param[in]     pState      points to the state buffer.
  @return        none

  @par           Coefficient and State Ordering
                   The coefficients are stored in the array <code>pCoeffs</code> in the following order:
  <pre>
      {b10, b11, b12, a11, a12, b20, b21, b22, a21, a22, ...}
  </pre>

  @par
                   where <code>b1x</code> and <code>a1x</code> are the coefficients for the first stage,
                   <code>b2x</code> and <code>a2x</code> are the coefficients for the second stage,
                   and so on. The <code>pCoeffs</code> array contains a total of <code>5*numStages</code> values.
  @par
                   The <code>pState</code> is a pointer to state array.
                   Each Biquad stage has 4 state variables <code>x[n-1], x[n-2], y[n-1],</code> and <code>y[n-2]</code>.
                   The state variables are arranged in the <code>pState</code> array as:
  <pre>
      {x[n-1], x[n-2], y[n-1], y[n-2]}
  </pre>
                   The 4 state variables for stage 1 are first, then the 4 state variables for stage 2, and so on.
                   The state array has a total length of <code>4*numStages</code> values.
                   The state variables are updated after each block of data is processed; the coefficients are untouched.
 
  @par             For MVE code, an additional buffer of modified coefficients is required.
                   Its size is numStages and each element of this buffer has type riscv_biquad_mod_coef_f16.
                   So, its total size is 96*numStages float16_t elements.

                   The initialization function which must be used is riscv_biquad_cascade_df1_mve_init_f16.
 */


void riscv_biquad_cascade_df1_init_f16(
        riscv_biquad_casd_df1_inst_f16 * S,
        uint8_t numStages,
  const float16_t * pCoeffs,
        float16_t * pState)
{
  /* Assign filter stages */
  S->numStages = numStages;

  /* Assign coefficient pointer */
  S->pCoeffs = pCoeffs;

  /* Clear state buffer and size is always 4 * numStages */
  memset(pState, 0, (4U * (uint32_t) numStages) * sizeof(float16_t));

  /* Assign state pointer */
  S->pState = pState;
}


/**
  @} end of BiquadCascadeDF1 group
 */
#endif /* #if defined(RISCV_FLOAT16_SUPPORTED) */
