#include "../strptime/strptime.h"
#include "ruby/encoding.h"
#include <time.h>

VALUE rb_cStrftime;

#define GetStrftimeval(obj, tobj) ((tobj) = get_strftimeval(obj))
#define GetNewStrftimeval(obj, tobj) ((tobj) = get_new_strftimeval(obj))
#define StrfTIME_INIT_P(tobj) ((tobj)->isns)

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



static VALUE
strftime_exec0(void **pc, VALUE fmt, struct timespec *tsp, int gmtoff)
{
    VALUE result;
    if (UNLIKELY(tsp == NULL)) {
	static const void *const insns_address_table[] = {
	    LABEL_PTR(A),   LABEL_PTR(B), LABEL_PTR(C),   LABEL_PTR(D),
	    LABEL_PTR(E),   LABEL_PTR(F), LABEL_PTR(G),   LABEL_PTR(H),
	    LABEL_PTR(I),   NULL,	 NULL,		  LABEL_PTR(L),
	    LABEL_PTR(M),   LABEL_PTR(N), LABEL_PTR(O),   LABEL_PTR(P),
	    LABEL_PTR(Q),   LABEL_PTR(R), LABEL_PTR(S),   LABEL_PTR(T),
	    LABEL_PTR(U),   LABEL_PTR(V), LABEL_PTR(W),   LABEL_PTR(X),
	    LABEL_PTR(Y),   LABEL_PTR(Z), LABEL_PTR(_25), LABEL_PTR(_2b),
	    LABEL_PTR(_3a), NULL,	 LABEL_PTR(_5f), LABEL_PTR(_60),
	    LABEL_PTR(a),   LABEL_PTR(B), LABEL_PTR(c),   LABEL_PTR(d),
	    LABEL_PTR(d),   NULL,	 LABEL_PTR(g),   LABEL_PTR(B),
	    NULL,	   LABEL_PTR(j), NULL,		  LABEL_PTR(l),
	    LABEL_PTR(m),   LABEL_PTR(n), NULL,		  LABEL_PTR(p),
	    NULL,	   LABEL_PTR(r), LABEL_PTR(s),   LABEL_PTR(t),
	    LABEL_PTR(u),   LABEL_PTR(v), LABEL_PTR(w),   LABEL_PTR(x),
	    LABEL_PTR(y),   LABEL_PTR(z),
	};
	*pc = (void *)insns_address_table;
	return Qnil;
    }

    result = rb_str_buf_new(0);
    struct tm tm;
    gmtime_r(&tsp->tv_sec, &tm);

    INSN_DISPATCH();
    INSN_ENTRY(A)
    {
	ADD_PC(1);
	END_INSN(A)
    }
    INSN_ENTRY(B)
    {
	ADD_PC(1);
	END_INSN(B)
    }
    INSN_ENTRY(C)
    {
	ADD_PC(1);
	END_INSN(C)
    }
    INSN_ENTRY(D)
    {
	ADD_PC(1);
	END_INSN(D)
    }
    INSN_ENTRY(E)
    {
	ADD_PC(1);
	END_INSN(E)
    }
    INSN_ENTRY(F)
    {
	ADD_PC(1);
	END_INSN(F)
    }
    INSN_ENTRY(G)
    {
	ADD_PC(1);
	END_INSN(G)
    }
    INSN_ENTRY(H)
    {
	char buf[2];
	buf[0] = '0' + (tm.tm_hour / 10);
	buf[1] = '0' + (tm.tm_hour % 10);
	rb_str_buf_cat(result, buf, 2);
	ADD_PC(1);
	END_INSN(H)
    }
    INSN_ENTRY(I)
    {
	ADD_PC(1);
	END_INSN(I)
    }
    INSN_ENTRY(L)
    {
	char buf[3];
	int msec = tsp->tv_nsec / 1000000;
	buf[2] = '0' + (msec % 10);
	msec /= 10;
	buf[1] = '0' + (msec % 10);
	buf[0] = '0' + (msec / 10);
	rb_str_buf_cat(result, buf, 3);
	ADD_PC(1);
	END_INSN(L)
    }
    INSN_ENTRY(M)
    {
	char buf[2];
	buf[0] = '0' + (tm.tm_min / 10);
	buf[1] = '0' + (tm.tm_min % 10);
	rb_str_buf_cat(result, buf, 2);
	ADD_PC(1);
	END_INSN(M)
    }
    INSN_ENTRY(N)
    {
	int len = 9;
	int i;
	char buf[9];
	int base = 1;
	int subsec = tsp->tv_nsec;
	for (i=0; i < 9-len; i++) {
	    base *= 10;
	}
	subsec /= base;
	for (i=0; i < len; i++) {
	    buf[len-i-1] = '0' + subsec % 10;
	    subsec /= 10;
	}
	rb_str_buf_cat(result, buf, len);
	ADD_PC(1);
	END_INSN(N)
    }
    INSN_ENTRY(O)
    {
	ADD_PC(1);
	END_INSN(O)
    }
    INSN_ENTRY(P)
    {
	ADD_PC(1);
	END_INSN(P)
    }
    INSN_ENTRY(Q)
    {
	ADD_PC(1);
	END_INSN(Q)
    }
    INSN_ENTRY(R)
    {
	ADD_PC(1);
	END_INSN(R)
    }
    INSN_ENTRY(S)
    {
	char buf[2];
	buf[0] = '0' + (tm.tm_sec / 10);
	buf[1] = '0' + (tm.tm_sec % 10);
	rb_str_buf_cat(result, buf, 2);
	ADD_PC(1);
	END_INSN(S)
    }
    INSN_ENTRY(T)
    {
	ADD_PC(1);
	END_INSN(T)
    }
    INSN_ENTRY(U)
    {
	ADD_PC(1);
	END_INSN(U)
    }
    INSN_ENTRY(V)
    {
	ADD_PC(1);
	END_INSN(V)
    }
    INSN_ENTRY(W)
    {
	ADD_PC(1);
	END_INSN(W)
    }
    INSN_ENTRY(X)
    {
	ADD_PC(1);
	END_INSN(X)
    }
    INSN_ENTRY(Y)
    {
	// TODO: Y10K
	char buf[4];
	int i, y = tm.tm_year;
	y += y < 69 ? 2000 : 1900;
	for (i = 0; i < 4; i++) {
	    buf[3-i] = '0' + y % 10;
	    y /= 10;
	}
	rb_str_buf_cat(result, buf, 4);
	ADD_PC(1);
	END_INSN(Y)
    }
    INSN_ENTRY(Z)
    {
	ADD_PC(1);
	END_INSN(Z)
    }
    INSN_ENTRY(a)
    {
	ADD_PC(1);
	END_INSN(a)
    }
    INSN_ENTRY(c)
    {
	ADD_PC(1);
	END_INSN(c)
    }
    INSN_ENTRY(d)
    {
	char buf[2];
	buf[0] = '0' + (tm.tm_mday / 10);
	buf[1] = '0' + (tm.tm_mday % 10);
	rb_str_buf_cat(result, buf, 2);
	ADD_PC(1);
	END_INSN(d)
    }
    INSN_ENTRY(g)
    {
	ADD_PC(1);
	END_INSN(g)
    }
    INSN_ENTRY(j)
    {
	ADD_PC(1);
	END_INSN(j)
    }
    INSN_ENTRY(l)
    {
	ADD_PC(1);
	END_INSN(l)
    }
    INSN_ENTRY(m)
    {
	char buf[2];
	int mon = tm.tm_mon + 1;
	buf[0] = '0' + (mon / 10);
	buf[1] = '0' + (mon % 10);
	rb_str_buf_cat(result, buf, 2);
	ADD_PC(1);
	END_INSN(m)
    }
    INSN_ENTRY(n)
    {
	rb_str_buf_cat(result, "\n", 1);
	ADD_PC(1);
	END_INSN(n)
    }
    INSN_ENTRY(p)
    {
	ADD_PC(1);
	END_INSN(p)
    }
    INSN_ENTRY(r)
    {
	ADD_PC(1);
	END_INSN(r)
    }
    INSN_ENTRY(s)
    {
	ADD_PC(1);
	END_INSN(s)
    }
    INSN_ENTRY(t)
    {
	ADD_PC(1);
	END_INSN(t)
    }
    INSN_ENTRY(u)
    {
	ADD_PC(1);
	END_INSN(u)
    }
    INSN_ENTRY(v)
    {
	ADD_PC(1);
	END_INSN(v)
    }
    INSN_ENTRY(w)
    {
	ADD_PC(1);
	END_INSN(w)
    }
    INSN_ENTRY(x)
    {
	ADD_PC(1);
	END_INSN(x)
    }
    INSN_ENTRY(y)
    {
	char buf[2];
	int y = tm.tm_year % 100;
	buf[0] = '0' + (y / 10);
	buf[1] = '0' + (y % 10);
	rb_str_buf_cat(result, buf, 2);
	ADD_PC(1);
	END_INSN(y)
    }
    INSN_ENTRY(z)
    {
	// TODO: other than UTC
	rb_str_buf_cat(result, "Z", 1);
	ADD_PC(1);
	END_INSN(z)
    }
    INSN_ENTRY(_25)
    {
	ADD_PC(1);
	END_INSN(_25)
    }
    INSN_ENTRY(_2b)
    {
	ADD_PC(1);
	END_INSN(_2b)
    }
    INSN_ENTRY(_3a)
    {
	ADD_PC(1);
	END_INSN(_3a)
    }
    INSN_ENTRY(_60)
    {
	size_t v = (size_t)GET_OPERAND(1);
	size_t off = v & 0xFFFF;
	size_t len = v >> 16;
	rb_str_buf_cat(result, RSTRING_PTR(fmt) + off, len);
	pc += 2;
	END_INSN(_60)
    }
    INSN_ENTRY(_5f)
    {
	rb_enc_set_index(result, rb_enc_get_index(fmt));
	return result;
	END_INSN(_5f)
    }
    END_INSNS_DISPATCH();

    /* unreachable */
    rb_bug("strftime_exec0: unreachable");
    UNREACHABLE;
}

