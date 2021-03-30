#ifndef MACROS_H
#define MACROS_H

#define array_count(a) (sizeof((a)) / sizeof(*(a)))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define add_and_clamp_to_zero(a, b) (((a) + (b) <= 0) ? 0 : (a) + (b))

#define MAXLEN 512

#endif
