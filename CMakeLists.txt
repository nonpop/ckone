cmake_minimum_required(VERSION 2.8)
project(ckone)

set(CMAKE_C_FLAGS "-std=c99 -Wall -Wextra -Werror -pedantic -g3")

add_subdirectory(src)
add_subdirectory(test)
