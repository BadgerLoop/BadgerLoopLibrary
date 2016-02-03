all: can_parse can_parse_single

can_parse: can_parse.c
	gcc -o can_parse can_parse.c -lrt -Wall -Werror

can_parse_single: can_parse_single.c
	gcc -o can_parse_single can_parse_single.c -lrt -Wall -l wiringPi -Werror 