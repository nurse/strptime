#ifndef STRPTIME_H
#define STRPTIME_H 1

#include "ruby.h"
VALUE timespec_new(const struct timespec *ts, int offset);

#endif /* STRPTIME_H */
