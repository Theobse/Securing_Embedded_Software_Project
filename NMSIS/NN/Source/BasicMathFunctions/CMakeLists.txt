cmake_minimum_required (VERSION 3.14)

project(NMSISNNBasicMaths)

file(GLOB SRC "./*_*.c")

add_library(NMSISNNBasicMaths STATIC ${SRC})

### Includes
target_include_directories(NMSISNNBasicMaths PUBLIC "${NN}/Include")
