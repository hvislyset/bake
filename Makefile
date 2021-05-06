PROJECT = bake

SRC = src
CC = gcc -std=gnu99
CFLAGS = -Werror -Wall -Wextra -pedantic
LIBS = -lcurl

all:
	$(CC) $(CFLAGS) $(SRC)/*.c -o $(PROJECT) $(LIBS)

clean:
	rm -rf $(PROJECT) *.o
	