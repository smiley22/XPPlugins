NAME    = util.a
SRC     = $(wildcard *.c)
OBJ     = $(SRC:.c=.o)
CC      = clang
CFLAGS  = -Wall -DAPL -O2 -Wno-unused-variable -Wno-return-type

all: $(NAME)

$(NAME): $(OBJ)
	ar rcs $(NAME) $(OBJ)

clean:
	rm -f *.o *.a