#ifndef COMMON_H_
#define COMMON_H_

#include <boost/cstdint.hpp>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdarg>

#define MAX_FORMAT_LEN 32*1024

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t byte;

typedef std::vector<byte> BinaryData;

#endif
