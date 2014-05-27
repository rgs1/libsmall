CC = gcc
CFLAGS = -Wall

SOURCES = \
	queue.c \
	dict.c \
	list.c \
	util.c \
	slab.c \
	pool.c \
	$(NULL)

TESTS = \
	queue-test \
	dict-test \
	list-test \
	pool-test \
	$(NULL)

OBJECTS = \
	$(SOURCES:.c=.o) \
	$(TESTS:=.o) \
	$(NULL)


queue.o: queue.c queue.h
	$(CC) $(CFLAGS) -c $< -o $@

dict.o: dict.c dict.h
	$(CC) $(CFLAGS) -c $< -o $@

list.o: list.c list.h
	$(CC) $(CFLAGS) -c $< -o $@

util.o: util.c util.h
	$(CC) $(CFLAGS) -c $< -o $@

slab.o: slab.c slab.h
	$(CC) $(CFLAGS) -c $< -o $@

pool.o: pool.c pool.h
	$(CC) $(CFLAGS) -c $< -o $@

queue-test.o: queue.c queue.h
	$(CC) $(CFLAGS) -DRUN_TESTS -c $< -o $@

queue-test: queue-test.o util.o
	$(CC) $(CFLAGS) -DRUN_TESTS -lpthread $^ -o $@

dict-test.o: dict.c dict.h
	$(CC) $(CFLAGS) -DRUN_TESTS -c $< -o $@

dict-test: dict-test.o util.o list.o pool.o slab.o
	$(CC) $(CFLAGS) -DRUN_TESTS -lpthread $^ -o $@

list-test.o: list.c list.h
	$(CC) $(CFLAGS) -DRUN_TESTS -c $< -o $@

list-test: list-test.o util.o pool.o slab.o
	$(CC) $(CFLAGS) -DRUN_TESTS -lpthread $^ -o $@

pool-test.o: pool.c pool.h
	$(CC) $(CFLAGS) -DRUN_TESTS -c $< -o $@

pool-test: pool-test.o util.o slab.o
	$(CC) $(CFLAGS) -DRUN_TESTS -lpthread $^ -o $@

tests: $(TESTS)
	for test in $^; do ./$$test ; done

clean:
	rm -rf $(OBJECTS) $(TESTS)
