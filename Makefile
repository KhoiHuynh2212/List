CC      = gcc
CFLAGS  = -Wall -Wextra
DEBUG   = -g
RELEASE = -O2

test_tagged:
	$(CC) $(CFLAGS) $(DEBUG) -I tagged_list/ -o tagged_list/test tagged_list/test.c tagged_list/list.c
	./tagged_list/test

valgrind_tagged:
	$(CC) $(CFLAGS) $(DEBUG) -I tagged_list/ -o tagged_list/test tagged_list/test.c tagged_list/list.c
	valgrind --leak-check=full --track-origins=yes ./tagged_list/test

bench_tagged:
	$(CC) $(CFLAGS) $(RELEASE) -I tagged_list/ -o tagged_list/bench tagged_list/bench.c tagged_list/list.c
	./tagged_list/bench

test_vp:
	$(CC) $(CFLAGS) $(DEBUG) -I void_list/ -o void_list/test void_list/test_vp.c void_list/list_vp.c
	./void_list/test

valgrind_vp:
	$(CC) $(CFLAGS) $(DEBUG) -I void_list/ -o void_list/test void_list/test_vp.c void_list/list_vp.c
	valgrind --leak-check=full --track-origins=yes ./void_list/test

bench_vp:
	$(CC) $(CFLAGS) $(RELEASE) -I void_list/ -o void_list/bench void_list/bench_vp.c void_list/list_vp.c
	./void_list/bench

clean:
	rm -f tagged_list/test tagged_list/bench void_list/test void_list/bench