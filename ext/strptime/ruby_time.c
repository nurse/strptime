#include "ruby.h"

typedef uint64_t WIDEVALUE;
typedef WIDEVALUE wideval_t;
#ifndef PACKED_STRUCT_UNALIGNED
#define PACKED_STRUCT_UNALIGNED(x) x
#endif
PACKED_STRUCT_UNALIGNED(struct vtm {
    VALUE year;	/* 2000 for example.  Integer. */
    VALUE subsecx;     /* 0 <= subsecx < TIME_SCALE.  possibly Rational. */
    VALUE utc_offset;  /* -3600 as -01:00 for example.  possibly Rational. */
    const char *zone;  /* "JST", "EST", "EDT", etc. */
    uint16_t yday : 9; /* 1..366 */
    uint8_t mon : 4;   /* 1..12 */
    uint8_t mday : 5;  /* 1..31 */
    uint8_t hour : 5;  /* 0..23 */
    uint8_t min : 6;   /* 0..59 */
    uint8_t sec : 6;   /* 0..60 */
    uint8_t wday : 3;  /* 0:Sunday, 1:Monday, ..., 6:Saturday 7:init */
    uint8_t isdst : 2; /* 0:StandardTime 1:DayLightSavingTime 3:init */
});
PACKED_STRUCT_UNALIGNED(struct time_object {
    wideval_t timew; /* time_t value * TIME_SCALE.  possibly Rational. */
    struct vtm vtm;
    uint8_t gmt : 3; /* 0:utc 1:localtime 2:fixoff 3:init */
    uint8_t tm_got : 1;
});

VALUE
timespec_new(const struct timespec *ts, int offset)
{
    VALUE obj = rb_time_nano_new(ts->tv_sec, ts->tv_nsec);
    struct time_object *tobj = DATA_PTR(obj);
    tobj->tm_got = 0;
    tobj->gmt = 2;
    tobj->vtm.utc_offset = INT2FIX(offset);
    tobj->vtm.zone = NULL;
    return obj;
}
