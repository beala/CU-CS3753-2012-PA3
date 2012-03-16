CC = gcc
CFLAGS = -c -g -Wall -Wextra
LFLAGS = -Wall -Wextra

.PHONY: all clean

all: cpu-bound

pi: pi.o
	$(CC) $(LFLAGS) $^ -o $@ -lm

cpu-bound: cpu-bound.o
	$(CC) $(LFLAGS) $^ -o $@ -lm

pi.o: pi.c
	$(CC) $(CFLAGS) $<

cpu-bound.o: cpu-bound.c
	$(CC) $(CFLAGS) $<

clean:
	rm -f pi cpu-bound
	rm -f *.o
	rm -f *~
	rm -f handout/*~
	rm -f handout/*.log
	rm -f handout/*.aux
