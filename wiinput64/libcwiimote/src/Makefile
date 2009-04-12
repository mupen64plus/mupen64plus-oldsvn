# Makefile for libcwiimote

CC=gcc
AR=ar
RANLIB=ranlib
CFLAGS=-Wall -D_ENABLE_TILT -D_ENABLE_FORCE

SRC=	\
wiimote_api.c		\
wiimote_classic.c	\
wiimote_error.c		\
wiimote_event.c		\
wiimote_io.c		\
wiimote_ir.c		\
wiimote_link.c		\
wiimote_mii.c		\
wiimote_nunchuk.c	\
wiimote_report.c	\
wiimote_speaker.c

OBJ=$(SRC:.c=.o)

all: ../libcwiimote.a

../libcwiimote.a: $(OBJ)
	$(AR) rc $@ $^
	ranlib $@

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -rf ../libcwiimote.a *.o
