#include "strptime.h"

VALUE rb_cStrptime;

#define GetStrptimeval(obj, tobj) ((tobj) = get_strptimeval(obj))
#define GetNewStrptimeval(obj, tobj) ((tobj) = get_new_strptimeval(obj))
#define STRPTIME_INIT_P(tobj) ((tobj)->isns)

#define LIKELY(x)   (__builtin_expect((x), 1))
#define UNLIKELY(x) (__builtin_expect((x), 0))

#define REG_PC  (pc)
#define GET_PC()           REG_PC
#define SET_PC(x)          (REG_PC = (x))
#define GET_CURRENT_INSN() (*GET_PC())
#define GET_OPERAND(n)     (GET_PC()[(n)])
#define ADD_PC(n)          (SET_PC(REG_PC + (n)))

#define JUMP(dst)          (REG_PC += (dst))


#define LABEL(x)  INSN_LABEL_##x
#define ELABEL(x) INSN_ELABEL_##x
#define LABEL_PTR(x) &&LABEL(x)

#define INSN_ENTRY(insn) \
  LABEL(insn): \

#define TC_DISPATCH(insn) \
  goto *(void const *)GET_CURRENT_INSN(); \
  ;
#define END_INSN(insn)      \
  TC_DISPATCH(insn);

#define INSN_DISPATCH()     \
  TC_DISPATCH(__START__)    \
  {

#define END_INSNS_DISPATCH()    \
      rb_bug("unknown insn: %p", GET_CURRENT_INSN());   \
  }   /* end of while loop */   \

#define NEXT_INSN() TC_DISPATCH(__NEXT_INSN__)

static const char *day_names[] = {
    "Sunday", "Monday", "Tuesday", "Wednesday",
    "Thursday", "Friday", "Saturday",
    "Sun", "Mon", "Tue", "Wed",
    "Thu", "Fri", "Sat"
};

