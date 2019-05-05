NAME    = mac.xpl
SRC     = $(wildcard *.c)
CC      = clang
CFLAGS  = -Wall -DAPL -O2
LDFLAGS = ../XP/Libs/XPLM ../Util/util.a ../FMOD/Libs/libfmod.dylib -dynamiclib -fvisibility=hidden

all: $(NAME)

$(NAME): $(SRC)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -f *.o $(NAME)