/*
 * SPDX-FileCopyrightText: Copyright 2010-2023 Arm Limited and/or its affiliates <open-source-office@arm.com>
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

/* ----------------------------------------------------------------------
 * Project:      NMSIS NN Library
 * Title:        riscv_relu_q15.c
 * Description:  Q15 version of ReLU
 *
 * $Date:        31 January 2023
 * $Revision:    V.1.1.1
 *
 * Target Processor: RISC-V Cores
 *
 * -------------------------------------------------------------------- */

#include "riscv_nnfunctions.h"
#include "riscv_nnsupportfunctions.h"

/**
 *  @ingroup Public
 */

/**
 * @addtogroup Acti
 * @{
 */

/*
 * Q15 ReLu function
 *
 * Refer header file for details.
 *
 */

void riscv_relu_q15(int16_t *data, uint16_t size)
{

#if defined(RISCV_MATH_VECTOR)
    uint16_t blkCnt = size & (~RVV_OPT_THRESHOLD);                               /* Loop counter */
    uint16_t tmp_i = blkCnt;
    size_t l;
    vint16m8_t vx;
    int16_t zero = 0;

    for (; (l = __riscv_vsetvl_e16m8(blkCnt)) > 0; blkCnt -= l) {
        vx = __riscv_vle16_v_i16m8(data, l);
        /* if data >= zero, return data, else return zero */
        __riscv_vse16_v_i16m8(data, __riscv_vmax_vx_i16m8(vx, zero, l), l);
        data += l;
    }

    for (uint16_t i = tmp_i; i < size; i++)
    {
        if (data[i] < 0)
            data[i] = 0;
    }
#else
#if defined(RISCV_MATH_DSP)
    /* Run the following code for M cores with DSP extension */

    uint16_t i = size >> 1;
    int16_t *input = data;
    int16_t *output = data;
    int32_t in;
    int32_t buf;
    int32_t mask;

    while (i)
    {
        in = riscv_nn_read_q15x2_ia((const int16_t **)&input);

        /* extract the first bit */
        buf = __ROR(in & 0x80008000, 15);

        /* if MSB=1, mask will be 0xFF, 0x0 otherwise */
        mask = __QSUB16(0x00000000, buf);

        riscv_nn_write_q15x2_ia(&output, in & (~mask));
        i--;
    }

    if (size & 0x1)
    {
        if (*input < 0)
        {
            *input = 0;
        }
        input++;
    }
#else
    /* Run the following code as reference implementation for M cores without DSP extension */
    uint16_t i;

    for (i = 0; i < size; i++)
    {
        if (data[i] < 0)
            data[i] = 0;
    }
#endif /* RISCV_MATH_DSP */
#endif /* RISCV_MATH_VECTOR */
}

/**
 * @} end of Acti group
 */
