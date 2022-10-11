cmake_minimum_required(VERSION 3.15)

project(
    mex
    VERSION 1.0.0
    DESCRIPTION "TeX-inspired macro processor"
    HOMEPAGE_URL "https://github.com/aslrousta/mex"
    LANGUAGES C
)
set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

add_subdirectory(libmex)
add_subdirectory(mex)
