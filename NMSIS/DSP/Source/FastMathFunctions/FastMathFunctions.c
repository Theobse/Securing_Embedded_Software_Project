/* ----------------------------------------------------------------------
 * Project:      NMSIS DSP Library
 * Title:        FastMathFunctions.c
 * Description:  Combination of all fast math function source files.
 *
 * $Date:        16. March 2020
 * $Revision:    V1.1.0
 *
 * Target Processor: RISC-V Cores
 * -------------------------------------------------------------------- */
/*
 * Copyright (C) 2019-2020 ARM Limited or its affiliates. All rights reserved.
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


#include "riscv_cos_f32.c"

#include "riscv_cos_q15.c"

#include "riscv_cos_q31.c"

#include "riscv_sin_f32.c"

#include "riscv_sin_q15.c"

#include "riscv_sin_q31.c"

#include "riscv_sqrt_q31.c"

#include "riscv_sqrt_q15.c"


#include "riscv_vexp_f32.c"
#include "riscv_vexp_f64.c"
#include "riscv_vlog_f32.c"
#include "riscv_vlog_f64.c"
#include "riscv_divide_q15.c"
#include "riscv_divide_q31.c"
#include "riscv_vlog_q31.c"
#include "riscv_vlog_q15.c"
#include "riscv_atan2_f32.c"
#include "riscv_atan2_q31.c"
#include "riscv_atan2_q15.c"
