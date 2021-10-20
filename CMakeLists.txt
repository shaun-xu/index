cmake_minimum_required(VERSION 3.10)
project(ts)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -Wall -Wextra")

find_package(Threads REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)

file(GLOB INCLUDE_H "include/ts/*.h")
file(GLOB INCLUDE_H "include/rs/*.h")
file(GLOB INCLUDE_H "include/thirdpart/libdivide/*.h")


set(EXAMPLE_FILES example.cc)
set(BENCH_END_TO_END_FILES bench_end_to_end.cc include/rmi/models/model.h
        include/rmi/models/linear.h include/rmi/traindata.h include/rmi/models/robust_linear.h
        include/rmi/models/log_linear.h include/rmi/models/utils.h include/rmi/models/radix.h
        include/thirdpart/libdivide/libdivide.h   include/thirdpart/libdivide/s16_ldparams.h
        include/thirdpart/libdivide/u16_ldparams.h  include/thirdpart/libdivide/constant_fast_div.h
        include/rmi/models/radix_table.h include/rmi/models/linear_spline.h
        include/rmi/models/balanced_radix.h include/rmi/models/cubic_spline.h include/rmi/models/normal.h
        include/rmi/models/log_normal.h include/rmi/rmi_models.h include/rmi/builder.h include/rmi/model_creator.h include/rmi/rmi_spline.h)

add_executable(example ${INCLUDE_H} ${EXAMPLE_FILES})
add_executable(bench_end_to_end ${INCLUDE_H} ${BENCH_END_TO_END_FILES})