static const char *month_names[] = {
    "January", "February", "March", "April",
    "May", "June", "July", "August", "September",
    "October", "November", "December",
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

#define sizeof_array(o) (sizeof o / sizeof o[0])

#define f_negate(x) rb_funcall(x, rb_intern("-@"), 0)
#define f_add(x,y) rb_funcall(x, '+', 1, y)
#define f_sub(x,y) rb_funcall(x, '-', 1, y)
#define f_mul(x,y) rb_funcall(x, '*', 1, y)
#define f_div(x,y) rb_funcall(x, '/', 1, y)
#define f_idiv(x,y) rb_funcall(x, rb_intern("div"), 1, y)
#define f_mod(x,y) rb_funcall(x, '%', 1, y)
#define f_expt(x,y) rb_funcall(x, rb_intern("**"), 1, y)

#define f_lt_p(x,y) rb_funcall(x, '<', 1, y)
#define f_gt_p(x,y) rb_funcall(x, '>', 1, y)
#define f_le_p(x,y) rb_funcall(x, rb_intern("<="), 1, y)
#define f_ge_p(x,y) rb_funcall(x, rb_intern(">="), 1, y)

#define f_match(r,s) rb_funcall(r, rb_intern("match"), 1, s)
#define f_aref(o,i) rb_funcall(o, rb_intern("[]"), 1, i)
#define f_end(o,i) rb_funcall(o, rb_intern("end"), 1, i)

#define issign(c) ((c) == '-' || (c) == '+')

/* imply NUL-terminated string */
static long
read_digits(const char *s, VALUE *n, size_t width)
{
    const char *s0 = s;
    const char *se = s + width;
    int64_t r = 0;

    for ( ; s < se && '0' <= *s && *s <= '9'; s++) {
	r *= 10;
	r += *s - '0';
    }
    *n = LL2NUM(r);
    return (long)(s-s0);
}

#define fail() \
{ \
    return Qnil; \
}

#define READ_DIGITS(n,w) \
{ \
    size_t l; \
    l = read_digits(&str[si], &n, w); \
    if (l == 0) \
	fail();	\
    si += l; \
}

#define READ_DIGITS_MAX(n) READ_DIGITS(n, LONG_MAX)

static int
valid_range_p(VALUE v, int a, int b)
{
    if (FIXNUM_P(v)) {
	int vi = FIX2INT(v);
	return !(vi < a || vi > b);
    }
    return !(f_lt_p(v, INT2NUM(a)) || f_gt_p(v, INT2NUM(b)));
}
VALUE
strptime_exec0(void **pc, const char *fmt, size_t flen,
	const char *str, size_t slen)
{
    size_t si = 0;
    if (UNLIKELY(pc == NULL)) {
	static const void *const insns_address_table[] = {
LABEL_PTR(A), LABEL_PTR(B), LABEL_PTR(C), LABEL_PTR(D), LABEL_PTR(E),
LABEL_PTR(F), LABEL_PTR(G), LABEL_PTR(H), LABEL_PTR(I), NULL,
NULL, LABEL_PTR(L), LABEL_PTR(M), LABEL_PTR(N), LABEL_PTR(O),
LABEL_PTR(P), LABEL_PTR(Q), LABEL_PTR(R), LABEL_PTR(S), LABEL_PTR(T),
LABEL_PTR(U), LABEL_PTR(V), LABEL_PTR(W), LABEL_PTR(X), LABEL_PTR(Y),
LABEL_PTR(Z), LABEL_PTR(_25), LABEL_PTR(_2b), LABEL_PTR(_3a), NULL, LABEL_PTR(_5f), LABEL_PTR(_60),
LABEL_PTR(a), LABEL_PTR(B), LABEL_PTR(c), LABEL_PTR(d), LABEL_PTR(d),
NULL, LABEL_PTR(g), LABEL_PTR(B), NULL, LABEL_PTR(j),
NULL, LABEL_PTR(l), LABEL_PTR(m), LABEL_PTR(n), NULL,
LABEL_PTR(p), NULL, LABEL_PTR(r), LABEL_PTR(s), LABEL_PTR(t),
LABEL_PTR(u), LABEL_PTR(v), LABEL_PTR(w), LABEL_PTR(x), LABEL_PTR(y), LABEL_PTR(z),
	};
	return (VALUE)insns_address_table;
    }
    VALUE year=INT2FIX(1970), mon=INT2FIX(1), mday=INT2FIX(1), hour=INT2FIX(0), min=INT2FIX(0), sec=INT2FIX(0);

  first:
    INSN_DISPATCH();
    INSN_ENTRY(A){ END_INSN(A)}
    INSN_ENTRY(B){
	int i;

	//fprintf(stderr,"%d: %s\n",__LINE__,str+si);
	for (i = 0; i < (int)sizeof_array(month_names); i++) {
	    size_t l = strlen(month_names[i]);
	    if (strncasecmp(month_names[i], &str[si], l) == 0) {
		si += l;
		mon = INT2FIX((i % 12) + 1);
		ADD_PC(1);
		END_INSN(B)
	    }
	}
	fail();
	}
    INSN_ENTRY(C){ END_INSN(C)}
    INSN_ENTRY(D){ END_INSN(D)}
    INSN_ENTRY(E){ END_INSN(E)}
    INSN_ENTRY(F){ END_INSN(F)}
    INSN_ENTRY(G){ END_INSN(G)}
    INSN_ENTRY(H){
	VALUE n;

	if (str[si] == ' ') {
	    si++;
	    READ_DIGITS(n, 1);
	} else {
	    READ_DIGITS(n, 2);
	}
	if (!valid_range_p(n, 0, 23))
	    fail();
	hour = n;
	pc++;
	END_INSN(H)}
    INSN_ENTRY(I){ END_INSN(I)}
    INSN_ENTRY(L){ END_INSN(L)}
    INSN_ENTRY(M){
	VALUE n;

	READ_DIGITS(n, 2);
	if (!valid_range_p(n, 0, 59))
	    fail();
	min = n;
	pc++;
	END_INSN(M)} 
    INSN_ENTRY(N){ END_INSN(N)}
    INSN_ENTRY(O){ END_INSN(O)}
    INSN_ENTRY(P){ END_INSN(P)}
    INSN_ENTRY(Q){ END_INSN(Q)}
    INSN_ENTRY(R){ END_INSN(R)}
    INSN_ENTRY(S){
	VALUE n;

	READ_DIGITS(n, 2);
	if (!valid_range_p(n, 0, 60))
	    fail();
	sec = n;
	pc++;
	END_INSN(S)}
    INSN_ENTRY(T){ END_INSN(T)}
    INSN_ENTRY(U){ END_INSN(U)}
    INSN_ENTRY(V){ END_INSN(V)}
    INSN_ENTRY(W){ END_INSN(W)}
    INSN_ENTRY(X){ END_INSN(X)}
    INSN_ENTRY(Y){
       VALUE n;
       int sign = 1;

       if (issign(str[si])) {
	   if (str[si] == '-')
	       sign = -1;
	   si++;
       }
       READ_DIGITS(n, 4);
       if (sign == -1)
	   n = f_negate(n);
       year = n;
       ADD_PC(1);
    END_INSN(Y)}
    INSN_ENTRY(Z){ END_INSN(Z)}
    INSN_ENTRY(a){ END_INSN(a)}
    INSN_ENTRY(c){ END_INSN(c)}
    INSN_ENTRY(d){
	VALUE n;

	if (str[si] == ' ') {
	    si++;
	    READ_DIGITS(n, 1);
	} else {
	    READ_DIGITS(n, 2);
	}
	if (!valid_range_p(n, 1, 31))
	    fail();
	mday = n;
	pc++;
    END_INSN(d)}
    INSN_ENTRY(g){ END_INSN(g)}
    INSN_ENTRY(j){ END_INSN(j)}
    INSN_ENTRY(l){ END_INSN(l)}
    INSN_ENTRY(m){
	VALUE n;

	READ_DIGITS(n, 2);
	if (!valid_range_p(n, 1, 12))
	    fail();
	mon = n;
	pc++;
    END_INSN(m)}
    INSN_ENTRY(n){ END_INSN(n)}
    INSN_ENTRY(p){ END_INSN(p)}
    INSN_ENTRY(r){ END_INSN(r)}
    INSN_ENTRY(s){ END_INSN(s)}
    INSN_ENTRY(t){ END_INSN(t)}
    INSN_ENTRY(u){ END_INSN(u)}
    INSN_ENTRY(v){ END_INSN(v)}
    INSN_ENTRY(w){ END_INSN(w)}
    INSN_ENTRY(x){ END_INSN(x)}
    INSN_ENTRY(y){ END_INSN(y)}
    INSN_ENTRY(z){ END_INSN(z)}
    INSN_ENTRY(_25){ END_INSN(_25)}
    INSN_ENTRY(_2b){ END_INSN(_2b)}
    INSN_ENTRY(_3a){ END_INSN(_3a)}
    INSN_ENTRY(_60){
	size_t v = (size_t)GET_OPERAND(1);
	size_t fi = v & 0xFFFF;
	size_t cnt = v >> 16;
	if (memcmp(str+si, fmt+fi, cnt)) return Qnil;
	pc += 2;
	si += cnt;
    END_INSN(_60)}
    INSN_ENTRY(_5f){
	// int argc = 6;
	// VALUE args[] = {year, mon, mday, hour, min, sec};
	struct tm tm = {FIX2INT(sec), FIX2INT(min), FIX2INT(hour), FIX2INT(mday), FIX2INT(mon)-1, FIX2INT(year)-1900, 0, 0, 0, 0, ""};
	time_t t;
	static time_t ct;
	static struct tm cache;
	if (ct && cache.tm_year == tm.tm_year && cache.tm_mon == tm.tm_mon &&
		cache.tm_mday == tm.tm_mday) {
	    t = ct + (tm.tm_hour-cache.tm_hour)*3600 + (tm.tm_min-cache.tm_min)*60 +
		(tm.tm_sec-cache.tm_sec);
	}
	else {
	    ct = t = timegm(&tm);
	    memcpy((void *)&cache, &tm, sizeof(struct tm));
	}
	return rb_time_num_new(LONG2NUM(t), INT2FIX(0));
	// return rb_funcallv(rb_cTime, rb_intern("utc"), argc, args);
	END_INSN(_5f)}
    END_INSNS_DISPATCH();

    /* unreachable */
    rb_bug("vm_eval: unreachable");
    goto first;
}

void **
strptime_compile(const char *fmt, size_t flen)
{
    size_t fi = 0;
    char c;
    void **isns0 = ALLOC_N(void*, flen+2);
    void **isns = isns0;
    void **insns_address_table = (void **)strptime_exec0(NULL, NULL, 0, NULL, 0);
    void *tmp;

    while (fi < flen) {
	switch (fmt[fi]) {
	  case '%':
	    fi++;
	    c = fmt[fi];
	    if ('A' <= c && c <= 'z') {
		tmp = insns_address_table[c-'A'];
		if (tmp) {
		    *isns++ = tmp;
		    fi++;
		    continue;
		}
	    }
	    rb_raise(rb_eArgError, "invalid format");
	    break;
	  case ' ':
	  case '\t':
	  case '\n':
	  case '\v':
	  case '\f':
	  case '\r':
	    *isns++ = insns_address_table['n'-'A'];
	    fi++;
	    break;
	  default:
	    {
		const char *p0 = fmt+fi, *p = p0, *pe = fmt+flen;
		size_t v = fi;
		while (p < pe && *p != '%' && !ISSPACE(*p)) p++;
		v += (p - p0) << 16;
		fi += p - p0;
		*isns++ = insns_address_table['`'-'A'];
		*isns++ = (void *)v;
	    }
	    break;
	}
    }
    *isns++ = insns_address_table['_'-'A'];
    REALLOC_N(isns0, void*, isns-isns0);
    return isns0;
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

static size_t
strptime_memsize(const void *tobj)
{
    return sizeof(struct strptime_object);
}

static const rb_data_type_t strptime_data_type = {
    "strptime",
    {strptime_mark, RUBY_TYPED_DEFAULT_FREE, strptime_memsize,},
    0, 0, RUBY_TYPED_FREE_IMMEDIATELY
};

static VALUE
strptime_s_alloc(VALUE klass)
{
    VALUE obj;
    struct strptime_object *tobj;

    obj = TypedData_Make_Struct(klass, struct strptime_object, &strptime_data_type, tobj);

    return obj;
}

static struct strptime_object *
get_strptimeval(VALUE obj)
{
    struct strptime_object *tobj;
    TypedData_Get_Struct(obj, struct strptime_object, &strptime_data_type, tobj);
    if (!STRPTIME_INIT_P(tobj)) {
	rb_raise(rb_eTypeError, "uninitialized %"PRIsVALUE, rb_obj_class(obj));
    }
    return tobj;
}

static struct strptime_object *
get_new_strptimeval(VALUE obj)
{
    struct strptime_object *tobj;
    TypedData_Get_Struct(obj, struct strptime_object, &strptime_data_type, tobj);
    if (STRPTIME_INIT_P(tobj)) {
	rb_raise(rb_eTypeError, "already initialized %"PRIsVALUE, rb_obj_class(obj));
    }
    return tobj;
}

static VALUE
strptime_init(VALUE self, VALUE fmt)
{
    struct strptime_object *tobj;
    void **isns;
    TypedData_Get_Struct(self, struct strptime_object, &strptime_data_type, tobj);
    isns = strptime_compile(RSTRING_PTR(fmt), RSTRING_LEN(fmt));
    tobj->isns = isns;
    tobj->fmt = rb_str_new_frozen(fmt);
    return self;
}

/* :nodoc: */
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

static VALUE
strptime_exec(VALUE self, VALUE str)
{
    struct strptime_object *tobj;
    VALUE v;
    GetStrptimeval(self, tobj);

    v = strptime_exec0(tobj->isns, RSTRING_PTR(tobj->fmt), RSTRING_LEN(tobj->fmt),
	    RSTRING_PTR(str), RSTRING_LEN(str));
    if (NIL_P(v)) rb_raise(rb_eArgError, "invalid date");
    return v;
}

static VALUE
strptime_source(VALUE self)
{
    struct strptime_object *tobj;
    GetStrptimeval(self, tobj);

    return tobj->fmt;
}

void
Init_strptime(void)
{
    rb_cStrptime = rb_define_class("Strptime", rb_cObject);
    rb_define_alloc_func(rb_cStrptime, strptime_s_alloc);
    rb_define_method(rb_cStrptime, "initialize", strptime_init, 1);
    rb_define_method(rb_cStrptime, "initialize_copy", strptime_init_copy, 1);
    rb_define_method(rb_cStrptime, "exec", strptime_exec, 1);
    rb_define_method(rb_cStrptime, "source", strptime_source, 0);
}