static void **
strftime_compile(const char *fmt, size_t flen)
{
    size_t fi = 0;
    char c;
    void **isns0, **isns;
    void **insns_address_table;
    void *tmp;
    strftime_exec0((void **)&insns_address_table, Qnil, NULL, 0);

    if (flen > 65535) {
	rb_raise(rb_eArgError, "too long format string (>65335)");
    }
    isns0 = ALLOC_N(void *, flen + 2);
    isns = isns0;

    while (fi < flen) {
	switch (fmt[fi]) {
	case '%':
	    fi++;
	    c = fmt[fi];
	    switch (c) {
	    case 'B':
	    case 'H':
	    case 'L':
	    case 'M':
	    case 'N':
	    case 'S':
	    case 'Y':
	    case 'b':
	    case 'd':
	    case 'e':
	    case 'h':
	    case 'm':
	    case 'n':
	    case 'y':
	    case 'z':
		tmp = insns_address_table[c - 'A'];
		if (tmp) {
		    *isns++ = tmp;
		    fi++;
		    continue;
		}
	    default:
		xfree(isns0);
		rb_raise(rb_eArgError, "invalid format");
		break;
	    }
	default: {
	    const char *p0 = fmt + fi, *p = p0, *pe = fmt + flen;
	    size_t v = fi;
	    while (p < pe && *p != '%')
		p++;
	    v += (p - p0) << 16;
	    fi += p - p0;
	    *isns++ = insns_address_table['`' - 'A'];
	    *isns++ = (void *)v;
	} break;
	}
    }
    *isns++ = insns_address_table['_' - 'A'];
    REALLOC_N(isns0, void *, isns - isns0);
    return isns0;
}

