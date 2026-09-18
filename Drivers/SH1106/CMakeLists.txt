cmake_minimum_required(VERSION 3.22)
project(sh1106 C)

add_library(sh1106 INTERFACE)

target_include_directories(sh1106 INTERFACE
    Inc
)

target_sources(sh1106 INTERFACE
    Src/sh1106.c
)
