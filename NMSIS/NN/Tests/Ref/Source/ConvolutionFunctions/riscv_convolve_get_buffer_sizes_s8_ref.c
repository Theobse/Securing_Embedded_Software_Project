/*
 * SPDX-FileCopyrightText: Copyright 2023 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * Copyright (c) 2019 Nuclei Limited. All rights reserved.
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
 * Title:        riscv_convolve_get_buffer_sizes_s8.c
 * Description:  Collection of get buffer size functions for the various s8 convolution layer functions.
 *
 * $Date:        8 March 2023
 * $Revision:    V.1.1.0
 *
 * Target :  RISC-V Cores
 *
 * -------------------------------------------------------------------- */

#include "ref_functions.h"
#include "riscv_nnsupportfunctions.h"

/**
 *  @ingroup NNConv
 */

/**
 * @addtogroup GetBufferSizeNNConv
 * @{
 */

int32_t riscv_convolve_s8_get_buffer_size_ref(const nmsis_nn_dims *input_dims, const nmsis_nn_dims *filter_dims)
{
    return (2 * input_dims->c * filter_dims->w * filter_dims->h) * (int32_t)sizeof(int16_t);
}

int32_t riscv_convolve_1_x_n_s8_get_buffer_size_ref(const nmsis_nn_dims *input_dims, const nmsis_nn_dims *filter_dims)
{
    return riscv_convolve_s8_get_buffer_size_ref(input_dims, filter_dims);
}

int32_t riscv_convolve_1x1_s8_fast_get_buffer_size_ref(const nmsis_nn_dims *input_dims)
{
    (void)input_dims;
    return 0;
}

/*
 * Get the required buffer size for riscv_convolve_wrapper_s8. This is the recommended function convolve wrapper s8
 * function.
 *
 * Refer to header file for details.
 *
 */
int32_t riscv_convolve_wrapper_s8_get_buffer_size_ref(const nmsis_nn_conv_params *conv_params,
                                                const nmsis_nn_dims *input_dims,
                                                const nmsis_nn_dims *filter_dims,
                                                const nmsis_nn_dims *output_dims)
{
    (void)output_dims;
    if ((conv_params->padding.w == 0) && (conv_params->padding.h == 0) && (filter_dims->w == 1) &&
        (filter_dims->h == 1) && (conv_params->dilation.w == 1 && conv_params->dilation.h == 1))
    {
        if ((conv_params->stride.w == 1) && (conv_params->stride.h == 1))
        {
            return riscv_convolve_1x1_s8_fast_get_buffer_size_ref(input_dims);
        }
        else
        {
            return 0;
        }
    }
    else if ((input_dims->h == 1) && (conv_params->dilation.w == 1) && (filter_dims->h == 1) &&
             (conv_params->stride.w * input_dims->c % 4 == 0))
    {
        return riscv_convolve_1_x_n_s8_get_buffer_size_ref(input_dims, filter_dims);
    }
    else
    {
        return riscv_convolve_s8_get_buffer_size_ref(input_dims, filter_dims);
    }
}


/**
 * @} end of GetBufferSizeNNConv group
 */
