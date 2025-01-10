#pragma once
#include <stddef.h>
#include <stdint.h>
#include <printf.h>
#include <print.hpp>

const size_t KIB = 1024;
const size_t MIB = 1024 * 1024;
const size_t GIB = 1024 * 1024 * 1024;

inline const size_t div_ceil(size_t value, size_t divisor) {
    return (value + divisor - 1) / divisor;
}

inline const char* size_human_readable_ceil(size_t s) {
    size_t size = s;
    char* unit = "\0";

    if ((s / GIB) > 0) {
        size = div_ceil(s, GIB);
        unit = "GiB";
    } else if ((s / MIB) > 0) {
        size = div_ceil(s, MIB);
        unit = "MiB";
    } else if ((s / KIB) > 0) {
        size = div_ceil(s, KIB);
        unit = "KiB";
    } else {
        unit = "Byte"; 
    }


    char* output;
    snprintf_(output, 10, "%d %s", size, unit);
    return output;
}