struct strftime_object {
    void **isns;
    VALUE fmt;
};

static void
strftime_mark(void *ptr)
{
    struct strftime_object *tobj = ptr;
    rb_gc_mark(tobj->fmt);
}

static void
strftime_free(void *ptr)
{
    struct strftime_object *tobj = ptr;
    if (tobj->isns) ruby_xfree(tobj->isns);
}

static size_t
strftime_memsize(const void *tobj)
{
    return sizeof(struct strftime_object);
}

static const rb_data_type_t strftime_data_type = {
    "strftime",
    {
	strftime_mark, strftime_free, strftime_memsize,
    },
#ifdef RUBY_TYPED_FREE_IMMEDIATELY
    0,
    0,
    RUBY_TYPED_FREE_IMMEDIATELY
#endif
};

static VALUE
strftime_s_alloc(VALUE klass)
{
    VALUE obj;
    struct strftime_object *tobj;

    obj = TypedData_Make_Struct(klass, struct strftime_object,
				&strftime_data_type, tobj);

    return obj;
}

static struct strftime_object *
get_strftimeval(VALUE obj)
{
    struct strftime_object *tobj;
    TypedData_Get_Struct(obj, struct strftime_object, &strftime_data_type,
			 tobj);
    if (!StrfTIME_INIT_P(tobj)) {
	rb_raise(rb_eTypeError, "uninitialized %" PRIsVALUE, rb_obj_class(obj));
    }
    return tobj;
}

