CC = gcc
CFLAGS = -Wall -g
executable_file = server

all:
	$(CC) $(CFLAGS) -o $(executable_file) main.c ftp_session/ftp_session.c ftp_server/ftp_server.c ../net/net.c ../event/event.c ../command/command.c

clean:
	rm -f $(executable_file)
	