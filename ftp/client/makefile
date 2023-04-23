CC = gcc
CFLAGS = -Wall -g
executable_file = client

all:
	$(CC) $(CFLAGS) -o $(executable_file) main.c ../net/net.c ../command/command.c ./ftp_client/ftp_client.c 
# ./prompt/prompt.c -lncurses

clean:
	rm -f $(executable_file)