static struct strftime_object *
get_new_strftimeval(VALUE obj)
{
    struct strftime_object *tobj;
    TypedData_Get_Struct(obj, struct strftime_object, &strftime_data_type,
			 tobj);
    if (StrfTIME_INIT_P(tobj)) {
	rb_raise(rb_eTypeError, "already initialized %" PRIsVALUE,
		 rb_obj_class(obj));
    }
    return tobj;
}

/*
 * @overload new(format)
 *   @param format [String] strftime(3) style format string.
 *
 * returns generator object
 */
static VALUE
strftime_init(VALUE self, VALUE fmt)
{
    struct strftime_object *tobj;
    void **isns;
    StringValueCStr(fmt);
    TypedData_Get_Struct(self, struct strftime_object, &strftime_data_type,
			 tobj);
    isns = strftime_compile(RSTRING_PTR(fmt), RSTRING_LEN(fmt));
    tobj->isns = isns;
    tobj->fmt = rb_str_new_frozen(fmt);
    return self;
}

/* @api private
 * For Ruby VM internal.
 */
static VALUE
strftime_init_copy(VALUE copy, VALUE self)
{
    struct strftime_object *tobj, *tcopy;

    if (!OBJ_INIT_COPY(copy, self)) return copy;
    GetStrftimeval(self, tobj);
    GetNewStrftimeval(copy, tcopy);
    MEMCPY(tcopy, tobj, struct strftime_object, 1);

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
strftime_exec(VALUE self, VALUE time)
{
    struct strftime_object *tobj;
    int r, gmtoff = INT_MAX;
    struct timespec ts = rb_time_timespec(time);
    GetStrftimeval(self, tobj);

    return strftime_exec0(tobj->isns, tobj->fmt, &ts, 0);
}

/*
 * @overload source
 * @return [String] source format string
 */
static VALUE
strftime_source(VALUE self)
{
    struct strftime_object *tobj;
    GetStrftimeval(self, tobj);

    return tobj->fmt;
}


/*
 * Document-class: Strftime
 *
 * Strftime is a faster way to format time string like strftime(3).
 *
 * @example
 *    generator = Strftime.new('%Y-%m-%dT%H:%M:%S%z')
 *    generator.source #=> "%Y-%m-%dT%H:%M:%S%z"
 *    generator.exec(Time.now) #=> 2017-12-25T12:34:56+09:00
 */
void
Init_strftime(void)
{
    rb_cStrftime = rb_define_class("Strftime", rb_cObject);
    rb_define_alloc_func(rb_cStrftime, strftime_s_alloc);
    rb_define_method(rb_cStrftime, "initialize", strftime_init, 1);
    rb_define_method(rb_cStrftime, "initialize_copy", strftime_init_copy, 1);
    rb_define_method(rb_cStrftime, "exec", strftime_exec, 1);
    rb_define_method(rb_cStrftime, "source", strftime_source, 0);
}
