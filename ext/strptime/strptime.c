#include "strptime.h"
#include <time.h>

VALUE rb_cStrptime;

#define GetStrptimeval(obj, tobj) ((tobj) = get_strptimeval(obj))
#define GetNewStrptimeval(obj, tobj) ((tobj) = get_new_strptimeval(obj))
#define STRPTIME_INIT_P(tobj) ((tobj)->isns)

#define LIKELY(x) (__builtin_expect((x), 1))
#define UNLIKELY(x) (__builtin_expect((x), 0))

#define REG_PC (pc)
#define GET_PC() REG_PC
#define SET_PC(x) (REG_PC = (x))
#define GET_CURRENT_INSN() (*GET_PC())
#define GET_OPERAND(n) (GET_PC()[(n)])
#define ADD_PC(n) (SET_PC(REG_PC + (n)))

#define JUMP(dst) (REG_PC += (dst))

#define LABEL(x) INSN_LABEL_##x
#define ELABEL(x) INSN_ELABEL_##x
#define LABEL_PTR(x) &&LABEL(x)

#define INSN_ENTRY(insn) LABEL(insn) :

#define TC_DISPATCH(insn)                                                      \
    goto *(void const *)GET_CURRENT_INSN();                                    \
    ;
#define END_INSN(insn) TC_DISPATCH(insn);

#define INSN_DISPATCH()                                                        \
    TC_DISPATCH(__START__)                                                     \
    {

#define END_INSNS_DISPATCH()                                                   \
    rb_bug("strptime: unknown insn: %p", GET_CURRENT_INSN());                  \
    } /* end of while loop */

#define NEXT_INSN() TC_DISPATCH(__NEXT_INSN__)

static const char *day_names[] = {
    "Sunday", "Monday", "Tuesday", "Wednesday",
    "Thursday", "Friday", "Saturday",
    "Sun", "Mon", "Tue", "Wed",
    "Thu", "Fri", "Sat"
};
static const char *month_names[] = {
    "January", "February", "March",     "April",   "May",      "June",
    "July",    "August",   "September", "October", "November", "December",
    "Jan",     "Feb",      "Mar",       "Apr",     "May",      "Jun",
    "Jul",     "Aug",      "Sep",       "Oct",     "Nov",      "Dec"};

