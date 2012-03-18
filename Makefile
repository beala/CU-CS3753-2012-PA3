CC = gcc
CFLAGS = -c -g -Wall -Wextra
LFLAGS = -Wall -Wextra

.PHONY: all clean

all: cpu-bound mixed io-bound

pi: pi.o
	$(CC) $(LFLAGS) $^ -o $@ -lm

cpu-bound: cpu-bound.o
	$(CC) $(LFLAGS) $^ -o $@ -lm

io-bound: io-bound.o
	$(CC) $(LFLAGS) $^ -o $@ -lm

mixed: mixed.o
	$(CC) $(LFLAGS) $^ -o $@ -lm

pi.o: pi.c
	$(CC) $(CFLAGS) $<

cpu-bound.o: cpu-bound.c
	$(CC) $(CFLAGS) $<

io-bound.o: io-bound.c
	$(CC) $(CFLAGS) $<

mixed.o: mixed.c
	$(CC) $(CFLAGS) $<

clean:
	rm -f pi cpu-bound mixed io-bound
	rm -f *.o
	rm -f *~
	rm -f handout/*~
	rm -f handout/*.log
	rm -f handout/*.aux
