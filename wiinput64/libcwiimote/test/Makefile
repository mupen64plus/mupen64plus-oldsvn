# Makefile for libcwiimote examples

CC=gcc
CFLAGS=-Wall -D_ENABLE_TILT -D_ENABLE_FORCE -I../src
LDFLAGS=-L.. -lcwiimote -lm -lbluetooth

all: test1 test2 test3 test4

test1: ../libcwiimote.a test1.o
	$(CC) -o $@ $^ $(LDFLAGS)

test2: ../libcwiimote.a test2.o
	$(CC) -o $@ $^ $(LDFLAGS)

test3: ../libcwiimote.a test3.o
	$(CC) -o $@ $^ $(LDFLAGS)

test4: ../libcwiimote.a test4.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -rf *.o test1 test2 test3 test4
