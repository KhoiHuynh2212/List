CC      = gcc
CFLAGS  = -Wall -Wextra
DEBUG   = -g
RELEASE = -O2

ASAN_FLAGS = -fsanitize=address,undefined -g

ASAN_RUN   = ASAN_OPTIONS=detect_leaks=0 

listi: 
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -I intrusive_list/ -o intrusive_list/test intrusive_list/test-api.c 
	@echo "--- RUNNING INTRUSIVE LIST ---" 
	./intrusive_list/test

listi2: 
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -I intrusive_list/ -o intrusive_list/test2 intrusive_list/test-em.c 
	@echo "--- RUNNING INTRUSIVE LIST ---" 
	./intrusive_list/test2	

bench_i:
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -I intrusive_list/ -o intrusive_list/bench intrusive_list/bench_intrusive.c
	@echo "--- RUNNING INTRUSIVE LIST ---" 
	./intrusive_list/bench
listvp: 
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -I void_list/ -o main.out main.c void_list/list_vp.c
	@echo "--- RUNNING MOTHER SANDBOX--"
	@$(ASAN_RUN) ./main.out				

listvp_leak:
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -I void_list/ -o main.out main.c void_list/list_vp.c
	@echo "--- RUNNING MOTHER SANDBOX WITH ASAN LEAK CHECK --"
	./main.out	

listvp_valgrind:
	$(CC) $(CFLAGS) $(DEBUG) -I void_list/ -o main.out main.c void_list/list_vp.c
	@echo "--- RUNNING MOTHER SANDBOX WITH VALGRIND ---"
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./main.out

test_macro : 
	$(CC) $(CFLAGS) $(DEBUG) -I list/ -o list/test list/test_macro.c 
	./list/test

valgrind_macro: 
	$(CC) $(CFLAGS) $(DEBUG) -I list/ -o list/test list/test_macro.c
	valgrind --leak-check=full --track-origins=yes ./list/test

bench_macro:
	$(CC) $(CFLAGS) $(RELEASE) -I list/ -o list/bench_macro list/bench_macro.c
	./list/bench_macro

test_tagged:
	$(CC) $(CFLAGS) $(DEBUG) -I tagged_list/ -o tagged_list/test tagged_list/test.c tagged_list/tagged_list.c
	./tagged_list/test

valgrind_tagged:
	$(CC) $(CFLAGS) $(DEBUG) -I tagged_list/ -o tagged_list/test tagged_list/test.c tagged_list/tagged_list.c
	valgrind --leak-check=full --track-origins=yes ./tagged_list/test

bench_tagged:
	$(CC) $(CFLAGS) $(RELEASE) -I tagged_list/ -o tagged_list/tagged_bench tagged_list/tagged_bench.c tagged_list/tagged_list.c
	./tagged_list/tagged_bench

test_vp:
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -I void_list/ -o void_list/test void_list/test_vp.c void_list/list_vp.c
	./void_list/test

valgrind_vp:
	$(CC) $(CFLAGS) $(DEBUG) -I void_list/ -o void_list/test void_list/test_vp.c void_list/list_vp.c
	valgrind --leak-check=full --track-origins=yes ./void_list/test

bench_vp:
	$(CC) $(CFLAGS) $(RELEASE) -I void_list/ -o void_list/bench void_list/bench_vp.c void_list/list_vp.c
	./void_list/bench

clean:
	rm -f tagged_list/test tagged_list/tagged_bench tagged_list/test
	 void_list/test void_list/bench_vp list/bench_macro list/test
	 