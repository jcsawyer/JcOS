#ifndef _LIMITS_H
#define _LIMITS_H

#define CHAR_BIT 8         // Number of bits in a char
#define SCHAR_MIN (-128)   // Minimum value for a signed char
#define SCHAR_MAX 127      // Maximum value for a signed char
#define UCHAR_MAX 255      // Maximum value for an unsigned char
#define CHAR_MIN SCHAR_MIN // Minimum value for a char (if signed)
#define CHAR_MAX SCHAR_MAX // Maximum value for a char (if signed)

#define SHRT_MIN (-32768) // Minimum value for a short
#define SHRT_MAX 32767    // Maximum value for a short
#define USHRT_MAX 65535   // Maximum value for an unsigned short

#define INT_MIN (-2147483648) // Minimum value for an int
#define INT_MAX 2147483647    // Maximum value for an int
#define UINT_MAX 4294967295U  // Maximum value for an unsigned int

#define LONG_MIN (-9223372036854775808L) // Minimum value for a long
#define LONG_MAX 9223372036854775807L    // Maximum value for a long
#define ULONG_MAX 18446744073709551615UL // Maximum value for an unsigned long

#endif // _LIMITS_H
