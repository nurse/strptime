require "mkmf"

have_func 'timegm', 'time.h' or 'timegm(3) is required'

create_makefile("strptime/strptime")