#define sizeof_array(o) (sizeof o / sizeof o[0])
#define str_readable(p, pe, len) (pe - p >= len)
#define GETC(p, pe) (p < pe ? *p++ : -1)
#define issign(c) ((c) == '-' || (c) == '+')
#undef tolower
#define tolower(c) ((unsigned char)(c)|0x20u)
#undef isalpha
#define isalpha(c) ((tolower(c) - 'a') <= 26u)
#undef isdigit
#define isdigit(c) ((unsigned char)((c) - '0') <= 9u)
#undef isspace
#define isspace(c)                                                             \
    ((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\v' || (c) == '\f' || \
     (c) == '\r')
#define MIN(a,b) ((a) < (b) ? (a) : (b))

/* imply NUL-terminated string */
static size_t
read_digits(const char *s, const char *se, int *n)
{
    const char *s0 = s;
    int r = 0;

    for (; s < se && isdigit(*s); s++) {
	r *= 10;
	r += (unsigned char)((*s) - '0');
    }
    *n = r;
    return (size_t)(s - s0);
}

#define fail()                                                                 \
    {                                                                          \
	return -1;                                                             \
    }

#define READ_DIGITS(n, w)                                                      \
    {                                                                          \
	size_t l;                                                              \
	l = read_digits(p, MIN(p+w,pe), &n);                                   \
	if (l == 0) fail();                                                    \
	p += l;                                                               \
    }

static int
valid_range_p(int v, int a, int b)
{
    return !(v < a || v > b);
}

#define FLAG_EQ(f, v) ((f)&(v)==(v))
#define FLAG_SEC     (1 <<  0)
#define FLAG_MIN     (1 <<  1)
#define FLAG_HOUR    (1 <<  2)
#define FLAG_MDAY    (1 <<  3)
#define FLAG_MON     (1 <<  4)
#define FLAG_YEAR    (1 <<  5)
#define FLAG_WDAY    (1 <<  6)
#define FLAG_YDAY    (1 <<  7)
#define FLAG_gmtoff  (1 <<  8)
#define FLAG_nsec    (1 <<  9)
#define FLAG_worigin (1 << 10)
#define FLAG_wdayoff (1 << 11)
#define FLAG_weekoff (1 << 12)
#define FLAG_hour12  (1 << 13)
#define FLAG_pm      (1 << 14)
#define FLAG_CENTURY (1 << 15)
#define FLAG_YY      (1 << 16)

struct sptm {
    int flags;
    int sec;
    int min;
    int hour;
    int mday;
    int mon;
    VALUE year;
    int century;
    int wday;
    int yday;
    int gmtoff;
    int nsec;
    int week_origin;
    int day_offset;
    int week_offset;
};

static const int common_year_yday_offset[] = {
    -1,
    -1 + 31,
    -1 + 31 + 28,
    -1 + 31 + 28 + 31,
    -1 + 31 + 28 + 31 + 30,
    -1 + 31 + 28 + 31 + 30 + 31,
    -1 + 31 + 28 + 31 + 30 + 31 + 30,
    -1 + 31 + 28 + 31 + 30 + 31 + 30 + 31,
    -1 + 31 + 28 + 31 + 30 + 31 + 30 + 31 + 31,
    -1 + 31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
    -1 + 31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
    -1 + 31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30
      /* 1    2    3    4    5    6    7    8    9    10   11 */
};
static const int leap_year_yday_offset[] = {
    -1,
    -1 + 31,
    -1 + 31 + 29,
    -1 + 31 + 29 + 31,
    -1 + 31 + 29 + 31 + 30,
    -1 + 31 + 29 + 31 + 30 + 31,
    -1 + 31 + 29 + 31 + 30 + 31 + 30,
    -1 + 31 + 29 + 31 + 30 + 31 + 30 + 31,
    -1 + 31 + 29 + 31 + 30 + 31 + 30 + 31 + 31,
    -1 + 31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
    -1 + 31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
    -1 + 31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30
      /* 1    2    3    4    5    6    7    8    9    10   11 */
};
static int
leap_year_p(int y)
{
    return ((y % 4 == 0) && (y % 100 != 0)) || (y % 400 == 0);
}
static void
sptm_init(struct sptm *sptm) {
    sptm->day_offset = -1;
}
static int
sptm2timespec(struct sptm *sptm, struct timespec *tsp, int *gmtoffp)
{
	struct tm tm;
	time_t t;
	int gmtoff = *gmtoffp;
	int gmt = gmtoff >= INT_MAX-1 ? INT_MAX-gmtoff : 2;

	if (!(sptm->flags & (FLAG_CENTURY | FLAG_YEAR | FLAG_YY))) {
	    /* get current time with timezone */
	    static time_t ct;
	    static struct tm ctm;
	    static long ctmoff;
	    static long localoff;
	    rb_timespec_now(tsp);
	    if (ct != tsp->tv_sec) {
		ct = tsp->tv_sec;
		localtime_with_gmtoff_zone(&ct, &ctm, &ctmoff, NULL);
		localoff = ctmoff;
	    }
	    if (gmtoff == INT_MAX) {
		gmtoff = localoff;
	    }
	    else if (gmtoff == INT_MAX-1) {
		gmtoff = 0;
	    }
	    if (gmtoff != ctmoff) {
		tm_add_offset(&ctm, gmtoff - ctmoff);
		ctmoff = gmtoff;
	    }
	    sptm->year = ctm.tm_year;
	    if (sptm->flags & FLAG_YDAY) {
	    }
	}

	if (FLAG_EQ(sptm->flags, FLAG_HOUR|FLAG_PM)) {
	    sptm->hour += 12;
	}

	if (sptm->flags & FLAG_CENTURY) {
	    if (sptm->flags & (FLAG_YEAR | FLAG_YY)) {
		sptm->year %= 100;
	    }
	    sptm->year += (sptm->century - 19) * 100;
	    sptm->flags |= FLAG_YEAR;
	}

	if (sptm->flags & (FLAG_YDAY|FLAG_YEAR) == FLAG_YEAR) {
	    if (FLAG_EQ(sptm->flags, FLAG_MON|FLAG_MDAY)) {
		sptm->yday = sptm->mday;
		if (leap_year_p(tm_year + 1900))
		    tm_yday += leap_year_yday_offset[tm->tm_mon];
		else
		    tm_yday += common_year_yday_offset[tm->tm_mon];
		sptm->flags |= FLAG_YDAY;
	    }
	    else if (sptm->day_offset != -1) {
			if (!(flags & FLAG_WDAY)) {
				tm->tm_wday = day_offset;
				flags |= FLAG_WDAY;
			}
			tm->tm_yday = (7 -
			    first_wday_of(tm->tm_year + TM_YEAR_BASE) +
			    day_offset) % 7 + (week_offset - 1) * 7 +
			    tm->tm_wday - day_offset;
			flags |= FLAG_YDAY;
	    }
	}

	if (sptm->year != INT_MAX) {
	    if (mon == -1) mon = 1;
	setmonth:
	    if (mday == -1) mday = 1;
	setmday:
	    if (hour == -1) hour = 0;
	sethour:
	    tm.tm_hour = hour;
	    if (min == -1) min = 0;
	setmin:
	    tm.tm_min = min;
	    if (sec == -1) sec = 0;
	    tm.tm_sec = sec;
	}
	else {
	    if (mon != -1) goto setmonth;
	    if (mday != -1) goto setmday;
	    if (hour != -1) goto sethour;
	    if (min != -1) goto setmin;
	    if (sec != -1) tm.tm_sec = sec;
	}

	t = timegm_noleapsecond(&tm);
	if (gmt != 1) t -= gmtoff;
	tsp->tv_sec = t;
	tsp->tv_nsec = nsec;
	*gmtoffp = gmtoff;
}

static int
strptime_exec0(void **pc, const char *p, const char *pe,
	       struct timespec *tsp, int *gmtoffp)
{
    struct sptm sptm = {};
    if (UNLIKELY(tsp == NULL)) {
	static const void *const insns_address_table[] = {
	    LABEL_PTR(A),  LABEL_PTR(B),  LABEL_PTR(C),  NULL,
	    NULL,          NULL,          NULL,          LABEL_PTR(H),
	    LABEL_PTR(I),  NULL,          NULL,          LABEL_PTR(L),
	    LABEL_PTR(M),  LABEL_PTR(N),  NULL,          LABEL_PTR(P),
	    NULL,          NULL,          LABEL_PTR(S),  NULL,
	    LABEL_PTR(U),  LABEL_PTR(V),  LABEL_PTR(W),  NULL,
	    LABEL_PTR(Y),  LABEL_PTR(Z),  NULL,          NULL,
	    NULL,          LABEL_PTR(_5e),LABEL_PTR(_5f),LABEL_PTR(_60),
	    LABEL_PTR(A),  LABEL_PTR(B),  NULL,          LABEL_PTR(d),
	    LABEL_PTR(d),  NULL,          NULL,          LABEL_PTR(B),
	    NULL,          LABEL_PTR(j),  LABEL_PTR(H),  LABEL_PTR(I),
	    LABEL_PTR(m),  LABEL_PTR(n),  NULL,          LABEL_PTR(P),
	    NULL,          NULL,          LABEL_PTR(s),  LABEL_PTR(t),
	    LABEL_PTR(w),  NULL,          LABEL_PTR(w),  NULL,
	    LABEL_PTR(y),  LABEL_PTR(z),
	};
	*pc = (void *)insns_address_table;
	return 0;
    }

    INSN_DISPATCH();
    INSN_ENTRY(A)
    {
	int i;
	for (i = 0; i < (int)sizeof_array(day_names); i++) {
	    size_t l = strlen(day_names[i]);
	    if (!str_readable(p, pe, l)) continue;
	    if (strncasecmp(day_names[i], p, l) == 0) {
		p += l;
		sptm->wday = i % 7;
		ADD_PC(1);
		END_INSN(A);
	    }
	}
	fail();
    }
    INSN_ENTRY(B)
    {
	int i;
	for (i = 0; i < (int)sizeof_array(month_names); i++) {
	    size_t l = strlen(month_names[i]);
	    if (!str_readable(p, pe, l)) continue;
	    if (strncasecmp(month_names[i], p, l) == 0) {
		p += l;
		sptm->mon = (i % 12) + 1;
		ADD_PC(1);
		END_INSN(B)
	    }
	}
	fail();
    }
    INSN_ENTRY(C)
    {
	READ_DIGITS(sptm->century, 2);
	sptm->flags |= FLAG_CENTURY;
	ADD_PC(1);
	END_INSN(C)
    }
    INSN_ENTRY(H)
    {
	if (!str_readable(p, pe, 1)) fail();
	if (*p == ' ') {
	    p++;
	    READ_DIGITS(sptm->hour, 1);
	} else {
	    READ_DIGITS(sptm->hour, 2);
	}
	if (!valid_range_p(sptm->hour, 0, 23)) fail();
	sptm->flags |= FLAG_HOUR | FLAG_HOUR24;
	ADD_PC(1);
	END_INSN(H)
    }
    INSN_ENTRY(I)
    {
	if (!str_readable(p, pe, 1)) fail();
	if (*p == ' ') {
	    p++;
	    READ_DIGITS(sptm->hour, 1);
	} else {
	    READ_DIGITS(sptm->hour, 2);
	}
	if (!valid_range_p(sptm->hour, 0, 12)) fail();
	sptm->flags |= FLAG_HOUR;
	sptm->flags &= ~FLAG_HOUR24;
	ADD_PC(1);
	END_INSN(I)
    }
    INSN_ENTRY(L)
    {
	size_t l;
	l = read_digits(p, MIN(p+3,pe), &nsec);
	if (!l) fail();
	p += l;
	for (; l < 3; l++) {
	    sptm->nsec *= 10;
	}
	sptm->nsec *= 1000000;
	ADD_PC(1);
	END_INSN(L)
    }
    INSN_ENTRY(M)
    {
	READ_DIGITS(sptm->min, 2);
	if (!valid_range_p(sptm->min, 0, 59)) fail();
	ADD_PC(1);
	END_INSN(M)
    }
    INSN_ENTRY(N)
    {
	size_t l;
	l = read_digits(p, MIN(p+9,pe), &nsec);
	if (!l) fail();
	p += l;
	for (; l < 9; l++) {
	    sptm->nsec *= 10;
	}
	ADD_PC(1);
	END_INSN(N)
    }
    INSN_ENTRY(P)
    {
	unsigned char uc;
	str_readable(p, pe, 2);
	uc = tolower(*p);
	if (uc == 'a')
	    sptm->flags &= ~FLAG_PM;
	else if (uc == 'p')
	    sptm->flags |= FLAG_PM;
	else
	    fail();
	uc = tolower(*p++);
	if (uc != 'm') fail();
	ADD_PC(1);
	END_INSN(P)
    }
    INSN_ENTRY(S)
    {
	READ_DIGITS(sptm->sec, 2);
	if (!valid_range_p(sptm->sec, 0, 60)) fail();
	ADD_PC(1);
	END_INSN(S)
    }
    INSN_ENTRY(U)
    {
	READ_DIGITS(sptm->week_offset, 2);
	if (!valid_range_p(sptm->week_offset, 0, 53)) fail();
	sptm->day_offset = 0;
	ADD_PC(1);
	END_INSN(U)
    }
    INSN_ENTRY(V)
    {
	READ_DIGITS(sptm->week_offset, 2);
	if (!valid_range_p(sptm->week_offset, 1, 53)) fail();
	ADD_PC(1);
	END_INSN(V)
    }
    INSN_ENTRY(W)
    {
	READ_DIGITS(sptm->week_offset, 2);
	if (!valid_range_p(sptm->week_offset, 0, 53)) fail();
	sptm->day_offset = 1;
	ADD_PC(1);
	END_INSN(W)
    }
    INSN_ENTRY(Y)
    {
	int c;
	str_readable(p, pe, 1);
	c = *p;
	if (issign(c)) p++;
	READ_DIGITS(sptm->year, 4);
	if (c == '-') sptm->year *= -1;
	sptm->year -= 1900;
	ADD_PC(1);
	END_INSN(Y)
    }
    INSN_ENTRY(Z)
    {
	ADD_PC(1);
	END_INSN(Z)
    }
    INSN_ENTRY(d)
    {
	READ_DIGITS(sptm->mday, 2);
	if (!valid_range_p(sptm->mday, 1, 31)) fail();
	ADD_PC(1);
	END_INSN(d)
    }
    INSN_ENTRY(j)
    {
	READ_DIGITS(sptm->yday, 3);
	if (!valid_range_p(sptm->yday, 1, 366)) fail();
	sptm->yday--;
	ADD_PC(1);
	END_INSN(j)
    }
    INSN_ENTRY(m)
    {
	READ_DIGITS(sptm->mon, 2);
	if (!valid_range_p(sptm->mon, 1, 12)) fail();
	ADD_PC(1);
	END_INSN(m)
    }
    INSN_ENTRY(n)
    {
	for (; p < pe && isspace(*p); p++) {
	}
	ADD_PC(1);
	END_INSN(n)
    }
    INSN_ENTRY(s)
    {
	const char *p0 = p;
	const char *pet = p + 10;
	int64_t v;
	int c = GETC(p, pe);
	if (!isdigit(c)) fail();
	v = c - '0';
	for (; p < pet && isdigit(*p); p++) {
	    v *= 10;
	    v += (unsigned char)((*p) - '0');
	}
	if (v > INT32_MAX) {
	    /* TODO */
	    fail();
	}
	else {
	    time_t t = (time_t)v;
	    struct tm tm;
	    long off;
	    localtime_with_gmtoff_zone(&t, &tm, &off, NULL);
	    /* TODO */
	}
	ADD_PC(1);
	END_INSN(s)
    }
    INSN_ENTRY(t)
    {
	if (*p++ != '\t') fail();
	ADD_PC(1);
	END_INSN(t)
    }
    INSN_ENTRY(w)
    {
	READ_DIGITS(sptm->wday, 1);
	if (!valid_range_p(sptm->wday, 0, 7)) fail();
	sptm->wday %= 7;
	ADD_PC(1);
	END_INSN(w)
    }
    INSN_ENTRY(y)
    {
	READ_DIGITS(sptm->year, 2);
	if (sptm->year < 69) sptm->year += 100;
	sptm->flags |= FLAG_YY;
	ADD_PC(1);
	END_INSN(y)
    }
    INSN_ENTRY(z)
    {
	const char *p0 = p;
	int r;
	size_t len;
	str_readable(p, pe, 1);
	if (tolower(*p) == 'z') {
	    gmtoff = 0;
	    ADD_PC(1);
	    END_INSN(z);
	}
	if (issign(*p)) p++;
	READ_DIGITS(r, 2);
	gmtoff = r * 60;
	str_readable(p, pe, 1);
	if (*p == ':') p++;
	len = read_digits(p, MIN(p+2,pe), &r);
	if (len) {
	    p += len;
	    sptm->gmtoff += r;
	}
	sptm->gmtoff *= 60;
	if (*p0 == '-') sptm->gmtoff = -sptm->gmtoff;
	ADD_PC(1);
	END_INSN(z)
    }
    INSN_ENTRY(_5e)
    {
	const char *q0 = (char *)&GET_OPERAND(1);
	const char *q = q0;
	/* like C string but the first character is a char even if NUL */
	str_readable(p, pe, 1);
	do {
	    if (*p++ != *q++) return Qnil;
	} while (*q && p < pe);
	pc += 1 + (q - q0 + sizeof(void *) - 1) / sizeof(void *);
	END_INSN(__5e)
    }
    INSN_ENTRY(_5f)
    {
	rb_raise(rb_eArgError, "invalid strptime bytecode _60");
	ADD_PC(1);
	END_INSN(_5f)
    }
    INSN_ENTRY(_60)
    {
	sptm2timespec(&sptm, tsp, gmtoffp);
	return 0;
    }
    END_INSNS_DISPATCH();

    /* unreachable */
    rb_bug("strptime_exec0: unreachable");
    UNREACHABLE;
}

struct irbuf {
    void **ptr;
    size_t count;
    size_t capa;
};
static void
irbuf_shrink(struct irbuf *buf)
{
    buf->capa = buf->count;
    REALLOC_N(buf->ptr, void *, buf->capa);
}
static void
irbuf_ensure_writable(struct irbuf *buf, size_t delta)
{
    while (buf->count + delta > buf->capa) {
	buf->capa *= 2;
    }
    REALLOC_N(buf->ptr, void *, buf->capa);
}
static void
irbuf_put(struct irbuf *buf, void *ptr)
{
    irbuf_ensure_writable(buf, 1);
    buf->ptr[buf->count++] = ptr;
}
#define irbuf_putc(buf, c) irbuf_put(buf, insns_address_table[(unsigned char)(c) - 'A'])
static void
irbuf_write(struct irbuf *buf, const char *p, size_t len)
{
    size_t vlen = (len + 1 + sizeof(void *) - 1) / sizeof(void *);
    char *q;
    irbuf_ensure_writable(buf, vlen);
    q = (char *)&buf->ptr[buf->count];
    memcpy(q, p, len);
    q[len] = 0;
    buf->count += vlen;
}

#define recur(str) strptime_compile0(str, str+strlen(str), buf, insns_address_table)
static void
strptime_compile0(const char *p, const char *pe,
	struct irbuf *buf, void **insns_address_table)
{
    while (p < pe) {
	int c = GETC(p, pe);
	int modifier = 0;
	switch (c) {
	case '%':
label:
	    c = GETC(p, pe);
	    if (isalpha(c)) {
		void *tmp = insns_address_table[c - 'A'];
		if (tmp) {
		    irbuf_put(buf, tmp);;
		    continue;
		}
	    }
	    switch (c) {
	      case 'E': case 'O':
		if (modifier) break;
		modifier = c;
		goto label;
	      case '%':
		goto ordinal;
	      case 'c':
		recur("%a %b %e %H:%M:%S %Y");
		continue;
	    }
	    rb_raise(rb_eArgError, "invalid format");
	case ' ':
	case '\t':
	case '\n':
	case '\v':
	case '\f':
	case '\r':
	    irbuf_putc(buf, 'n');;
	    break;
	default:
ordinal:
	    irbuf_putc(buf, '^');;
	    {
		const char *p0 = p - 1;
		while (p < pe && *p && *p != '%' && !ISSPACE(*p))
		    p++;
		irbuf_write(buf, p0, p - p0);
	    }
	    break;
	}
    }
}

static void **
strptime_compile(const char *p, const char *pe)
{
    struct irbuf bufst;
    struct irbuf *buf = &bufst;
    void **insns_address_table;
    buf->count = 0;
    buf->capa = 256;
    buf->ptr = ALLOC_N(void *, buf->capa);
    strptime_exec0((void **)&insns_address_table, NULL, NULL, NULL, NULL);
    strptime_compile0(p, pe, buf, insns_address_table);
    irbuf_putc(buf, '`');;
    irbuf_shrink(buf);
    return buf->ptr;
}

struct strptime_object {
    void **isns;
    VALUE fmt;
};

static void
strptime_mark(void *ptr)
{
    struct strptime_object *tobj = ptr;
    rb_gc_mark(tobj->fmt);
}

static void
strptime_free(void *ptr)
{
    struct strptime_object *tobj = ptr;
    ruby_xfree(tobj->isns);
}

static size_t
strptime_memsize(const void *tobj)
{
    return sizeof(struct strptime_object);
}

static const rb_data_type_t strptime_data_type = {
    "strptime",
    {
	strptime_mark, strptime_free, strptime_memsize,
    },
#ifdef RUBY_TYPED_FREE_IMMEDIATELY
    0,
    0,
    RUBY_TYPED_FREE_IMMEDIATELY
#endif
};

static VALUE
strptime_s_alloc(VALUE klass)
{
    VALUE obj;
    struct strptime_object *tobj;

    obj = TypedData_Make_Struct(klass, struct strptime_object,
				&strptime_data_type, tobj);

    return obj;
}

static struct strptime_object *
get_strptimeval(VALUE obj)
{
    struct strptime_object *tobj;
    TypedData_Get_Struct(obj, struct strptime_object, &strptime_data_type,
			 tobj);
    if (!STRPTIME_INIT_P(tobj)) {
	rb_raise(rb_eTypeError, "uninitialized %" PRIsVALUE, rb_obj_class(obj));
    }
    return tobj;
}

static struct strptime_object *
get_new_strptimeval(VALUE obj)
{
    struct strptime_object *tobj;
    TypedData_Get_Struct(obj, struct strptime_object, &strptime_data_type,
			 tobj);
    if (STRPTIME_INIT_P(tobj)) {
	rb_raise(rb_eTypeError, "already initialized %" PRIsVALUE,
		 rb_obj_class(obj));
    }
    return tobj;
}

/*
 * @overload new(format)
 *   @param format [String] strptime(3) style format string.
 *
 * returns parser object
 */
static VALUE
strptime_init(VALUE self, VALUE fmt)
{
    struct strptime_object *tobj;
    void **isns;
    StringValueCStr(fmt);
    TypedData_Get_Struct(self, struct strptime_object, &strptime_data_type,
			 tobj);
    isns = strptime_compile(RSTRING_PTR(fmt), RSTRING_END(fmt));
    tobj->isns = isns;
    tobj->fmt = rb_str_new_frozen(fmt);
    return self;
}

/* @api private
 * For Ruby VM internal.
 */
static VALUE
strptime_init_copy(VALUE copy, VALUE self)
{
    struct strptime_object *tobj, *tcopy;

    if (!OBJ_INIT_COPY(copy, self)) return copy;
    GetStrptimeval(self, tobj);
    GetNewStrptimeval(copy, tcopy);
    MEMCPY(tcopy, tobj, struct strptime_object, 1);

    return copy;
}

/*
 * @overload exec(str)
 *   @param str [String] string to parse
 * @return [Time] the time object given string means
 *
 * Parse given string, and return Time object
 *
 */
static VALUE
strptime_exec(VALUE self, VALUE str)
{
    struct strptime_object *tobj;
    int r, gmtoff = INT_MAX;
    struct timespec ts;
    StringValue(str);
    GetStrptimeval(self, tobj);

    r = strptime_exec0(tobj->isns, RSTRING_PTR(str),
		       RSTRING_END(str), &ts, &gmtoff);
    if (r) rb_raise(rb_eArgError, "string doesn't match");
    return rb_time_timespec_new(&ts, gmtoff);
}

/*
 * @overload execi(str)
 *   @param str [String] string to parse
 * @return [Integer] the Unix epoch given string means
 *
 * Parse given string, and return epoch as integer
 */
static VALUE
strptime_execi(VALUE self, VALUE str)
{
    struct strptime_object *tobj;
    struct timespec ts;
    int r, gmtoff = INT_MAX;
    StringValue(str);
    GetStrptimeval(self, tobj);

    r = strptime_exec0(tobj->isns, RSTRING_PTR(str),
		       RSTRING_END(str), &ts, &gmtoff);
    if (r) rb_raise(rb_eArgError, "string doesn't match");
    return TIMET2NUM(ts.tv_sec);
}

/*
 * @overload source
 * @return [String] source format string
 */
static VALUE
strptime_source(VALUE self)
{
    struct strptime_object *tobj;
    GetStrptimeval(self, tobj);

    return tobj->fmt;
}

/*
 * Document-class: Strptime
 *
 * Strptime is a faster way to parse time strings like strptime(3).
 *
 * @example
 *    parser = Strptime.new('%Y-%m-%dT%H:%M:%S%z')
 *    parser.source #=> "%Y-%m-%dT%H:%M:%S%z"
 *    parser.exec('2015-12-25T12:34:56+09') #=> 2015-12-25 12:34:56 +09:00
 *    parser.execi('2015-12-25T12:34:56+09') #=> 1451014496
 */
void
Init_strptime(void)
{
    rb_cStrptime = rb_define_class("Strptime", rb_cObject);
    rb_define_alloc_func(rb_cStrptime, strptime_s_alloc);
    rb_define_method(rb_cStrptime, "initialize", strptime_init, 1);
    rb_define_method(rb_cStrptime, "initialize_copy", strptime_init_copy, 1);
    rb_define_method(rb_cStrptime, "exec", strptime_exec, 1);
    rb_define_method(rb_cStrptime, "execi", strptime_execi, 1);
    rb_define_method(rb_cStrptime, "source", strptime_source, 0);
}
