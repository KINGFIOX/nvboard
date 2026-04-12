# bin2c.cmake -- Convert a binary file to a C header with a byte array.
#
# Usage:
#   cmake -DINPUT_FILE=<path> -DOUTPUT_FILE=<path> -DVAR_NAME=<name> -P bin2c.cmake

cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED INPUT_FILE OR NOT DEFINED OUTPUT_FILE OR NOT DEFINED VAR_NAME)
  message(FATAL_ERROR "Usage: cmake -DINPUT_FILE=<f> -DOUTPUT_FILE=<f> -DVAR_NAME=<n> -P bin2c.cmake")
endif()

file(READ "${INPUT_FILE}" hex HEX)
string(LENGTH "${hex}" hex_len)
math(EXPR byte_count "${hex_len} / 2")

string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," hex "${hex}")
# Insert a newline every 16 bytes for readability.
string(REGEX REPLACE "(0x[0-9a-f][0-9a-f],){16}" "\\0\n  " hex "${hex}")

file(WRITE "${OUTPUT_FILE}"
"#pragma once
// Auto-generated from ${INPUT_FILE} -- do not edit.
#include <cstddef>
inline const unsigned char ${VAR_NAME}[] = {
  ${hex}
};
inline constexpr size_t ${VAR_NAME}_size = ${byte_count};
")
