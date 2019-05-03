CC      = clang
CFLAGS  = -Wall
LFLAGS  = -v ../Util/util.a
SRC     = $(wildcard *.c)

all: $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)