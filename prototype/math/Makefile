# =============================================================================
#   Copyright © 2020 Skynet Consulting Ltd.
#
#   This file is the property of Skynet Consulting Ltd. and shall not be reproduced,
#   copied, or used as the basis for the manufacture or sale of equipment without
#   the express written permission of Skynet Consulting Ltd.
#   =============================================================================
#   This system was written and tested against Debian 9/10 Linux
#   =============================================================================
all:	combination combination2 endian log math_test \
	sort 

combination: combination.o
	gcc combination.o -o combination

combination.o: combination.c
	gcc -c combination.c -o combination.o

combination2: combination2.o
	gcc combination2.o -o combination2

combination2.o: combination2.c
	gcc -c combination2.c -o combination2.o

endian: endian.o
	gcc endian.o -o endian

endian.o: endian.c
	gcc -c endian.c -o endian.o

log: log.o
	gcc log.o -o log

log.o: log.c
	gcc -c log.c -o log.o

math_test: math_test.o
	gcc math_test.o -o math_test

math_test.o: math_test.c
	gcc -c math_test.c -o math_test.o

sort: sort.o
	gcc sort.o -o sort

sort.o: sort.c
	gcc -c sort.c -o sort.o

clean:
	rm -f combination.o combination combination2.o combination2 \
		endian.o endian log.o log math_test.o math_test \
		sort.o sort 