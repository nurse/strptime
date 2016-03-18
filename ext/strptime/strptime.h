#ifndef STRPTIME_H
#define STRPTIME_H 1

#include "ruby.h"
# ifndef HAVE_RB_TIME_TIMESPEC_NEW
VALUE rb_time_timespec_new(const struct timespec *ts, int offset);
# endif
struct tm * localtime_with_gmtoff_zone(const time_t *t, struct tm *result, long *gmtoff, const char **zone);
# ifndef HAVE_RB_TIMESPEC_NOW
void rb_timespec_now(struct timespec *ts);
# endif
time_t timegm_noleapsecond(struct tm *tm);
const char *find_time_t(struct tm *tptr, int utc_p, time_t *tp);
void tm_add_offset(struct tm *tm, long diff);

#endif /* STRPTIME_H */
