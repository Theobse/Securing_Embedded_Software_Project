cmake_minimum_required(VERSION 3.14)

project(riscv_support)

INCLUDE_DIRECTORIES(${DSP}/Include)

add_library(riscv_support STATIC)

file(GLOB SRCS "./riscv_*.c")
#file(GLOB F16SRCS "./riscv_*f16.c")
#list(REMOVE_ITEM SRCS ${F16SRCS})

target_sources(riscv_support PRIVATE ${SRCS})
