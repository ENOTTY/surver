CFLAGS = -g -Wall -Wextra -Werror
# if mingw, you need to add -lws2_32 and -lgdi32
LDFLAGS = -lssl -lcrypto

all:
	gcc $(CFLAGS) -c sc.c -o sc.o
	gcc $(CFLAGS) -c main.c -o main.o
	gcc -o s sc.o main.o $(LDFLAGS)

clean:
	rm s *.o
