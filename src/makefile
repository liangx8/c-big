.PHONY: clean
CC=gcc
RM=rm -rf
CFLAGS=-Wall -g
#CFLAGS+=-DNDEBUG
#CFLAGS+=$(shell pkgconf --cflags json-c)
TARGET=big


SRCS=$(wildcard *.c)
OBJS=${SRCS:.c=.o}
LDFLAGS=-Wall
LDFLAGS+=$(shell pkgconf --libs jansson)
LDFLAGS+=# -static

all:${TARGET}
	@echo "*** Done ***"
release:${OBJS}
	${CC} -o ${TARGET} ${OBJS} ${LDFLAGS} -static
	strip	${TARGET}
	@echo "*** Done ***"
${TARGET}:${OBJS}
	${CC} -o $@ ${OBJS} ${LDFLAGS}
clean:
	${RM} *.o ${TARGET} *~

#%.o:%.cpp
.c.o:
	${CC} -c $< ${CFLAGS}

