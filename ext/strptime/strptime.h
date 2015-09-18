#ifndef STRPTIME_H
#define STRPTIME_H 1

#include "ruby.h"
VALUE rbtime_timespec_new(const struct timespec *ts, int offset);
struct tm * localtime_with_gmtoff_zone(const time_t *t, struct tm *result, long *gmtoff, const char **zone);
void timespec_now(struct timespec *ts);
time_t timegm_noleapsecond(struct tm *tm);
void tm_add_offset(struct tm *tm, long diff);

#endif /* STRPTIME_H */
