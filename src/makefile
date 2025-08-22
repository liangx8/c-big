.PHONY: clean
CC=gcc
RM=rm -rf
CFLAGS:=-Wall
#CFLAGS+=-DFIXING_BUG
#CFLAGS+=-DNDEBUG
#CFLAGS+=$(shell pkgconf --cflags json-c)
TARGET=big

#CFLAGS+=-O2
#CFLAGS+=-g
ifeq (${OPTS}x, x)
	CFLAGS+=-O2
else
	CFLAGS+=${OPTS}
endif
SRCS=$(wildcard *.c nameid/*.c)
#SRCS=$(shell find . -name \*.c -and -type f)
OBJS=${SRCS:.c=.o}
LDFLAGS:=-Wall -lsqlite3
#LDFLAGS+=$(shell pkgconf --libs jansson)
#LDFLAGS+= -static

all:${TARGET}
	@echo "*** Done ***"
release:${OBJS}
	${CC} -o ${TARGET} ${OBJS} ${LDFLAGS} -DNDEBUG
	strip	${TARGET}
	@echo "*** Done ***"
${TARGET}:${OBJS}
	${CC} -o $@ ${OBJS} ${LDFLAGS}
clean:
	${RM} ${TARGET}
	find . -name \*~ -or -name \*.o -exec ${RM} \{} \;

#%.o:%.cpp
.c.o:
	${CC} -c $< ${CFLAGS} -o $@

