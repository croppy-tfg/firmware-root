cmake_minimum_required(VERSION 3.22)
project(tsl2591 C)

add_library(tsl2591 INTERFACE)

target_include_directories(tsl2591 INTERFACE
    Inc
)

target_sources(tsl2591 INTERFACE
    Src/tsl2591.c
)
